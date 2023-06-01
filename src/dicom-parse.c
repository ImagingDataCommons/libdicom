/*
 * Implementation of Part 10 of the DICOM standard: Media Storage and File
 * Format for Media Interchange.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strdup and strcpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "utarray.h"

#include <dicom/dicom.h>
#include "pdicom.h"


/* The size of the buffer we use for reading smaller element values. This is 
 * large enough for most VRs.
 */
#define INPUT_BUFFER_SIZE (256)


typedef struct _DcmParseState {
    DcmError **error;
    DcmIO *io;
    bool implicit;
    bool byteswap;
    const DcmParse *parse;
    void *client;

    DcmDataSet *meta;
    int64_t offset;
    int64_t pixel_data_offset;
    uint64_t *extended_offset_table;
} DcmParseState;


static int64_t dcm_read(DcmParseState *state,
    char *buffer, int64_t length, int64_t *position)
{
    int64_t bytes_read = dcm_io_read(state->error, state->io, buffer, length);
    if (bytes_read < 0) {
        return bytes_read;
    }

    *position += bytes_read;

    return bytes_read;
}


static bool dcm_require(DcmParseState *state,
    char *buffer, int64_t length, int64_t *position)
{
    while (length > 0) {
        int64_t bytes_read = dcm_read(state, buffer, length, position);

        if (bytes_read < 0) {
            return false;
        } else if (bytes_read == 0) {
            dcm_error_set(state->error, DCM_ERROR_CODE_IO,
                "End of filehandle",
                "Needed %zd bytes beyond end of filehandle", length);
            return false;
        }

        buffer += bytes_read;
        length -= bytes_read;
    }

    return true;
}


/* will we need these?
static bool dcm_seekset(DcmParseState *state, int64_t offset)
{
    int64_t new_offset = dcm_io_seek(state->error, state->io, offset, SEEK_SET);
    return new_offset >= 0;
}

static bool dcm_offset(DcmParseState *state, int64_t *offset)
{
    int64_t new_offset = dcm_io_seek(state->error, state->io, 0, SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *offset = new_offset;

    return true;
}
 */


