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
    bool big_endian;
    const DcmParse *parse;
    void *client;

    DcmDataSet *meta;
    int64_t offset;
    int64_t pixel_data_offset;
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
                "end of filehandle",
                "needed %zd bytes beyond end of filehandle", length);
            return false;
        }

        buffer += bytes_read;
        length -= bytes_read;
    }

    return true;
}


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


/* TRUE for big-endian machines, like PPC. We need to byteswap DICOM
 * numeric types in this case. Run time tests for this are much
 * simpler to manage when cross-compiling.
 */
static bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}


#define SWAP16(V) \
    ((uint16_t) ( \
        (uint16_t) ((uint16_t) (V) >> 8) | \
        (uint16_t) ((uint16_t) (V) << 8) \
    ))

#define SWAP32(V) \
    ((uint32_t) ( \
        (((uint32_t) (V) & UINT32_C(0x000000ff)) << 24) | \
        (((uint32_t) (V) & UINT32_C(0x0000ff00)) <<  8) | \
        (((uint32_t) (V) & UINT32_C(0x00ff0000)) >>  8) | \
        (((uint32_t) (V) & UINT32_C(0xff000000)) >> 24) \
    ))

#define SWAP64(V) \
    ((uint64_t) ( \
        (((uint64_t) (V) & UINT64_C(0x00000000000000ff)) << 56) | \
        (((uint64_t) (V) & UINT64_C(0x000000000000ff00)) << 40) | \
        (((uint64_t) (V) & UINT64_C(0x0000000000ff0000)) << 24) | \
        (((uint64_t) (V) & UINT64_C(0x00000000ff000000)) <<  8) | \
        (((uint64_t) (V) & UINT64_C(0x000000ff00000000)) >>  8) | \
        (((uint64_t) (V) & UINT64_C(0x0000ff0000000000)) >> 24) | \
        (((uint64_t) (V) & UINT64_C(0x00ff000000000000)) >> 40) | \
        (((uint64_t) (V) & UINT64_C(0xff00000000000000)) >> 56) \
    ))

