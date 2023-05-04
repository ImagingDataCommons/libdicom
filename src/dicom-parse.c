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


#define TAG_ITEM                  0xFFFEE000
#define TAG_ITEM_DELIM            0xFFFEE00D
#define TAG_SQ_DELIM              0xFFFEE0DD
#define TAG_TRAILING_PADDING      0xFFFCFFFC
#define TAG_EXTENDED_OFFSET_TABLE 0x7FE00001
#define TAG_PIXEL_DATA            0x7FE00010
#define TAG_FLOAT_PIXEL_DATA      0x7FE00008
#define TAG_DOUBLE_PIXEL_DATA     0x7FE00009

/* The size of the buffer we use for reading smaller element values. This is 
 * large enough for most VRs.
 */
#define INPUT_BUFFER_SIZE (256)


typedef struct _DcmParseState {
    DcmError **error;
    DcmIOhandle *handle;
    DcmParse *parse;
    bool byteswap;
    void *client;

    DcmDataSet *meta;
    int64_t offset;
    int64_t pixel_data_offset;
    uint64_t *extended_offset_table;
} DcmParseState;


static int64_t dcm_read(DcmParseState *state,
    char *buffer, int64_t length, int64_t *position)
{
    int64_t bytes_read = state->handle->io->read(state->error,
                                                 state->handle, 
                                                 buffer, 
                                                 length);
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
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "End of filehandle",
                "Needed %zd bytes beyond end of filehandle", length);
            return false;
        }

        buffer += bytes_read;
        length -= bytes_read;
    }

    return true;
}


static bool dcm_seekset(DcmParseState *state, int64_t offset)
{
    int64_t new_offset = state->handle->io->seek(state->error, 
                                                 state->handle, 
                                                 offset, 
                                                 SEEK_SET);
    return new_offset >= 0;
}


