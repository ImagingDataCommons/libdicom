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

struct PixelDescription {
    uint16_t rows;
    uint16_t columns;
    uint16_t samples_per_pixel;
    uint16_t bits_allocated;
    uint16_t bits_stored;
    uint16_t high_bit;
    uint16_t pixel_representation;
    uint16_t planar_configuration;
    const char *photometric_interpretation;
};


struct _DcmFilehandle {
    const DcmIO *io;
    void *fp;
    DcmDataSet *meta;
    int64_t offset;
    char *transfer_syntax_uid;
    int64_t pixel_data_offset;
    uint64_t *extended_offset_table;
    bool byteswap;
};


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


DcmFilehandle *dcm_filehandle_create(DcmError **error,
                                     const DcmIO *io,
                                     void *client)
{
    DcmFilehandle *filehandle = DCM_NEW(error, DcmFilehandle);
    if (filehandle == NULL) {
        return NULL;
    }

    filehandle->io = io;
    filehandle->fp = NULL;
    filehandle->meta = NULL;
    filehandle->offset = 0;
    filehandle->transfer_syntax_uid = NULL;
    filehandle->pixel_data_offset = 0;
    filehandle->extended_offset_table = NULL;
    filehandle->byteswap = is_big_endian();

    filehandle->fp = filehandle->io->open(error, client);
    if (filehandle->fp == NULL) {
        dcm_filehandle_destroy(filehandle);
        return NULL;
    }

    return filehandle;
}


void dcm_filehandle_destroy(DcmFilehandle *filehandle)
{
    if (filehandle) {
        if (filehandle->transfer_syntax_uid) {
            free(filehandle->transfer_syntax_uid);
        }

        if (filehandle->fp) {
            (void)filehandle->io->close(NULL, filehandle->fp);
            filehandle->fp = NULL;
        }

        free(filehandle);
    }
}


static int64_t dcm_read(DcmError **error, DcmFilehandle *filehandle,
    char *buffer, int64_t length, int64_t *position)
{
    int64_t bytes_read = filehandle->io->read(error,
                                              filehandle->fp, buffer, length);
    if (bytes_read < 0) {
        return bytes_read;
    }

    *position += bytes_read;

    return bytes_read;
}


