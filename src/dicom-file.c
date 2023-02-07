/*
 * Implementation of Part 10 of the DICOM standard: Media Storage and File
 * Format for Media Interchange.
 */

#ifdef _WIN32
// the Windows CRT considers strncpy unsafe
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "utarray.h"

#include "dicom.h"


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
    char *photometric_interpretation;
};


typedef struct ItemHeader {
    uint32_t tag;
    uint64_t length;
} IHeader;


typedef struct ElementHeader {
    uint32_t tag;
    char vr[3];
    uint64_t length;
} EHeader;


static IHeader *iheader_create(DcmError **error, 
    uint32_t tag, uint64_t length)
{
    if (tag != TAG_ITEM &&
        tag != TAG_ITEM_DELIM &&
        tag != TAG_SQ_DELIM) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing header of Item failed",
                      "Invalid Item Tag '%08X'",
                      tag);
        return NULL;
    }
    IHeader *header = DCM_NEW(error, struct ItemHeader);
    if (header == NULL) {
        return NULL;
    }
    header->tag = tag;
    header->length = length;
    return header;
}


static uint32_t iheader_get_tag(IHeader *item)
{
    assert(item);
    return item->tag;
}


static uint64_t iheader_get_length(IHeader *item)
{
    assert(item);
    return item->length;
}


static void iheader_destroy(IHeader *item)
{
    if (item) {
        free(item);
        item = NULL;
    }
}


static EHeader *eheader_create(DcmError **error, 
    uint32_t tag, const char *vr, uint64_t length)
{
    if (!dcm_is_valid_tag(tag)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing header of Data Element failed",
                      "Invalid Tag: '%08X'",
                      tag);
        return NULL;
    }
    if (!dcm_is_valid_vr(vr)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing header of Data Element failed",
                      "Invalid Value Representation: '%s'",
                      vr);
        return NULL;
    }

    EHeader *header = DCM_NEW(error, struct ElementHeader);
    if (header == NULL) {
        return NULL;
    }
    header->tag = tag;

    strncpy(header->vr, vr, 3);
    header->vr[2] = '\0';

    header->length = length;
    return header;
}


static uint16_t eheader_get_group_number(EHeader *header)
{
    assert(header);
    return (uint16_t)(header->tag >> 16);
}


static uint32_t eheader_get_tag(EHeader *header)
{
    assert(header);
    return header->tag;
}


static bool eheader_check_vr(EHeader *header, const char *vr)
{
    assert(header);
    return (strcmp(header->vr, vr) == 0);
}


static uint64_t eheader_get_length(EHeader *header)
{
    assert(header);
    return header->length;
}


static void eheader_destroy(EHeader *header)
{
    if (header) {
        free(header);
        header = NULL;
    }
}


struct _DcmFile {
    FILE *fp;
    DcmDataSet *meta;
    size_t offset;
    char *transfer_syntax_uid;
    size_t pixel_data_offset;
    uint64_t *extended_offset_table;
};


static uint32_t read_tag(FILE *fp, size_t *n)
{
    uint16_t group_num, elem_num;
    *n += fread(&group_num, 1, sizeof(group_num), fp);
    *n += fread(&elem_num, 1, sizeof(elem_num), fp);
    return ((uint32_t)group_num << 16) + elem_num;
}


static char **parse_character_string(DcmError **error, 
                                     char *string, uint32_t *vm)
{
    uint32_t i;
    uint32_t n;
    char **token_ptr;
    char **parts;
    char *token;

    UT_array *array;
    utarray_new(array, &ut_str_icd);

    if (strlen(string) == 0) {
        token = "";
        utarray_push_back(array, &token);
        goto finish;
    }
    token = strtok(string, "\\");
    while(token) {
        utarray_push_back(array, &token);
        token = strtok(NULL, "\\");
    }
    goto finish;

finish:
    n = utarray_len(array);
    parts = DCM_NEW_ARRAY(error, n, char *);
    if (parts == NULL) {
        free(string);
        utarray_free(array);
        return NULL;
    }

    for (i = 0; i < n; i++) {
        token_ptr = utarray_eltptr(array, i);
        parts[i] = DCM_MALLOC(error, strlen(*token_ptr) + 1);
        if (parts[i] == NULL) {
            free(parts);
            free(string);
            utarray_free(array);
            return NULL;
        }
        strcpy(parts[i], *token_ptr);
    }

    *vm = n;
    utarray_free(array);
    free(string);
    return parts;
}


static IHeader *read_item_header(DcmError **error, FILE *fp, size_t *n)
{
    uint32_t tag = read_tag(fp, n);
    uint32_t length;
    *n += fread(&length, 1, sizeof(length), fp);
    return iheader_create(error, tag, length);
}


