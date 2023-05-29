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
    DcmIO *io;
    int64_t offset;
    char *transfer_syntax_uid;
    int64_t pixel_data_offset;
    uint64_t *extended_offset_table;
    bool byteswap;
    bool implicit;
    uint32_t last_tag;
    uint32_t frame_number;
    uint32_t tiles_across;
    uint32_t num_frames;
    uint32_t *frame_index;

    // push and pop these while we parse
    UT_array *dataset_stack;
    UT_array *sequence_stack;
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

DcmFilehandle *dcm_filehandle_create(DcmError **error, DcmIO *io)
{
    DcmFilehandle *filehandle = DCM_NEW(error, DcmFilehandle);
    if (filehandle == NULL) {
        return NULL;
    }

    filehandle->io = io;
    filehandle->offset = 0;
    filehandle->transfer_syntax_uid = NULL;
    filehandle->pixel_data_offset = 0;
    filehandle->extended_offset_table = NULL;
    filehandle->byteswap = is_big_endian();
    filehandle->last_tag = 0xffffffff;
    filehandle->frame_index = NULL;
    utarray_new(filehandle->dataset_stack, &ut_ptr_icd); 
    utarray_new(filehandle->sequence_stack, &ut_ptr_icd); 

    return filehandle;
}


DcmFilehandle *dcm_filehandle_create_from_file(DcmError **error, 
                                               const char *filepath)
{
    DcmIO *io = dcm_io_create_from_file(error, filepath);
    if (io == NULL) {
        return NULL;
    }

    return dcm_filehandle_create(error, io);
}


DcmFilehandle *dcm_filehandle_create_from_memory(DcmError **error,
                                                 const char *buffer, 
                                                 int64_t length)
{
    DcmIO *io = dcm_io_create_from_memory(error, buffer, length);
    if (io == NULL) {
        return NULL;
    }

    return dcm_filehandle_create(error, io);
}


static void dcm_filehandle_clear(DcmFilehandle *filehandle)
{
    unsigned int i;

    for (i = 0; i < utarray_len(filehandle->dataset_stack); i++) {
        DcmDataSet *dataset = *((DcmDataSet **) 
                utarray_eltptr(filehandle->dataset_stack, i));

        dcm_dataset_destroy(dataset);
    }

    utarray_clear(filehandle->dataset_stack);

    for (i = 0; i < utarray_len(filehandle->dataset_stack); i++) {
        DcmSequence *sequence = *((DcmSequence **) 
                utarray_eltptr(filehandle->sequence_stack, i));

        dcm_sequence_destroy(sequence);
    }

    utarray_clear(filehandle->sequence_stack);
}


void dcm_filehandle_destroy(DcmFilehandle *filehandle)
{
    if (filehandle) {
        dcm_filehandle_clear(filehandle);

        if (filehandle->transfer_syntax_uid) {
            free(filehandle->transfer_syntax_uid);
        }

        if (filehandle->frame_index) {
            free(filehandle->frame_index);
        }

        (void) dcm_io_close(NULL, filehandle->io);

        utarray_free(filehandle->dataset_stack);
        utarray_free(filehandle->sequence_stack);

        free(filehandle);
    }
}


static int64_t dcm_read(DcmError **error, DcmFilehandle *filehandle,
    char *buffer, int64_t length, int64_t *position)
{
    int64_t bytes_read = dcm_io_read(error, filehandle->io, buffer, length);
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
    return dcm_io_seek(error, filehandle->io, offset, SEEK_SET) >= 0;
}


