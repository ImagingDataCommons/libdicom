/*
 * Implementation of Part 5 of the DICOM standard: Data Structures and Encoding.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../lib/utarray.h"
#include "../lib/uthash.h"

#include "dicom.h"

struct _DcmElement {
    uint32_t tag;
    char vr[3];
    uint32_t length;
    uint32_t vm;
    union {
        // Numeric value (multiplicity 1-n)
        double *fd_multi;
        float *fl_multi;
        int16_t *ss_multi;
        int32_t *sl_multi;
        int64_t *sv_multi;
        uint16_t *us_multi;
        uint32_t *ul_multi;
        uint64_t *uv_multi;
        // Character string value (multiplicity 1-n)
        char **str_multi;
        // Binary value (multiplicity 1)
        char *bytes;
        // Sequence value (multiplicity 1)
        DcmSequence *sq;
    } value;
    UT_hash_handle hh;
};


struct _DcmSequence {
    UT_array *items;
    bool is_locked;
};


struct _DcmDataSet {
    DcmElement *elements;
    bool is_locked;
};


struct _DcmFrame {
    uint32_t number;
    const char *data;
    uint32_t length;
    uint16_t rows;
    uint16_t columns;
    uint16_t samples_per_pixel;
    uint16_t bits_allocated;
    uint16_t bits_stored;
    uint16_t high_bit;
    uint16_t pixel_representation;
    uint16_t planar_configuration;
    char *photometric_interpretation;
    char *transfer_syntax_uid;
};


struct _DcmBOT {
    uint32_t num_frames;
    ssize_t *offsets;
};


struct SequenceItem {
    DcmDataSet *dataset;
};


static struct SequenceItem *create_sequence_item(DcmDataSet *dataset)
{
    assert(dataset);

    struct SequenceItem *item = NULL;

    item = malloc(sizeof(struct SequenceItem));
    if (item == NULL) {
        dcm_log_error("Creation of Sequence Item failed."
                      "Could not allocate memory.");
        return NULL;
    }
    item->dataset = dataset;
    item->dataset->is_locked = true;
    return item;
}


static void copy_sequence_item_icd(void *_dst_item, const void *_src_item)
{
    struct SequenceItem *dst_item = (struct SequenceItem *) _dst_item;
    struct SequenceItem *src_item = (struct SequenceItem *) _src_item;
    dst_item->dataset = src_item->dataset;
}


static void destroy_sequence_item_icd(void *_item)
{
    if (_item) {
        struct SequenceItem *item = (struct SequenceItem *) _item;
        if (item) {
            dcm_dataset_destroy(item->dataset);
            item->dataset = NULL;
        }
    }
}


static UT_icd sequence_item_icd = {
    sizeof(struct SequenceItem *),
    NULL,
    copy_sequence_item_icd,
    destroy_sequence_item_icd
};


static int compare_tags(const void * a, const void * b)
{
   return ( *(uint32_t*)a - *(uint32_t*)b );
}


// Data Elements

static DcmElement *create_element(uint32_t tag, const char *vr, uint32_t length)
{
    DcmElement *element = NULL;

    dcm_log_debug("Create Data Element '%08X'.", tag);
    element = malloc(sizeof(struct _DcmElement));
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed."
                      "Could not allocate memory for Data Element '%08X'.",
                      tag);
        return NULL;
    }
    element->tag = tag;
    strcpy(element->vr, vr);
    element->vr[2] = '\0';
    if (length % 2 != 0) {
        // Zero padding
        length += 1;
    }
    element->length = length;
    element->vm = 0;
    return element;
}


void dcm_element_destroy(DcmElement *element)
{
    uint32_t i;

    if (element) {
        dcm_log_debug("Destroy Data Element '%08X'.", element->tag);
        if (strcmp(element->vr, "SQ") == 0) {
            if (element->value.sq) {
                dcm_sequence_destroy(element->value.sq);
            }
        } else if (strcmp(element->vr, "AE") == 0 ||
                 strcmp(element->vr, "AS") == 0 ||
                 strcmp(element->vr, "AT") == 0 ||
                 strcmp(element->vr, "CS") == 0 ||
                 strcmp(element->vr, "DA") == 0 ||
                 strcmp(element->vr, "DS") == 0 ||
                 strcmp(element->vr, "DT") == 0 ||
                 strcmp(element->vr, "LO") == 0 ||
                 strcmp(element->vr, "IS") == 0 ||
                 strcmp(element->vr, "PN") == 0 ||
                 strcmp(element->vr, "SH") == 0 ||
                 strcmp(element->vr, "ST") == 0 ||
                 strcmp(element->vr, "TM") == 0 ||
                 strcmp(element->vr, "UI") == 0) {
            if (element->value.str_multi) {
                for (i = 0; i < element->vm; i++) {
                    free(element->value.str_multi[i]);
                }
                free(element->value.str_multi);
            }
        } else if (strcmp(element->vr, "LT") == 0 ||
                 strcmp(element->vr, "OB") == 0 ||
                 strcmp(element->vr, "OD") == 0 ||
                 strcmp(element->vr, "OF") == 0 ||
                 strcmp(element->vr, "OV") == 0 ||
                 strcmp(element->vr, "UC") == 0 ||
                 strcmp(element->vr, "UN") == 0 ||
                 strcmp(element->vr, "UR") == 0 ||
                 strcmp(element->vr, "UT") == 0) {
            if (element->value.bytes) {
                free(element->value.bytes);
            }
        } else if (strcmp(element->vr, "FD") == 0) {
            if (element->value.fd_multi) {
                free(element->value.fd_multi);
            }
        } else if (strcmp(element->vr, "FL") == 0) {
            if (element->value.fl_multi) {
                free(element->value.fl_multi);
            }
        } else if (strcmp(element->vr, "SS") == 0) {
            if (element->value.ss_multi) {
                free(element->value.ss_multi);
            }
        } else if (strcmp(element->vr, "SV") == 0) {
            if (element->value.sv_multi) {
                free(element->value.sv_multi);
            }
        } else if (strcmp(element->vr, "US") == 0) {
            if (element->value.us_multi) {
                free(element->value.us_multi);
            }
        } else if (strcmp(element->vr, "UL") == 0) {
            if (element->value.ul_multi) {
                free(element->value.ul_multi);
            }
        } else if (strcmp(element->vr, "UV") == 0) {
            if (element->value.uv_multi) {
                free(element->value.uv_multi);
            }
        }
        free(element);
        element = NULL;
    }
}


uint16_t dcm_element_get_group_number(DcmElement *element)
{
    assert(element);
    return (uint16_t)(element->tag >> 16);
}


uint16_t dcm_element_get_element_number(DcmElement *element)
{
    assert(element);
    return (uint16_t)(element->tag);
}


uint32_t dcm_element_get_tag(DcmElement *element)
{
    assert(element);
    return element->tag;
}


bool dcm_element_check_vr(DcmElement *element, const char *vr)
{
    assert(element);
    if (strcmp(element->vr, vr) == 0) {
        return true;
    } else {
        return false;
    }
}


uint32_t dcm_element_get_vm(DcmElement *element)
{
    assert(element);
    return element->vm;
}

DcmElement *dcm_element_clone(DcmElement *element)
{
    assert(element);
    uint32_t i;
    DcmElement *clone = NULL;

    dcm_log_debug("Clone Data Element '%08X'.", element->tag);
    clone = malloc(sizeof(struct _DcmElement));
    if (clone == NULL) {
        dcm_log_error("Cloning of Data Element failed."
                      "Could not allocate memory for clone of "
                      "Data Element '%08X'.",
                      element->tag);
        return NULL;
    }
    clone->tag = element->tag;
    strcpy(clone->vr, element->vr);
    clone->length = element->length;
    clone->vm = element->vm;

    if (strcmp(element->vr, "SQ") == 0) {
        // Sequences
        if (element->value.sq) {
            DcmSequence *seq = NULL;
            DcmDataSet *ds = NULL;
            DcmDataSet *item = NULL;
            DcmElement *elem = NULL;
            // Copy each data set in sequence
            seq = dcm_sequence_create();
            ds = dcm_dataset_create();
            for (i = 0; i < dcm_sequence_count(element->value.sq); i++) {
                item = dcm_sequence_get(element->value.sq, i);
                // Copy each element in data set
                for(elem = item->elements; elem; elem = elem->hh.next) {
                    dcm_dataset_insert(ds, dcm_element_clone(elem));
                }
                dcm_sequence_append(seq, ds);
            }
            clone->value.sq = seq;
        }
    } else if (strcmp(element->vr, "AE") == 0 ||
               strcmp(element->vr, "AS") == 0 ||
               strcmp(element->vr, "AT") == 0 ||
               strcmp(element->vr, "CS") == 0 ||
               strcmp(element->vr, "DA") == 0 ||
               strcmp(element->vr, "DT") == 0 ||
               strcmp(element->vr, "LO") == 0 ||
               strcmp(element->vr, "PN") == 0 ||
               strcmp(element->vr, "SH") == 0 ||
               strcmp(element->vr, "ST") == 0 ||
               strcmp(element->vr, "TM") == 0 ||
               strcmp(element->vr, "UI") == 0) {
        // Character Strings
        if (element->value.str_multi) {
            clone->value.str_multi = malloc(element->vm * sizeof(char *));
            if (clone->value.str_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.str_multi[i] = malloc(
                    strlen(element->value.str_multi[i]) + 1
                );
                if (clone->value.str_multi[i] == NULL) {
                    dcm_log_error("Cloning of Data Element failed."
                                  "Could not allocate memory for value of clone "
                                  "clone of Data Element '%08X'.",
                                  element->tag);
                    free(clone);
                    return NULL;
                }
                strcpy(clone->value.str_multi[i],
                       element->value.str_multi[i]);
            }
        }
    } else if (strcmp(element->vr, "LT") == 0 ||
               strcmp(element->vr, "OB") == 0 ||
               strcmp(element->vr, "OD") == 0 ||
               strcmp(element->vr, "OF") == 0 ||
               strcmp(element->vr, "OV") == 0 ||
               strcmp(element->vr, "UC") == 0 ||
               strcmp(element->vr, "UN") == 0 ||
               strcmp(element->vr, "UR") == 0 ||
               strcmp(element->vr, "UT") == 0) {
        // Bytes
        if (element->value.bytes) {
            clone->value.bytes = malloc(element->length);
            if (clone->value.bytes == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            memcpy(clone->value.bytes,
                   element->value.bytes,
                   element->length);
        }
    } else if (strcmp(element->vr, "FD") == 0) {
        if (element->value.fd_multi) {
            clone->value.fd_multi = malloc(element->vm * sizeof(double));
            if (clone->value.fd_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.fd_multi[i] = element->value.fd_multi[i];
            }
        }
    } else if (strcmp(element->vr, "FL") == 0) {
        if (element->value.fl_multi) {
            clone->value.fl_multi = malloc(element->vm * sizeof(float));
            if (clone->value.fl_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.fl_multi[i] = element->value.fl_multi[i];
            }
        }
    } else if (strcmp(element->vr, "SS") == 0) {
        if (element->value.ss_multi) {
            clone->value.ss_multi = malloc(element->vm * sizeof(int16_t));
            if (clone->value.ss_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.ss_multi[i] = element->value.ss_multi[i];
            }
        }
    } else if (strcmp(element->vr, "SV") == 0) {
        if (element->value.sv_multi) {
            clone->value.sv_multi = malloc(element->vm * sizeof(int64_t));
            if (clone->value.sv_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.sv_multi[i] = element->value.sv_multi[i];
            }
        }
    } else if (strcmp(element->vr, "US") == 0) {
        if (element->value.us_multi) {
            clone->value.us_multi = malloc(element->vm * sizeof(uint16_t));
            if (clone->value.us_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.us_multi[i] = element->value.us_multi[i];
            }
        }
    } else if (strcmp(element->vr, "UL") == 0) {
        if (element->value.ul_multi) {
            clone->value.ul_multi = malloc(element->vm * sizeof(uint32_t));
            if (clone->value.ul_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.ul_multi[i] = element->value.ul_multi[i];
            }
        }
    } else if (strcmp(element->vr, "UV") == 0) {
        if (element->value.uv_multi) {
            clone->value.uv_multi = malloc(element->vm * sizeof(uint64_t));
            if (clone->value.uv_multi == NULL) {
                dcm_log_error("Cloning of Data Element failed."
                              "Could not allocate memory for value of clone "
                              "clone of Data Element '%08X'.",
                              element->tag);
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.uv_multi[i] = element->value.uv_multi[i];
            }
        }
    }

    return clone;
}


bool dcm_element_is_multivalued(DcmElement *element)
{
    assert(element);
    return element->vm > 1;
}


uint32_t dcm_element_get_length(DcmElement *element)
{
    assert(element);
    return element->length;
}


static inline void assert_vr(DcmElement *element, const char *vr)
{
    DEBUG_ONLY(bool success =) dcm_element_check_vr(element, vr);
    assert(success);
}


static inline bool check_value_index(DcmElement *element, uint32_t index)
{
    return index <= (element->vm - 1);
}


static inline void assert_value_index(DcmElement *element, uint32_t index)
{
    DEBUG_ONLY(bool success =) check_value_index(element, index);
    assert(success);
}


// Data Elements with character string Value Representation

static bool check_value_str_multi(DcmElement *element,
                                  char **values,
                                  uint32_t vm,
                                  uint32_t capacity)
{
    uint32_t i;
    size_t actual_length;
    char *v = NULL;

    for (i = 0; i < vm; i++) {
        v = values[i];
        actual_length = strlen(v);
        if (actual_length > (capacity + 1)) {
            dcm_log_warning("Checking value of Data Element failed. "
                            "Value #%d of Data Element '%08X' exceeds "
                            "maximum length of Value Representation '%s' "
                            "(%d > %d).",
                            i + 1,
                            element->tag,
                            element->vr,
                            actual_length - 1,
                            capacity);
            return false;
        }
    }
    return true;
}


static bool set_value_str_multi(DcmElement *element,
                                char **values,
                                uint32_t vm,
                                uint32_t capacity) {
    assert(element);
    assert(values);
    uint32_t i;

    if (!check_value_str_multi(element, values, vm, capacity)) {
        if (values) {
            for (i = 0; i < vm; i++) {
                if (values[i]) {
                    free(values[i]);
                }
            }
            free(values);
        }
        return false;
    }
    element->value.str_multi = values;
    element->vm = vm;
    return true;
}


static DcmElement *create_element_str(uint32_t tag,
                                      const char *vr,
                                      char *value,
                                      uint32_t capacity)
{
    uint32_t vm = 1;
    uint32_t length = 0;
    DcmElement *element = NULL;
    char **values = NULL;

    values = malloc(sizeof(char *));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(value);
        return NULL;
    }
    values[0] = value;

    length = strlen(value);
    element = create_element(tag, vr, length);
    if (element == NULL) {
        free(value);
        if (values[0]) {
            free(values[0]);
        }
        free(values);
        return NULL;
    }
    if (!set_value_str_multi(element, values, vm, capacity)) {
        free(value);
        dcm_element_destroy(element);
        return NULL;
    }
    return element;
}

static DcmElement *create_element_str_multi(uint32_t tag,
                                            const char *vr,
                                            char **values,
                                            uint32_t vm,
                                            uint32_t capacity)
{
    uint32_t i;
    uint32_t length = 0;
    char *v = NULL;
    DcmElement *element = NULL;

    for (i = 0; i < vm; i++) {
        v = values[i];
        length += strlen(v);
        if (i < (vm - 1)) {
            // Separator "\\"
            length += 2;
        }
    }
    element = create_element(tag, vr, length);
    if (element == NULL) {
        for (i = 0; i < vm; i++) {
            free(values[i]);
        }
        free(values);
        return NULL;
    }
    if (!set_value_str_multi(element, values, vm, capacity)) {
        dcm_element_destroy(element);
        return NULL;
    }
    return element;
}


DcmElement *dcm_element_create_AE(uint32_t tag, char *value)
{
    return create_element_str(tag, "AE", value, DCM_CAPACITY_AE);
}


DcmElement *dcm_element_create_AE_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "AE", values, vm, DCM_CAPACITY_AE);
}


DcmElement *dcm_element_create_AS(uint32_t tag, char *value)
{
    return create_element_str(tag, "AS", value, DCM_CAPACITY_AS);
}


DcmElement *dcm_element_create_AS_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "AS", values, vm, DCM_CAPACITY_AS);
}


DcmElement *dcm_element_create_AT(uint32_t tag, char *value)
{
    return create_element_str(tag, "AT", value, DCM_CAPACITY_AT);
}


DcmElement *dcm_element_create_AT_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "AT", values, vm, DCM_CAPACITY_AT);
}


DcmElement *dcm_element_create_CS(uint32_t tag, char *value)
{
    return create_element_str(tag, "CS", value, DCM_CAPACITY_CS);
}


DcmElement *dcm_element_create_CS_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "CS", values, vm, DCM_CAPACITY_CS);
}


DcmElement *dcm_element_create_DA(uint32_t tag, char *value)
{
    return create_element_str(tag, "DA", value, DCM_CAPACITY_DA);
}


DcmElement *dcm_element_create_DA_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "DA", values, vm, DCM_CAPACITY_DA);
}


DcmElement *dcm_element_create_DT(uint32_t tag, char *value)
{
    return create_element_str(tag, "DT", value, DCM_CAPACITY_DT);
}


DcmElement *dcm_element_create_DT_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "DT", values, vm, DCM_CAPACITY_DT);
}


DcmElement *dcm_element_create_LO(uint32_t tag, char *value)
{
    return create_element_str(tag, "LO", value, DCM_CAPACITY_LO);
}


DcmElement *dcm_element_create_LO_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "LO", values, vm, DCM_CAPACITY_LO);
}


DcmElement *dcm_element_create_PN(uint32_t tag, char *value)
{
    return create_element_str(tag, "PN", value, DCM_CAPACITY_PN);
}



DcmElement *dcm_element_create_PN_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "PN", values, vm, DCM_CAPACITY_PN);
}


DcmElement *dcm_element_create_SH(uint32_t tag, char *value)
{
    return create_element_str(tag, "SH", value, DCM_CAPACITY_SH);
}


DcmElement *dcm_element_create_SH_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "SH", values, vm, DCM_CAPACITY_SH);
}


DcmElement *dcm_element_create_TM(uint32_t tag, char *value)
{
    return create_element_str(tag, "TM", value, DCM_CAPACITY_TM);
}


DcmElement *dcm_element_create_TM_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "TM", values, vm, DCM_CAPACITY_TM);
}


DcmElement *dcm_element_create_ST(uint32_t tag, char *value)
{
    return create_element_str(tag, "ST", value, DCM_CAPACITY_ST);
}


DcmElement *dcm_element_create_UI(uint32_t tag, char *value)
{
    return create_element_str(tag, "UI", value, DCM_CAPACITY_UI);
}


DcmElement *dcm_element_create_UI_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "UI", values, vm, DCM_CAPACITY_UI);
}


void dcm_element_copy_value_UI(DcmElement *element,
                               uint32_t index,
                               char *value)
{
    assert(element);
    assert_vr(element, "UI");
    assert_value_index(element, index);
    strcpy(value, element->value.str_multi[index]);
    value[strlen(element->value.str_multi[index])] = '\0';
}


void dcm_element_copy_value_CS(DcmElement *element,
                               uint32_t index,
                               char *value)
{
    assert(element);
    assert_vr(element, "CS");
    assert_value_index(element, index);
    strcpy(value, element->value.str_multi[index]);
    value[strlen(element->value.str_multi[index])] = '\0';
}


void dcm_element_copy_value_ST(DcmElement *element, char *value)
{
    assert(element);
    assert_vr(element, "ST");
    assert_value_index(element, 0);
    strcpy(value, element->value.str_multi[0]);
    value[strlen(element->value.str_multi[0])] = '\0';
}


static inline void print_element_value_AE(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_AT(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_AS(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_CS(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_DA(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_DT(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_LO(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_PN(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_SH(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_TM(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_UI(DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}



// Data Elements with numeric Value Representation

DcmElement *dcm_element_create_FD(uint32_t tag, double value)
{
    uint32_t length = sizeof(double);
    double *values = malloc(sizeof(double));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "FD", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.fd_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_FD_multi(uint32_t tag,
                                        double *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(double);
    DcmElement *element = create_element(tag, "FD", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.fd_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_FL(uint32_t tag, float value)
{
    uint32_t length = sizeof(double);
    float *values = malloc(sizeof(float));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "FL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.fl_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_FL_multi(uint32_t tag,
                                        float *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(float);
    DcmElement *element = create_element(tag, "FL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.fl_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_DS(uint32_t tag, char *value)
{
    return create_element_str(tag, "DS", value, DCM_CAPACITY_DS);
}


DcmElement *dcm_element_create_DS_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "DS", values, vm, DCM_CAPACITY_DS);
}


DcmElement *dcm_element_create_IS(uint32_t tag, char *value)
{
    return create_element_str(tag, "IS", value, DCM_CAPACITY_IS);
}


DcmElement *dcm_element_create_IS_multi(uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(tag, "IS", values, vm, DCM_CAPACITY_IS);
}


DcmElement *dcm_element_create_SS(uint32_t tag, int16_t value)
{
    uint32_t length = sizeof(int16_t);
    int16_t *values = malloc(sizeof(int16_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "SS", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.ss_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SS_multi(uint32_t tag,
                                        int16_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int16_t);
    DcmElement *element = create_element(tag, "SS", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.ss_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_SL(uint32_t tag, int32_t value)
{
    uint32_t length = sizeof(int32_t);
    int32_t *values = malloc(sizeof(int32_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "SL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.sl_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SL_multi(uint32_t tag,
                                        int32_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int32_t);
    DcmElement *element = create_element(tag, "SL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.sl_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_SV(uint32_t tag, int64_t value)
{
    uint32_t length = sizeof(int64_t);
    int64_t *values = malloc(sizeof(int64_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "SV", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.sv_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SV_multi(uint32_t tag,
                                        int64_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int64_t);
    DcmElement *element = create_element(tag, "SV", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.sv_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_UL(uint32_t tag, uint32_t value)
{
    uint32_t length = sizeof(uint32_t);
    uint32_t *values = malloc(sizeof(uint32_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "UL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.ul_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UL_multi(uint32_t tag,
                                        uint32_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint32_t);
    DcmElement *element = create_element(tag, "UL", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.ul_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_US(uint32_t tag, uint16_t value)
{
    uint32_t length = sizeof(uint16_t);
    uint16_t *values = malloc(sizeof(uint16_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "US", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.us_multi = values;
    element->vm = 1;
    return element;
}

DcmElement *dcm_element_create_US_multi(uint32_t tag,
                                        uint16_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint16_t);
    DcmElement *element = create_element(tag, "US", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.us_multi = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_UV(uint32_t tag, uint64_t value)
{
    uint32_t length = sizeof(uint64_t);
    uint64_t *values = malloc(sizeof(uint64_t));
    if (values == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(tag, "UV", length);
    if (element == NULL) {
        dcm_log_error("Creation of Data Element failed. "
                      "Could not allocate memory.");
        free(values);
        return NULL;
    }
    element->value.uv_multi = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UV_multi(uint32_t tag,
                                        uint64_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint64_t);
    DcmElement *element = create_element(tag, "UV", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.uv_multi = values;
    element->vm = vm;
    return element;
}


void dcm_element_copy_value_FD(DcmElement *element,
                               uint32_t index,
                               double *value)
{
    assert_vr(element, "FD");
    assert_value_index(element, index);
    *value = element->value.fd_multi[index];
}


void dcm_element_copy_value_FL(DcmElement *element,
                               uint32_t index,
                               float *value)
{
    assert_vr(element, "FL");
    assert_value_index(element, index);
    *value = element->value.fl_multi[index];
}


void dcm_element_copy_value_IS(DcmElement *element,
                               uint32_t index,
                               char *value)
{
    assert_vr(element, "IS");
    assert_value_index(element, index);
    strcpy(value, element->value.str_multi[index]);
    value[strlen(element->value.str_multi[index])] = '\0';
}


void dcm_element_copy_value_SS(DcmElement *element,
                               uint32_t index,
                               int16_t *value)
{
    assert_vr(element, "SS");
    assert_value_index(element, index);
    *value = element->value.ss_multi[index];
}


void dcm_element_copy_value_SL(DcmElement *element,
                               uint32_t index,
                               int32_t *value)
{
    assert_vr(element, "SL");
    assert_value_index(element, index);
    *value = element->value.ss_multi[index];
}


void dcm_element_copy_value_SV(DcmElement *element,
                               uint32_t index,
                               int64_t *value)
{
    assert_vr(element, "SV");
    assert_value_index(element, index);
    *value = element->value.sv_multi[index];
}


void dcm_element_copy_value_UL(DcmElement *element,
                               uint32_t index,
                               uint32_t *value)
{
    assert_vr(element, "UL");
    assert_value_index(element, index);
    *value = element->value.ul_multi[index];
}


void dcm_element_copy_value_US(DcmElement *element,
                               uint32_t index,
                               uint16_t *value)
{
    assert_vr(element, "US");
    assert_value_index(element, index);
    *value = element->value.us_multi[index];
}


void dcm_element_copy_value_UV(DcmElement *element,
                               uint32_t index,
                               uint16_t *value)
{
    assert_vr(element, "UV");
    assert_value_index(element, index);
    *value = element->value.uv_multi[index];
}


static void print_element_value_DS(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%s", element->value.str_multi[index]);
}


static void print_element_value_FD(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%f", element->value.fd_multi[index]);
}


static void print_element_value_FL(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%f", element->value.fl_multi[index]);
}


static void print_element_value_IS(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%s", element->value.str_multi[index]);
}


static void print_element_value_SS(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%d", element->value.ss_multi[index]);
}


static void print_element_value_SL(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%d", element->value.sl_multi[index]);
}


static void print_element_value_SV(DcmElement *element, uint32_t index)
{
    assert(element);
    // Cast seems necessary for some platforms
    printf("%lld", (long long)element->value.sv_multi[index]);
}


static void print_element_value_US(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%hu", element->value.us_multi[index]);
}


static void print_element_value_UL(DcmElement *element, uint32_t index)
{
    assert(element);
    printf("%u", element->value.ul_multi[index]);
}



// Data Elements with binary Value Representation

static void set_value_bytes(DcmElement *element, char *value)
{
    assert(element);
    assert(value);
    element->value.bytes = value;
}


DcmElement *dcm_element_create_LT(uint32_t tag, char *value)
{
    return create_element_str(tag, "LT", value, DCM_CAPACITY_LT);
}


DcmElement *dcm_element_create_OB(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "OB", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OD(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "OD", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OF(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "OF", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OV(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "OV", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OW(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "OW", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UC(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "UC", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UN(uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(tag, "UN", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UR(uint32_t tag, char *value)
{
    return create_element_str(tag, "UR", value, DCM_CAPACITY_UR);
}


DcmElement *dcm_element_create_UT(uint32_t tag, char *value)
{
    return create_element_str(tag, "UT", value, DCM_CAPACITY_UT);
}


void dcm_element_copy_value_OB(DcmElement *element,
                               char *value,
                               ssize_t n)
{
    assert_vr(element, "OB");
    memmove(value, element->value.bytes, n);
}


// Sequence Data Element

DcmElement *dcm_element_create_SQ(uint32_t tag, DcmSequence *value)
{
    uint32_t length = 0;
    uint32_t seq_length = 0;
    uint32_t i;
    DcmDataSet *item = NULL;
    DcmElement *element = NULL;
    DcmElement *e = NULL;

    seq_length = dcm_sequence_count(value);
    for (i = 0; i < seq_length; i++) {
        item = dcm_sequence_get(value, i);
        for (e = item->elements; e; e = e->hh.next) {
            length += e->length;
        }
    }
    element = create_element(tag, "SQ", length);
    if (element == NULL) {
        dcm_sequence_destroy(value);
        return NULL;
    }
    element->value.sq = value;
    element->vm = 1;
    return element;
}


DcmSequence *dcm_element_get_value_SQ(DcmElement *element)
{
    assert(element);
    assert_vr(element, "SQ");
    assert(element->value.sq);
    element->value.sq->is_locked = true;
    return element->value.sq;
}


void dcm_element_print(DcmElement *element, uint8_t indentation)
{
    assert(element);
    uint32_t i;
    uint32_t n_items;
    uint8_t num_indent = indentation * 2;
    uint8_t num_indent_next = (indentation + 1) * 2;
    DcmSequence *sequence = NULL;

    if (dcm_is_public_tag(element->tag)) {
        const char *keyword = dcm_dict_lookup_keyword(element->tag);
        printf("%*.*s(%04X,%04X) %s | %s",
               num_indent,
               num_indent,
               "                                   ",
               dcm_element_get_group_number(element),
               dcm_element_get_element_number(element),
               keyword,
               element->vr);
    } else {
        printf("%*.*s (%04X,%04X) | %s",
               num_indent,
               num_indent,
               "                                   ",
               dcm_element_get_group_number(element),
               dcm_element_get_element_number(element),
               element->vr);
    }

    if (strcmp(element->vr, "SQ") != 0) {
        printf(" | %u | ", element->length);

        if (element->vm > 1) {
            printf("[");
        }
        for (i = 0; i < element->vm; i++) {
            if (strcmp(element->vr, "AE") == 0) {
                print_element_value_AE(element, i);
            } else if (strcmp(element->vr, "AS") == 0) {
                print_element_value_AS(element, i);
            } else if (strcmp(element->vr, "AT") == 0) {
                print_element_value_AT(element, i);
            } else if (strcmp(element->vr, "CS") == 0) {
                print_element_value_CS(element, i);
            } else if (strcmp(element->vr, "DA") == 0) {
                print_element_value_DA(element, i);
            } else if (strcmp(element->vr, "DS") == 0) {
                print_element_value_DS(element, i);
            } else if (strcmp(element->vr, "DT") == 0) {
                print_element_value_DT(element, i);
            } else if (strcmp(element->vr, "FD") == 0) {
                print_element_value_FD(element, i);
            } else if (strcmp(element->vr, "FL") == 0) {
                print_element_value_FL(element, i);
            } else if (strcmp(element->vr, "IS") == 0) {
                print_element_value_IS(element, i);
            } else if (strcmp(element->vr, "LO") == 0) {
                print_element_value_LO(element, i);
            } else if (strcmp(element->vr, "PN") == 0) {
                print_element_value_PN(element, i);
            } else if (strcmp(element->vr, "SH") == 0) {
                print_element_value_SH(element, i);
            } else if (strcmp(element->vr, "SS") == 0) {
                print_element_value_SS(element, i);
            } else if (strcmp(element->vr, "SL") == 0) {
                print_element_value_SL(element, i);
            } else if (strcmp(element->vr, "SV") == 0) {
                print_element_value_SV(element, i);
            } else if (strcmp(element->vr, "TM") == 0) {
                print_element_value_TM(element, i);
            } else if (strcmp(element->vr, "UI") == 0) {
                print_element_value_UI(element, i);
            } else if (strcmp(element->vr, "US") == 0) {
                print_element_value_US(element, i);
            } else if (strcmp(element->vr, "UL") == 0) {
                print_element_value_UL(element, i);
            }

            if (element->vm > 1) {
                if (i == (element->vm - 1)) {
                    printf("]");
                } else {
                    printf(", ");
                }
            }
        }
        printf("\n");
    } else {
        sequence = dcm_element_get_value_SQ(element);
        n_items = dcm_sequence_count(sequence);
        if (n_items == 0) {
            printf(" | [");
        } else {
            printf(" | [\n");
        }
        for (i = 0; i < n_items; i++) {
            printf("%*.*s---Item #%d---\n",
                   num_indent_next,
                   num_indent_next,
                   "                                   ",
                   i + 1);
            DcmDataSet *item = dcm_sequence_get(sequence, i);

            dcm_dataset_print(item, indentation+1);
        }
        printf("%*.*s]\n",
               num_indent,
               num_indent,
               "                                   ");
    }
}


// Datasets

DcmDataSet *dcm_dataset_create(void)
{
    DcmDataSet *dataset = NULL;

    dcm_log_debug("Create Data Set.");
    dataset = malloc(sizeof(struct _DcmDataSet));
    if (dataset == NULL) {
        dcm_log_error("Creation of Data Set failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    dataset->elements = NULL;
    dataset->is_locked = false;
    return dataset;
}


bool dcm_dataset_insert(DcmDataSet *dataset, DcmElement *element)
{
    assert(dataset);
    assert(element);
    DcmElement *matched_element = NULL;

    dcm_log_debug("Insert Data Element '%08X' into Data Set.", element->tag);
    if (dataset->is_locked) {
        dcm_log_error("Inserting Data Element into Data Set failed. "
                      "Data Set is locked and cannot be modified.");
        dcm_element_destroy(element);
        return false;
    }

    HASH_FIND_INT(dataset->elements, &element->tag, matched_element);
    if (matched_element) {
        dcm_log_warning("Inserting Data Element '%08X' into Data Set failed. "
                        "Element already exists.",
                        element->tag);
        dcm_element_destroy(element);
        return false;
    }

    HASH_ADD_INT(dataset->elements, tag, element);

    return true;
}


bool dcm_dataset_remove(DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);

    DcmElement *matched_element = NULL;

    dcm_log_debug("Remove Data Element '%08X' from Data Set.", tag);
    if (dataset->is_locked) {
        dcm_log_error("Removing Data Element from Data Set failed. "
                      "Data Set is locked and cannot be modified.");
        exit(1);
    }

    matched_element = dcm_dataset_get(dataset, tag);
    if (matched_element == NULL) {
        dcm_log_warning("Removing Data Element '%08X' from Data Set failed. "
                        "Could not find Data Element.",
                        tag);
        return false;
    }

    HASH_DEL(dataset->elements, matched_element);
    dcm_element_destroy(matched_element);

    return true;
}


DcmElement *dcm_dataset_get_clone(DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);
    DcmElement *element = NULL;

    dcm_log_debug("Copy Data Element '%08X' from Data Set.", tag);
    HASH_FIND_INT(dataset->elements, &tag, element);
    if (element == NULL) {
        dcm_log_warning("Getting Data Element '%08X' from Data Set failed. "
                        "Could not find Data Element.",
                        tag);
    }
    return dcm_element_clone(element);
}


DcmElement *dcm_dataset_get(DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);
    DcmElement *element = NULL;

    dcm_log_debug("Get Data Element '%08X' from Data Set.", tag);
    HASH_FIND_INT(dataset->elements, &tag, element);
    if (element == NULL) {
        dcm_log_warning("Getting Data Element '%08X' from Data Set failed. "
                        "Could not find Data Element.",
                        tag);
    }

    return element;
}


void dcm_dataset_foreach(DcmDataSet *dataset,
                         void (*fn)(DcmElement *element))
{
    assert(dataset);
    DcmElement *e = NULL;

    HASH_SORT(dataset->elements, compare_tags);
    for(e = dataset->elements; e; e = e->hh.next) {
        fn(e);
    }
}


bool dcm_dataset_contains(DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);
    DcmElement *matched_element = NULL;

    matched_element = dcm_dataset_get(dataset, tag);
    if (matched_element == NULL) {
        return false;
    }
    return true;
}


uint32_t dcm_dataset_count(DcmDataSet *dataset)
{
    assert(dataset);
    uint32_t num_users;

    num_users = HASH_COUNT(dataset->elements);
    return num_users;
}


void dcm_dataset_copy_tags(DcmDataSet *dataset, uint32_t *tags)
{
    assert(dataset);
    uint32_t i = 0;
    DcmElement *e = NULL;

    HASH_SORT(dataset->elements, compare_tags);
    for(e = dataset->elements; e; e = e->hh.next) {
        tags[i] = e->tag;
        i += 1;
    }
    return;
}


void dcm_dataset_print(DcmDataSet *dataset, uint8_t indentation)
{
    assert(dataset);
    DcmElement *e = NULL;

    HASH_SORT(dataset->elements, compare_tags);
    for(e = dataset->elements; e; e = e->hh.next) {
        dcm_element_print(e, indentation);
    }
}


void dcm_dataset_lock(DcmDataSet *dataset)
{
    dataset->is_locked = true;
}


bool dcm_dataset_is_locked(DcmDataSet *dataset)
{
    return dataset->is_locked;
}


void dcm_dataset_destroy(DcmDataSet *dataset)
{
    DcmElement *current_element = NULL, *tmp = NULL;

    if (dataset) {
        HASH_ITER(hh, dataset->elements, current_element, tmp) {
            HASH_DEL(dataset->elements, current_element);
            dcm_element_destroy(current_element);
        }
        free(dataset);
        dataset = NULL;
    }
}


// Sequences

DcmSequence *dcm_sequence_create(void)
{
    DcmSequence *seq;
    UT_array *items;

    seq = malloc(sizeof(struct _DcmSequence));
    if (seq == NULL) {
        dcm_log_error("Creation of Sequence failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    utarray_new(items, &sequence_item_icd);
    if (items == NULL) {
        dcm_log_error("Creation of Sequence failed. "
                      "Could not allocate memory.");
        free(seq);
        return NULL;
    }
    seq->items = items;
    seq->is_locked = false;
    return seq;
}


bool dcm_sequence_append(DcmSequence *seq, DcmDataSet *item)
{
    assert(seq);
    assert(item);

    dcm_log_debug("Append item to Sequence.");
    if (seq->is_locked) {
        dcm_log_error("Appending item to Sequence failed. "
                      "Sequence is locked and cannot be modified.");
        dcm_dataset_destroy(item);
        return false;
    }

    struct SequenceItem *item_handle = create_sequence_item(item);
    utarray_push_back(seq->items, item_handle);

    return true;
}


DcmDataSet *dcm_sequence_get(DcmSequence *seq, uint32_t index)
{
    assert(seq);
    uint32_t length;
    struct SequenceItem *item_handle = NULL;

    dcm_log_debug("Get item #%i of Sequence.", index);
    length = utarray_len(seq->items);
    if (index >= length) {
        dcm_log_error("Getting item of Sequence failed. "
                      "Index %i exceeds length of sequence %i.",
                      index, length);
        exit(1);
    }
    item_handle = utarray_eltptr(seq->items, index);
    item_handle->dataset->is_locked = true;
    return item_handle->dataset;
}


void dcm_sequence_foreach(DcmSequence *seq,
                          void (*fn)(DcmDataSet *item))
{
    assert(seq);
    uint32_t i;
    uint32_t length;
    struct SequenceItem *item_handle = NULL;

    length = utarray_len(seq->items);
    for (i = 0; i < length; i++) {
        item_handle = utarray_eltptr(seq->items, i);
        item_handle->dataset->is_locked = true;
        fn(item_handle->dataset);
    }
}


void dcm_sequence_remove(DcmSequence *seq, uint32_t index)
{
    assert(seq);
    if (seq->is_locked) {
        dcm_log_error("Removing item from Sequence failed. "
                      "Sequence is locked and cannot be modified.");
        exit(1);
    }
    dcm_log_debug("Remove item #%i from Sequence.", index);
    uint32_t length = utarray_len(seq->items);
    if (index >= length) {
        dcm_log_error("Removing item from Sequence failed. "
                      "Index %i exceeds length of sequence %i.",
                      index, length);
        exit(1);
    }
    utarray_erase(seq->items, index, 1);
}


uint32_t dcm_sequence_count(DcmSequence *seq)
{
    uint32_t length = utarray_len(seq->items);
    return length;
}


void dcm_sequence_lock(DcmSequence *seq)
{
    seq->is_locked = true;
}


bool dcm_sequence_is_locked(DcmSequence *seq)
{
    return seq->is_locked;
}


void dcm_sequence_destroy(DcmSequence *seq)
{
    if (seq) {
        utarray_free(seq->items);
        seq->items = NULL;
        free(seq);
        seq = NULL;
    }
}


// Frames

DcmFrame *dcm_frame_create(uint32_t number,
                           char *data,
                           uint32_t length,
                           uint16_t rows,
                           uint16_t columns,
                           uint16_t samples_per_pixel,
                           uint16_t bits_allocated,
                           uint16_t bits_stored,
                           uint16_t pixel_representation,
                           uint16_t planar_configuration,
                           const char *photometric_interpretation,
                           const char *transfer_syntax_uid)
{
    DcmFrame *frame = NULL;

    if (data == NULL || length == 0) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Pixel data cannot be empty.");
        return NULL;
    }
    if (!(bits_allocated == 1 || bits_allocated % 8 == 0)) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Wrong number of bits allocated.");
        free(data);
        return NULL;
    }
    if (!(bits_stored == 1 || bits_stored % 8 == 0)) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Wrong number of bits stored.");
        return NULL;
    }
    if (!(pixel_representation == 0 || pixel_representation == 1)) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Wrong pixel representation.");
        return NULL;
    }
    if (!(planar_configuration == 0 || planar_configuration == 1)) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Wrong planar configuration.");
        return NULL;
    }

    frame = malloc(sizeof(struct _DcmFrame));
    if (frame == NULL) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Could not allocate memory.");
        return NULL;
    }
    frame->number = number;
    frame->data = data;
    frame->length = length;
    frame->rows = rows;
    frame->columns = columns;
    frame->samples_per_pixel = samples_per_pixel;
    frame->bits_allocated = bits_allocated;
    frame->bits_stored = bits_stored;
    frame->high_bit = bits_stored - 1;
    frame->pixel_representation = pixel_representation;
    frame->planar_configuration = planar_configuration;

    frame->photometric_interpretation = malloc(DCM_CAPACITY_CS + 1);
    if (frame->photometric_interpretation == NULL) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Could not allocate memory.");
        free(data);
        free((char *)transfer_syntax_uid);
        free(frame);
        return NULL;
    }
    strcpy(frame->photometric_interpretation, photometric_interpretation);
    frame->photometric_interpretation[DCM_CAPACITY_CS] = '\0';

    frame->transfer_syntax_uid = malloc(DCM_CAPACITY_UI + 1);
    if (frame->transfer_syntax_uid == NULL) {
        dcm_log_error("Constructing Frame Item failed. "
                      "Could not allocate memory.");
        free(data);
        free((char *)transfer_syntax_uid);
        free(frame->photometric_interpretation);
        free(frame);
        return NULL;
    }
    strcpy(frame->transfer_syntax_uid, transfer_syntax_uid);
    frame->transfer_syntax_uid[DCM_CAPACITY_UI] = '\0';

    return frame;
}


uint32_t dcm_frame_get_number(DcmFrame *frame)
{
    assert(frame);
    return frame->number;
}

uint32_t dcm_frame_get_length(DcmFrame *frame)
{
    assert(frame);
    return frame->length;
}

uint16_t dcm_frame_get_rows(DcmFrame *frame)
{
    assert(frame);
    return frame->rows;
}

uint16_t dcm_frame_get_columns(DcmFrame *frame)
{
    assert(frame);
    return frame->columns;
}

uint16_t dcm_frame_get_samples_per_pixel(DcmFrame *frame)
{
    assert(frame);
    return frame->samples_per_pixel;
}

uint16_t dcm_frame_get_bits_allocated(DcmFrame *frame)
{
    assert(frame);
    return frame->bits_allocated;
}

uint16_t dcm_frame_get_bits_stored(DcmFrame *frame)
{
    assert(frame);
    return frame->bits_stored;
}

uint16_t dcm_frame_get_high_bit(DcmFrame *frame)
{
    assert(frame);
    return frame->high_bit;
}

uint16_t dcm_frame_get_pixel_representation(DcmFrame *frame)
{
    assert(frame);
    return frame->pixel_representation;
}

uint16_t dcm_frame_get_planar_configuration(DcmFrame *frame)
{
    assert(frame);
    return frame->planar_configuration;
}

const char *dcm_frame_get_photometric_interpretation(DcmFrame *frame)
{
    assert(frame);
    return frame->photometric_interpretation;
}

const char *dcm_frame_get_transfer_syntax_uid(DcmFrame *frame)
{
    assert(frame);
    return frame->transfer_syntax_uid;
}


const char *dcm_frame_get_value(DcmFrame *frame)
{
    assert(frame);
    return frame->data;
}


void dcm_frame_destroy(DcmFrame *frame)
{
    if (frame) {
        if (frame->data) {
            free((char*)frame->data);
        }
        if (frame->photometric_interpretation) {
            free(frame->photometric_interpretation);
        }
        if (frame->transfer_syntax_uid) {
            free(frame->transfer_syntax_uid);
        }
        free(frame);
        frame = NULL;
    }
}



// Basic Offset Table

DcmBOT *dcm_bot_create(ssize_t *offsets, uint32_t num_frames)
{
    DcmBOT *bot = malloc(sizeof(struct _DcmBOT));
    if (offsets == NULL) {
        dcm_log_error("Constructing Basic Offset Table failed. "
                      "No offsets were provided.");
        return NULL;
    }
    if (bot == NULL) {
        dcm_log_error("Constructing Basic Offset Table failed. "
                      "Could not allocate memory.");
        free(offsets);
        return NULL;
    }
    if (num_frames == 0) {
        dcm_log_error("Constructing Basic Offset Table failed. "
                      "Expected offsets of %ld Frame Items.",
                      num_frames);
        free(offsets);
        free(bot);
        return NULL;
    }
    bot->num_frames = num_frames;
    bot->offsets = offsets;
    return bot;
}


void dcm_bot_print(DcmBOT *bot)
{
    assert(bot);
    printf("[");
    uint32_t i;
    for(i = 0; i < bot->num_frames; i++) {
        if (i == (bot->num_frames - 1)) {
            printf("%zd]\n", bot->offsets[i]);
        } else {
            printf("%zd,", bot->offsets[i]);
        }
    }
}


uint32_t dcm_bot_get_num_frames(DcmBOT *bot)
{
    assert(bot);
    return bot->num_frames;
}


ssize_t dcm_bot_get_frame_offset(DcmBOT *bot, uint32_t number)
{
    assert(bot);
    assert(number > 0 && number < bot->num_frames);
    uint32_t index = number - 1;
    return bot->offsets[index];
}


void dcm_bot_destroy(DcmBOT *bot)
{
    if (bot) {
        if (bot->offsets) {
            free(bot->offsets);
        }
        free(bot);
        bot = NULL;
    }
}


bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid) {
    if (strcmp(transfer_syntax_uid, "1.2.840.10008.1.2") == 0 ||
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1") == 0 ||
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1.99") == 0 ||
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.2") == 0) {
        return false;
    }
    return true;
}