static bool dcm_require(DcmError **error, DcmFilehandle *filehandle,
    char *buffer, int64_t length, int64_t *position)
{
    while (length > 0) {
        int64_t bytes_read = dcm_read(error,
                                      filehandle, buffer, length, position);

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


static bool dcm_seekset(DcmError **error,
                        DcmFilehandle *filehandle, int64_t offset)
{
    return filehandle->io->seek(error, filehandle->fp, offset, SEEK_SET) >= 0;
}


static bool dcm_seekcur(DcmError **error, DcmFilehandle *filehandle,
    int64_t offset, int64_t *position)
{
    int64_t new_offset = filehandle->io->seek(error,
                                              filehandle->fp, offset, SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *position += offset;

    return true;
}


static bool dcm_offset(DcmError **error,
                       DcmFilehandle *filehandle, int64_t *offset)
{
    *offset = filehandle->io->seek(error, filehandle->fp, 0, SEEK_CUR);
    if (*offset < 0) {
        return false;
    }

    return true;
}


static bool dcm_is_eof(DcmFilehandle *filehandle)
{
    int64_t position = 0;
    bool eof = true;

    char buffer[1];
    int64_t bytes_read = filehandle->io->read(NULL, filehandle->fp, buffer, 1);
    if (bytes_read > 0) {
        eof = false;
        (void) dcm_seekcur(NULL, filehandle, -1, &position);
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


static bool read_uint16(DcmError **error, DcmFilehandle *filehandle,
    uint16_t *value, int64_t *position)
{
    union {
        uint16_t i;
        char c[2];
    } buffer;

    if (!dcm_require(error, filehandle, buffer.c, 2, position)) {
        return false;
    }

    if (filehandle->byteswap) {
        byteswap(buffer.c, 2, 2);
    }

    *value = buffer.i;

    return true;
}


static bool read_uint32(DcmError **error, DcmFilehandle *filehandle,
    uint32_t *value, int64_t *position)
{
    union {
        uint32_t i;
        char c[4];
    } buffer;

    if (!dcm_require(error, filehandle, buffer.c, 4, position)) {
        return false;
    }

    if (filehandle->byteswap) {
        byteswap(buffer.c, 4, 4);
    }

    *value = buffer.i;

    return true;
}


static bool read_tag(DcmError **error, DcmFilehandle *filehandle,
    uint32_t *value, int64_t *position)
{
    uint16_t group, elem;

    if (!read_uint16(error, filehandle, &group, position) ||
        !read_uint16(error, filehandle, &elem, position)) {
        return false;
    }

    *value = ((uint32_t)group << 16) | elem;

    return true;
}


static bool read_iheader(DcmError **error, DcmFilehandle *filehandle,
        uint32_t *item_tag, uint32_t *item_length, int64_t *position)
{
    if (!read_tag(error, filehandle, item_tag, position) ||
        !read_uint32(error, filehandle, item_length, position)) {
        return false;
    }

    return true;
}


char **dcm_parse_character_string(DcmError **error,
                                  char *string, uint32_t *vm)
{
    int n_segments = 1;
    for (int i = 0; string[i]; i++) {
        if (string[i] == '\\') {
            n_segments += 1;
        }
    }

    char **parts = DCM_NEW_ARRAY(error, n_segments, char *);
    if (parts == NULL) {
        return NULL;
    }

    char *p = string;
    for (int segment = 0; segment < n_segments; segment++) {
        int i;
        for (i = 0; p[i] && p[i] != '\\'; i++)
            ;

        parts[segment] = DCM_MALLOC(error, i + 1);
        if (parts[segment] == NULL) {
            dcm_free_string_array(parts, n_segments);
            return NULL;
        }

        strncpy(parts[segment], p, i);
        parts[segment][i] = '\0';

        p += i + 1;
    }

    *vm = n_segments;

    return parts;
}


static DcmElement *read_element_header(DcmError **error,
                                       DcmFilehandle *filehandle,
                                       uint32_t *length,
                                       int64_t *position,
                                       bool implicit)
{
    uint32_t tag;
    if (!read_tag(error, filehandle, &tag, position)) {
        return NULL;
    }

    DcmVR vr;
    if (implicit) {
        vr = dcm_vr_from_tag(tag);
        if (vr == DCM_VR_ERROR) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element header failed",
                          "Tag %08X not allowed in implicit mode", tag);
        }
        if (!read_uint32(error, filehandle, length, position)) {
            return NULL;
        }
    } else {
        // Value Representation
        char vr_str[3];
        if (!dcm_require(error, filehandle, vr_str, 2, position)) {
            return NULL;
        }
        vr_str[2] = '\0';
        vr = dcm_dict_vr_from_str(vr_str);

        if (!dcm_is_valid_vr_for_tag(vr, tag)) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element header failed",
                          "Unknown or mismatched VR %s", vr_str);
            return NULL;
        }

        if (dcm_dict_vr_header_length(vr) == 2) {
            // These VRs have a short length of only two bytes
            uint16_t short_length;
            if (!read_uint16(error, filehandle, &short_length, position)) {
                return NULL;
            }
            *length = (uint32_t) short_length;
        } else {
            // Other VRs have two reserved bytes before length of four bytes
            uint16_t reserved;
            if (!read_uint16(error, filehandle, &reserved, position) ||
                !read_uint32(error, filehandle, length, position)) {
               return NULL;
            }

            if (reserved != 0x0000) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element header failed",
                              "Unexpected value for reserved bytes "
                              "of Data Element %08X with VR '%s'.",
                              tag, vr);
                return NULL;
            }
        }
    }

    return dcm_element_create(error, tag, vr);
}


// fwd ref this
static DcmElement *read_element(DcmError **error,
                                DcmFilehandle *filehandle,
                                int64_t *position,
                                bool implicit);