static bool dcm_seekcur(DcmParseState *state, int64_t offset, int64_t *position)
{
    int64_t new_offset = dcm_io_seek(state->error, state->io, offset, SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *position += offset;

    return true;
}


static bool dcm_is_eof(DcmParseState *state)
{
    bool eof = true;

    char buffer[1];
    int64_t bytes_read = dcm_io_read(NULL, state->io, buffer, 1);
    if (bytes_read > 0) {
        eof = false;
        int64_t position = 0;
        (void) dcm_seekcur(state, -1, &position);
    }

    return eof;
}


static void byteswap(char *data, size_t length, size_t size)
{
    assert(length >= size);

    if (size > 1) {
        assert(length % size == 0);
        assert(size % 2 == 0);

        size_t half_size = size / 2;

        for (size_t i = 0; i < length; i += size) {
            for (size_t j = 0; j < half_size; j++) {
                char *p = data + i;
                char t = p[j];
                p[j] = p[size - j - 1];
                p[size - j - 1] = t;
            }
        }
    }
}


static bool read_uint16(DcmParseState *state,
    uint16_t *value, int64_t *position)
{
    union {
        uint16_t i;
        char c[2];
    } buffer;

    if (!dcm_require(state, buffer.c, 2, position)) {
        return false;
    }

    if (state->byteswap) {
        byteswap(buffer.c, 2, 2);
    }

    *value = buffer.i;

    return true;
}


static bool read_uint32(DcmParseState *state, 
    uint32_t *value, int64_t *position)
{
    union {
        uint32_t i;
        char c[4];
    } buffer;

    if (!dcm_require(state, buffer.c, 4, position)) {
        return false;
    }

    if (state->byteswap) {
        byteswap(buffer.c, 4, 4);
    }

    *value = buffer.i;

    return true;
}


static bool read_tag(DcmParseState *state, uint32_t *tag, int64_t *position)
{
    uint16_t group, elem;

    if (!read_uint16(state, &group, position) ||
        !read_uint16(state, &elem, position)) {
        return false;
    }

    *tag = ((uint32_t)group << 16) | elem;

    return true;
}


/* This is used recursively.
 */
static bool parse_element(DcmParseState *state,
                          int64_t *position);


static bool parse_element_header(DcmParseState *state, 
                                 uint32_t *tag,
                                 DcmVR *vr,
                                 uint32_t *length,
                                 int64_t *position)
{
    if (!read_tag(state, tag, position)) {
        return false;
    }

    if (state->implicit) {
        // this can be an ambiguious VR, eg. pixeldata is allowed in implicit
        // mode and has to be disambiguated later from other tags
        *vr = dcm_vr_from_tag(*tag);
        if (*vr == DCM_VR_ERROR) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element header failed",
                          "Tag %08X not allowed in implicit mode", *tag);
            return false;
        }

        if (!read_uint32(state, length, position)) {
            return false;
        }
    } else {
        // Value Representation
        char vr_str[3];
        if (!dcm_require(state, vr_str, 2, position)) {
            return false;
        }
        vr_str[2] = '\0';
        *vr = dcm_dict_vr_from_str(vr_str);

        if (!dcm_is_valid_vr_for_tag(*vr, *tag)) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element header failed",
                          "Tag %08X cannot have VR '%s'", *tag, vr_str);
            return false;
        }

        if (dcm_dict_vr_header_length(*vr) == 2) {
            // These VRs have a short length of only two bytes
            uint16_t short_length;
            if (!read_uint16(state, &short_length, position)) {
                return false;
            }
            *length = (uint32_t) short_length;
        } else {
            // Other VRs have two reserved bytes before length of four bytes
            uint16_t reserved;
            if (!read_uint16(state, &reserved, position) ||
                !read_uint32(state, length, position)) {
               return false;
            }

            if (reserved != 0x0000) {
                dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element header failed",
                              "Unexpected value for reserved bytes "
                              "of Data Element %08X with VR '%s'.",
                              tag, vr);
                return false;
            }
        }
    }

    return true;
}


static bool parse_element_sequence(DcmParseState *state, 
                                   uint32_t seq_tag,
                                   DcmVR seq_vr,
                                   uint32_t seq_length,
                                   int64_t *position)
{
    if (state->parse->sequence_begin &&
        !state->parse->sequence_begin(state->error, state->client)) {
        return false;
    }

    int index = 0;
    while (*position < seq_length) {
        dcm_log_debug("Read Item #%d.", index);
        uint32_t item_tag;
        uint32_t item_length;
        if (!read_tag(state, &item_tag, position) ||
            !read_uint32(state, &item_length, position)) {
            return false;
        }

        if (item_tag == TAG_SQ_DELIM) {
            dcm_log_debug("Stop reading Data Element. "
                          "Encountered Sequence Delimination Tag.");
            break;
        }

        if (item_tag != TAG_ITEM) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Expected tag '%08X' instead of '%08X' "
                          "for Item #%d",
                          TAG_ITEM,
                          item_tag,
                          index);
            return false;
        }

        if (item_length == 0xFFFFFFFF) {
            dcm_log_debug("Item #%d has undefined length.", index);
        } else {
            dcm_log_debug("Item #%d has defined length %d.",
                          index, item_length);
        }

        if (state->parse->dataset_begin &&
            !state->parse->dataset_begin(state->error, state->client)) {
            return false;
        }

        int64_t item_position = 0;
        while (item_position < item_length) {
            // peek the next tag
            if (!read_tag(state, &item_tag, &item_position)) {
                return false;
            }

            if (item_tag == TAG_ITEM_DELIM) {
                dcm_log_debug("Stop reading Item #%d. "
                              "Encountered Item Delimination Tag.",
                              index);
                // step over the tag length
                if (!dcm_seekcur(state, 4, &item_position)) {
                    return false;
                }

                break;
            }

            // back to start of element
            if (!dcm_seekcur(state, -4, &item_position)) {
                return false;
            }

            if (!parse_element(state, &item_position)) {
                return false;
            }
        }

        *position += item_position;

        if (state->parse->dataset_end &&
            !state->parse->dataset_end(state->error, state->client)) {
            return false;
        }

        index += 1;
    }

    if (state->parse->sequence_end &&
        !state->parse->sequence_end(state->error, 
                                    state->client,
                                    seq_tag,
                                    seq_vr,
                                    seq_length)) {
        return false;
    }

    return true;
}


