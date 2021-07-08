/*
 * Implementation of Part 10 of the DICOM standard: Media Storage and File
 * Format for Media Interchange.
 */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../lib/utarray.h"

#include "dicom.h"


enum SpecialTag {
    TAG_ITEM = 0xFFFEE000,
    TAG_ITEM_DELIM = 0xFFFEE00D,
    TAG_SQ_DELIM = 0xFFFEE0DD,
    TAG_TRAILING_PADDING = 0xFFFCFFFC,
    TAG_EXTENDED_OFFSET_TABLE = 0x7FE00001,
    TAG_PIXEL_DATA = 0x7FE00010,
    TAG_FLOAT_PIXEL_DATA = 0x7FE00008,
    TAG_DOUBLE_PIXEL_DATA = 0x7FE00009,
};


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
} iheader_t;


typedef struct ElementHeader {
    uint32_t tag;
    char vr[3];
    uint64_t length;
} eheader_t;


static iheader_t *iheader_create(uint32_t tag, uint64_t length)
{
    if (!(tag == TAG_ITEM ||
          tag == TAG_ITEM_DELIM ||
          tag == TAG_SQ_DELIM)) {
        dcm_log_error("Constructing header of Item failed. "
                      "Encountered invalid Item Tag '%08X'.",
                      tag);
        return NULL;
    }
    iheader_t *header = malloc(sizeof(struct ItemHeader));
    if (header == NULL) {
        dcm_log_error("Constructing header of Item failed. "
                      "Failed to allocate memory for header of Item '%08X'.",
                      tag);
        return NULL;
    }
    header->tag = tag;
    header->length = length;
    return header;
}


static uint32_t iheader_get_tag(iheader_t *item)
{
    assert(item);
    return item->tag;
}


static uint64_t iheader_get_length(iheader_t *item)
{
    assert(item);
    return item->length;
}


static void iheader_destroy(iheader_t *item)
{
    if (item != NULL) {
        free(item);
        item = NULL;
    }
}


static eheader_t *eheader_create(uint32_t tag, const char *vr, uint64_t length)
{
    bool is_valid_tag = dcm_is_valid_tag(tag);
    if (!is_valid_tag) {
        dcm_log_error("Constructing header of Data Element failed. "
                      "Encountered invalid Tag: '%08X'.",
                      tag);
        return NULL;
    }
    eheader_t *header = malloc(sizeof(struct ElementHeader));
    if (header == NULL) {
        dcm_log_error("Constructing header of Data Element failed. "
                      "Could not allocate memory for "
                      "header of Data Element '%08X'.",
                      tag);
        return NULL;
    }
    header->tag = tag;

    if (!dcm_is_valid_vr(vr)) {
        dcm_log_error("Constructing header of Data Element failed. "
                      "Encountered invalid Value Representation: '%s'.",
                      vr);
        return NULL;
    }
    strcpy(header->vr, vr);
    header->vr[2] = '\0';

    header->length = length;
    return header;
}


static uint16_t eheader_get_group_number(eheader_t *header)
{
    assert(header);
    return (uint16_t)(header->tag >> 16);
}


static uint32_t eheader_get_tag(eheader_t *header)
{
    assert(header);
    return header->tag;
}


static bool eheader_check_vr(eheader_t *header, const char *vr)
{
    assert(header);
    if (strcmp(header->vr, vr) == 0) {
        return true;
    } else {
        return false;
    }
}


static uint64_t eheader_get_length(eheader_t *header)
{
    assert(header);
    return header->length;
}


static void eheader_destroy(eheader_t *header)
{
    if (header != NULL) {
        free(header);
        header = NULL;
    }
}



struct dcm_File {
    FILE *fp;
    dcm_dataset_t *meta;
    size_t offset;
    char *transfer_syntax_uid;
    size_t pixel_data_offset;
    uint64_t *extended_offset_table;
};


static uint32_t read_tag(FILE *fp, size_t *n)
{
    uint16_t group_num;
    uint16_t elem_num;
    *n += fread(&group_num, 1, sizeof(group_num), fp);
    *n += fread(&elem_num, 1, sizeof(elem_num), fp);
    return ((uint32_t)group_num << 16) + elem_num;
}


