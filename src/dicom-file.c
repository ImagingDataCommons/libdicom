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
    const uint32_t *stop_tags;

    // start of image metadata
    int64_t offset;
    // just after read_metadata
    int64_t after_read_metadata;
    // start of pixel metadata
    int64_t pixel_data_offset;
    // distance from pixel metadata to start of first frame
    int64_t first_frame_offset;

    // the file metadata and selected DICOM metadata
    DcmDataSet *file_meta;
    DcmDataSet *meta;

    // image properties we need to track
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t num_frames;
    uint32_t frame_offset;
    struct PixelDescription desc;
    DcmLayout layout;

    // zero-indexed and length num_frames
    int64_t *offset_table;

    // frames form a grid of tiles, there can be more tiles than frames in
    // sparse mode
    uint32_t tiles_across;
    uint32_t tiles_down;
    uint32_t num_tiles;

    // zero-indexed and of length num_tiles
    uint32_t *frame_index;

    // the last top level tag the scanner saw
    uint32_t last_tag;

    // used to count frames as we scan per frame functional group sequence
    uint32_t frame_number;
    int32_t column_position;
    int32_t row_position;

    // indent for file print
    int indent;

    // dataset index for file print
    UT_array *index_stack;

    // push and pop these while we parse
    UT_array *dataset_stack;
    UT_array *sequence_stack;

    // skip to tags during parse
    uint32_t *skip_to_tags;

    // set if we see an ext offset table
    bool have_extended_offset_table;
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
    utarray_new(filehandle->index_stack, &ut_int_icd);
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

    utarray_clear(filehandle->index_stack);

    // we always need at least 1 item on the index stack
    int index = 0;
    utarray_push_back(filehandle->index_stack, &index);

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

        utarray_free(filehandle->index_stack);
        utarray_free(filehandle->dataset_stack);
        utarray_free(filehandle->sequence_stack);

        if (filehandle->meta) {
            dcm_dataset_destroy(filehandle->meta);
        }

        if (filehandle->file_meta) {
            dcm_dataset_destroy(filehandle->file_meta);
        }

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
                "end of filehandle",
                "needed %zd bytes beyond end of filehandle", length);
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
                      "BasicOffsetTable read failed",
                      "value of NumberOfFrames is malformed");
        return false;
    }

    *number_of_frames = num_frames;

    return true;
}


static bool get_frame_offset(DcmError **error,
                             const DcmDataSet *metadata,
                             uint32_t *frame_offset)
{
    (void) error;

    // optional, defaults to 0
    int64_t value;
    if (!get_tag_int(NULL,
        metadata, "ConcatenationFrameOffsetNumber", &value)) {
        value = 0;
    }

    // it's a uint32 in the DICOM file
    *frame_offset = (uint32_t) value;

    return true;
}


static bool get_frame_size(DcmError **error,
                           const DcmDataSet *metadata,
                           uint32_t *frame_width,
                           uint32_t *frame_height)
{
    int64_t width;
    int64_t height;

    if (!get_tag_int(error, metadata, "Columns", &width) ||
        !get_tag_int(error, metadata, "Rows", &height)) {
        return false;
    }
    if (width <= 0 || height <= 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "frame read failed",
                      "value of Columns or Rows is out of range");
        return false;
    }

    *frame_width = (uint32_t) width;
    *frame_height = (uint32_t) height;

    return true;
}


static bool get_tiles(DcmError **error,
                      const DcmDataSet *metadata,
                      uint32_t *tiles_across,
                      uint32_t *tiles_down)
{
    int64_t width;
    int64_t height;
    uint32_t frame_width;
    uint32_t frame_height;

    if (!get_frame_size(error, metadata, &frame_width, &frame_height)) {
        return false;
    }

    // TotalPixelMatrixColumns is optional and defaults to Columns, ie. one
    // frame across
    width = frame_width;
    (void) get_tag_int(NULL, metadata, "TotalPixelMatrixColumns", &width);

    // TotalPixelMatrixColumns is optional and defaults to Columns, ie. one
    // frame across
    height = frame_width;
    (void) get_tag_int(NULL, metadata, "TotalPixelMatrixRows", &height);

    if (width <= 0 || height <= 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "frame read failed",
                      "value of TotalPixelMatrixColumns or TotalPixelMatrixRows is out of range");
        return false;
    }

    *tiles_across = (uint32_t) width / frame_width + !!(width % frame_width);
    *tiles_down = (uint32_t) height / frame_height + !!(height % frame_height);

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
                      "reading of file meta information failed",
                      "prefix DICM not found");
        return false;
    }

    return true;
}