static bool read_element_sequence(DcmError **error,
                                  DcmFilehandle *filehandle,
                                  DcmSequence *sequence,
                                  uint32_t length,
                                  int64_t *position,
                                  bool implicit)
{
    int index = 0;
    int64_t seq_position = 0;
    while (seq_position < length) {
        dcm_log_debug("Read Item #%d.", index);
        uint32_t item_tag;
        uint32_t item_length;
        if (!read_iheader(error, filehandle,
                          &item_tag, &item_length, &seq_position)) {
            return false;
        }

        if (item_tag == TAG_SQ_DELIM) {
            dcm_log_debug("Stop reading Data Element. "
                          "Encountered Sequence Delimination Tag.");
            break;
        }

        if (item_tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Expected tag '%08X' instead of '%08X' "
                          "for Item #%d",
                          TAG_ITEM,
                          item_tag,
                          index);
            return false;
        }

        if (length == 0xFFFFFFFF) {
            dcm_log_debug("Item #%d has undefined length.", index);
        } else {
            dcm_log_debug("Item #%d has defined length %d.",
                          index, item_length);
        }

        DcmDataSet *dataset = dcm_dataset_create(error);
        if (dataset == NULL) {
            return false;
        }

        int64_t item_position = 0;
        while (item_position < item_length) {
            // peek the next tag
            if (!read_tag(error, filehandle, &item_tag, &item_position)) {
                dcm_dataset_destroy(dataset);
                return false;
            }

            if (item_tag == TAG_ITEM_DELIM) {
                // step over the tag length
                dcm_log_debug("Stop reading Item #%d. "
                              "Encountered Item Delimination Tag.",
                              index);
                if (!dcm_seekcur(error, filehandle, 4, &item_position)) {
                    dcm_dataset_destroy(dataset);
                    return false;
                }

                break;
            }

            // back to start of element
            if (!dcm_seekcur(error, filehandle, -4, &item_position)) {
                dcm_dataset_destroy(dataset);
                return false;
            }

            DcmElement *element = read_element(error, filehandle,
                                               &item_position, implicit);
            if (element == NULL) {
                dcm_dataset_destroy(dataset);
                return false;
            }

            if (!dcm_dataset_insert(error, dataset, element)) {
                dcm_dataset_destroy(dataset);
                dcm_element_destroy(element);
                return false;
            }
        }

        seq_position += item_position;

        if (!dcm_sequence_append(error, sequence, dataset)) {
            dcm_dataset_destroy(dataset);
        }

        index += 1;
    }

    *position += seq_position;

    return true;
}