static bool dcm_seekcur(DcmError **error, DcmFilehandle *filehandle,
    int64_t offset, int64_t *position)
{
    int64_t new_offset = dcm_io_seek(error, filehandle->io, offset, SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *position += offset;

    return true;
}


static bool dcm_offset(DcmError **error,
                       DcmFilehandle *filehandle, int64_t *offset)
{
    int64_t new_offset = dcm_io_seek(error, filehandle->io, 0, SEEK_CUR);
    if (new_offset < 0) {
        return false;
    }

    *offset = new_offset;

    return true;
}


static bool dcm_is_eof(DcmFilehandle *filehandle)
{
    int64_t position = 0;
    bool eof = true;

    char buffer[1];
    int64_t bytes_read = dcm_io_read(NULL, filehandle->io, buffer, 1);
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


static bool get_tag_int(DcmError **error,
                        const DcmDataSet *dataset,
                        const char *keyword,
                        int64_t *result) 
{
    uint32_t tag = dcm_dict_tag_from_keyword(keyword);
    DcmElement *element = dcm_dataset_get(error, dataset, tag);
    return element &&
         dcm_element_get_value_integer(error, element, 0, result);
}


static bool get_tiles_across(DcmError **error,
                             const DcmDataSet *metadata,
                             uint32_t *tiles_across)
{
    int64_t width;
    int64_t tile_width;

    if (!get_tag_int(error, metadata, "TotalPixelMatrixColumns", &width) ||
        !get_tag_int(error, metadata, "Columns", &tile_width)) {
        return false;
    }

    *tiles_across = width / tile_width + !!(width % tile_width);

    return true;
}


static DcmElement *read_element_header(DcmError **error,
                                       DcmFilehandle *filehandle,
                                       uint32_t *length,
                                       int64_t *position)
{
    uint32_t tag;
    if (!read_tag(error, filehandle, &tag, position)) {
        return NULL;
    }

    DcmVR vr;
    if (filehandle->implicit) {
        // this can be an ambiguious VR, eg. pixeldata is allowed in implicit
        // mode and has to be disambiguated later from other tags
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
                          "Tag %08X cannot have VR '%s'", tag, vr_str);
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
                                              &position);
    if (element == NULL) {
        return NULL;
    }
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


static bool parse_meta_dataset_begin(DcmError **error, 
                                     void *client) 
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    DcmDataSet *dataset = dcm_dataset_create(error);
    if (dataset == NULL) {
        return false;
    }

    utarray_push_back(filehandle->dataset_stack, &dataset);

    return true;
}


static bool parse_meta_dataset_end(DcmError **error, 
                                   void *client) 
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    DcmDataSet *dataset = *((DcmDataSet **) 
            utarray_back(filehandle->dataset_stack));
    DcmSequence *sequence = *((DcmSequence **)
            utarray_back(filehandle->sequence_stack));

    if (!dcm_sequence_append(error, sequence, dataset)) {
        return false;
    }

    utarray_pop_back(filehandle->dataset_stack);

    return true;
}


static bool parse_meta_sequence_begin(DcmError **error, 
                                      void *client) 
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return false;
    }

    utarray_push_back(filehandle->sequence_stack, &sequence);

    return true;
}


static bool parse_meta_sequence_end(DcmError **error, 
                                    void *client,
                                    uint32_t tag,
                                    DcmVR vr,
                                    uint32_t length)
{
    USED(length);

    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    DcmElement *element = dcm_element_create(error, tag, vr);
    if (element == NULL) {
        return false;
    }

    DcmSequence *sequence = *((DcmSequence **) 
            utarray_back(filehandle->sequence_stack));
    if (!dcm_element_set_value_sequence(error, element, sequence)) {
        dcm_element_destroy(element);
        return false;
    }

    utarray_pop_back(filehandle->sequence_stack);

    DcmDataSet *dataset = *((DcmDataSet **)
            utarray_back(filehandle->dataset_stack));
    if (!dcm_dataset_insert(error, dataset, element)) {
        dcm_element_destroy(element);
        return false;
    }

    return true;
}


static bool parse_meta_element_create(DcmError **error, 
                                      void *client, 
                                      uint32_t tag,
                                      DcmVR vr,
                                      char *value,
                                      uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    DcmElement *element = dcm_element_create(error, tag, vr);
    if (element == NULL) {
        return false;
    }

    DcmDataSet *dataset = *((DcmDataSet **)
            utarray_back(filehandle->dataset_stack));
    if (!dcm_element_set_value(error, element, value, length, false) ||
        !dcm_dataset_insert(error, dataset, element)) {
        dcm_element_destroy(element);
        return false;
    }

    return true;
}