static DcmDataSet *dcm_filehandle_read_file_meta(DcmError **error,
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
        return NULL;
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

    DcmDataSet *file_meta = dcm_sequence_get(error, sequence, 0);
    if (file_meta == NULL) {
        return NULL;
    }

    // steal file_meta to stop it being destroyed
    (void) dcm_sequence_steal(NULL, sequence, 0);
    dcm_filehandle_clear(filehandle);

    return file_meta;
}


static bool dcm_filehandle_set_transfer_syntax(DcmError **error,
                                               DcmFilehandle *filehandle,
                                               const char *transfer_syntax_uid)
{
    if (filehandle->transfer_syntax_uid) {
        free(filehandle->transfer_syntax_uid);
    }

    filehandle->transfer_syntax_uid = dcm_strdup(error, transfer_syntax_uid);
    if (filehandle->transfer_syntax_uid == NULL) {
        return false;
    }

    if (strcmp(filehandle->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
        filehandle->implicit = true;
    }

    filehandle->desc.transfer_syntax_uid = filehandle->transfer_syntax_uid;

    return true;
}


const DcmDataSet *dcm_filehandle_get_file_meta(DcmError **error,
                                               DcmFilehandle *filehandle)
{
    if (filehandle->file_meta == NULL) {
        DcmDataSet *file_meta =
            dcm_filehandle_read_file_meta(error, filehandle);
        if (file_meta == NULL) {
            return NULL;
        }

        // record the start point for the image metadata
        if (!dcm_offset(error, filehandle, &filehandle->offset)) {
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        DcmElement *element = dcm_dataset_get(error, file_meta, 0x00020010);
        if (element == NULL) {
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        const char *transfer_syntax_uid;
        if (!dcm_element_get_value_string(error,
                                          element,
                                          0,
                                          &transfer_syntax_uid)) {
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        if (!dcm_filehandle_set_transfer_syntax(error,
                                                filehandle,
                                                transfer_syntax_uid)) {
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        filehandle->file_meta = file_meta;
    } else {
        // move the read point to the start of the slide metadata
        if (!dcm_seekset(error, filehandle, filehandle->offset)) {
            return NULL;
        }
    }

    return filehandle->file_meta;
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

    for (int i = 0; filehandle->stop_tags[i]; i++) {
        if (tag == filehandle->stop_tags[i]) {
            return true;
        }
    }

    return false;
}


static bool set_pixel_description(DcmError **error,
                                  const DcmDataSet *metadata,
                                  struct PixelDescription *desc)
{
    DcmElement *element;
    int64_t value;
    const char *string;

    element = dcm_dataset_get(error, metadata, 0x00280010);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->rows = (uint16_t) value;

    element = dcm_dataset_get(error, metadata, 0x00280011);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->columns = (uint16_t) value;

    element = dcm_dataset_get(error, metadata, 0x00280002);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->samples_per_pixel = (uint16_t) value;

    element = dcm_dataset_get(error, metadata, 0x00280100);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->bits_allocated = (uint16_t) value;

    element = dcm_dataset_get(error, metadata, 0x00280101);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->bits_stored = (uint16_t) value;

    element = dcm_dataset_get(error, metadata, 0x00280103);
    if (element == NULL ||
        !dcm_element_get_value_integer(error, element, 0, &value)) {
        return false;
    }
    desc->pixel_representation = (uint16_t) value;

    // required if samples per pixel > 1, defaults to 0 (interleaved)
    desc->planar_configuration = 0;
    if (desc->samples_per_pixel > 1) {
        element = dcm_dataset_get(error, metadata, 0x00280006);
        if (element == NULL ||
            !dcm_element_get_value_integer(error, element, 0, &value)) {
            return false;
        }
        desc->planar_configuration = (uint16_t) value;
    }

    element = dcm_dataset_get(error, metadata, 0x00280004);
    if (element == NULL ||
        !dcm_element_get_value_string(error, element, 0, &string)) {
        return false;
    }
    desc->photometric_interpretation = string;

    return true;
}


DcmDataSet *dcm_filehandle_read_metadata(DcmError **error,
                                         DcmFilehandle *filehandle,
                                         const uint32_t *stop_tags)
{
    // by default, we don't stop anywhere (except pixeldata)
    static const uint32_t default_stop_tags[] = {
        TAG_PIXEL_DATA,
        TAG_FLOAT_PIXEL_DATA,
        TAG_DOUBLE_PIXEL_DATA,
        0,
    };

    static DcmParse parse = {
        // we don't need to define the pixeldata callbacks since we have no
        // concrete representation for them
        .dataset_begin = parse_meta_dataset_begin,
        .dataset_end = parse_meta_dataset_end,
        .sequence_begin = parse_meta_sequence_begin,
        .sequence_end = parse_meta_sequence_end,
        .element_create = parse_meta_element_create,
        .stop = parse_meta_stop,
    };

    // only get the file_meta if it's not there ... we don't want to rewind
    // filehandle every time
    if (filehandle->file_meta == NULL) {
        const DcmDataSet *file_meta = dcm_filehandle_get_file_meta(error,
                                                                   filehandle);
        if (file_meta == NULL) {
            return NULL;
        }
    }

    dcm_filehandle_clear(filehandle);
    filehandle->stop_tags = stop_tags == NULL ? default_stop_tags : stop_tags;
    DcmSequence *sequence = dcm_sequence_create(error);
    if (sequence == NULL) {
        return NULL;
    }
    utarray_push_back(filehandle->sequence_stack, &sequence);

    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           &parse,
                           filehandle)) {
        return NULL;
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
    if (meta == NULL) {
        return NULL;
    }

    // steal meta to stop it being destroyed
    (void) dcm_sequence_steal(NULL, sequence, 0);
    dcm_filehandle_clear(filehandle);

    return meta;
}


const DcmDataSet *dcm_filehandle_get_metadata_subset(DcmError **error,
                                                     DcmFilehandle *filehandle)
{
    // we stop on any of the tags that start a huge group that
    // would take a long time to parse
    static uint32_t stop_tags[] = {
        TAG_REFERENCED_IMAGE_NAVIGATION_SEQUENCE,
        TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE,
        TAG_EXTENDED_OFFSET_TABLE,
        TAG_PIXEL_DATA,
        TAG_FLOAT_PIXEL_DATA,
        TAG_DOUBLE_PIXEL_DATA,
        0,
    };

    if (filehandle->meta == NULL) {
        // always rewind the filehandle
        const DcmDataSet *file_meta = dcm_filehandle_get_file_meta(error,
                                                                   filehandle);
        if (file_meta == NULL) {
            return NULL;
        }

        DcmDataSet *meta = dcm_filehandle_read_metadata(error,
                                                        filehandle,
                                                        stop_tags);
        if (meta == NULL) {
            return NULL;
        }

        // record the position of the tag that stopped the read
        if (!dcm_offset(error, filehandle, &filehandle->after_read_metadata)) {
            dcm_dataset_destroy(meta);
            return NULL;
        }

        // useful values for later
        if (!get_frame_size(error,
                           meta,
                           &filehandle->frame_width,
                           &filehandle->frame_height) ||
            !get_num_frames(error, meta, &filehandle->num_frames) ||
            !get_frame_offset(error, meta, &filehandle->frame_offset) ||
            !get_tiles(error, meta,
                &filehandle->tiles_across, &filehandle->tiles_down) ||
            !set_pixel_description(error, meta, &filehandle->desc)) {
            dcm_dataset_destroy(meta);
            return NULL;
        }
        filehandle->num_tiles = filehandle->tiles_across *
            filehandle->tiles_down;

        // we support sparse and full frame layout, defaulting to full if
        // no type is specified
        //
        // we flip to SPARSE if there's a per frame functional group sequence
        // containing frame positions, see below
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

        filehandle->meta = meta;
    } else {
        // move the read point to the tag we stopped read on
        if (!dcm_seekset(error, filehandle, filehandle->after_read_metadata)) {
            return NULL;
        }
    }

    return filehandle->meta;
}


static bool parse_frame_index_sequence_begin(DcmError **error,
                                             void *client,
                                             uint32_t tag,
                                             DcmVR vr,
                                             uint32_t length)
{
    USED(error);
    USED(vr);
    USED(length);

    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    if (tag == TAG_PLANE_POSITION_SLIDE_SEQUENCE) {
        filehandle->column_position = -1;
        filehandle->row_position = -1;
    }

    return true;
}


static bool parse_frame_index_sequence_end(DcmError **error,
                                           void *client,
                                           uint32_t tag,
                                           DcmVR vr,
                                           uint32_t length)
{
    USED(vr);
    USED(length);

    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    // have we seen a valid pair of tile positions
    if (tag == TAG_PLANE_POSITION_SLIDE_SEQUENCE &&
        filehandle->column_position != -1 &&
        filehandle->row_position != -1) {
        // we don't support fractional tile positioning ... they must be
        // exactly aligned on tile boundaries
        if ((filehandle->column_position - 1) % filehandle->frame_width != 0 ||
            (filehandle->row_position - 1) % filehandle->frame_height != 0) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading PerFrameFunctionalGroupsSequence failed",
                          "unsupported frame alignment");
            return false;
        }

        // map the position of the tile to the frame number
        int col = (filehandle->column_position - 1) / filehandle->frame_width;
        int row = (filehandle->row_position - 1) / filehandle->frame_height;
        uint32_t index = col + row * filehandle->tiles_across;
        if (index < filehandle->num_tiles) {
            filehandle->frame_index[index] = filehandle->frame_number;

            // we have something meaningful in per frame functional group
            // sequence, so we must display in SPARSE mode
            filehandle->layout = DCM_LAYOUT_SPARSE;
        }

        // end of TAG_PLANE_POSITION_SLIDE_SEQUENCE, so we're on to the next
        // frame
        filehandle->frame_number += 1;
    }

    return true;
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

    switch (tag) {
    case TAG_COLUMN_POSITION_IN_TOTAL_IMAGE_PIXEL_MATRIX:
        if (vr == DCM_VR_SL && length == 4) {
            int32_t *sl = (int32_t *) value;

            filehandle->column_position = sl[0];
        }
        break;

    case TAG_ROW_POSITION_IN_TOTAL_IMAGE_PIXEL_MATRIX:
        if (vr == DCM_VR_SL && length == 4) {
            int32_t *sl = (int32_t *) value;

            filehandle->row_position = sl[0];
        }
        break;

    default:
        break;
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
        .sequence_begin = parse_frame_index_sequence_begin,
        .sequence_end = parse_frame_index_sequence_end,
        .element_create = parse_frame_index_element_create,
        .stop = parse_frame_index_stop,
    };

    dcm_log_debug("reading PerFrameFunctionalGroupSequence");

    filehandle->frame_index = DCM_NEW_ARRAY(error,
                                            filehandle->num_tiles,
                                            uint32_t);
    if (filehandle->frame_index == NULL) {
        return false;
    }

    // we may not have all frames ... set to missing initially
    for (uint32_t i = 0; i < filehandle->num_tiles; i++) {
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


static bool parse_skip_to(void *client,
                          uint32_t tag,
                          DcmVR vr,
                          uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(vr);
    USED(length);

    filehandle->last_tag = tag;

    for (int i = 0; filehandle->skip_to_tags[i]; i++)
        if (tag == filehandle->skip_to_tags[i])
            return true;

    return false;

}


static bool read_skip_to(DcmError **error,
                         DcmFilehandle *filehandle,
                         uint32_t *skip_to_tags)

{
    static DcmParse parse = {
        .stop = parse_skip_to,
    };

    filehandle->skip_to_tags = skip_to_tags;
    if (!dcm_parse_dataset(error,
                           filehandle->io,
                           filehandle->implicit,
                           &parse,
                           filehandle)) {
        return false;
    }

    return true;
}


static bool parse_extended_offsets_element_create(DcmError **error,
                                                  void *client,
                                                  uint32_t tag,
                                                  DcmVR vr,
                                                  char *value,
                                                  uint32_t length)
{
    USED(error);
    USED(vr);

    DcmFilehandle *filehandle = (DcmFilehandle *) client;
    int64_t expected_size = filehandle->num_frames * sizeof(int64_t);

    if (tag == TAG_EXTENDED_OFFSET_TABLE && length == expected_size) {
        memcpy(filehandle->offset_table, value, length);
        filehandle->have_extended_offset_table = true;

        // the size of the pixeldata header, plus the size of the empty frame
        // 0 (the BOT)
        filehandle->first_frame_offset = 20;
    }

    return true;
}


static bool parse_extended_offsets_stop(void *client,
                                        uint32_t tag,
                                        DcmVR vr,
                                        uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(vr);
    USED(length);

    filehandle->last_tag = tag;

    return tag != TAG_EXTENDED_OFFSET_TABLE;
}


static bool
read_extended_offsets(DcmError **error,
                      DcmFilehandle *filehandle)
{
    static DcmParse parse = {
        .element_create = parse_extended_offsets_element_create,
        .stop = parse_extended_offsets_stop,
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


bool dcm_filehandle_prepare_read_frame(DcmError **error,
                                       DcmFilehandle *filehandle)
{
    if (filehandle->offset_table == NULL) {
        // move to the first of our stop tags
        if (dcm_filehandle_get_metadata_subset(error, filehandle) == NULL) {
            return false;
        }

        filehandle->offset_table = DCM_NEW_ARRAY(error,
                                                 filehandle->num_frames,
                                                 int64_t);
        if (filehandle->offset_table == NULL) {
            return false;
        }

        if (filehandle->layout == DCM_LAYOUT_UNKNOWN) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading PixelData failed",
                          "unsupported DimensionOrganisationType");
            return false;
        }

        // skip ahead to per frame functional group, if present, and read it
        uint32_t skip_to_per_frame[] = {
            TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE,
            TAG_EXTENDED_OFFSET_TABLE,
            TAG_PIXEL_DATA,
            TAG_FLOAT_PIXEL_DATA,
            TAG_DOUBLE_PIXEL_DATA,
            0
        };
        if (!read_skip_to(error, filehandle, skip_to_per_frame)) {
            return false;
        }
        if (filehandle->last_tag == TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE &&
            !read_frame_index(error, filehandle)) {
            return false;
        }

        // skip ahead to extended offset table, if present
        uint32_t skip_to_offset[] = {
            TAG_EXTENDED_OFFSET_TABLE,
            TAG_PIXEL_DATA,
            TAG_FLOAT_PIXEL_DATA,
            TAG_DOUBLE_PIXEL_DATA,
            0
        };
        if (!read_skip_to(error, filehandle, skip_to_offset)) {
            return false;
        }
        if (filehandle->last_tag == TAG_EXTENDED_OFFSET_TABLE &&
            !read_extended_offsets(error, filehandle)) {
            return false;
        }

        // skip to pixel data
        uint32_t skip_to_pixel_data[] = {
            TAG_PIXEL_DATA,
            TAG_FLOAT_PIXEL_DATA,
            TAG_DOUBLE_PIXEL_DATA,
            0
        };
        if (!read_skip_to(error, filehandle, skip_to_pixel_data)) {
            return false;
        }
        if (filehandle->last_tag != TAG_PIXEL_DATA &&
            filehandle->last_tag != TAG_FLOAT_PIXEL_DATA &&
            filehandle->last_tag != TAG_DOUBLE_PIXEL_DATA) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading PixelData failed",
                          "could not determine offset of PixelData element");
            return false;
        }

        if (!dcm_offset(error, filehandle, &filehandle->pixel_data_offset)) {
            return false;
        }

        // if there was no ext offset table, we must read the basic one, or
        // create it
        if (!filehandle->have_extended_offset_table) {
            const char *syntax =
                dcm_filehandle_get_transfer_syntax_uid(filehandle);
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

                filehandle->first_frame_offset = 12;
            }
        }
    } else {
        // always position at pixel_data
        if (!dcm_seekset(error, filehandle, filehandle->pixel_data_offset)) {
            return false;
        }
    }

    return true;
}


DcmFrame *dcm_filehandle_read_frame(DcmError **error,
                                    DcmFilehandle *filehandle,
                                    uint32_t frame_number)
{
    dcm_log_debug("read frame number #%u", frame_number);

    if (!dcm_filehandle_prepare_read_frame(error, filehandle)) {
        return NULL;
    }

    if (frame_number == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "reading frame item failed",
                      "frame number must be non-zero");
        return NULL;
    }
    if (frame_number > filehandle->num_frames) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "reading frame item failed",
                      "frame number must be less than %u",
                      filehandle->num_frames);
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