static char **parse_character_string(char *string, uint32_t *vm)
{
    uint32_t n;
    uint32_t i;
    char *token = NULL;
    char **token_ptr = NULL;
    UT_array *array = NULL;
    const char delim[2] = "\\";
    char **parts = NULL;

    utarray_new(array, &ut_str_icd);

    if (strlen(string) == 0) {
        token = "";
        utarray_push_back(array, &token);
        goto finish;
    }
    token = strtok(string, delim);
    while(token != NULL) {
        utarray_push_back(array, &token);
        token = strtok(NULL, delim);
    }
    goto finish;

finish:
    n = utarray_len(array);
    parts = malloc(n * sizeof(char *));
    for (i = 0; i < n; i++) {
        token_ptr = utarray_eltptr(array, i);
        parts[i] = malloc(strlen(*token_ptr) + 1);
        strcpy(parts[i], *token_ptr);
    }
    *vm = n;
    utarray_free(array);
    return parts;
}


static iheader_t *read_item_header(FILE *fp, size_t *n)
{
    uint32_t tag;
    uint32_t length;

    tag = read_tag(fp, n);
    *n += fread(&length, 1, sizeof(length), fp);
    return iheader_create(tag, length);
}


static eheader_t *read_element_header(FILE *fp, size_t *n, bool implicit)
{
    uint32_t tag;
    char vr[3];
    uint32_t length;
    uint16_t short_length;
    uint16_t reserved;

    tag = read_tag(fp, n);
    if (implicit) {
        // Value Representation
        const char *tmp = dcm_dict_lookup_vr(tag);
        strcpy(vr, tmp);

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
                dcm_log_error("Reading of Data Element header failed. "
                              "Unexpected value for reserved bytes "
                              "of Data Element %08X with VR '%s'.",
                              tag,
                              vr);
                return NULL;
            }
            *n += fread(&length, 1, sizeof(length), fp);
        }
    }

    eheader_t *header = eheader_create(tag, vr, length);
    return header;
}