static EHeader *read_element_header(DcmError **error, 
    FILE *fp, size_t *n, bool implicit)
{
    char vr[3];
    uint32_t length;
    uint16_t short_length;
    uint16_t reserved;

    uint32_t tag = read_tag(fp, n);
    if (implicit) {
        // Value Representation
        const char *tmp = dcm_dict_lookup_vr(tag);
        strncpy(vr, tmp, 2);
        vr[2] = '\0';

        // Value Length
        *n += fread(&length, 1, sizeof(length), fp);
    } else {
        // Value Representation
        *n += fread(&vr, 1, 2, fp);
        vr[2] = '\0';

        // Value Length
        if (strcmp(vr, "AE") == 0 ||
            strcmp(vr, "AS") == 0 ||
            strcmp(vr, "AT") == 0 ||
            strcmp(vr, "CS") == 0 ||
            strcmp(vr, "DA") == 0 ||
            strcmp(vr, "DS") == 0 ||
            strcmp(vr, "DT") == 0 ||
            strcmp(vr, "FL") == 0 ||
            strcmp(vr, "FD") == 0 ||
            strcmp(vr, "IS") == 0 ||
            strcmp(vr, "LO") == 0 ||
            strcmp(vr, "LT") == 0 ||
            strcmp(vr, "PN") == 0 ||
            strcmp(vr, "SH") == 0 ||
            strcmp(vr, "SL") == 0 ||
            strcmp(vr, "SS") == 0 ||
            strcmp(vr, "ST") == 0 ||
            strcmp(vr, "TM") == 0 ||
            strcmp(vr, "UI") == 0 ||
            strcmp(vr, "UL") == 0 ||
            strcmp(vr, "US") == 0) {
            // These VRs have a short length of only two bytes
            *n += fread(&short_length, 1, sizeof(short_length), fp);
            length = (uint32_t) short_length;
        } else {
            // Other VRs have two reserved bytes before length of four bytes
            *n += fread(&reserved, 1, sizeof(reserved), fp);
            if (reserved != 0x0000) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element header failed",
                              "Unexpected value for reserved bytes "
                              "of Data Element %08X with VR '%s'.",
                              tag, vr);
                return NULL;
            }
            *n += fread(&length, 1, sizeof(length), fp);
        }
    }

    EHeader *header = eheader_create(error, tag, vr, length);
    return header;
}


