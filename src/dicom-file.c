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

typedef enum _DcmLayout {
    DCM_LAYOUT_SPARSE,
    DCM_LAYOUT_FULL,
    DCM_LAYOUT_UNKNOWN,
} DcmLayout;

struct _DcmFilehandle {
    DcmIO *io;
    char *transfer_syntax_uid;
    bool implicit;

    // start of image metadata
    int64_t offset;
    // start of pixel metadata
    int64_t pixel_data_offset;
    // distance from pixel metadata to start of first frame
    int64_t first_frame_offset;

    // image properties we need to track
    uint32_t tiles_across;
    uint32_t num_frames;
    struct PixelDescription desc;
    DcmLayout layout;

    // both zero-indexed and length num_frames
    uint32_t *frame_index;
    int64_t *offset_table;

    // the last top level tag the scanner saw
    uint32_t last_tag;

    // used to count frames as we scan perframefunctionalgroup
    uint32_t frame_number;

    // indent for file print
    int indent;

    // dataset index for file print
    int index;

    // push and pop these while we parse
    UT_array *dataset_stack;
    UT_array *sequence_stack;
};


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
    filehandle->last_tag = 0xffffffff;
    filehandle->frame_number = 0;
    filehandle->layout = DCM_LAYOUT_FULL;
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

    for (i = 0; i < utarray_len(filehandle->sequence_stack); i++) {
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

        if (filehandle->offset_table) {
            free(filehandle->offset_table);
        }

        dcm_io_close(filehandle->io);

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


static bool get_tag_str(DcmError **error,
                        const DcmDataSet *dataset,
                        const char *keyword,
                        const char **result)
{
    uint32_t tag = dcm_dict_tag_from_keyword(keyword);
    DcmElement *element = dcm_dataset_get(error, dataset, tag);
    return element &&
         dcm_element_get_value_string(error, element, 0, result);
}


static bool get_num_frames(DcmError **error,
                           const DcmDataSet *metadata,
                           uint32_t *number_of_frames)
{
    const char *value;
    if (!get_tag_str(error, metadata, "NumberOfFrames", &value)) {
        return false;
    }

    uint32_t num_frames = strtol(value, NULL, 10);
    if (num_frames == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Basic Offset Table read failed",
                      "Value of Data Element 'Number of Frames' is malformed");
        return false;
    }

    *number_of_frames = num_frames;

    return true;
}


static bool get_tiles_across(DcmError **error,
                             const DcmDataSet *metadata,
                             uint32_t *tiles_across)
{
    int64_t width;
    int64_t tile_width;

    if (!get_tag_int(error, metadata, "Columns", &tile_width)) {
        return false;
    }

    // TotalPixelMatrixColumns is optional and defaults to Columns, ie. one
    // tile across
    width = tile_width;
    (void) get_tag_int(NULL, metadata, "TotalPixelMatrixColumns", &width);

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
                                      void *client,
                                      uint32_t tag,
                                      DcmVR vr,
                                      uint32_t length)
{
    USED(tag);
    USED(vr);
    USED(length);

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


static bool parse_preamble(DcmError **error,
                           DcmFilehandle *filehandle,
                           int64_t *position)
{
    *position = 128;
    if (!dcm_seekset(error, filehandle, *position)) {
        return false;
    }

    // DICOM Prefix
    char prefix[5];
    if (!dcm_require(error,
                     filehandle, prefix, sizeof(prefix) - 1, position)) {
        return false;
    }
    prefix[4] = '\0';

    if (strcmp(prefix, "DICM") != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading of File Meta Information failed",
                      "Prefix 'DICM' not found.");
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

    // skip File Preamble
    int64_t position = 0;
    if (!parse_preamble(error, filehandle, &position)) {
        return NULL;
    }

    dcm_filehandle_clear(filehandle);
    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return NULL;
    }
    utarray_push_back(filehandle->sequence_stack, &sequence);

    // parse all of the first group
    if (!dcm_parse_group(error,
                         filehandle->io,
                         false,
                         &parse,
                         filehandle)) {
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

    if (strcmp(filehandle->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
        filehandle->implicit = true;
    }

    // steal file_meta to stop it being destroyed
    (void) dcm_sequence_steal(NULL, sequence, 0);
    dcm_filehandle_clear(filehandle);

    return file_meta;
}


const char *dcm_filehandle_get_transfer_syntax_uid(const DcmFilehandle *filehandle)
{
    return filehandle->transfer_syntax_uid;
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

    return tag == TAG_REFERENCED_IMAGE_NAVIGATION_SEQUENCE ||
           tag == TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE ||
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

    dcm_filehandle_clear(filehandle);
    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return NULL;
    }
    utarray_push_back(filehandle->sequence_stack, &sequence);

    // parse up to perframefunctionalgroupsequence, or the pixel data
    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           &parse,
                           filehandle)) {
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

    filehandle->desc.transfer_syntax_uid =
        dcm_filehandle_get_transfer_syntax_uid(filehandle);

    // we support sparse and full tile layout, defaulting to full if no type
    // is specified
    const char *type;
    if (get_tag_str(NULL, meta, "DimensionOrganizationType", &type)) {
        if (strcmp(type, "TILED_SPARSE") == 0 || strcmp(type, "3D") == 0) {
            filehandle->layout = DCM_LAYOUT_SPARSE;
        } else if (strcmp(type, "TILED_FULL") == 0) {
            filehandle->layout = DCM_LAYOUT_FULL;
        } else {
            filehandle->layout = DCM_LAYOUT_UNKNOWN;
        }
    }

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

    // steal meta to stop it being destroyed
    (void) dcm_sequence_steal(NULL, sequence, 0);
    dcm_filehandle_clear(filehandle);

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
                           &parse,
                           filehandle)) {
        return false;
    }

    return true;
}


