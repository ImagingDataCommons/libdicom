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


struct _DcmFilehandle {
    DcmIO *io;
    char *transfer_syntax_uid;
    bool byteswap;
    bool implicit;

    // start of image metadata
    int64_t offset;
    // start of pixel metadata
    int64_t pixel_data_offset;
    // distance from pixel metadata to start of first frame
    ssize_t first_frame_offset;

    // image properties we need to track
    uint32_t tiles_across;
    uint32_t num_frames;
    struct PixelDescription desc;

    // both zero-indexed and length num_frames
    uint32_t *frame_index;
    int64_t *offset_table;

    // the last top level tag the scanner saw
    uint32_t last_tag;

    // used to count frames as we scan perframefunctionalgroup
    uint32_t frame_number;

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

    num_frames = strtol(value, NULL, 10);
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

    // useful values for later
    if (!get_tiles_across(error, meta, &filehandle->tiles_across) ||
        !get_num_frames(error, meta, &filehandle->num_frames)) {
        return false;
    }

    if (!set_pixel_description(error, &filehandle->desc, meta)) {
        return false;
    }
    filehandle->desc.transfer_syntax_uid = filehandle->transfer_syntax_uid;

    // did we stop on pixel data? record the offset
    if (filehandle->last_tag == TAG_PIXEL_DATA ||
        filehandle->last_tag == TAG_FLOAT_PIXEL_DATA ||
        filehandle->last_tag == TAG_DOUBLE_PIXEL_DATA) {
        if (!dcm_offset(error,
                        filehandle, 
                        &filehandle->pixel_data_offset)) {
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
                             DcmFilehandle *filehandle)
{
    static DcmParse parse = {
        .element_create = parse_frame_index_element_create,
        .stop = parse_frame_index_stop,
    };

    filehandle->frame_index = DCM_NEW_ARRAY(error, 
                                            filehandle->num_frames, 
                                            uint32_t);
    if (filehandle->frame_index == NULL) {
        return false;
    }

    // we may not have all frames ... set to missing initially
    for (uint32_t i = 0; i < filehandle->num_frames; i++) {
        filehandle->frame_index[i] = 0xffffffff;
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


bool dcm_filehandle_read_pixeldata(DcmError **error,
                                   DcmFilehandle *filehandle)
{
    // did we stop on per-frame func group? parse that to build the index
    if (filehandle->last_tag == TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE &&
        !read_frame_index(error, filehandle)) {
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
                      "Reading PixelData failed",
                      "Could not determine offset of Pixel Data Element. "
                      "Read metadata first");
        return NULL;
    }

    if (!dcm_seekset(error, filehandle, filehandle->pixel_data_offset)) {
        return NULL;
    }

    filehandle->offset_table = DCM_NEW_ARRAY(error, 
                                             filehandle->num_frames, 
                                             int64_t);
    if (filehandle->offset_table == NULL) {
        return NULL;
    }

    dcm_log_debug("Reading PixelData.");

    if (dcm_is_encapsulated_transfer_syntax(filehandle->transfer_syntax_uid)) {
        // read the bot if available, otherwise parse pixeldata to find
        // offsets
        if (!dcm_parse_pixeldata(error,
                                 filehandle->io,
                                 filehandle->implicit,
                                 filehandle->byteswap,
                                 &filehandle->first_frame_offset,
                                 filehandle->offset_table,
                                 filehandle->num_frames)) {
            return false;
        }
    } else {
        for (uint32_t i = 0; i < filehandle->num_frames; i++) {
            filehandle->offset_table[i] = i * 
                                          filehandle->desc.rows * 
                                          filehandle->desc.columns * 
                                          filehandle->desc.samples_per_pixel;
        }

        // Header of Pixel Data Element
        filehandle->first_frame_offset = 10;
    }

    return true;
}


DcmFrame *dcm_filehandle_read_frame(DcmError **error,
                                    DcmFilehandle *filehandle,
                                    uint32_t frame_number)
{
    dcm_log_debug("Read frame number #%u.", frame_number);

    if (filehandle->offset_table == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame Item failed",
                      "No offset table loaded");
        return NULL;
    }

    if (frame_number == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame Item failed",
                      "Frame Number must be non-zero");
        return NULL;
    }
    if (frame_number > filehandle->num_frames) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame Item failed",
                      "Frame Number must be less than %u", 
                      filehandle->num_frames);
        return NULL;
    }

    // we are zero-based from here on
    uint32_t i = frame_number - 1;

    if (filehandle->frame_index) {
        i = filehandle->frame_index[i];
        if (i == 0xffffffff) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Frame Item failed",
                          "No such frame"); 
            return NULL;
        }
    }

    ssize_t total_frame_offset = filehandle->pixel_data_offset + 
                                 filehandle->first_frame_offset + 
                                 filehandle->offset_table[i];
    if (!dcm_seekset(error, filehandle, total_frame_offset)) {
        return NULL;
    }

    uint32_t length;
    char *frame_data = dcm_parse_frame(error,
                                       filehandle->io,
                                       filehandle->implicit,
                                       filehandle->byteswap,
                                       &filehandle->desc,
                                       &length);
    if (frame_data == NULL) {
        return NULL;
    }

    return dcm_frame_create(error,
                            frame_number,
                            frame_data,
                            length,
                            filehandle->desc.rows,
                            filehandle->desc.columns,
                            filehandle->desc.samples_per_pixel,
                            filehandle->desc.bits_allocated,
                            filehandle->desc.bits_stored,
                            filehandle->desc.pixel_representation,
                            filehandle->desc.planar_configuration,
                            filehandle->desc.photometric_interpretation,
                            filehandle->desc.transfer_syntax_uid);
}