static bool parse_element_body(DcmParseState *state, 
                               uint32_t tag,
                               DcmVR vr,
                               uint32_t length,
                               int64_t *position)
{
    DcmVRClass klass = dcm_dict_vr_class(vr);
    size_t size = dcm_dict_vr_size(vr);
    char *value;

    char *value_free = NULL;
    char input_buffer[INPUT_BUFFER_SIZE];

    dcm_log_debug("Read Data Element body '%08X'", tag);

    switch (klass) {
        case DCM_CLASS_STRING_SINGLE:
        case DCM_CLASS_STRING_MULTI:
        case DCM_CLASS_NUMERIC:
        case DCM_CLASS_BINARY:
            if (klass == DCM_CLASS_NUMERIC) {
                // all numeric classes have a size
                if (length % size != 0) {
                    dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                                  "Reading of Data Element failed",
                                  "Bad length for tag '%08X'",
                                  tag);
                    return false;
                }
            }

            // read to a static char buffer, if possible
            if (length + 1 >= INPUT_BUFFER_SIZE) {
                value = value_free = DCM_MALLOC(state->error, length + 1);
                if (value == NULL) {
                    return false;
                }
            } else {
                value = input_buffer;
            }

            if (!dcm_require(state, value, length, position)) {
                if (value_free != NULL) {
                    free(value_free);
                }
                return false;
            }
            value[length] = '\0';

            if (length > 0) {
                if (vr != DCM_VR_UI) {
                    if (isspace(value[length - 1])) {
                        value[length - 1] = '\0';
                    }
                }
            }

            if (klass == DCM_CLASS_NUMERIC) {
                if (state->byteswap) {
                    byteswap(value, length, size);
                }
            }

            if (state->parse->element_create &&
                !state->parse->element_create(state->error, 
                                              state->client,
                                              tag,
                                              vr,
                                              value,
                                              length)) {
                if (value_free != NULL) {
                    free(value_free);
                }
                return false;
            }

            if (value_free != NULL) {
                free(value_free);
            }

            break;

        case DCM_CLASS_SEQUENCE:
            if (length == 0xFFFFFFFF) {
                dcm_log_debug("Sequence of Data Element '%08X' "
                              "has undefined length.",
                              tag);
            } else {
                dcm_log_debug("Sequence of Data Element '%08X' "
                              "has defined length %d.",
                              tag, length);
            }

            int64_t seq_position = 0;
            if (!parse_element_sequence(state, 
                                        tag,
                                        vr,
                                        length,
                                        &seq_position)) {
                return false;
            }
            *position += seq_position;

            break;

        default:
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Data Element '%08X' has unexpected "
                          "Value Representation", tag);
            return false;
    }

    return true;
}


static bool parse_element(DcmParseState *state,
                          int64_t *position)
{
    uint32_t tag;
    DcmVR vr;
    uint32_t length;
    if (!parse_element_header(state, &tag, &vr, &length, position) ||
        !parse_element_body(state, tag, vr, length, position)) {
        return false;
    }

    return true;
}

/* Top-level datasets don't have an enclosing length, and can broken by a
 * stop function.
 */