static bool dcm_seekcur(DcmParseState *state, int64_t offset, int64_t *position)
{
    int64_t new_offset = state->handle->io->seek(state->error,
                                                 state->handle, 
                                                 offset, 
                                                 SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *position += offset;

    return true;
}


static bool dcm_offset(DcmParseState *state, int64_t *offset)
{
    int64_t new_offset = state->handle->io->seek(state->error, 
                                                 state->handle, 
                                                 0, 
                                                 SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *offset = new_offset;

    return true;
}


static bool dcm_is_eof(DcmParseState *state)
{
    bool eof = true;

    char buffer[1];
    int64_t bytes_read = state->handle->io->read(NULL, 
                                                 state->handle, 
                                                 buffer, 
                                                 1);
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


static bool read_tag(DcmParseState *state, uint32_t *value, int64_t *position)
{
    uint16_t group, elem;

    if (!read_uint16(state, &group, position) ||
        !read_uint16(state, &elem, position)) {
        return false;
    }

    *value = ((uint32_t)group << 16) | elem;

    return true;
}


static bool parse_element_header(DcmParseState *state, 
                                 bool implicit,
                                 uint32_t *tag,
                                 DcmVR *vr,
                                 uint32_t *length,
                                 int64_t *position)
{
    if (!read_tag(error, state->handle, tag, position)) {
        return false;
    }

    if (implicit) {
        // this can be an ambiguious VR, eg. pixeldata is allowed in implicit
        // mode and has to be disambiguated later from other tags
        *vr = dcm_vr_from_tag(*tag);
        if (*vr == DCM_VR_ERROR) {
            dcm_error_set(state->error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element header failed",
                          "Tag %08X not allowed in implicit mode", *tag);
            return false;
        }

        if (!read_uint32(error, filehandle, length, position)) {
            return false;
        }
    } else {
        // Value Representation
        char vr_str[3];
        if (!dcm_require(error, filehandle, vr_str, 2, position)) {
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

        if (dcm_dict_vr_header_length(vr) == 2) {
            // These VRs have a short length of only two bytes
            uint16_t short_length;
            if (!read_uint16(error, filehandle, &short_length, position)) {
                return false;
            }
            *length = (uint32_t) short_length;
        } else {
            // Other VRs have two reserved bytes before length of four bytes
            uint16_t reserved;
            if (!read_uint16(error, filehandle, &reserved, position) ||
                !read_uint32(error, filehandle, length, position)) {
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
                                   bool implicit,
                                   uint32_t tag,
                                   DcmVR vr,
                                   uint32_t seq_length,
                                   int64_t *position)
{
    if (state->parse->sequence_begin(error, state->client,
                                     tag,
                                     vr,
                                     seq_length)) {
        return false;
    }

    int index = 0;
    while (*position < seq_length) {
        dcm_log_debug("Read Item #%d.", index);
        uint32_t item_tag;
        uint32_t item_length;
        if (!read_tag(state->error, 
                     state->filehandle, 
                     &item_tag, 
                     position) ||
            !read_uint32(state->error, 
                         state->filehandle, 
                         &item_length, 
                         position)) {
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

        if (state->parse->dataset_start(state->error, state->client)) {
            return false;
        }

        int64_t item_position = 0;
        while (item_position < item_length) {
            // peek the next tag
            if (!read_tag(state->error, 
                          state->filehandle, 
                          &item_tag, 
                          &item_position)) {
                return false;
            }

            if (item_tag == TAG_ITEM_DELIM) {
                dcm_log_debug("Stop reading Item #%d. "
                              "Encountered Item Delimination Tag.",
                              index);
                // step over the tag length
                if (!dcm_seekcur(state->error, 
                                 state->filehandle, 
                                 4, 
                                 &item_position)) {
                    return false;
                }

                break;
            }

            // back to start of element
            if (!dcm_seekcur(state->error, 
                             state->filehandle, 
                             -4, 
                             &item_position)) {
                return false;
            }

            if (!parse_element(state, implicit, &item_position)) {
                return false;
            }
        }

        seq_position += item_position;

        if (state->parse->dataset_end(state->error, state->client)) {
            return false;
        }

        index += 1;
    }

    *position += seq_position;

    if (state->parse->sequence_end(state->error, state->client)) {
        return false;
    }

    return true;
}


static bool parse_element_body(DcmParseState *state, 
                               bool implicit,
                               uint32_t tag,
                               DcmVR vr,
                               uint32_t length,
                               int64_t position)
{
    DcmVRClass klass = dcm_dict_vr_class(vr);
    size_t size = dcm_dict_vr_size(vr);
    uint32_t vm = length / size;
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
                g_assert (size != 0);
                if (length % size != 0) {
                    dcm_error_set(error, DCM_ERROR_CODE_PARSE,
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

            if (!dcm_require(state->error, 
                             state->filehandle, 
                             value, 
                             length, 
                             position)) {
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
                if (filehandle->byteswap) {
                    byteswap(value, length, size);
                }
            }

            if (state->parse->element_create(error, 
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
                                        implicit,
                                        tag,
                                        vr,
                                        length,
                                        &seq_position)) {
                return false;
            }
            *position += seq_position;

            break;

        default:
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Data Element '%08X' has unexpected "
                          "Value Representation", tag);
            return false;
    }

    return true;
}


static bool parse_element(DcmParseState *state,
                          bool implicit,
                          int64_t *position)
{
    uint32_t tag;
    DcmVR vr;
    uint32_t length;
    if (!parse_element_header(state, implicit, &tag, &vr, &length, position) ||
        !parse_element_body(state, implicit, tag, vr, length, position)) {
        return false;
    }

    return true;
}

/* Top-level datasets don't have an enclosing length, and can broken by a
 * stop function.
 */
static bool parse_toplevel_dataset(DcmParseState *state,
                                   bool implicit,
                                   int64_t *position)
{
    if (!state->parse->dataset_begin(state->error, state->client)) {
        return false;
    }

    for (;;) {
        if (dcm_is_eof(state->filehandle)) {
            dcm_log_info("Stop reading Data Set. Reached end of filehandle.");
            break;
        }

        uint32_t tag;
        DcmVR vr;
        uint32_t length;
        if (!parse_element_header(state, implicit, 
            &tag, &vr, &length, position)) {
            return false;
        }

        if (tag == TAG_TRAILING_PADDING) {
            dcm_log_info("Stop reading Data Set",
                         "Encountered Data Set Trailing Tag");
            break;
        }

        if (state->parse->stop(state->client, implicit, tag, vr, length)) {
            break;
        }

        if (!parse_element_body(state, implicit, tag, vr, length, position)) {
            return false;
        }
    }

    if (!state->parse->dataset_end(state->error, state->client)) {
        return false;
    }

    return true;
}


/* Parse a dataset from a filehandle.
 */
bool dcm_parse_dataset(DcmError **error, 
                       DcmIOHandle *handle,
                       bool implicit,
                       DcmParse *parse, 
                       bool byteswap,
                       void *client)
{
    DcmParseState state = { error, handle, parse, byteswap, client };

    int64_t position = 0;
    if (!state.parse->parse_begin(state.error, state.client) ||
        !parse_toplevel_dataset(&state, implicit, &position) ||
        !state.parse->parse_end(state.error, state.client)) {
        return false;
    }

    return true;
}