DcmDataSet *dcm_filehandle_read_file_meta(DcmError **error,
                                          DcmFilehandle *filehandle)
{
    static DcmParse parse = {
        .dataset_begin = parse_meta_dataset_begin,
        .dataset_end = parse_meta_dataset_end,
        .sequence_begin = parse_meta_sequence_begin,
        .sequence_end = parse_meta_sequence_end,
        .element_create = parse_meta_element_create,
        .stop = NULL,
    };

    int64_t position = 0;

    // File Preamble
    char preamble[129];
    if (!dcm_require(error,
                     filehandle, preamble, sizeof(preamble) - 1, &position)) {
        return NULL;
    }
    preamble[128] = '\0';

    // DICOM Prefix
    char prefix[5];
    if (!dcm_require(error,
                     filehandle, prefix, sizeof(prefix) - 1, &position)) {
        return NULL;
    }
    prefix[4] = '\0';

    if (strcmp(prefix, "DICM") != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading of File Meta Information failed",
                      "Prefix 'DICM' not found.");
        return NULL;
    }

    // sanity ... the parse stacks should be empty
    if (utarray_len(filehandle->dataset_stack) != 0 ||
        utarray_len(filehandle->sequence_stack) != 0) {
        abort();
    }

    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return NULL;
    }
    utarray_push_back(filehandle->sequence_stack, &sequence);

    // parse all of the first group
    if (!dcm_parse_group(error,
                         filehandle->io,
                         false,
                         false,
                         &parse,
                         filehandle)) {
        dcm_filehandle_clear(filehandle);
        return false;
    }

    /* Sanity check. We should have parsed a single dataset into the sequence
     * we put on the stack.
     */
    if (utarray_len(filehandle->dataset_stack) != 0 ||
        utarray_len(filehandle->sequence_stack) != 1) {
        abort();
    }

    // record the start point for the image metadata
    if (!dcm_offset(error, filehandle, &filehandle->offset)) {
        return NULL;
    }

    // we must read sequence back off the stack since it may have been
    // realloced
    sequence = *((DcmSequence **) utarray_back(filehandle->sequence_stack));
    if (dcm_sequence_count(sequence) != 1) {
        abort();
    }

    // FIXME ... add dcm_sequence_steal() so we can destroy sequence without
    // also destroying meta
    // right now we leak sequence
    DcmDataSet *file_meta = dcm_sequence_get(error, sequence, 0);
    if (file_meta == NULL ) {
        return false;
    }

    DcmElement *element = dcm_dataset_get(error, file_meta, 0x00020010);
    if (element == NULL) {
        return NULL;
    }

    const char *transfer_syntax_uid;
    if (!dcm_element_get_value_string(error,
                                      element, 
                                      0, 
                                      &transfer_syntax_uid)) {
        return NULL;
    }

    filehandle->transfer_syntax_uid = dcm_strdup(error, transfer_syntax_uid);
    if (filehandle->transfer_syntax_uid == NULL) {
        return NULL;
    }

    // we need to pop sequence off the dataset stack to stop it being destroyed
    utarray_pop_back(filehandle->sequence_stack);

    dcm_dataset_lock(file_meta);

    return file_meta;
}



static bool parse_meta_stop(void *client, 
                            uint32_t tag,
                            DcmVR vr,
                            uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(vr);
    USED(length);

    filehandle->last_tag = tag;

    return tag == TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE ||
           tag == TAG_PIXEL_DATA ||
           tag == TAG_FLOAT_PIXEL_DATA ||
           tag == TAG_DOUBLE_PIXEL_DATA;
}