static bool read_element_body(DcmError **error,
                              DcmElement *element,
                              DcmFilehandle *filehandle,
                              uint32_t length,
                              int64_t *position,
                              bool implicit)
{
    uint32_t tag = dcm_element_get_tag(element);
    DcmVR vr = dcm_element_get_vr(element);
    DcmVRClass klass = dcm_dict_vr_class(vr);
    size_t size = dcm_dict_vr_size(vr);
    char *value;

    dcm_log_debug("Read Data Element body '%08X'", tag);

    switch (klass) {
        case DCM_CLASS_STRING_SINGLE:
        case DCM_CLASS_STRING_MULTI:
            value = DCM_MALLOC(error, length + 1);
            if (value == NULL) {
                return false;
            }

            if (!dcm_require(error, filehandle, value, length, position)) {
                free(value);
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

            if (!dcm_element_set_value_string(error, element, value, true)) {
                free(value);
                return false;
            }

            break;

        case DCM_CLASS_NUMERIC:
            if (length % size != 0) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Bad length for tag '%08X'",
                              tag);
                return false;
            }
            uint32_t vm = length / size;

            char *values = DCM_MALLOC(error, length);
            if (values == NULL) {
                return false;
            }

            if (!dcm_require(error, filehandle, values, length, position)) {
                free(values);
                return false;
            }

            if (filehandle->byteswap) {
                byteswap(values, length, size);
            }

            if( !dcm_element_set_value_numeric_multi(error,
                                                     element,
                                                     (int *) values,
                                                     vm,
                                                     true)) {
                free(values);
                return false;
            }

            break;

        case DCM_CLASS_BINARY:
            value = DCM_MALLOC(error, length);
            if (value == NULL) {
                return false;
            }

            if (!dcm_require(error, filehandle, value, length, position)) {
                free(value);
                return false;
            }

            if( !dcm_element_set_value_binary(error, element,
                                              value, length, true)) {
                free(value);
                return false;
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

            DcmSequence *seq = dcm_sequence_create(error);
            if (seq == NULL) {
                return NULL;
            }

            int64_t seq_position = 0;
            if (!read_element_sequence(error,
                                       filehandle, seq, length,
                                       &seq_position, implicit)) {
                dcm_sequence_destroy(seq);
                return false;
            }
            *position += seq_position;

            if (!dcm_element_set_value_sequence(error, element, seq)) {
                return false;
            }

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


DcmElement *read_element(DcmError **error,
                         DcmFilehandle *filehandle,
                         int64_t *position,
                         bool implicit)
{
    DcmElement *element;

    uint32_t length;
    element = read_element_header(error,
                                  filehandle, &length, position, implicit);
    if (element == NULL) {
        return NULL;
    }

    if (!read_element_body(error, element, filehandle,
                           length, position, implicit)) {
        dcm_element_destroy(element);
        return NULL;
    }

    return element;
}


DcmDataSet *dcm_filehandle_read_file_meta(DcmError **error,
                                          DcmFilehandle *filehandle)
{
    const bool implicit = false;

    int64_t position;
    uint16_t group_number;
    DcmElement *element;

    DcmDataSet *file_meta = dcm_dataset_create(error);
    if (file_meta == NULL) {
        return NULL;
    }

    position = 0;

    // File Preamble
    char preamble[129];
    if (!dcm_require(error,
                     filehandle, preamble, sizeof(preamble) - 1, &position)) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    preamble[128] = '\0';

    // DICOM Prefix
    char prefix[5];
    if (!dcm_require(error,
                     filehandle, prefix, sizeof(prefix) - 1, &position)) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    prefix[4] = '\0';

    if (strcmp(prefix, "DICM") != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading of File Meta Information failed",
                      "Prefix 'DICM' not found.");
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    position = 0;

    // File Meta Information Group Length
    element = read_element(error, filehandle, &position, implicit);
    if (element == NULL) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    int64_t group_length;
    if (!dcm_element_get_value_integer(error, element, 0, &group_length)) {
        dcm_element_destroy(element);
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    dcm_element_destroy(element);

    // File Meta Information Version
    element = read_element(error, filehandle, &position, implicit);
    if (element == NULL) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    dcm_element_destroy(element);

    while (position < group_length) {
        uint32_t length;
        element = read_element_header(error,
                                      filehandle, &length, &position, implicit);
        if (element == NULL) {
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        group_number = dcm_element_get_group_number(element);
        if (group_number != 0x0002) {
            dcm_element_destroy(element);
            break;
        }

        if (!read_element_body(error, element, filehandle,
                               length, &position, implicit) ||
            !dcm_dataset_insert(error, file_meta, element)) {
            dcm_element_destroy(element);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }
    }

    if (!dcm_offset(error, filehandle, &filehandle->offset)) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    element = dcm_dataset_get(error, file_meta, 0x00020010);
    if (element == NULL) {
        filehandle->offset = 0;
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    const char *transfer_syntax_uid;
    if (!dcm_element_get_value_string(error,
                                      element, 0, &transfer_syntax_uid)) {
        filehandle->offset = 0;
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    filehandle->transfer_syntax_uid = dcm_strdup(error, transfer_syntax_uid);
    if (filehandle->transfer_syntax_uid == NULL) {
        filehandle->offset = 0;
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    dcm_dataset_lock(file_meta);

    return file_meta;
}


DcmDataSet *dcm_filehandle_read_metadata(DcmError **error,
                                         DcmFilehandle *filehandle)
{
    bool implicit;

    if (filehandle->offset == 0) {
        DcmDataSet *meta = dcm_filehandle_read_file_meta(error, filehandle);
        if (meta== NULL) {
            return NULL;
        }
        dcm_dataset_destroy(meta);
    }

    if (!dcm_seekset(error, filehandle, filehandle->offset)) {
        return NULL;
    }

    implicit = false;
    if (filehandle->transfer_syntax_uid) {
        if (strcmp(filehandle->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
            implicit = true;
        }
    }

    DcmDataSet *dataset = dcm_dataset_create(error);
    if (dataset == NULL) {
        return NULL;
    }

    int64_t position = 0;
    for (;;) {
        if (dcm_is_eof(filehandle)) {
            dcm_log_info("Stop reading Data Set. Reached end of filehandle.");
            break;
        }

        uint32_t length;
        DcmElement *element = read_element_header(error,
                                                  filehandle,
                                                  &length,
                                                  &position,
                                                  implicit);
        if (element == NULL) {
            dcm_dataset_destroy(dataset);
            return NULL;
        }
        uint32_t tag = dcm_element_get_tag(element);

        if (tag == TAG_TRAILING_PADDING) {
            dcm_log_info("Stop reading Data Set",
                         "Encountered Data Set Trailing Tag");
            dcm_element_destroy(element);
            break;
        }

        if (tag == TAG_PIXEL_DATA ||
            tag == TAG_FLOAT_PIXEL_DATA ||
            tag == TAG_DOUBLE_PIXEL_DATA) {
            dcm_log_debug("Stop reading Data Set. "
                          "Encountered Tag of Pixel Data Element.");
            dcm_element_destroy(element);

            // Set filehandle pointer to the first byte of the
            // pixel data element
            if (implicit) {
                // Tag: 4 bytes, Value Length: 4 bytes
                if (!dcm_seekcur(error, filehandle, -8, &position)) {
                    dcm_dataset_destroy(dataset);
                    return NULL;
                }

            } else {
                // Tag: 4 bytes, VR: 2 bytes + 2 bytes, Value Length: 4 bytes
                if (!dcm_seekcur(error, filehandle, -12, &position)) {
                    dcm_dataset_destroy(dataset);
                    return NULL;
                }
            }

            if (!dcm_offset(error,
                            filehandle, &filehandle->pixel_data_offset)) {
                dcm_dataset_destroy(dataset);
                return NULL;
            }

            break;
        }

        if (dcm_element_get_group_number(element) == 0x0002) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Set failed",
                          "Encountered File Meta Information group");
            dcm_element_destroy(element);
            dcm_dataset_destroy(dataset);
            return NULL;
        }

        if (!read_element_body(error, element, filehandle,
                               length, &position, implicit) ||
            !dcm_dataset_insert(error, dataset, element)) {
            dcm_element_destroy(element);
            dcm_dataset_destroy(dataset);
            return NULL;
        }
    }

    dcm_dataset_lock(dataset);

    return dataset;
}


static bool get_num_frames(DcmError **error,
                           const DcmDataSet *metadata,
                           uint32_t *number_of_frames)
{
    const uint32_t tag = 0x00280008;
    const char *value;
    uint32_t num_frames;

    DcmElement *element = dcm_dataset_get(error, metadata, tag);
    if (element == NULL ||
        !dcm_element_get_value_string(error, element, 0, &value)) {
        return false;
    }

    num_frames = (uint32_t) strtol(value, NULL, 10);
    if (num_frames == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Basic Offset Table read failed",
                      "Value of Data Element 'Number of Frames' is malformed");
        return false;
    }

    *number_of_frames = num_frames;

    return true;
}


DcmBOT *dcm_filehandle_read_bot(DcmError **error,
                                DcmFilehandle *filehandle,
                                DcmDataSet *metadata)
{
    uint64_t value;

    dcm_log_debug("Reading Basic Offset Table.");

    if (!dcm_is_encapsulated_transfer_syntax(filehandle->transfer_syntax_uid)) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Data Set with transfer syntax '%s' should not contain "
                      "a Basic Offset Table because it is not encapsulated",
                      filehandle->transfer_syntax_uid);
        return NULL;
    }

    uint32_t num_frames;
    if (!get_num_frames(error, metadata, &num_frames)) {
        return NULL;
    }

    if (filehandle->pixel_data_offset == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Could not determine offset of Pixel Data Element. "
                      "Read metadata first");
        return NULL;
    }

    if (!dcm_seekset(error, filehandle, filehandle->pixel_data_offset)) {
        return NULL;
    }

    // measure distance to first frame from pixel_data_offset
    int64_t position = 0;
    DcmElement *element;
    uint32_t length;
    element = read_element_header(error, filehandle, &length, &position, false);
    uint32_t tag = dcm_element_get_tag(element);
    dcm_element_destroy(element);

    if (tag != TAG_PIXEL_DATA &&
        tag != TAG_FLOAT_PIXEL_DATA &&
        tag != TAG_DOUBLE_PIXEL_DATA) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "File pointer not positioned at Pixel Data Element");
        return NULL;
    }

    // The header of the BOT Item
    if (!read_iheader(error, filehandle, &tag, &length, &position)) {
        return NULL;
    }
    if (tag != TAG_ITEM) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Unexpected Tag found for Basic Offset Table Item");
        return NULL;
    }

    ssize_t *offsets = DCM_NEW_ARRAY(error, num_frames, ssize_t);
    if (offsets == NULL) {
        return NULL;
    }

    // The BOT Item must be present, but the value is optional
    ssize_t first_frame_offset;
    if (length > 0) {
        dcm_log_info("Read Basic Offset Table value.");

        // Read offset values from BOT Item value
        // FIXME .. could do this with a single require to a uint32_t array,
        // see numeric array read above
        for (uint32_t i = 0; i < num_frames; i++) {
            uint32_t ui32;
            if (!read_uint32(error, filehandle, &ui32, &position)) {
                free(offsets);
                return NULL;
            }

            uint64_t value = ui32;
            if (value == TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading Basic Offset Table failed",
                              "Encountered unexpected Item Tag "
                              "in Basic Offset Table");
                free(offsets);
                return NULL;
            }

            offsets[i] = value;
        }

        // and that's the offset to the item header on the first frame
        first_frame_offset = position;
    } else {
        dcm_log_info("Basic Offset Table is empty");
        // Handle Extended Offset Table attribute
        const DcmElement *eot_element = dcm_dataset_contains(metadata,
                                                             0x7FE00001);
        if (eot_element == NULL) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Basic Offset Table failed",
                          "No Basic Offset Table, "
                          "and no Extended Offset Table");
            free(offsets);
            return NULL;
        }

        dcm_log_info("Found Extended Offset Table.");

        const char *blob;
        if (!dcm_element_get_value_binary(error, eot_element, &blob)) {
            free(offsets);
            return NULL;
        }

        for (uint32_t i = 0; i < num_frames; i++) {
            char *end_ptr;
            value = (uint64_t) strtoull(blob, &end_ptr, 64);
            // strtoull returns 0 in case of error
            // FIXME and also sets end_ptr to blob
            if (value == 0 && i > 0) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading Basic Offset Table failed",
                              "Failed to parse value of Extended Offset "
                              "Table element for frame #%d", i + 1);
                free(offsets);
                return NULL;
            }
            offsets[i] = value;
            blob = end_ptr;
        }

        // FIXME is this correct?
        first_frame_offset = position;
    }

    return dcm_bot_create(error, offsets, num_frames, first_frame_offset);
}