bool dcm_filehandle_get_frame_number(DcmError **error,
                                     DcmFilehandle *filehandle,
                                     uint32_t column,
                                     uint32_t row,
                                     uint32_t *frame_number)
{
    dcm_log_debug("Get frame number at (%u, %u)", column, row);

    if (!dcm_filehandle_prepare_read_frame(error, filehandle)) {
        return false;
    }

    if (column >= filehandle->tiles_across ||
        row >= filehandle->tiles_down) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "reading Frame position failed",
                      "column and Row must be less than %u, %u",
                      filehandle->tiles_across,
                      filehandle->tiles_down);
        return false;
    }

    int64_t index = column + row * filehandle->tiles_across;
    if (filehandle->layout == DCM_LAYOUT_SPARSE) {
        index = filehandle->frame_index[index];
        if (index == 0xffffffff) {
            dcm_error_set(error, DCM_ERROR_CODE_MISSING_FRAME,
                          "no frame",
                          "no frame at position (%u, %u)", column, row);
            return false;
        }
    } else {
        // subtract the start of this file, for catenation support
        index -= filehandle->frame_offset;
        if (index < 0 || index >= (int64_t) filehandle->num_frames) {
            dcm_error_set(error, DCM_ERROR_CODE_MISSING_FRAME,
                          "no frame",
                          "no frame at position (%u, %u)", column, row);
            return false;
        }
    }

    // frame numbers are from 1, and are always uint32
    if (frame_number)
        *frame_number = (uint32_t) (index + 1);

    return true;
}