static dcm_element_t *read_element(FILE *fp,
                                   eheader_t *header,
                                   size_t *n,
                                   bool implicit)
{
    assert(header);
    uint32_t tag;
    const char *vr = NULL;
    uint32_t length;
    uint32_t vm;
    uint32_t i;
    uint32_t item_index = 0;
    uint32_t item_tag;
    uint32_t item_length;
    dcm_element_t *element = NULL;
    iheader_t *item_iheader = NULL;
    dcm_dataset_t *item_dataset = NULL;
    eheader_t *item_eheader = NULL;
    dcm_element_t *item_element = NULL;
    size_t n_seq = 0;
    size_t *n_seq_ptr = &n_seq;
    size_t n_item = 0;
    size_t *n_item_ptr = &n_item;

    tag =  eheader_get_tag(header);
    length = eheader_get_length(header);

    dcm_log_debug("Read Data Element '%08X' with VR '%s'", tag, vr);

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
        char *value = malloc(length + 1);
        if (value == NULL) {
            dcm_log_error("Reading of Data Element failed. "
                          "Could not allocate memory for "
                          "value of Data Element '%08X'.",
                          tag);
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
        char **strings = parse_character_string(value, &vm);
        free(value);

        if (eheader_check_vr(header, "AE")) {
            return dcm_element_create_AE_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "AS")) {
            return dcm_element_create_AS_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "AT")) {
            return dcm_element_create_AT_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "CS")) {
            return dcm_element_create_CS_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "DA")) {
            return dcm_element_create_DA_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "DS")) {
            return dcm_element_create_DS_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "DT")) {
            return dcm_element_create_DT_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "IS")) {
            return dcm_element_create_IS_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "LO")) {
            return dcm_element_create_LO_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "PN")) {
            return dcm_element_create_PN_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "SH")) {
            return dcm_element_create_SH_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "ST")) {
            // This VM shall always have VM 1.
            length = eheader_get_length(header);
            char *str = malloc(length + 1);
            strcpy(str, strings[0]);
            str[length + 1] = '\0';
            free(strings);
            return dcm_element_create_ST(tag, str);
        } else if (eheader_check_vr(header, "TM")) {
            return dcm_element_create_TM_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "UI")) {
            return dcm_element_create_UI_multi(tag, strings, vm);
        } else if (eheader_check_vr(header, "LT")) {
            // This VM shall always have VM 1.
            length = eheader_get_length(header);
            char *str = malloc(length + 1);
            if (str == NULL) {
                dcm_log_error("Reading of Data Element failed. "
                              "Could not allocate memory.");
                free(strings);
                return NULL;
            }
            strcpy(str, strings[0]);
            str[length + 1] = '\0';
            free(strings);
            return dcm_element_create_LT(tag, str);
        } else if (eheader_check_vr(header, "UR")) {
            // This VM shall always have VM 1.
            length = eheader_get_length(header);
            char *str = malloc(length + 1);
            if (str == NULL) {
                dcm_log_error("Reading of Data Element failed. "
                              "Could not allocate memory.");
                free(strings);
                return NULL;
            }
            strcpy(str, strings[0]);
            str[length + 1] = '\0';
            free(strings);
            return dcm_element_create_UR(tag, str);
        } else if (eheader_check_vr(header, "UT")) {
            // This VM shall always have VM 1.
            length = eheader_get_length(header);
            char *str = malloc(length + 1);
            if (str == NULL) {
                dcm_log_error("Reading of Data Element failed. "
                              "Could not allocate memory.");
                free(strings);
                return NULL;
            }
            strcpy(str, strings[0]);
            str[length + 1] = '\0';
            free(strings);
            return dcm_element_create_UT(tag, str);
        } else {
            dcm_log_error("Reading of Data Element failed. "
                          "Encountered unexpected Value Representation '%s' "
                          "for Data Element '%08X'.",
                          vr, tag);
            free(strings);
            return NULL;
        }
    } else if (eheader_check_vr(header, "SQ")) {
        dcm_sequence_t *value = dcm_sequence_create();
        if (value == NULL) {
            dcm_log_error("Reading of Data Element failed. "
                          "Could not construct Sequence for "
                          "Data Element '%08X'.");
            return NULL;
        }
        if (length == 0) {
            return dcm_element_create_SQ(tag, value);
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
            item_iheader = read_item_header(fp, n_seq_ptr);
            if (item_iheader == NULL) {
                dcm_log_error("Reading of Data Element failed. "
                              "Could not construct Item #%d of "
                              "Data Element '%08X'.", item_index, tag);
                dcm_sequence_destroy(value);
                return NULL;
            }
            item_tag = iheader_get_tag(item_iheader);
            item_length = iheader_get_length(item_iheader);
            if (item_tag == TAG_SQ_DELIM) {
                dcm_log_debug("Stop reading Data Element '%08X'. "
                              "Encountered Sequence Delimination Tag.",
                              tag);
                break;
            }
            if (item_tag != TAG_ITEM) {
                dcm_log_error("Reading of Data Element failed. "
                              "Expected tag '%08X' instead of '%08X' "
                              "for Item #%d of Data Element '%08X'.",
                              TAG_ITEM,
                              item_tag,
                              item_index,
                              tag);
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

            item_dataset = dcm_dataset_create();
            if (item_dataset == NULL) {
                dcm_log_error("Reading of Data Element failed. "
                              "Could not construct Data Set for "
                              "Item #%d of Data Element '%08X'.",
                              item_index,
                              tag);
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

                item_eheader = read_element_header(fp,
                                                       n_item_ptr,
                                                       implicit);
                if (item_eheader == NULL) {
                    dcm_log_error("Reading of Data Element failed. "
                                  "Could not read header of Item #%d "
                                  "of Data Element '%08X'.",
                                  item_index, tag);
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }

                item_element = read_element(fp,
                                                item_eheader,
                                                n_item_ptr,
                                                implicit);
                if (item_element == NULL) {
                    dcm_log_error("Reading of Data Element failed. "
                                  "Could not read value of Item #%d of "
                                  "Data Element '%08X'.",
                                  item_index, tag);
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }
                if (!dcm_dataset_insert(item_dataset, item_element)) {
                    dcm_log_error("Inserting Item #%d of Data Element '%08X' "
                                  "into Data Set failed.", item_index, tag);
                    iheader_destroy(item_iheader);
                    eheader_destroy(item_eheader);
                    dcm_sequence_destroy(value);
                    return NULL;
                }
                eheader_destroy(item_eheader);
            }
            n_seq += n_item;
            dcm_sequence_append(value, item_dataset);
            iheader_destroy(item_iheader);
            item_index += 1;
        }
        *n += n_seq;
        return dcm_element_create_SQ(tag, value);
    } else if (eheader_check_vr(header, "FD")) {
        vm = length / sizeof(double);
        double *values = malloc(vm * sizeof(double));
        for (i = 0; i < vm; i++) {
            double val;
            *n += fread(&val, 1, sizeof(double), fp);
            values[i] = val;
        }
        return dcm_element_create_FD_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "FL")) {
        vm = length / sizeof(float);
        float *values = malloc(vm * sizeof(float));
        for (i = 0; i < vm; i++) {
            float val;
            *n += fread(&val, 1, sizeof(float), fp);
            values[i] = val;
        }
        return dcm_element_create_FL_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "SS")) {
        vm = length / sizeof(int16_t);
        int16_t *values = malloc(vm * sizeof(int16_t));
        for (i = 0; i < vm; i++) {
            int16_t val;
            *n += fread(&val, 1, sizeof(int16_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SS_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "SL")) {
        vm = length / sizeof(int32_t);
        int32_t *values = malloc(vm * sizeof(int32_t));
        for (i = 0; i < vm; i++) {
            int32_t val;
            *n += fread(&val, 1, sizeof(int32_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SL_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "SV")) {
        vm = length / sizeof(int64_t);
        int64_t *values = malloc(vm * sizeof(int64_t));
        for (i = 0; i < vm; i++) {
            int64_t val;
            *n += fread(&val, 1, sizeof(int64_t), fp);
            values[i] = val;
        }
        return dcm_element_create_SV_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "UL")) {
        vm = length / sizeof(uint32_t);
        uint32_t *values = malloc(vm * sizeof(uint32_t));
        for (i = 0; i < vm; i++) {
            uint32_t val;
            *n += fread(&val, 1, sizeof(uint32_t), fp);
            values[i] = val;
        }
        return dcm_element_create_UL_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "US")) {
        vm = length / sizeof(uint16_t);
        uint16_t *values = malloc(vm * sizeof(uint16_t));
        for (i = 0; i < vm; i++) {
            uint16_t val;
            *n += fread(&val, 1, sizeof(uint16_t), fp);
            values[i] = val;
        }
        return dcm_element_create_US_multi(tag, values, vm);
    } else if (eheader_check_vr(header, "UV")) {
        vm = length / sizeof(uint64_t);
        uint64_t *values = malloc(vm * sizeof(uint64_t));
        for (i = 0; i < vm; i++) {
            uint64_t val;
            *n += fread(&val, 1, sizeof(uint64_t), fp);
            values[i] = val;
        }
        return dcm_element_create_UV_multi(tag, values, vm);
    } else {
        char *value = malloc(length);
        if (value == NULL) {
            dcm_log_error("Reading of Data Element failed. "
                          "Could not allocate memory for "
                          "value of Data Element '%08X'.",
                          tag);
            return NULL;
        }
        *n += fread(value, 1, length, fp);

        if (eheader_check_vr(header, "OB")) {
            return dcm_element_create_OB(tag, value, length);
        } else if (eheader_check_vr(header, "OD")) {
            return dcm_element_create_OD(tag, value, length);
        } else if (eheader_check_vr(header, "OF")) {
            return dcm_element_create_OF(tag, value, length);
        } else if (eheader_check_vr(header, "OV")) {
            return dcm_element_create_OV(tag, value, length);
        } else if (eheader_check_vr(header, "OW")) {
            return dcm_element_create_OW(tag, value, length);
        } else if (eheader_check_vr(header, "UC")) {
            return dcm_element_create_UC(tag, value, length);
        } else if (eheader_check_vr(header, "UN")) {
            return dcm_element_create_UN(tag, value, length);
        } else {
            tag = eheader_get_tag(header);
            dcm_log_error("Reading of Data Element failed. "
                          "Data Element '%08X' has unexpected "
                          "Value Representation.", tag);
            return NULL;
        }
    }
    return element;
}


dcm_file_t *dcm_file_create(const char *file_path, char mode)
{
    dcm_file_t *file = NULL;
    char file_mode[3];

    if (mode != 'r' && mode != 'w') {
        dcm_log_error("Creation of file failed. "
                      "Wrong file mode specified.");
        exit(1);
    }

    file = malloc(sizeof(struct dcm_File));
    if (file == NULL) {
        dcm_log_error("Creation of file failed. "
                      "Could not allocate memory for file.");
        return NULL;
    }

    file_mode[0] = mode;
    file_mode[1] = 'b';
    file_mode[2] = '\0';
    file->fp = fopen(file_path, file_mode);
    if (file->fp == NULL) {
        dcm_log_error("Could not open file for reading: %s", file_path);
        return NULL;
    }

    file->offset = 0;
    file->pixel_data_offset = 0;
    file->transfer_syntax_uid = NULL;

    return file;
}


dcm_dataset_t *dcm_file_read_file_meta(dcm_file_t *file)
{
    size_t size = 0;
    size_t *n = &size;
    char preamble[129];
    char prefix[5];
    bool implicit = false;
    uint32_t tag;
    uint16_t group_number;
    uint32_t group_length;
    dcm_dataset_t *file_meta = NULL;
    eheader_t *header = NULL;
    dcm_element_t *element = NULL;
    uint8_t n_elem = 0;

    file_meta = dcm_dataset_create();
    if (file_meta == NULL) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Could not construct Data Set.");
        return NULL;
    }

    // File Preamble
    size += fread(preamble, 1, sizeof(preamble) - 1, file->fp);
    preamble[128] = '\0';

    // DICOM Prefix
    size += fread(prefix, 1, sizeof(prefix) - 1, file->fp);
    prefix[4] = '\0';
    if (strcmp(prefix, "DICM") != 0) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Prefix 'DICM' not found.");
        dcm_dataset_destroy(file_meta);
        return NULL;
    }

    size = 0;

    // File Meta Information Group Length
    header = read_element_header(file->fp, n, implicit);
    if (header == NULL) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Header of Data Element 'Group Length' "
                      "could not be read.");
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    element = read_element(file->fp, header, n, implicit);
    if (element == NULL) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Value of Data Element 'Group Length' "
                      "could not be read.");
        eheader_destroy(header);
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    dcm_element_copy_value_UL(element, 0, &group_length);
    eheader_destroy(header);

    // File Meta Information Version
    header = read_element_header(file->fp, n, implicit);
    if (header == NULL) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Header of Data Element 'File Meta Information Version' "
                      "could not be read.");
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    element = read_element(file->fp, header, n, implicit);
    if (element == NULL) {
        dcm_log_error("Reading of File Meta Information failed. "
                      "Value of Data Element 'File Meta Information Version' "
                      "could not be read.");
        eheader_destroy(header);
        dcm_dataset_destroy(file_meta);
        return NULL;
    }
    while(true) {
        header = read_element_header(file->fp, n, implicit);
        if (header == NULL) {
            dcm_log_error("Reading of File Meta Information failed. "
                          "Could not read header of Data Element #%d.",
                          n_elem);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }
        tag = eheader_get_tag(header);
        group_number = eheader_get_group_number(header);
        if (group_number != 0x0002) {
            eheader_destroy(header);
            dcm_dataset_destroy(file_meta);
            break;
        }

        element = read_element(file->fp, header, n, implicit);
        if (element == NULL) {
            dcm_log_error("Reading File Meta Information failed. "
                          "Could not read value of Data Element '%08X'.", tag);
            dcm_dataset_destroy(file_meta);
            return NULL;
        }

        if (!dcm_dataset_insert(file_meta, element)) {
            dcm_log_error("Reading File Meta Information failed. "
                          "Could not insert Data Element '%08X' into "
                          "Data Set.", tag);
            return NULL;
        }

        if (size >= group_length) {
            eheader_destroy(header);
            break;
        }
        n_elem += 1;
        eheader_destroy(header);
    }

    file->offset = ftell(file->fp);

    element = dcm_dataset_get(file_meta, 0x00020010);
    file->transfer_syntax_uid = malloc(DCM_CAPACITY_UI + 1);
    dcm_element_copy_value_UI(element, 0, file->transfer_syntax_uid);
    file->transfer_syntax_uid[DCM_CAPACITY_UI] = '\0';

    dcm_dataset_lock(file_meta);
    return file_meta;
}