DcmDataSet *dcm_filehandle_read_metadata(DcmError **error,
                                         DcmFilehandle *filehandle)
{
    static DcmParse parse = {
        .dataset_begin = parse_meta_dataset_begin,
        .dataset_end = parse_meta_dataset_end,
        .sequence_begin = parse_meta_sequence_begin,
        .sequence_end = parse_meta_sequence_end,
        .element_create = parse_meta_element_create,
        .stop = parse_meta_stop,
    };

    if (filehandle->offset == 0) {
        DcmDataSet *file_meta = dcm_filehandle_read_file_meta(error,
                                                              filehandle);
        if (file_meta == NULL) {
            return NULL;
        }
        dcm_dataset_destroy(file_meta);
    }

    if (!dcm_seekset(error, filehandle, filehandle->offset)) {
        return NULL;
    }

    if (filehandle->transfer_syntax_uid) {
        if (strcmp(filehandle->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
            filehandle->implicit = true;
        }
    }

    // sanity ... the parse stacks should be empty
    if (utarray_len(filehandle->dataset_stack) != 0 ||
        utarray_len(filehandle->sequence_stack) != 0) {
        abort();
    }

    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return NULL;
    }
    utarray_push_back(filehandle->sequence_stack, &sequence);

    // parse up to perframefunctionalgroupsequence, or the pixel data
    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           filehandle->byteswap,
                           &parse,
                           filehandle)) {
        dcm_filehandle_clear(filehandle);
        return false;
    }

    /* Sanity check. We should have parsed a single dataset into the sequence
     * we put on the stack.
     */
    if (utarray_len(filehandle->dataset_stack) != 0 ||
        utarray_len(filehandle->sequence_stack) != 1) {
        abort();
    }

    // we must read sequence back off the stack since it may have been
    // realloced
    sequence = *((DcmSequence **) utarray_back(filehandle->sequence_stack));
    if (dcm_sequence_count(sequence) != 1) {
        abort();
    }

    // FIXME ... add dcm_sequence_steal() so we can destroy sequence without
    // also destroying meta
    // right now we leak sequence
    DcmDataSet *meta = dcm_sequence_get(error, sequence, 0);
    if (meta == NULL ) {
        return false;
    }

    // did we stop on pixel data? record the offset
    if (filehandle->last_tag == TAG_PIXEL_DATA ||
        filehandle->last_tag == TAG_FLOAT_PIXEL_DATA ||
        filehandle->last_tag == TAG_DOUBLE_PIXEL_DATA) {
        if (!dcm_offset(error,
                        filehandle, &filehandle->pixel_data_offset)) {
            return NULL;
        }
    }

    // we need to pop sequence off the dataset stack to stop it being destroyed
    utarray_pop_back(filehandle->sequence_stack);

    dcm_dataset_lock(meta);

    return meta;
}


static bool parse_frame_index_element_create(DcmError **error, 
                                             void *client, 
                                             uint32_t tag,
                                             DcmVR vr,
                                             char *value,
                                             uint32_t length)
{
    USED(error);

    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    if (vr == DCM_VR_UL && length == 8 && tag == TAG_DIMENSION_INDEX_VALUES) {
        // it will have already been byteswapped for us, if necessary
        uint32_t *ul = (uint32_t *) value;
        uint32_t col = ul[0];
        uint32_t row = ul[1];
        uint32_t index = (col - 1) + (row - 1) * filehandle->tiles_across;

        if (index < filehandle->num_frames) {
            filehandle->frame_index[index] = filehandle->frame_number;
            filehandle->frame_number += 1;
        }
    }

    return true;
}


static bool parse_frame_index_stop(void *client, 
                                   uint32_t tag,
                                   DcmVR vr,
                                   uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(vr);
    USED(length);

    filehandle->last_tag = tag;

    return tag != TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE;
}


static bool read_frame_index(DcmError **error,
                             DcmFilehandle *filehandle,
                             DcmDataSet *metadata)
{
    static DcmParse parse = {
        .element_create = parse_frame_index_element_create,
        .stop = parse_frame_index_stop,
    };

    if (!get_tiles_across(error, metadata, &filehandle->tiles_across) ||
        !get_num_frames(error, metadata, &filehandle->num_frames)) {
        return false;
    }

    filehandle->frame_index = DCM_NEW_ARRAY(error, 
                                            filehandle->num_frames, 
                                            uint32_t);
    if (filehandle->frame_index == NULL) {
        return false;
    }

    // we may not have all frames ... set to missing initially
    for (uint32_t i = 0; i < filehandle->num_frames; i++) {
        filehandle->frame_index[i] = -1;
    }

    // parse just the per-frame stuff
    filehandle->frame_number = 0;
    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           filehandle->byteswap,
                           &parse,
                           filehandle)) {
        return false;
    }

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

    // did we stop on per-frame func group? parse that to build the index
    if (filehandle->last_tag == TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE &&
        !read_frame_index(error, filehandle, metadata)) {
        return false;
    }

    // and hopefully we're now on the pixel data
    if (filehandle->last_tag == TAG_PIXEL_DATA ||
        filehandle->last_tag == TAG_FLOAT_PIXEL_DATA ||
        filehandle->last_tag == TAG_DOUBLE_PIXEL_DATA) {
        if (!dcm_offset(error,
                        filehandle, &filehandle->pixel_data_offset)) {
            return NULL;
        }
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
    element = read_element_header(error, filehandle, &length, &position);
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