DcmFrame *dcm_filehandle_read_frame_position(DcmError **error,
                                             DcmFilehandle *filehandle,
                                             uint32_t column,
                                             uint32_t row)
{
    dcm_log_debug("Read frame position (%u, %u)", column, row);

    uint32_t frame_number;
    if (!dcm_filehandle_get_frame_number(error,
            filehandle, column, row, &frame_number)) {
        return NULL;
    }

    return dcm_filehandle_read_frame(error, filehandle, frame_number);
}


static bool print_dataset_begin(DcmError **error,
                                void *client)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;
    int *index = (int *) utarray_back(filehandle->index_stack);

    USED(error);

    *index += 1;

    if (filehandle->indent > 0) {
        printf("%*.*s---Item #%d---\n",
                   filehandle->indent * 2,
                   filehandle->indent * 2,
                   "                                   ",
                   *index);
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
    int index = 0;
    utarray_push_back(filehandle->index_stack, &index);

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
    utarray_pop_back(filehandle->index_stack);

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
    int index = 0;
    utarray_push_back(filehandle->index_stack, &index);

    return true;
}


static bool print_pixeldata_end(DcmError **error, void *client)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;

    USED(error);

    filehandle->indent -= 1;
    utarray_pop_back(filehandle->index_stack);

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

    // we must record the transfer syntax in the file meta since we need it
    // to determine implicit/explicit encoding
    if (tag == TAG_TRANSFER_SYNTAX_UID &&
        vr == DCM_VR_UI &&
        value &&
        length > 10 &&
        !dcm_filehandle_set_transfer_syntax(error, filehandle, value)) {
        return false;
    }

    printf("%*.*s(%04x,%04x) ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           (tag & 0xffff0000) >> 16,
           tag & 0xffff);

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
            printf("| %u | %s\n", dcm_element_get_vm(element), str);
            free(str);
        }

        dcm_element_destroy(element);
    }

    return true;
}


static bool print_pixeldata_create(DcmError **error,
                                   void *client,
                                   uint32_t tag,
                                   DcmVR vr,
                                   char *value,
                                   uint32_t length)
{
    DcmFilehandle *filehandle = (DcmFilehandle *) client;
    size_t size = dcm_dict_vr_size(vr);
    int *index = (int *) utarray_back(filehandle->index_stack);

    USED(error);
    USED(tag);

    printf("%*.*sframe %d ",
           filehandle->indent * 2,
           filehandle->indent * 2,
           "                                   ",
           *index);

    printf("| %u | ", length);

    uint32_t n = MIN(16, length);
    for (uint32_t i = 0; i < n; i++) {
        printf("%02x", value[i] & 0xff);

        if (size > 0 &&
            i % size == size - 1) {
            printf(" ");
        }
    }

    if (length > 16) {
        printf("...");
    }

    printf("\n");

    *index += 1;

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
    dcm_filehandle_clear(filehandle);
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
                           filehandle->implicit,
                           &parse,
                           filehandle)) {
        return false;
    }

    return true;
}