static bool parse_toplevel_dataset(DcmParseState *state,
                                   int64_t *position)
{
    if (state->parse->dataset_begin &&
        !state->parse->dataset_begin(state->error, state->client)) {
        return false;
    }

    for (;;) {
        if (dcm_is_eof(state)) {
            dcm_log_info("Stop reading Data Set. Reached end of filehandle.");
            break;
        }

        uint32_t tag;
        DcmVR vr;
        uint32_t length;
        int64_t element_start = 0;
        if (!parse_element_header(state, &tag, &vr, &length, &element_start)) {
            return false;
        }

        if (tag == TAG_TRAILING_PADDING) {
            dcm_log_info("Stop reading Data Set",
                         "Encountered Data Set Trailing Tag");
            break;
        }

        if (state->parse->stop &&
            state->parse->stop(state->client, tag, vr, length)) {
            // seek back to the start of this element
            if (!dcm_seekcur(state, -element_start, &element_start)) {
                return false;
            }
            break;
        }

        *position += element_start;

        if (!parse_element_body(state, tag, vr, length, position)) {
            return false;
        }
    }

    if (state->parse->dataset_end &&
        !state->parse->dataset_end(state->error, state->client)) {
        return false;
    }

    return true;
}


/* Parse a dataset from a filehandle.
 */
bool dcm_parse_dataset(DcmError **error, 
                       DcmIO *io,
                       bool implicit,
                       bool byteswap,
                       const DcmParse *parse, 
                       void *client)
{
    DcmParseState state = { 
        .error = error, 
        .io = io, 
        .implicit = implicit, 
        .byteswap = byteswap, 
        .parse = parse, 
        .client = client 
    };

    int64_t position = 0;
    if (!parse_toplevel_dataset(&state, &position)) {
        return false;
    }

    return true;
}


/* Parse a group. A length element, followed by a list of elements.
 */
bool dcm_parse_group(DcmError **error, 
                     DcmIO *io,
                     bool implicit,
                     bool byteswap,
                     const DcmParse *parse, 
                     void *client)
{
    DcmParseState state = { 
        .error = error, 
        .io = io, 
        .implicit = implicit, 
        .byteswap = byteswap, 
        .parse = parse, 
        .client = client 
    };

    int64_t position = 0;

    /* Groups start with (xxxx0000, UL, 4), meaning a 32-bit length value.
     */
    uint32_t tag;
    DcmVR vr;
    uint32_t length;
    if (!parse_element_header(&state, &tag, &vr, &length, &position)) {
        return false;
    }
    uint16_t element_number = tag & 0xffff;
    uint16_t group_number = tag >> 16;
    if (element_number != 0x0000 || vr != DCM_VR_UL || length != 4) {
        dcm_error_set(state.error, DCM_ERROR_CODE_PARSE,
                      "Reading of Group failed",
                      "Bad Group length element");
        return false;
    }
    uint32_t group_length;
    if (!read_uint32(&state, &group_length, &position)) {
        return false;
    }

    // parse the elements in the group to a dataset
    if (state.parse->dataset_begin &&
        !state.parse->dataset_begin(state.error, state.client)) {
        return false;
    }

    while (position < group_length) {
        int64_t element_start = 0;
        if (!parse_element_header(&state, &tag, &vr, &length, &element_start)) {
            return false;
        }

        // stop if we read the first tag of the group beyond,
        // or if the stop function triggers
        if ((tag >> 16) != group_number ||
            (state.parse->stop &&
             state.parse->stop(state.client, tag, vr, length))) {
            // seek back to the start of this element
            if (!dcm_seekcur(&state, -element_start, &element_start)) {
                return false;
            }

            break;
        }

        position += element_start;

        if (!parse_element_body(&state, tag, vr, length, &position)) {
            return false;
        }
    }

    if (state.parse->dataset_end &&
        !state.parse->dataset_end(state.error, state.client)) {
        return false;
    }

    return true;
}