static DcmElement *read_element(DcmError **error,
                                FILE *fp,
                                EHeader *header,
                                size_t *n,
                                bool implicit)
{
    assert(header);
    uint32_t i;
    uint32_t vm;
    uint32_t item_index;
    uint32_t item_tag;
    uint32_t item_length;
    size_t n_seq;
    size_t *n_seq_ptr = &n_seq;
    size_t n_item;
    size_t *n_item_ptr = &n_item;
    DcmElement *element;
    IHeader *item_iheader;
    DcmDataSet *item_dataset;
    EHeader *item_eheader;
    DcmElement *item_element;

    uint32_t tag = eheader_get_tag(header);
    uint32_t length = eheader_get_length(header);

    n_seq = 0;
    n_item = 0;
    item_index = 0;

    dcm_log_debug("Read Data Element '%08X'", tag);

    vm = 1;
    // Character strings
    if (eheader_check_vr(header, "AE") ||
        eheader_check_vr(header, "AS") ||
        eheader_check_vr(header, "AT") ||
        eheader_check_vr(header, "CS") ||
        eheader_check_vr(header, "DA") ||
        eheader_check_vr(header, "DS") ||  // Decimal String
        eheader_check_vr(header, "DT") ||
        eheader_check_vr(header, "IS") ||  // Integer String
        eheader_check_vr(header, "LO") ||
        eheader_check_vr(header, "LT") ||
        eheader_check_vr(header, "PN") ||
        eheader_check_vr(header, "SH") ||
        eheader_check_vr(header, "ST") ||
        eheader_check_vr(header, "TM") ||
        eheader_check_vr(header, "UI") ||
        eheader_check_vr(header, "UR") ||
        eheader_check_vr(header, "UT")) {
        char *value = DCM_MALLOC(error, length + 1);
        if (value == NULL) {
            return NULL;
        }
        *n += fread(value, 1, length, fp);
        if (length > 0) {
            if (!eheader_check_vr(header, "UI")) {
                if (isspace(value[length - 1])) {
                    value[length - 1] = '\0';
                }
            }
        }
        value[length] = '\0';

        // Parse value and create array of strings
        char **strings = parse_character_string(error, value, &vm);

        if (eheader_check_vr(header, "AE")) {
            return dcm_element_create_AE_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "AS")) {
            return dcm_element_create_AS_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "AT")) {
            return dcm_element_create_AT_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "CS")) {
            return dcm_element_create_CS_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "DA")) {
            return dcm_element_create_DA_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "DS")) {
            return dcm_element_create_DS_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "DT")) {
            return dcm_element_create_DT_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "IS")) {
            return dcm_element_create_IS_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "LO")) {
            return dcm_element_create_LO_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "PN")) {
            return dcm_element_create_PN_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "SH")) {
            return dcm_element_create_SH_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "ST")) {
            // This VM shall always have VM 1.
            if (vm > 1) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Encountered unexpected Value Multiplicity %d "
                              "for Data Element '%08X'",
                              vm, tag);
                for (i = 0; i < vm; i++) {
                    free(strings[i]);
                }
                free(strings);
                return NULL;
            }
            char *str = strings[0];
            free(strings);
            return dcm_element_create_ST(error, tag, str);
        } else if (eheader_check_vr(header, "TM")) {
            return dcm_element_create_TM_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "UI")) {
            return dcm_element_create_UI_multi(error, tag, strings, vm);
        } else if (eheader_check_vr(header, "LT")) {
            // This VM shall always have VM 1.
            if (vm > 1) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Encountered unexpected Value Multiplicity %d "
                              "for Data Element '%08X'.",
                              vm, tag);
                for (i = 0; i < vm; i++) {
                    free(strings[i]);
                }
                free(strings);
                return NULL;
            }
            char *str = strings[0];
            free(strings);
            return dcm_element_create_LT(error, tag, str);
        } else if (eheader_check_vr(header, "UR")) {
            // This VM shall always have VM 1.
            if (vm > 1) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Encountered unexpected Value Multiplicity %d "
                              "for Data Element '%08X'.",
                              vm, tag);
                for (i = 0; i < vm; i++) {
                    free(strings[i]);
                }
                free(strings);
                return NULL;
            }
            char *str = strings[0];
            free(strings);
            return dcm_element_create_UR(error, tag, str);
        } else if (eheader_check_vr(header, "UT")) {
            // This VM shall always have VM 1.
            if (vm > 1) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Encountered unexpected Value Multiplicity %d "
                              "for Data Element '%08X'",
                              vm, tag);
                for (i = 0; i < vm; i++) {
                    free(strings[i]);
                }
                free(strings);
                return NULL;
            }
            char *str = strings[0];
            free(strings);
            return dcm_element_create_UT(error, tag, str);
        } else {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Encountered unexpected Value Representation "
                          "for Data Element '%08X'",
                          tag);
            for (i = 0; i < vm; i++) {
                free(strings[i]);
            }
            free(strings);
            return NULL;
        }
    } else if (eheader_check_vr(header, "SQ")) {
        vm = 1;
        DcmSequence *value = dcm_sequence_create(error);
        if (value == NULL) {
            return NULL;
        }
        if (length == 0) {
            return dcm_element_create_SQ(error, tag, value);
        } else if (length == 0xFFFFFFFF) {
            dcm_log_debug("Sequence of Data Element '%08X' "
                          "has undefined length.",
                          tag);
        } else {
            dcm_log_debug("Sequence of Data Element '%08X' "
                          "has defined length %d.",
                          tag, length);
        }

        n_seq = 0;
        while (n_seq < length) {
            dcm_log_debug("Read Item #%d of Data Element '%08X'.",
                          item_index, tag);
            item_iheader = read_item_header(error, fp, n_seq_ptr);
            if (item_iheader == NULL) {
                iheader_destroy(item_iheader);
                dcm_sequence_destroy(value);
                return NULL;
            }
            item_tag = iheader_get_tag(item_iheader);
            item_length = iheader_get_length(item_iheader);
            if (item_tag == TAG_SQ_DELIM) {
                dcm_log_debug("Stop reading Data Element '%08X'. "
                              "Encountered Sequence Delimination Tag.",
                              tag);
                iheader_destroy(item_iheader);
                break;
            }
            if (item_tag != TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Reading of Data Element failed",
                              "Expected tag '%08X' instead of '%08X' "
                              "for Item #%d of Data Element '%08X'",
                              TAG_ITEM,
                              item_tag,
                              item_index,
                              tag);
                iheader_destroy(item_iheader);
                dcm_sequence_destroy(value);
                return NULL;
            } else if (item_length == 0xFFFFFFFF) {
                dcm_log_debug("Item #%d of Data Element '%08X' "
                              "has undefined length.",
                              item_index, tag);
            } else {
                dcm_log_debug("Item #%d of Data Element '%08X' "
                              "has defined length %d.",
                              item_index, tag, item_length);
            }

            item_dataset = dcm_dataset_create(error);
            if (item_dataset == NULL) {
                iheader_destroy(item_iheader);
                dcm_sequence_destroy(value);
                return NULL;
            }

            n_item = 0;
            while (n_item < item_length) {
                if (read_tag(fp, n_item_ptr) == TAG_ITEM_DELIM) {
                    // Item with undefined length
                    dcm_log_debug("Stop reading Item #%d of "
                                  "Data Element '%08X'. "
                                  "Encountered Item Delimination Tag.",
                                  item_index, tag);
                    fseek(fp, 4, SEEK_CUR);
                    n_item += 4;
                    break;
                } else {
                    fseek(fp, -4, SEEK_CUR);
                    n_item -= 4;
                }

                item_eheader = read_element_header(error, 
                                                   fp, n_item_ptr, implicit);
                if (item_eheader == NULL) {
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }

                item_element = read_element(error,
                                            fp,
                                            item_eheader,
                                            n_item_ptr,
                                            implicit);
                if (item_element == NULL) {
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }
                if (!dcm_dataset_insert(error, item_dataset, item_element)) {
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }
                eheader_destroy(item_eheader);
            }
            n_seq += n_item;
            dcm_sequence_append(error, value, item_dataset);
            iheader_destroy(item_iheader);
            item_index += 1;
        }
        *n += n_seq;
        return dcm_element_create_SQ(error, tag, value);
    } else if (eheader_check_vr(header, "FD")) {
        vm = length / sizeof(double);
        double *values = DCM_NEW_ARRAY(error, vm, double);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            double val;
            *n += fread(&val, 1, sizeof(double), fp);
            values[i] = val;
        }
        return dcm_element_create_FD_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "FL")) {
        vm = length / sizeof(float);
        float *values = DCM_NEW_ARRAY(error, vm, float);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            float val;
            *n += fread(&val, 1, sizeof(float), fp);
            values[i] = val;
        }
        return dcm_element_create_FL_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "SS")) {
        vm = length / sizeof(int16_t);
        int16_t *values = DCM_NEW_ARRAY(error, vm, int16_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            int16_t val;
            *n += fread(&val, 1, sizeof(int16_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SS_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "SL")) {
        vm = length / sizeof(int32_t);
        int32_t *values = DCM_NEW_ARRAY(error, vm, int32_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            int32_t val;
            *n += fread(&val, 1, sizeof(int32_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SL_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "SV")) {
        vm = length / sizeof(int64_t);
        int64_t *values = DCM_NEW_ARRAY(error, vm, int64_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            int64_t val;
            *n += fread(&val, 1, sizeof(int64_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SV_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "UL")) {
        vm = length / sizeof(uint32_t);
        uint32_t *values = DCM_NEW_ARRAY(error, vm, uint32_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            uint32_t val;
            *n += fread(&val, 1, sizeof(uint32_t), fp);
            values[i] = val;
        }
        return dcm_element_create_UL_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "US")) {
        vm = length / sizeof(uint16_t);
        uint16_t *values = DCM_NEW_ARRAY(error, vm, uint16_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            uint16_t val;
            *n += fread(&val, 1, sizeof(uint16_t), fp);
            values[i] = val;
        }
        return dcm_element_create_US_multi(error, tag, values, vm);
    } else if (eheader_check_vr(header, "UV")) {
        vm = length / sizeof(uint64_t);
        uint64_t *values = DCM_NEW_ARRAY(error, vm, uint64_t);
        if (values == NULL) {
            return NULL;
        }
        for (i = 0; i < vm; i++) {
            uint64_t val;
            *n += fread(&val, 1, sizeof(uint64_t), fp);
            values[i] = val;
        }
        return dcm_element_create_UV_multi(error, tag, values, vm);
    } else {
        vm = 1;
        char *value = DCM_MALLOC(error, length);
        if (value == NULL) {
            return NULL;
        }
        *n += fread(value, 1, length, fp);

        if (eheader_check_vr(header, "OB")) {
            return dcm_element_create_OB(error, tag, value, length);
        } else if (eheader_check_vr(header, "OD")) {
            return dcm_element_create_OD(error, tag, value, length);
        } else if (eheader_check_vr(header, "OF")) {
            return dcm_element_create_OF(error, tag, value, length);
        } else if (eheader_check_vr(header, "OL")) {
            return dcm_element_create_OL(error, tag, value, length);
        } else if (eheader_check_vr(header, "OV")) {
            return dcm_element_create_OV(error, tag, value, length);
        } else if (eheader_check_vr(header, "OW")) {
            return dcm_element_create_OW(error, tag, value, length);
        } else if (eheader_check_vr(header, "UC")) {
            return dcm_element_create_UC(error, tag, value, length);
        } else if (eheader_check_vr(header, "UN")) {
            return dcm_element_create_UN(error, tag, value, length);
        } else {
            tag = eheader_get_tag(header);
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Element failed",
                          "Data Element '%08X' has unexpected "
                          "Value Representation", tag);
            return NULL;
        }
    }
    return element;
}


DcmFile *dcm_file_create(DcmError **error, 
                         const char *file_path, const char mode)
{
    if (mode != 'r' && mode != 'w') {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Open of file failed",
                      "Wrong file mode specified");
        return NULL;
    }

    DcmFile *file = DCM_NEW(error, DcmFile);
    if (file == NULL) {
        return NULL;
    }

    char file_mode[3];
    file_mode[0] = mode;
    file_mode[1] = 'b';
    file_mode[2] = '\0';
    file->fp = fopen(file_path, file_mode);
    if (file->fp == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
                      "Open of file failed",
                      "Could not open file for reading: %s", file_path);
        free(file);
        return NULL;
    }

    file->offset = 0;
    file->pixel_data_offset = 0;
    file->transfer_syntax_uid = NULL;

    return file;
}


DcmDataSet *dcm_file_read_file_meta(DcmError **error, DcmFile *file)
{
    const bool implicit = false;

    size_t size;
    size_t *n = &size;
    uint32_t tag;
    uint16_t group_number;
    EHeader *header;
    DcmElement *element;

    DcmDataSet *file_meta = dcm_dataset_create(error);
    if (file_meta == NULL) {
        return NULL;
    }

    size = 0;

    // File Preamble
    char preamble[129];
    size += fread(preamble, 1, sizeof(preamble) - 1, file->fp);
    preamble[128] = '\0';

    // DICOM Prefix
    char prefix[5];
    size += fread(prefix, 1, sizeof(prefix) - 1, file->fp);
    prefix[4] = '\0';
    if (strcmp(prefix, "DICM") != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading of File Meta Information failed",
                      "Prefix 'DICM' not found.");
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    size = 0;

    // File Meta Information Group Length
    header = read_element_header(error, file->fp, n, implicit);
    if (header == NULL) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    element = read_element(error, file->fp, header, n, implicit);
    if (element == NULL) {
        eheader_destroy(header);
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    uint32_t group_length = dcm_element_get_value_UL(element, 0);
    eheader_destroy(header);
    dcm_element_destroy(element);

    // File Meta Information Version
    header = read_element_header(error, file->fp, n, implicit);
    if (header == NULL) {
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    element = read_element(error, file->fp, header, n, implicit);
    if (element == NULL) {
        eheader_destroy(header);
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    eheader_destroy(header);
    dcm_element_destroy(element);

    while(true) {
        header = read_element_header(error, file->fp, n, implicit);
        if (header == NULL) {
            dcm_element_destroy(element);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }
        tag = eheader_get_tag(header);
        group_number = eheader_get_group_number(header);
        if (group_number != 0x0002) {
            eheader_destroy(header);
            break;
        }

        element = read_element(error, file->fp, header, n, implicit);
        if (element == NULL) {
            eheader_destroy(header);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        if (!dcm_dataset_insert(error, file_meta, element)) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading File Meta Information failed",
                          "Could not insert Data Element '%08X' into "
                          "Data Set", tag);
            eheader_destroy(header);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        if (size >= group_length) {
            eheader_destroy(header);
            break;
        }
        eheader_destroy(header);
    }

    file->offset = ftell(file->fp);

    element = dcm_dataset_get(error, file_meta, 0x00020010);
    const char *transfer_syntax_uid = dcm_element_get_value_UI(element, 0);
    file->transfer_syntax_uid = dcm_strdup(error, transfer_syntax_uid);
    if (file->transfer_syntax_uid == NULL) {
        file->offset = 0;  // reset state
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    dcm_dataset_lock(file_meta);
    return file_meta;
}


void dcm_file_destroy(DcmFile *file)
{
    if (file) {
        if (file->transfer_syntax_uid) {
            free(file->transfer_syntax_uid);
        }
        fclose(file->fp);
        free(file);
        file = NULL;
    }
}


DcmDataSet *dcm_file_read_metadata(DcmError **error, DcmFile *file)
{
    uint32_t tag;
    uint16_t group_number;
    size_t size = 0;
    size_t *n = &size;
    bool implicit;
    char tmp[1];
    DcmElement *element;
    EHeader *header;

    if (file->offset == 0) {
        DcmDataSet *file_meta = dcm_file_read_file_meta(error, file);
        if (file_meta == NULL) {
            return NULL;
        }
    }
    fseek(file->fp, file->offset, SEEK_SET);

    implicit = false;
    if (file->transfer_syntax_uid) {
        if (strcmp(file->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
            implicit = true;
        }
    }

    DcmDataSet *dataset = dcm_dataset_create(error);
    if (dataset == NULL) {
        return NULL;
    }

    while (!feof(file->fp)) {
        if (fread(tmp, 1, 1, file->fp) == 0) {
            dcm_log_info("Stop reading Data Set. Reached end of file.");
            break;
        }
        fseek(file->fp, -1L, SEEK_CUR);

        header = read_element_header(error, file->fp, n, implicit);
        if (header == NULL) {
            dcm_dataset_destroy(dataset);
            return NULL;
        }

        tag = eheader_get_tag(header);
        group_number = eheader_get_group_number(header);
        if (tag == TAG_TRAILING_PADDING) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Stop reading Data Set",
                          "Encountered Data Set Trailing Tag");
            eheader_destroy(header);
            break;
        } else if (tag == TAG_PIXEL_DATA ||
                   tag == TAG_FLOAT_PIXEL_DATA ||
                   tag == TAG_DOUBLE_PIXEL_DATA) {
            // Set file pointer to the first byte of the pixel data element
            if (implicit) {
                // Tag: 4 bytes, Value Length: 4 bytes
                fseek(file->fp, -8L, SEEK_CUR);
            } else {
                // Tag: 4 bytes, VR: 2 bytes + 2 bytes, Value Length: 4 bytes
                fseek(file->fp, -12L, SEEK_CUR);
            }
            file->pixel_data_offset = ftell(file->fp);
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Stop reading Data Set",
                          "Encountered Tag of Pixel Data Element");
            eheader_destroy(header);
            break;
        }
        if (group_number == 0x0002) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading of Data Set failed",
                          "Encountered File Meta Information group");
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }

        element = read_element(error, file->fp, header, n, implicit);
        if (element == NULL) {
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }
        if (!dcm_dataset_insert(error, dataset, element)) {
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }
        eheader_destroy(header);
    }
    dcm_dataset_lock(dataset);
    return dataset;
}


static bool get_num_frames(DcmError **error, 
                           const DcmDataSet *metadata,
                           uint32_t *number_of_frames)
{
    const uint32_t tag = 0x00280008;

    DcmElement *element = dcm_dataset_get(error, metadata, tag);
    if (element == NULL) {
        return false;
    }

    const char *value = dcm_element_get_value_IS(element, 0);
    *number_of_frames = (uint32_t) strtol(value, NULL, 10);

    return true;
}


DcmBOT *dcm_file_read_bot(DcmError **error,
                          const DcmFile *file, const DcmDataSet *metadata)
{
    uint32_t tmp_value;
    uint64_t value;
    uint32_t i;
    size_t tmp_offset;

    dcm_log_debug("Reading Basic Offset Table.");

    if (!dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Data Set with transfer syntax '%s' should not contain "
                      "a Basic Offset Table because it is not encapsulated",
                      file->transfer_syntax_uid);
        return NULL;
    }

    uint32_t num_frames;
    if (!get_num_frames(error, metadata, &num_frames)) {
        return NULL;
    }
    if (num_frames == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Value of Data Element 'Number of Frames' is "
                      "malformed");
        return NULL;
    }

    if (file->pixel_data_offset == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Could not determine offset of Pixel Data Element. "
                      "Read metadata first");
        return NULL;
    }
    fseek(file->fp, file->pixel_data_offset, SEEK_SET);

    EHeader *eheader = read_element_header(error, 
                                           file->fp, &tmp_offset, false);
    uint32_t eheader_tag = eheader_get_tag(eheader);
    eheader_destroy(eheader);
    if (!(eheader_tag == TAG_PIXEL_DATA ||
          eheader_tag == TAG_FLOAT_PIXEL_DATA ||
          eheader_tag == TAG_DOUBLE_PIXEL_DATA)) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "File pointer not positioned at Pixel Data Element");
        return NULL;
    }

    // The header of the BOT Item
    IHeader *iheader = read_item_header(error, file->fp, &tmp_offset);
    if (iheader == NULL) {
        iheader_destroy(iheader);
        return NULL;
    }
    uint32_t item_tag = iheader_get_tag(iheader);
    if (item_tag != TAG_ITEM) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Unexpected Tag found for Basic Offset Table Item");
        iheader_destroy(iheader);
        return NULL;
    }

    ssize_t *offsets = DCM_NEW_ARRAY(error, num_frames, ssize_t);
    if (offsets == NULL) {
        iheader_destroy(iheader);
        return NULL;
    }

    // The BOT Item must be present, but the value is optional
    uint32_t item_length = iheader_get_length(iheader);
    iheader_destroy(iheader);
    if (item_length > 0) {
        dcm_log_info("Read Basic Offset Table value.");
        // Read offset values from BOT Item value
        for (i = 0; i < num_frames; i++) {
            tmp_offset += fread(&tmp_value, 1, sizeof(tmp_value), file->fp);
            value = (uint64_t) tmp_value;
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
    } else {
        dcm_log_info("Basic Offset Table is empty.");
        // Handle Extended Offset Table attribute
        const DcmElement *eot_element = dcm_dataset_contains(metadata, 
                                                             0x7FE00001);
        if (eot_element) {
            dcm_log_info("Found Extended Offset Table.");
            const char *blob = dcm_element_get_value_OV(eot_element);
            char *end_ptr;
            for (i = 0; i < num_frames; i++) {
                value = (uint64_t) strtoull(blob, &end_ptr, 64);
                // strtoull returns 0 in case of error
                if (value == 0 && i > 0) {
                    dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                                  "Reading Basic Offset Table failed",
                                  "Failed to parse value of Extended Offset "
                                  "Table element for frame #%d", i + 1);
                    free(offsets);
                    return NULL;
                }
                offsets[i] = value;
            }
        }
        return NULL;
    }

    return dcm_bot_create(error, offsets, num_frames);
}