static bool set_pixel_description(DcmError **error,
                                  struct PixelDescription *desc,
                                  const DcmDataSet *metadata)
{
    DcmElement *element;
    int64_t value;
    const char *string;

    element = dcm_dataset_get(error, metadata, 0x00280010);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->rows = value;

    element = dcm_dataset_get(error, metadata, 0x00280011);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->columns = value;

    element = dcm_dataset_get(error, metadata, 0x00280002);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->samples_per_pixel = value;

    element = dcm_dataset_get(error, metadata, 0x00280100);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->bits_allocated = value;

    element = dcm_dataset_get(error, metadata, 0x00280101);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->bits_stored = value;

    element = dcm_dataset_get(error, metadata, 0x00280103);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->pixel_representation = value;

    element = dcm_dataset_get(error, metadata, 0x00280006);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->planar_configuration = value;

    element = dcm_dataset_get(error, metadata, 0x00280004);
    if (element == NULL ||
        !dcm_element_get_value_string(error, element, 0, &string)) {
        return false;
    }
    desc->photometric_interpretation = string;

    return true;
}


DcmBOT *dcm_filehandle_build_bot(DcmError **error,
                                 DcmFilehandle *filehandle,
                                 DcmDataSet *metadata)
{
    uint64_t i;

    dcm_log_debug("Building Basic Offset Table.");

    uint32_t num_frames;
    if (!get_num_frames(error, metadata, &num_frames)) {
        return NULL;
    }

    if (filehandle->pixel_data_offset == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Could not determine offset of Pixel Data Element. "
                      "Read metadata first.");
        return NULL;
    }

    if (!dcm_seekset(error, filehandle, filehandle->pixel_data_offset)) {
        return NULL;
    }

    // we measure offsets from this point
    int64_t position = 0;

    uint32_t length;
    DcmElement *element = read_element_header(error,
                                              filehandle,
                                              &length,
                                              &position,
                                              false);
    uint32_t tag = dcm_element_get_tag(element);
    dcm_element_destroy(element);

    if (tag != TAG_PIXEL_DATA &&
        tag != TAG_FLOAT_PIXEL_DATA &&
        tag != TAG_DOUBLE_PIXEL_DATA) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Building Basic Offset Table failed",
                      "Pixel data offset not positioned at Pixel Data Element");
        return NULL;
    }

    ssize_t *offsets = DCM_NEW_ARRAY(error, num_frames, ssize_t);
    if (offsets == NULL) {
        return NULL;
    }

    ssize_t first_frame_offset;

    if (dcm_is_encapsulated_transfer_syntax(filehandle->transfer_syntax_uid)) {
        uint32_t length;
        if (!read_iheader(error, filehandle, &tag, &length, &position)) {
            free(offsets);
            return NULL;
        }

        if (tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Building Basic Offset Table failed",
                          "Unexpected Tag found for Basic Offset Table Item");
            free(offsets);
            return NULL;
        }

        // Move filehandlepointer to the first byte of first Frame item
        if (!dcm_seekcur(error, filehandle, length, &position)) {
            free(offsets);
        }

        // and that's the offset to the first frame
        first_frame_offset = position;

        // now measure positions from the start of the first frame
        position = 0;

        i = 0;
        while (true) {
            if (!read_iheader(error, filehandle, &tag, &length, &position)) {
                free(offsets);
                return NULL;
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
                free(offsets);
                return NULL;
            }

            if (dcm_is_eof(filehandle)) {
                break;
            }

            // step back to the start of the item for this frame
            offsets[i] = position - 8;

            if (!dcm_seekcur(error, filehandle, length, &position)) {
                free(offsets);
                return NULL;
            }

            i += 1;
        }

        if (i != num_frames) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Building Basic Offset Table failed",
                          "Found incorrect number of Frame Items");
            free(offsets);
            return NULL;
        }
    } else {
        struct PixelDescription desc;
        if (!set_pixel_description(error, &desc, metadata)) {
            free(offsets);
            return NULL;
        }

        for (i = 0; i < num_frames; i++) {
            offsets[i] = i * desc.rows * desc.columns * desc.samples_per_pixel;
        }

        // Header of Pixel Data Element
        first_frame_offset = 10;
    }

    return dcm_bot_create(error, offsets, num_frames, first_frame_offset);
}