static void byteswap(char *data, size_t length, size_t size)
{
    // only swap if the data is "swappable"
    if (size > 0 &&
        length >= size &&
        length % size == 0) {
        size_t n_elements = length / size;

        switch (size) {
            case 2:
                for (size_t i = 0; i < n_elements; i++) {
                    uint16_t *v = &((uint16_t *) data)[i];
                    *v = SWAP16(*v);
                }
                break;

            case 4:
                for (size_t i = 0; i < n_elements; i++) {
                    uint32_t *v = &((uint32_t *) data)[i];
                    *v = SWAP32(*v);
                }
                break;

            case 8:
                for (size_t i = 0; i < n_elements; i++) {
                    uint64_t *v = &((uint64_t *) data)[i];
                    *v = SWAP64(*v);
                }
                break;

            default:
                break;
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

    if (state->big_endian) {
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

    if (state->big_endian) {
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
        // this can be an ambiguous VR, eg. pixeldata is allowed in implicit
        // mode and has to be disambiguated later from other tags
        *vr = dcm_vr_from_tag(*tag);
        if (*vr == DCM_VR_ERROR) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "reading of data element header failed",
                          "tag %08x not allowed in implicit mode", *tag);
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
                          "reading of data element header failed",
                          "tag %08x cannot have VR '%s'", *tag, vr_str);
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
                              "reading of data element header failed",
                              "unexpected value for reserved bytes "
                              "of data element %08x with VR '%s'",
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
        !state->parse->sequence_begin(state->error,
                                      state->client,
                                      seq_tag,
                                      seq_vr,
                                      seq_length)) {
        return false;
    }

    int index = 0;
    while (*position < seq_length) {
        dcm_log_debug("read Item #%d", index);
        uint32_t item_tag;
        uint32_t item_length;
        if (!read_tag(state, &item_tag, position) ||
            !read_uint32(state, &item_length, position)) {
            return false;
        }

        if (item_tag == TAG_SQ_DELIM) {
            dcm_log_debug("stop reading data element -- "
                          "encountered SequenceDelimination tag");
            break;
        }

        if (item_tag != TAG_ITEM) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "reading of data element failed",
                          "expected tag '%08x' instead of '%08x' "
                          "for item #%d",
                          TAG_ITEM,
                          item_tag,
                          index);
            return false;
        }

        if (item_length == 0xFFFFFFFF) {
            dcm_log_debug("item #%d has undefined length", index);
        } else {
            dcm_log_debug("item #%d has defined length %d",
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
                dcm_log_debug("stop reading Item #%d -- "
                              "encountered Item Delimination Tag",
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


static bool parse_pixeldata_item(DcmParseState *state,
                                 uint32_t tag,
                                 DcmVR vr,
                                 uint32_t length,
                                 uint32_t item_length,
                                 int64_t *position)
{
    // a read buffer on the stack for small objects
    char input_buffer[INPUT_BUFFER_SIZE];
    char *value;
    char *value_free = NULL;

    USED(tag);

    // read to our stack buffer, if possible
    if (item_length > INPUT_BUFFER_SIZE) {
        value = value_free = DCM_MALLOC(state->error, item_length);
        if (value_free == NULL) {
            return false;
        }
    } else {
        value = input_buffer;
    }

    if (!dcm_require(state, value, item_length, position)) {
        if (value_free != NULL) {
            free(value_free);
        }
        return false;
    }

    // native (not encapsulated) pixeldata is always little-endian and needs
    // byteswapping on big-endian machines
    if (length != 0xffffffff &&
        state->big_endian) {
        byteswap(value, item_length, dcm_dict_vr_size(vr));
    }

    if (state->parse->pixeldata_create &&
        !state->parse->pixeldata_create(state->error,
                                        state->client,
                                        tag,
                                        vr,
                                        value,
                                        item_length)) {
        if (value_free != NULL) {
            free(value_free);
        }
        return false;
    }

    if (value_free != NULL) {
        free(value_free);
    }

    return true;
}


static bool parse_pixeldata(DcmParseState *state,
                            uint32_t tag,
                            DcmVR vr,
                            uint32_t length,
                            int64_t *position)
{
    if (state->parse->pixeldata_begin &&
        !state->parse->pixeldata_begin(state->error,
                                       state->client,
                                       tag,
                                       vr,
                                       length)) {
        return false;
    }

    if (length == 0xffffffff) {
        // a sequence of encapsulated pixeldata items
        for (int index = 0; true; index++) {
            uint32_t item_tag;
            uint32_t item_length;

            dcm_log_debug("read Item #%d", index);
            if (!read_tag(state, &item_tag, position) ||
                !read_uint32(state, &item_length, position)) {
                return false;
            }

            if (item_tag == TAG_SQ_DELIM) {
                dcm_log_debug("stop reading data element -- "
                              "encountered SequenceDelimination Tag");
                break;
            }

            if (item_tag != TAG_ITEM) {
                dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                              "reading of data element failed",
                              "expected tag '%08x' instead of '%08x' "
                              "for Item #%d",
                              TAG_ITEM,
                              item_tag,
                              index);
                return false;
            }

            if (!parse_pixeldata_item(state,
                                      tag,
                                      vr,
                                      length,
                                      item_length,
                                      position)) {
                return false;
            }
        }
    } else {
        // a single native pixeldata item
        if (!parse_pixeldata_item(state, tag, vr, length, length, position)) {
            return false;
        }
    }

    if (state->parse->pixeldata_end &&
        !state->parse->pixeldata_end(state->error,
                                     state->client)) {
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
    DcmVRClass vr_class = dcm_dict_vr_class(vr);
    size_t size = dcm_dict_vr_size(vr);
    char *value;

    char *value_free = NULL;
    char input_buffer[INPUT_BUFFER_SIZE];

    /* We treat pixeldata as a special case so we can handle encapsulated
     * image sequences.
     */
    if (tag == TAG_PIXEL_DATA ||
        tag == TAG_FLOAT_PIXEL_DATA ||
        tag == TAG_DOUBLE_PIXEL_DATA) {
        return parse_pixeldata(state, tag, vr, length, position);
    }

    dcm_log_debug("Read Data Element body '%08x'", tag);

    switch (vr_class) {
        case DCM_VR_CLASS_STRING_SINGLE:
        case DCM_VR_CLASS_STRING_MULTI:
        case DCM_VR_CLASS_NUMERIC_DECIMAL:
        case DCM_VR_CLASS_NUMERIC_INTEGER:
        case DCM_VR_CLASS_BINARY:
            if (vr_class == DCM_VR_CLASS_NUMERIC_DECIMAL ||
                vr_class == DCM_VR_CLASS_NUMERIC_INTEGER) {
                // all numeric classes have a size
                if (size > 0 &&
                    length % size != 0) {
                    dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                                  "reading of data element failed",
                                  "bad length for tag '%08x'",
                                  tag);
                    return false;
                }
            }

            // read to a static char buffer, if possible
            if ((int64_t) length + 1 >= INPUT_BUFFER_SIZE) {
                value = value_free = DCM_MALLOC(state->error,
                                                (size_t) length + 1);
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

            if (length > 0 &&
                (vr_class == DCM_VR_CLASS_STRING_SINGLE ||
                 vr_class == DCM_VR_CLASS_STRING_MULTI) &&
                vr != DCM_VR_UI &&
                isspace(value[length - 1])) {
                value[length - 1] = '\0';
            }

            if (size > 0 && state->big_endian) {
                byteswap(value, length, size);
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

        case DCM_VR_CLASS_SEQUENCE:
            if (length == 0xFFFFFFFF) {
                dcm_log_debug("Sequence of Data Element '%08x' "
                              "has undefined length",
                              tag);
            } else {
                dcm_log_debug("Sequence of Data Element '%08x' "
                              "has defined length %d",
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
                          "reading of data element failed",
                          "data element '%08x' has unexpected VR", tag);
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
            dcm_log_info("stop reading Data Set -- reached end of filehandle");
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
                       const DcmParse *parse,
                       void *client)
{
    DcmParseState state = {
        .error = error,
        .io = io,
        .implicit = implicit,
        .big_endian = is_big_endian(),
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
                     const DcmParse *parse,
                     void *client)
{
    DcmParseState state = {
        .error = error,
        .io = io,
        .implicit = implicit,
        .big_endian = is_big_endian(),
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
                      "reading of group failed",
                      "bad group length element");
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
bool dcm_parse_pixeldata_offsets(DcmError **error,
                                 DcmIO *io,
                                 bool implicit,
                                 int64_t *first_frame_offset,
                                 int64_t *offsets,
                                 int num_frames)
{
    DcmParseState state = {
        .error = error,
        .io = io,
        .implicit = implicit,
        .big_endian = is_big_endian()
    };

    int64_t position = 0;

    dcm_log_debug("parsing PixelData");

    uint32_t tag;
    DcmVR vr;
    uint32_t length;
    uint32_t value;
    if (!parse_element_header(&state, &tag, &vr, &length, &position)) {
        return false;
    }

    if (tag != TAG_PIXEL_DATA &&
        tag != TAG_FLOAT_PIXEL_DATA &&
        tag != TAG_DOUBLE_PIXEL_DATA) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "parsing PixelData failed",
                      "file pointer not positioned at PixelData element");
        return false;
    }

    // The header of the 0th item (the BOT)
    if (!read_tag(&state, &tag, &position) ||
        !read_uint32(&state, &length, &position)) {
        return false;
    }
    if (tag != TAG_ITEM) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "reading BasicOffsetTable failed",
                      "unexpected tag found for BasicOffsetTable item");
        return false;
    }

    if (length > 0) {
        // There is a non-zero length BOT, use that
        dcm_log_info("reading Basic Offset Table");

        // Read offset values from BOT Item value
        // FIXME .. could do this with a single require to a uint32_t array,
        // see numeric array read above
        for (int i = 0; i < num_frames; i++) {
            if (!read_uint32(&state, &value, &position)) {
                return false;
            }
            if (value == TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "reading BasicOffsetTable failed",
                              "encountered unexpected item tag "
                              "in BasicOffsetTable");
                return false;
            }

            offsets[i] = value;
        }

        // and that's the offset to the item header on the first frame
        *first_frame_offset = position;

        // the next thing should be the tag for frame 1
        if (!read_tag(&state, &tag, &position)) {
            return false;
        }
        if (tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading BasicOffsetTable failed",
                          "BasicOffsetTable too large");
            return false;
        }
    } else {
        // the BOT is missing, we must scan pixeldata to find the position of
        // each frame

        // we could use our generic parser above ^^ but we have a special loop
        // here as an optimisation (we can skip over the pixel data itself)

        dcm_log_info("building Offset Table from Pixel Data");

        // 0 in the BOT is the offset to the start of frame 1, ie. here
        *first_frame_offset = position;

        position = 0;
        for (int i = 0; i < num_frames; i++) {
            if (!read_tag(&state, &tag, &position) ||
                !read_uint32(&state, &length, &position)) {
                return false;
            }

            if (tag == TAG_SQ_DELIM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "reading BasicOffsetTable failed",
                              "too few frames in PixelData");
                return false;
            }

            if (tag != TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "building BasicOffsetTable failed",
                              "frame Item #%d has wrong tag '%08x'",
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

        // the next thing should be the end of sequence tag
        if (!read_tag(&state, &tag, &position)) {
            return false;
        }
        if (tag != TAG_SQ_DELIM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading BasicOffsetTable failed",
                          "too many frames in PixelData");
            return false;
        }
    }

    return true;
}

char *dcm_parse_frame(DcmError **error,
                      DcmIO *io,
                      bool implicit,
                      struct PixelDescription *desc,
                      uint32_t *length)
{
    DcmParseState state = {
        .error = error,
        .io = io,
        .implicit = implicit,
        .big_endian = is_big_endian(),
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
                          "reading frame item failed",
                          "no item tag found for frame item");
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