void dcm_file_destroy(dcm_file_t *file)
{
    if (file != NULL) {
        if (file->transfer_syntax_uid != NULL) {
            free(file->transfer_syntax_uid);
        }
        fclose(file->fp);
        free(file);
        file = NULL;
    }
}


dcm_dataset_t *dcm_file_read_metadata(dcm_file_t *file)
{
    uint32_t tag;
    uint16_t group_number;
    size_t size = 0;
    size_t *n = &size;
    uint32_t n_elem = 0;
    bool implicit = false;
    dcm_element_t *element = NULL;
    eheader_t *header = NULL;
    dcm_dataset_t *file_meta = NULL;
    dcm_dataset_t *dataset = NULL;

    if (file->offset == 0) {
        file_meta = dcm_file_read_file_meta(file);
        if (file_meta == NULL) {
            dcm_log_error("Reading metadata failed. "
                          "Could not read File Meta Information.");
            return NULL;
        }
    }
    fseek(file->fp, file->offset, SEEK_SET);

    if (file->transfer_syntax_uid != NULL) {
        if (strcmp(file->transfer_syntax_uid, "1.2.840.10008.1.2") == 0) {
            implicit = true;
        }
    }

    dataset = dcm_dataset_create();
    if (dataset == NULL) {
        dcm_log_error("Reading of Data Set failed. "
                      "Could not construct Data Set.");
        return NULL;
    }

    while (!feof(file->fp)) {
        header = read_element_header(file->fp, n, implicit);
        if (header == NULL) {
            dcm_log_error("Reading of Data Set failed. "
                          "Could not read header of Data Element #%d.",
                          n_elem);
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }

        if (feof(file->fp)) {
            // This should not happen, but safety first.
            dcm_log_warning("Stop reading Data Set. Reached end of file.");
            break;
        }

        tag = eheader_get_tag(header);
        group_number = eheader_get_group_number(header);
        if (tag == TAG_TRAILING_PADDING) {
            dcm_log_debug("Stop reading Data Set. "
                          "Encountered Data Set Trailing Tag.");
            eheader_destroy(header);
            break;
        }
        if (tag == TAG_PIXEL_DATA ||
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
            dcm_log_debug("Stop reading Data Set. "
                          "Encountered Tag of Pixel Data Element.");
            eheader_destroy(header);
            break;
        }
        if (group_number == 0x0002) {
            dcm_log_error("Reading of Data Set failed. "
                          "Encountered File Meta Information group.");
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }

        element = read_element(file->fp, header, n, implicit);
        if (element == NULL) {
            dcm_log_error("Reading of Data Set failed. "
                          "Could not read value of Data Element '%08X'.",
                          tag);
            eheader_destroy(header);
            dcm_dataset_destroy(dataset);
            return NULL;
        }
        if (!dcm_dataset_insert(dataset, element)) {
            dcm_log_error("Inserting Data Element '%08X' into Data Set "
                          "failed.", tag);
            return NULL;
        }

        n_elem += 0;
    }
    dcm_dataset_lock(dataset);
    return dataset;
}