static bool parse_skip_to_index(void *client,
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


static bool read_skip_to_index(DcmError **error,
                             DcmFilehandle *filehandle)

{
    static DcmParse parse = {
        .stop = parse_skip_to_index,
    };

    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           &parse,
                           filehandle)) {
        return false;
    }

    return true;
}


bool dcm_filehandle_read_pixeldata(DcmError **error,
                                   DcmFilehandle *filehandle)
{
    if (filehandle->layout == DCM_LAYOUT_UNKNOWN) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading PixelData failed",
                      "Unsupported DimensionOrganisationType.");
        return false;
    }

    // we may have previously stopped for many reasons ... skip ahead to per
    // frame functional group, or pixel data
    if (!read_skip_to_index(error, filehandle)) {
        return false;
    }

    // if we're on per frame func, read that in
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
                      "Could not determine offset of Pixel Data Element.");
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

    const char *syntax = dcm_filehandle_get_transfer_syntax_uid(filehandle);
    if (dcm_is_encapsulated_transfer_syntax(syntax)) {
        // read the bot if available, otherwise parse pixeldata to find
        // offsets
        if (!dcm_parse_pixeldata_offsets(error,
                                         filehandle->io,
                                         filehandle->implicit,
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

    // load metadata around pixeldata, if we've not loaded it already
    if (filehandle->offset_table == NULL &&
        !dcm_filehandle_read_pixeldata(error, filehandle))  {
        return NULL;
    }

    // we are zero-based from here on
    uint32_t i = frame_number - 1;

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


DcmFrame *dcm_filehandle_read_frame_position(DcmError **error,
                                             DcmFilehandle *filehandle,
                                             uint32_t column,
                                             uint32_t row)
{
    dcm_log_debug("Read frame position (%u, %u)", column, row);

    // load metadata around pixeldata, if we've not loaded it already
    if (filehandle->offset_table == NULL &&
        !dcm_filehandle_read_pixeldata(error, filehandle))  {
        return NULL;
    }

    if (column >= filehandle->tiles_across) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame position failed",
                      "Column must be less than %u",
                      filehandle->tiles_across);
        return NULL;
    }

    uint32_t index = column + row * filehandle->tiles_across;

    if (index >= filehandle->num_frames) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame position failed",
                      "Row must be less than %u",
                      filehandle->num_frames / filehandle->tiles_across);
        return NULL;
    }

    if (filehandle->layout == DCM_LAYOUT_SPARSE) {
        index = filehandle->frame_index[index];
        if (index == 0xffffffff) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Frame position failed",
                          "No Frame at position (%u, %u)", column, row);
            return NULL;
        }
    }

    // read_frame() numbers from 1
    return dcm_filehandle_read_frame(error, filehandle, index + 1);
}


static bool print_dataset_begin(DcmError **error,
                                void *client)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);

    filehandle->index += 1;

    if (filehandle->indent > 0) {
        printf("%*.*s---Item #%d---\n",
                   filehandle->indent * 2,
                   filehandle->indent * 2,
                   "                                   ",
                   filehandle->index);
    }

    return true;
}