static struct PixelDescription *create_pixel_description(DcmError **error,
    const DcmDataSet *metadata)
{
    DcmElement *element;

    struct PixelDescription *desc = DCM_NEW(error, struct PixelDescription);
    if (desc == NULL) {
        return NULL;
    }

    element = dcm_dataset_get(error, metadata, 0x00280010);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->rows = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280011);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->columns = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280002);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->samples_per_pixel = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280100);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->bits_allocated = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280101);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->bits_stored = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280103);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->pixel_representation = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280006);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->planar_configuration = dcm_element_get_value_US(element, 0);

    element = dcm_dataset_get(error, metadata, 0x00280004);
    if (element == NULL) {
        free(desc);
        return NULL;
    }
    desc->photometric_interpretation = dcm_strdup(error,
                                                  dcm_element_get_value_CS(element, 0));
    if (desc->photometric_interpretation == NULL) {
        free(desc);
        return NULL;
    }

    return desc;
}


static void destroy_pixel_description(struct PixelDescription *desc)
{
    if (desc) {
        if (desc->photometric_interpretation) {
            free(desc->photometric_interpretation);
        }
        free(desc);
    }
}


DcmBOT *dcm_file_build_bot(DcmError **error,
                           const DcmFile *file, const DcmDataSet *metadata)
{
    uint32_t item_tag, iheader_tag;
    uint32_t item_length;
    uint64_t i;
    size_t current_offset, tmp_offset;
    IHeader *iheader;

    dcm_log_debug("Building Basic Offset Table.");

    uint32_t num_frames;
    if (!get_num_frames(error, metadata, &num_frames)) {
        return NULL;
    }
    if (num_frames == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Building Basic Offset Table failed",
                      "Value of Data Element 'Number of Frames' is "
                      "malformed.");
        return NULL;
    }

    if (file->pixel_data_offset == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Basic Offset Table failed",
                      "Could not determine offset of Pixel Data Element. "
                      "Read metadata first");
        return NULL;
    }
    fseek(file->fp, file->pixel_data_offset, SEEK_SET);
    tmp_offset = 0;

    EHeader *eheader = read_element_header(error, 
                                           file->fp, &tmp_offset, false);
    uint32_t eheader_tag = eheader_get_tag(eheader);
    eheader_destroy(eheader);
    if (eheader_tag != TAG_PIXEL_DATA &&
        eheader_tag != TAG_FLOAT_PIXEL_DATA &&
        eheader_tag != TAG_DOUBLE_PIXEL_DATA) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Building Basic Offset Table failed",
                      "File pointer not positioned at Pixel Data Element");
        return NULL;
    }

    ssize_t *offsets = DCM_NEW_ARRAY(error, num_frames, ssize_t);
    if (offsets == NULL) {
        return NULL;
    }

    if (dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        // The header of the BOT Item
        iheader = read_item_header(error, file->fp, &tmp_offset);
        if (iheader == NULL) {
            free(offsets);
            iheader_destroy(iheader);
            return NULL;
        }
        item_tag = iheader_get_tag(iheader);
        if (item_tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Building Basic Offset Table failed",
                          "Unexpected Tag found for Basic Offset Table Item");
            free(offsets);
            iheader_destroy(iheader);
            return NULL;
        }

        // The BOT Item must be present, but the value is optional
        item_length = iheader_get_length(iheader);
        iheader_destroy(iheader);
        // Move filepointer to first byte of first Frame item
        fseek(file->fp, item_length, SEEK_SET);
        i = 0;
        current_offset = 0;
        while (true) {
            iheader = read_item_header(error, file->fp, &current_offset);
            if (iheader == NULL) {
                free(offsets);
                iheader_destroy(iheader);
                return NULL;
            }
            iheader_tag = iheader_get_tag(iheader);
            if (iheader_tag == TAG_SQ_DELIM) {
                break;
            }
            if (iheader_tag != TAG_ITEM) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "Building Basic Offset Table failed",
                              "Frame Item #%d has wrong Tag '%08X'",
                              i + 1,
                              iheader_tag);
                free(offsets);
                iheader_destroy(iheader);
                return NULL;
            }
            if (feof(file->fp)) {
                break;
            }
            offsets[i] = current_offset;

            item_length = iheader_get_length(iheader);
            fseek(file->fp, item_length, SEEK_CUR);
            iheader_destroy(iheader);
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
        struct PixelDescription *desc = create_pixel_description(error,
                                                                 metadata);
        if (desc == NULL) {
            free(offsets);
            destroy_pixel_description(desc);
            return NULL;
        }
        for (i = 0; i < num_frames; i++) {
            offsets[i] = i * desc->rows * desc->columns * desc->samples_per_pixel;
        }
        destroy_pixel_description(desc);
    }

    return dcm_bot_create(error, offsets, num_frames);
}