static bool get_num_frames(dcm_dataset_t *metadata, uint32_t *num_frames)
{
    uint32_t number_of_frames_tag = 0x00280008;
    dcm_element_t *number_of_frames_element = NULL;
    char *number_of_frames = NULL;

    number_of_frames_element = dcm_dataset_get(metadata, number_of_frames_tag);
    if (number_of_frames_element == NULL) {
        dcm_log_error("Getting value of Data Element 'Number of Frames' "
                      "failed. Could not find Data Element with Tag '%08X'.",
                      number_of_frames_tag);
        return false;
    }
    number_of_frames = malloc(DCM_CAPACITY_IS + 1);
    if (number_of_frames == NULL) {
        dcm_log_error("Getting value of Data Element 'Number of Frames' "
                      "failed. Could not allocate memory for value.");
        return false;
    }
    dcm_element_copy_value_IS(number_of_frames_element, 0, number_of_frames);
    *num_frames = (uint32_t) strtol(number_of_frames, NULL, 10);
    free(number_of_frames);

    return true;
}


dcm_bot_t *dcm_file_read_bot(dcm_file_t *file, dcm_dataset_t *metadata)
{
    uint32_t item_tag;
    uint32_t eheader_tag;
    uint32_t item_length;
    uint32_t tmp_value;
    uint64_t value;
    uint32_t i;
    size_t tmp_offset = 0;
    ssize_t *offsets = NULL;
    eheader_t *eheader = NULL;
    iheader_t *iheader = NULL;
    bool implicit = false;
    uint32_t num_frames;

    dcm_log_debug("Reading Basic Offset Table.");

    if (!dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Data Set with transfer syntax '%s' shall not contain "
                      "a Basic Offset Table because it is not encapsulated.",
                      file->transfer_syntax_uid);
        return NULL;
    }

    if (!get_num_frames(metadata, &num_frames)) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Could not get value of Data Element 'Number of Frames'.");
        return NULL;
    }
    if (num_frames == 0) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Value of Data Element 'Number of Frames' is malformed.");
        return NULL;
    }

    if (file->pixel_data_offset == 0) {
        // Only done to determine the offset of the Pixel Data element.
        dcm_dataset_t *metadata = dcm_file_read_metadata(file);
        dcm_dataset_destroy(metadata);
    }
    fseek(file->fp, file->pixel_data_offset, SEEK_SET);

    eheader = read_element_header(file->fp, &tmp_offset, implicit);
    eheader_tag = eheader_get_tag(eheader);
    eheader_destroy(eheader);
    if (!(eheader_tag == TAG_PIXEL_DATA ||
          eheader_tag == TAG_FLOAT_PIXEL_DATA ||
          eheader_tag == TAG_DOUBLE_PIXEL_DATA)) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "File pointer not positioned at Pixel Data Element.");
        return NULL;
    }

    // The header of the BOT Item
    iheader = read_item_header(file->fp, &tmp_offset);
    if (iheader == NULL) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Could not read header of Basic Offset Table Item.");
        iheader_destroy(iheader);
        return NULL;
    }
    item_tag = iheader_get_tag(iheader);
    if (item_tag != TAG_ITEM) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Unexpected Tag found for Basic Offset Table Item.");
        iheader_destroy(iheader);
        return NULL;
    }

    offsets = malloc(num_frames * sizeof(ssize_t *));
    if (offsets == NULL) {
        dcm_log_error("Reading Basic Offset Table failed. "
                      "Could not allocate memory for values of "
                      "Basic Offset Table.");
        iheader_destroy(iheader);
        return NULL;
    }

    // The BOT Item must be present, but the value is optional
    item_length = iheader_get_length(iheader);
    iheader_destroy(iheader);
    if (item_length > 0) {
        dcm_log_info("Read Basic Offset Table value.");
        // Read offset values from BOT Item value
        for (i = 0; i < num_frames; i++) {
            tmp_offset += fread(&tmp_value, 1, sizeof(tmp_value), file->fp);
            value = (uint64_t) tmp_value;
            if (value == TAG_ITEM) {
                dcm_log_error("Reading Basic Offset Table failed. "
                              "Encountered unexpected Item Tag "
                              "in Basic Offset Table.");
                free(offsets);
                iheader_destroy(iheader);
                return NULL;
            }
            offsets[i] = value;
        }
    } else {
        dcm_log_info("Basic Offset Table is emtpy.");
        return NULL;
    }

    return dcm_bot_create(offsets, num_frames);
}