static bool print_sequence_begin(DcmError **error,
                                 void *client,
                                 uint32_t tag,
                                 DcmVR vr,
                                 uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);
    USED(vr);
    USED(length);

    printf("%*.*s(%04x,%04x) ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           (tag & 0xffff0000) >> 16,
           tag >> 16);

    if (dcm_is_public_tag(tag)) {
        printf("%s ", dcm_dict_keyword_from_tag(tag));
    }

    printf("[\n");

    filehandle->indent += 1;
    filehandle->index = 0;

    return true;
}


static bool print_sequence_end(DcmError **error,
                               void *client,
                               uint32_t tag,
                               DcmVR vr,
                               uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);
    USED(tag);
    USED(vr);
    USED(length);

    filehandle->indent -= 1;

    printf("%*.*s]\n",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ");

    return true;
}


static bool print_pixeldata_begin(DcmError **error,
                                  void *client,
                                  uint32_t tag,
                                  DcmVR vr,
                                  uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);

    printf("%*.*s(%04x,%04x) ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           (tag & 0xffff0000) >> 16,
           tag >> 16);

    if (dcm_is_public_tag(tag)) {
        printf("%s ", dcm_dict_keyword_from_tag(tag));
    }

    printf("| %s | %u ", dcm_dict_str_from_vr(vr), length);

    printf("[\n");

    filehandle->indent += 1;
    filehandle->index = 0;

    return true;
}


static bool print_pixeldata_end(DcmError **error, void *client)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);

    filehandle->indent -= 1;

    printf("%*.*s]\n",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ");

    return true;
}


static bool print_element_create(DcmError **error,
                                 void *client,
                                 uint32_t tag,
                                 DcmVR vr,
                                 char *value,
                                 uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);

    printf("%*.*s(%04x,%04x) ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           (tag & 0xffff0000) >> 16,
           tag >> 16);

    if (dcm_is_public_tag(tag)) {
        printf("%s ", dcm_dict_keyword_from_tag(tag));
    }

    printf("| %s | %u ", dcm_dict_str_from_vr(vr), length);

    // make an element so we can make a printable string (if possible)
    DcmElement *element = dcm_element_create(NULL, tag, vr);
    if (element != NULL) {
        char *str;
        if (dcm_element_set_value(NULL, element, value, length, false) &&
            (str = dcm_element_value_to_string(element))) {
            printf("| %s\n", str);
            free(str);
        }

        dcm_element_destroy(element);
    }

    return true;
}


static bool print_pixeldata_create(DcmError **,
                                   void *client,
                                   uint32_t tag,
                                   DcmVR vr,
                                   char *value,
                                   uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;
    size_t size = dcm_dict_vr_size(vr);

    USED(tag);

    printf("%*.*sframe %d ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           filehandle->index);

    printf("| %u | ", length);

    switch (size) {
        default:
        case 1:
            for (uint32_t i = 0; i < MIN(length, 17); i++) {
                printf("%02x ", value[i]);
            }

            if (length > 17) {
                printf("...");
            }

            break;

        case 2:
            for (uint32_t i = 0; i < MIN(length, 10); i++) {
                printf("%04x ", ((uint16_t *)value)[i]);
            }

            if (length > 10) {
                printf("...");
            }

            break;

        case 4:
            for (uint32_t i = 0; i < MIN(length, 6); i++) {
                printf("%08x ", ((uint32_t *)value)[i]);
            }

            if (length > 6) {
                printf("...");
            }

            break;
    }

    printf("\n");

    filehandle->index += 1;

    return true;
}

bool dcm_filehandle_print(DcmError **error,
                          DcmFilehandle *filehandle)
{
    static DcmParse parse = {
        .dataset_begin = print_dataset_begin,
        .sequence_begin = print_sequence_begin,
        .sequence_end = print_sequence_end,
        .pixeldata_begin = print_pixeldata_begin,
        .pixeldata_end = print_pixeldata_end,
        .element_create = print_element_create,
        .pixeldata_create = print_pixeldata_create,
        .stop = NULL,
    };

    // skip File Preamble
    int64_t position = 0;
    filehandle->indent = 0;
    filehandle->index = 0;
    if (!parse_preamble(error, filehandle, &position)) {
        return false;
    }

    // print the first group
    printf("===File Meta Information===\n");
    dcm_log_info("Read File Meta Information");
    if (!dcm_parse_group(error,
                         filehandle->io,
                         false,
                         &parse,
                         filehandle)) {
        return false;
    }

    // print the rest of the file
    printf("===Dataset===\n");
    dcm_log_info("Read metadata");
    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           // FIXME we should check transfer syntax for this
                           false,
                           &parse,
                           filehandle)) {
        return false;
    }

    return true;
}