DcmFrame *dcm_file_read_frame(DcmError **error,
                              const DcmFile *file,
                              const DcmDataSet *metadata,
                              const DcmBOT *bot,
                              uint32_t number)
{
    ssize_t first_frame_offset, total_frame_offset;
    uint32_t length;
    size_t current_offset = 0;
    size_t *n = &current_offset;

    dcm_log_debug("Read Frame Item #%d.", number);
    if (number == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                      "Reading Frame Item failed",
                      "Frame Number must be positive");
        return NULL;
    }
    ssize_t frame_offset = dcm_bot_get_frame_offset(bot, number);
    uint32_t num_frames = dcm_bot_get_num_frames(bot);
    if (dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        // Header of Pixel Data Element and Basic Offset Table
        first_frame_offset = 12 + 8 + 4 * num_frames;
    } else {
        // Header of Pixel Data Element
        first_frame_offset = 10;
    }

    total_frame_offset = (file->pixel_data_offset +
                          first_frame_offset +
                          frame_offset);
    fseek(file->fp, total_frame_offset, SEEK_SET);

    struct PixelDescription *desc = create_pixel_description(error, metadata);
    if (desc == NULL) {
        return NULL;
    }

    if (dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        IHeader *iheader = read_item_header(error, file->fp, n);
        if (iheader == NULL) {
            destroy_pixel_description(desc);
            return NULL;
        }
        uint32_t iheader_tag = iheader_get_tag(iheader);
        if (iheader_tag != TAG_ITEM) {
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "Reading Frame Item failed",
                          "No Item Tag found for Frame Item #%d",
                          number);
            destroy_pixel_description(desc);
            iheader_destroy(iheader);
            return NULL;
        }
        length = iheader_get_length(iheader);
        iheader_destroy(iheader);
    } else {
        length = desc->rows * desc->columns * desc->samples_per_pixel;
    }

    char *value = DCM_MALLOC(error, length);
    if (value == NULL) {
        destroy_pixel_description(desc);
        return NULL;
    }
    *n += fread(value, 1, length, file->fp);

    char *transfer_syntax_uid = dcm_strdup(error, file->transfer_syntax_uid);
    if (transfer_syntax_uid == NULL) {
        destroy_pixel_description(desc);
        free(value);
        return NULL;
    }

    char *photometric_interpretation = dcm_strdup(error, desc->photometric_interpretation);
    if (photometric_interpretation == NULL) {
        free(transfer_syntax_uid);
        free(value);
        destroy_pixel_description(desc);
        return NULL;
    }

    DcmFrame *frame = dcm_frame_create(error,
                                       number,
                                       value,
                                       length,
                                       desc->rows,
                                       desc->columns,
                                       desc->samples_per_pixel,
                                       desc->bits_allocated,
                                       desc->bits_stored,
                                       desc->pixel_representation,
                                       desc->planar_configuration,
                                       photometric_interpretation,
                                       transfer_syntax_uid);
    destroy_pixel_description(desc);

    return frame;
}