static struct PixelDescription *get_pixel_description(dcm_dataset_t *metadata)
{
    dcm_element_t *element = NULL;
    struct PixelDescription *desc = NULL;

    desc = malloc(sizeof(struct PixelDescription));
    if (desc == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not allocate memory.");
        return NULL;
    }

    element = dcm_dataset_get(metadata, 0x00280010);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Rows'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->rows);

    element = dcm_dataset_get(metadata, 0x00280011);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Columns'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->columns);

    element = dcm_dataset_get(metadata, 0x00280002);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Samples per Pixel'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->samples_per_pixel);

    element = dcm_dataset_get(metadata, 0x00280100);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Bits Allocated'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->bits_allocated);

    element = dcm_dataset_get(metadata, 0x00280101);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Bits Stored'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->bits_stored);

    element = dcm_dataset_get(metadata, 0x00280103);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Pixel Representation'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->pixel_representation);

    element = dcm_dataset_get(metadata, 0x00280006);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Planar Configuration'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_US(element, 0, &desc->planar_configuration);

    element = dcm_dataset_get(metadata, 0x00280004);
    if (element == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not get Data Element 'Photometric Interpretation'.");
        free(desc);
        return NULL;
    }
    desc->photometric_interpretation = malloc(DCM_CAPACITY_CS + 1);
    if (desc->photometric_interpretation == NULL) {
        dcm_log_error("Getting image pixel description failed. "
                      "Could not allocate memory for value of "
                      "Data Element 'Photometric Interpretation'.");
        free(desc);
        return NULL;
    }
    dcm_element_copy_value_CS(element, 0, desc->photometric_interpretation);