/* Walk pixeldata and set up offsets. We use the BOT, if present, otherwise we
 * have to scan the whole thing.
 *
 * Each offset is the seek from the start of pixeldata to the ITEM for that
 * frame.
 */
bool dcm_parse_pixeldata(DcmError **error,
                         DcmIO *io,
                         bool implicit,
                         bool byteswap,
                         ssize_t *first_frame_offset,
                         ssize_t *offsets,
                         int num_frames)
{
    DcmParseState state = { 
        .error = error, 
        .io = io, 
        .implicit = implicit, 
        .byteswap = byteswap 
    };

    int64_t position = 0;

    uint32_t tag;
    DcmVR vr;
    uint32_t length;
    if (!parse_element_header(&state, &tag, &vr, &length, &position)) {
        return false;
    }

    if (tag != TAG_PIXEL_DATA &&
        tag != TAG_FLOAT_PIXEL_DATA &&
        tag != TAG_DOUBLE_PIXEL_DATA) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Parsing PixelData failed",
                      "File pointer not positioned at Pixel Data Element");
        return false;
    }

    // The header of the 0th item (the BOT)
    if (!read_tag(&state, &tag, &position) ||
        !read_uint32(&state, &length, &position)) {
        return false;
    }
    if (tag != TAG_ITEM) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Unexpected Tag found for Basic Offset Table Item");
        return false;
    }

    if (length > 0) {
        // There is a non-zero length BOT, use that
        dcm_log_info("Read Basic Offset Table value.");

        // Read offset values from BOT Item value
        // FIXME .. could do this with a single require to a uint32_t array,
        // see numeric array read above
        for (int i = 0; i < num_frames; i++) {
            uint32_t ui32;
            if (!read_uint32(&state, &ui32, &position)) {
                return NULL;
            }
            uint64_t value = ui32;
            if (value == TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading Basic Offset Table failed",
                              "Encountered unexpected Item Tag "
                              "in Basic Offset Table");
                return NULL;
            }

            offsets[i] = value;
        }

        // and that's the offset to the item header on the first frame
        *first_frame_offset = position;
    } else {
        // the BOT is missing, we must scan pixeldata to find the position of
        // each frame

        // 0 in the BOT is the offset to the start of frame 1, ie. here
        *first_frame_offset = position;

        position = 0;
        for (int i = 0; i < num_frames; i++) {
            if (!read_tag(&state, &tag, &position) ||
                !read_uint32(&state, &length, &position)) {
                return false;
            }

            if (tag == TAG_SQ_DELIM) {
                break;
            }

            if (tag != TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Building Basic Offset Table failed",
                              "Frame Item #%d has wrong Tag '%08X'",
                              i + 1,
                              tag);
                return false;
            }

            // step back to the start of the item for this frame
            offsets[i] = position - 8;

            // and seek forward over the value
            if (!dcm_seekcur(&state, length, &position)) {
                return false;
            }
        }
    }

    return true;
}

char *dcm_parse_frame(DcmError **error,
                      DcmIO *io,
                      bool implicit,
                      bool byteswap,
                      struct PixelDescription *desc,
                      uint32_t *length)
{
    DcmParseState state = { 
        .error = error, 
        .io = io, 
        .implicit = implicit, 
        .byteswap = byteswap,
    };

    int64_t position = 0;

    if (dcm_is_encapsulated_transfer_syntax(desc->transfer_syntax_uid)) {
        uint32_t tag;
        if (!read_tag(&state, &tag, &position) ||
            !read_uint32(&state, length, &position)) {
            return NULL;
        }

        if (tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Frame Item failed",
                          "No Item Tag found for Frame Item");
            return NULL;
        }
    } else {
        *length = desc->rows * desc->columns * desc->samples_per_pixel;
    }

    char *value = DCM_MALLOC(error, *length);
    if (value == NULL) {
        return NULL;
    }
    if (!dcm_require(&state, value, *length, &position)) {
        free(value);
        return NULL;
    }

    return value;
}