DcmFrame *dcm_filehandle_read_frame(DcmError **error,
                                    DcmFilehandle *filehandle,
                                    DcmDataSet *metadata,
                                    DcmBOT *bot,
                                    uint32_t number)
{
    uint32_t length;

    dcm_log_debug("Read Frame Item #%d.", number);
    if (number == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame Item failed",
                      "Frame Number must be positive");
        return NULL;
    }

    ssize_t frame_offset = dcm_bot_get_frame_offset(bot, number);
    ssize_t total_frame_offset = filehandle->pixel_data_offset + frame_offset;
    if (!dcm_seekset(error, filehandle, total_frame_offset)) {
        return NULL;
    }

    struct PixelDescription desc;
    if (!set_pixel_description(error, &desc, metadata)) {
        return NULL;
    }

    int64_t position = 0;
    if (dcm_is_encapsulated_transfer_syntax(filehandle->transfer_syntax_uid)) {
        uint32_t tag;
        if (!read_iheader(error, filehandle, &tag, &length, &position)) {
            return NULL;
        }

        if (tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Frame Item failed",
                          "No Item Tag found for Frame Item #%d",
                          number);
            return NULL;
        }
    } else {
        length = desc.rows * desc.columns * desc.samples_per_pixel;
    }

    char *value = DCM_MALLOC(error, length);
    if (value == NULL) {
        return NULL;
    }
    if (!dcm_require(error, filehandle, value, length, &position)) {
        free(value);
        return NULL;
    }

    return dcm_frame_create(error,
                            number,
                            value,
                            length,
                            desc.rows,
                            desc.columns,
                            desc.samples_per_pixel,
                            desc.bits_allocated,
                            desc.bits_stored,
                            desc.pixel_representation,
                            desc.planar_configuration,
                            desc.photometric_interpretation,
                            filehandle->transfer_syntax_uid);
}