    return desc;
}


dcm_bot_t *dcm_file_build_bot(dcm_file_t *file, dcm_dataset_t *metadata)
{
    uint32_t item_tag;
    uint32_t eheader_tag;
    uint32_t iheader_tag;
    uint32_t item_length;
    uint64_t i;
    size_t current_offset = 0;
    size_t tmp_offset = 0;
    bool implicit = false;
    uint32_t num_frames;
    ssize_t *offsets = NULL;
    eheader_t *eheader = NULL;
    iheader_t *iheader = NULL;
    struct PixelDescription *desc = NULL;

    dcm_log_debug("Building Basic Offset Table.");

    if (!get_num_frames(metadata, &num_frames)) {
        dcm_log_error("Building Basic Offset Table failed. "
                      "Could not get value of Data Element 'Number of Frames'.");
        return NULL;
    }
    if (num_frames == 0) {
        dcm_log_error("Building Basic Offset Table failed. "
                      "Value of Data Element 'Number of Frames' is malformed.");
        return NULL;
    }

    if (file->pixel_data_offset == 0) {
        // Only done to determine the offset of the Pixel Data element.
        dcm_dataset_t *metadata = dcm_file_read_metadata(file);
        dcm_dataset_destroy(metadata);
    }
    fseek(file->fp, file->pixel_data_offset, SEEK_SET);

    eheader = read_element_header(file->fp, &tmp_offset, implicit);
    eheader_tag = eheader_get_tag(eheader);
    eheader_destroy(eheader);
    if (!(eheader_tag == TAG_PIXEL_DATA ||
          eheader_tag == TAG_FLOAT_PIXEL_DATA ||
          eheader_tag == TAG_DOUBLE_PIXEL_DATA)) {
        dcm_log_error("Building Basic Offset Table failed. "
                      "File pointer not positioned at Pixel Data Element.");
        return NULL;
    }

    offsets = malloc(num_frames * sizeof(ssize_t *));
    if (offsets == NULL) {
        dcm_log_error("Building Basic Offset Table failed. "
                      "Could not allocate memory for values of "
                      "Basic Offset Table.");
        return NULL;
    }

    if (dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        // The header of the BOT Item
        iheader = read_item_header(file->fp, &tmp_offset);
        if (iheader == NULL) {
            dcm_log_error("Building Basic Offset Table failed. "
                          "Could not read header of Basic Offset Table Item.");
            free(offsets);
            iheader_destroy(iheader);
            return NULL;
        }
        item_tag = iheader_get_tag(iheader);
        if (item_tag != TAG_ITEM) {
            dcm_log_error("Building Basic Offset Table failed. "
                          "Unexpected Tag found for Basic Offset Table Item.");
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
        while (true) {
            iheader = read_item_header(file->fp, &current_offset);
            if (iheader == NULL) {
                dcm_log_error("Building Basic Offset Table failed. "
                              "Could not read header of Frame Item #%d.",
                              i + 1);
            }
            iheader_tag = iheader_get_tag(iheader);
            if (iheader_tag == TAG_SQ_DELIM) {
                break;
            }
            if (iheader_tag != TAG_ITEM) {
                dcm_log_error("Building Basic Offset Table failed. "
                              "Frame Item #%d has wrong Tag '%08X'.",
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
            dcm_log_error("Building Basic Offset Table failed. "
                          "Found incorrect number of Frame Items.");
            free(offsets);
            return NULL;
        }
    } else {
        desc = get_pixel_description(metadata);
        if (desc == NULL) {
            dcm_log_error("Building Basic Offset Table failed. "
                          "Could not get image pixel description.");
        }
        for (i = 0; i < num_frames; i++) {
            offsets[i] = i * desc->rows * desc->columns * desc->samples_per_pixel;
        }
    }

    return dcm_bot_create(offsets, num_frames);
}


dcm_frame_t *dcm_file_read_frame(dcm_file_t *file,
                                 dcm_dataset_t *metadata,
                                 dcm_bot_t *bot,
                                 uint32_t number)
{
    ssize_t frame_offset;
    ssize_t first_frame_offset;
    ssize_t total_frame_offset;
    size_t current_offset = 0;
    size_t *n = &current_offset;
    uint32_t length;
    char *value = NULL;
    uint32_t iheader_tag;
    uint32_t num_frames;
    iheader_t *iheader = NULL;
    char *transfer_syntax_uid = NULL;
    struct PixelDescription *desc = NULL;

    dcm_log_debug("Read Frame Item #%d.", number);
    if (number == 0) {
        dcm_log_error("Reading Frame Item failed. "
                      "Frame Number must be positive.");
        return NULL;
    }
    frame_offset = dcm_bot_get_frame_offset(bot, number);
    num_frames = dcm_bot_get_num_frames(bot);
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

    desc = get_pixel_description(metadata);
    if (desc == NULL) {
        dcm_log_error("Reading Frame Item failed. "
                      "Could not get image pixel description.");
        return NULL;
    }

    if (dcm_is_encapsulated_transfer_syntax(file->transfer_syntax_uid)) {
        iheader = read_item_header(file->fp, n);
        if (iheader == NULL) {
            dcm_log_error("Reading Frame Item failed. "
                          "Could not read header of Frame Item #%d.",
                          number);
            return NULL;
        }
        iheader_tag = iheader_get_tag(iheader);
        if (iheader_tag != TAG_ITEM) {
            dcm_log_error("Reading Frame Item failed. "
                          "No Item Tag found for Frame Item #%d.",
                          number);
            iheader_destroy(iheader);
            return NULL;
        }
        length = iheader_get_length(iheader);
        iheader_destroy(iheader);
    } else {
        length = desc->rows * desc->columns * desc->samples_per_pixel;
    }

    value = malloc(length);
    if (value == NULL) {
        dcm_log_error("Reading Frame Item failed. "
                      "Could not allocate memory for Frame Item #%d.",
                      number);
        return NULL;
    }
    *n += fread(value, 1, length, file->fp);

    transfer_syntax_uid = malloc(strlen(file->transfer_syntax_uid));
    if (transfer_syntax_uid == NULL) {
        dcm_log_error("Reading Frame Item failed. "
                      "Could not allocate memory for Frame item #%d.",
                      number);
        return NULL;
    }
    strcpy(transfer_syntax_uid, file->transfer_syntax_uid);

    return dcm_frame_create(number,
                            value,
                            length,
                            desc->rows,
                            desc->columns,
                            desc->samples_per_pixel,
                            desc->bits_allocated,
                            desc->bits_stored,
                            desc->pixel_representation,
                            desc->planar_configuration,
                            desc->photometric_interpretation,
                            transfer_syntax_uid);
}
