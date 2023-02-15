/*
 * Implementation of Part 5 of the DICOM standard: Data Structures and Encoding.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strdup and strcpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "utarray.h"
#include "uthash.h"

#include <dicom/dicom.h>
#include "pdicom.h"


struct _DcmElement {
    uint32_t tag;
    char vr[3];
    uint32_t length;
    uint32_t vm;
    union {
        // Numeric value (multiplicity 1-n)
        float *fl_multi;
        double *fd_multi;
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
    void *value_pointer;
    char **value_pointer_array;
    DcmSequence *sequence_pointer;
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
    const char *photometric_interpretation;
    const char *transfer_syntax_uid;
};


struct _DcmBOT {
    uint32_t num_frames;
    ssize_t *offsets;
};


struct SequenceItem {
    DcmDataSet *dataset;
};


static bool is_vr_string(const char *vr) 
{
    if (strcmp(vr, "AE") == 0 ||
        strcmp(vr, "AS") == 0 ||
        strcmp(vr, "AT") == 0 ||
        strcmp(vr, "CS") == 0 ||
        strcmp(vr, "DA") == 0 ||
        strcmp(vr, "DS") == 0 ||
        strcmp(vr, "DT") == 0 ||
        strcmp(vr, "IS") == 0 ||
        strcmp(vr, "LO") == 0 ||
        strcmp(vr, "LT") == 0 ||
        strcmp(vr, "PN") == 0 ||
        strcmp(vr, "SH") == 0 ||
        strcmp(vr, "ST") == 0 ||
        strcmp(vr, "TM") == 0 ||
        strcmp(vr, "UI") == 0 ||
        strcmp(vr, "UR") == 0 ||
        strcmp(vr, "UT") == 0) {
        return true;
    } else {
        return false;
    }
}


static bool is_vr_bytes(const char *vr) 
{
    if (strcmp(vr, "OB") == 0 ||
        strcmp(vr, "OD") == 0 ||
        strcmp(vr, "OF") == 0 ||
        strcmp(vr, "OV") == 0 ||
        strcmp(vr, "UC") == 0 ||
        strcmp(vr, "UN") == 0) {
        return true;
    } else {
        return false;
    }
}


static struct SequenceItem *create_sequence_item(DcmError **error,
                                                 DcmDataSet *dataset)
{
    assert(dataset);

    struct SequenceItem *item = DCM_NEW(error, struct SequenceItem);
    if (item == NULL) {
        return NULL;
    }
    item->dataset = dataset;
    dcm_dataset_lock(item->dataset);
    return item;
}


static void copy_sequence_item_icd(void *_dst_item, const void *_src_item)
{
    struct SequenceItem *dst_item = (struct SequenceItem *) _dst_item;
    struct SequenceItem *src_item = (struct SequenceItem *) _src_item;
    dst_item->dataset = src_item->dataset;
    dcm_dataset_lock(dst_item->dataset);
}


static void destroy_sequence_item_icd(void *_item)
{
    if (_item) {
        struct SequenceItem *item = (struct SequenceItem *) _item;
        if (item) {
            if (item->dataset) {
                dcm_dataset_destroy(item->dataset);
                item->dataset = NULL;
            }
            // utarray frees the memory of the item itself
        }
    }
}


static UT_icd sequence_item_icd = {
    sizeof(struct SequenceItem),
    NULL,
    copy_sequence_item_icd,
    destroy_sequence_item_icd
};


static int compare_tags(const void *a, const void *b)
{
   return ( *(uint32_t*)a - *(uint32_t*)b );
}


// Data Elements

static DcmElement *create_element(DcmError **error, 
                                  uint32_t tag, const char *vr, uint32_t length)
{
    dcm_log_debug("Create Data Element '%08X'.", tag);
    DcmElement *element = DCM_NEW(error, DcmElement);
    if (element == NULL) {
        return NULL;
    }
    element->tag = tag;
    strncpy(element->vr, vr, 3);
    element->vr[2] = '\0';
    if (length % 2 != 0) {
        // Zero padding
        length += 1;
    }
    element->length = length;
    element->vm = 0;
    element->value.str_multi = NULL;
    element->value_pointer = NULL;
    element->value_pointer_array = NULL;
    element->sequence_pointer = NULL;
    return element;
}


void dcm_element_destroy(DcmElement *element)
{
    if (element) {
        dcm_log_debug("Destroy Data Element '%08X'.", element->tag);
        if(element->sequence_pointer) {
            dcm_sequence_destroy(element->sequence_pointer);
        }
        if(element->value_pointer) {
            free(element->value_pointer);
        }
        if(element->value_pointer_array) {
            dcm_free_string_array(element->value_pointer_array, element->vm);
        }
        free(element);
        element = NULL;
    }
}


uint16_t dcm_element_get_group_number(const DcmElement *element)
{
    assert(element);
    return (uint16_t)(element->tag >> 16);
}


uint16_t dcm_element_get_element_number(const DcmElement *element)
{
    assert(element);
    return (uint16_t)(element->tag);
}


uint32_t dcm_element_get_tag(const DcmElement *element)
{
    assert(element);
    return element->tag;
}


bool dcm_element_check_vr(const DcmElement *element, const char *vr)
{
    assert(element);
    return strcmp(element->vr, vr) == 0;
}


uint32_t dcm_element_get_vm(const DcmElement *element)
{
    assert(element);
    return element->vm;
}

DcmElement *dcm_element_clone(DcmError **error, const DcmElement *element)
{
    assert(element);
    uint32_t i;

    dcm_log_debug("Clone Data Element '%08X'.", element->tag);
    DcmElement *clone = DCM_NEW(error, DcmElement);
    if (clone == NULL) {
        return NULL;
    }
    clone->tag = element->tag;
    strncpy(clone->vr, element->vr, 3);
    clone->length = element->length;
    clone->vm = element->vm;

    if (strcmp(element->vr, "SQ") == 0) {
        if (element->value.sq) {
            // Copy each data set in sequence
            DcmSequence *seq = dcm_sequence_create(error);
            DcmDataSet *item;
            DcmDataSet *cloned_item;
            for (i = 0; i < dcm_sequence_count(element->value.sq); i++) {
                item = dcm_sequence_get(error, element->value.sq, i);
                if (item == NULL) {
                    dcm_sequence_destroy(seq);
                    dcm_element_destroy(clone);
                    return NULL;
                }
                cloned_item = dcm_dataset_clone(error, item);
                if (cloned_item == NULL) {
                    dcm_sequence_destroy(seq);
                    dcm_element_destroy(clone);
                    return NULL;
                }
                dcm_sequence_append(error, seq, cloned_item);
                dcm_dataset_destroy(cloned_item);
            }
            clone->value.sq = seq;
            clone->sequence_pointer = seq;
        }
    } else if (is_vr_string(element->vr)) {
        if (element->value.str_multi) {
            clone->value.str_multi = DCM_NEW_ARRAY(error, element->vm, char *);
            if (clone->value.str_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.str_multi[i] = dcm_strdup(error, 
                                                       element->
                                                       value.str_multi[i]);
                if (clone->value.str_multi[i] == NULL) {
                    // FIXME: free memory allocated for previous values
                    free(clone->value.str_multi);
                    free(clone);
                    return NULL;
                }
            }
            clone->value_pointer_array = clone->value.str_multi;
        }
    } else if (is_vr_bytes(element->vr)) {
        if (element->value.bytes) {
            clone->value.bytes = DCM_MALLOC(error, element->length);
            if (clone->value.bytes == NULL) {
                free(clone);
                return NULL;
            }
            memcpy(clone->value.bytes,
                   element->value.bytes,
                   element->length);
            clone->value_pointer = clone->value.bytes;
        }
    } else if (strcmp(element->vr, "FL") == 0) {
        if (element->value.fl_multi) {
            clone->value.fl_multi = DCM_NEW_ARRAY(error, element->vm, float);
            if (clone->value.fl_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.fl_multi[i] = element->value.fl_multi[i];
            }
            clone->value_pointer = clone->value.fl_multi;
        }
    } else if (strcmp(element->vr, "FD") == 0) {
        if (element->value.fd_multi) {
            clone->value.fd_multi = DCM_NEW_ARRAY(error, element->vm, double);
            if (clone->value.fd_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.fd_multi[i] = element->value.fd_multi[i];
            }
            clone->value_pointer = clone->value.fd_multi;
        }
    } else if (strcmp(element->vr, "SS") == 0) {
        if (element->value.ss_multi) {
            clone->value.ss_multi = DCM_NEW_ARRAY(error, element->vm, int16_t);
            if (clone->value.ss_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.ss_multi[i] = element->value.ss_multi[i];
            }
            clone->value_pointer = clone->value.ss_multi;
        }
    } else if (strcmp(element->vr, "SL") == 0) {
        if (element->value.sl_multi) {
            clone->value.sl_multi = DCM_NEW_ARRAY(error, element->vm, int32_t);
            if (clone->value.sl_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.sl_multi[i] = element->value.sl_multi[i];
            }
            clone->value_pointer = clone->value.sl_multi;
        }
    } else if (strcmp(element->vr, "SV") == 0) {
        if (element->value.sv_multi) {
            clone->value.sv_multi = DCM_NEW_ARRAY(error, element->vm, int64_t);
            if (clone->value.sv_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.sv_multi[i] = element->value.sv_multi[i];
            }
            clone->value_pointer = clone->value.sv_multi;
        }
    } else if (strcmp(element->vr, "US") == 0) {
        if (element->value.us_multi) {
            clone->value.us_multi = DCM_NEW_ARRAY(error, element->vm, uint16_t);
            if (clone->value.us_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.us_multi[i] = element->value.us_multi[i];
            }
            clone->value_pointer = clone->value.us_multi;
        }
    } else if (strcmp(element->vr, "UL") == 0) {
        if (element->value.ul_multi) {
            clone->value.ul_multi = DCM_NEW_ARRAY(error, element->vm, uint32_t);
            if (clone->value.ul_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.ul_multi[i] = element->value.ul_multi[i];
            }
            clone->value_pointer = clone->value.ul_multi;
        }
    } else if (strcmp(element->vr, "UV") == 0) {
        if (element->value.uv_multi) {
            clone->value.uv_multi = DCM_NEW_ARRAY(error, element->vm, uint64_t);
            if (clone->value.uv_multi == NULL) {
                free(clone);
                return NULL;
            }
            for (i = 0; i < element->vm; i++) {
                clone->value.uv_multi[i] = element->value.uv_multi[i];
            }
            clone->value_pointer = clone->value.uv_multi;
        }
    }

    return clone;
}


bool dcm_element_is_multivalued(const DcmElement *element)
{
    assert(element);
    return element->vm > 1;
}


uint32_t dcm_element_get_length(const DcmElement *element)
{
    assert(element);
    return element->length;
}


static inline void assert_vr(const DcmElement *element, const char *vr)
{
    DCM_DEBUG_ONLY(bool success =) dcm_element_check_vr(element, vr);
    assert(success);
}


static inline bool check_value_index(const DcmElement *element, uint32_t index)
{
    return index <= (element->vm - 1);
}


static inline void assert_value_index(const DcmElement *element, uint32_t index)
{
    DCM_DEBUG_ONLY(bool success =) check_value_index(element, index);
    assert(success);
}


// Data Elements with character string Value Representation

static bool check_value_str_multi(const DcmElement *element,
                                  char **values,
                                  uint32_t vm,
                                  uint32_t capacity)
{
    uint32_t i;
    size_t actual_length;
    char *v;

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

    if (!check_value_str_multi(element, values, vm, capacity)) {
        if (values) {
            dcm_free_string_array(values, vm);
        }
        return false;
    }
    element->value.str_multi = values;
    element->value_pointer_array = values;
    element->vm = vm;
    return true;
}


static DcmElement *create_element_str(DcmError **error,
                                      uint32_t tag,
                                      const char *vr,
                                      char *value,
                                      uint32_t capacity)
{
    char **values = DCM_NEW_ARRAY(error, 1, char *);
    if (values == NULL) {
        free(value);
        return NULL;
    }
    values[0] = value;

    uint32_t length = strlen(value);
    DcmElement *element = create_element(error, tag, vr, length);
    if (element == NULL) {
        free(value);
        dcm_free_string_array(values, 1);
        return NULL;
    }
    if (!set_value_str_multi(element, values, 1, capacity)) {
        free(value);
        dcm_element_destroy(element);
        return NULL;
    }
    return element;
}

static DcmElement *create_element_str_multi(DcmError **error,
                                            uint32_t tag,
                                            const char *vr,
                                            char **values,
                                            uint32_t vm,
                                            uint32_t capacity)
{
    uint32_t i;
    uint32_t length;
    char *v;

    length = 0;
    for (i = 0; i < vm; i++) {
        v = values[i];
        length += strlen(v);
        if (i < (vm - 1)) {
            // Separator "\\"
            length += 2;
        }
    }

    DcmElement *element = create_element(error, tag, vr, length);
    if (element == NULL) {
        dcm_free_string_array(values, vm);
        return NULL;
    }

    if (!set_value_str_multi(element, values, vm, capacity)) {
        dcm_element_destroy(element);
        return NULL;
    }

    return element;
}


DcmElement *dcm_element_create_AE(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "AE", value, DCM_CAPACITY_AE);
}


DcmElement *dcm_element_create_AE_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "AE", values, vm, DCM_CAPACITY_AE);
}


DcmElement *dcm_element_create_AS(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "AS", value, DCM_CAPACITY_AS);
}


DcmElement *dcm_element_create_AS_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "AS", values, vm, DCM_CAPACITY_AS);
}


DcmElement *dcm_element_create_AT(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "AT", value, DCM_CAPACITY_AT);
}


DcmElement *dcm_element_create_AT_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "AT", values, vm, DCM_CAPACITY_AT);
}


DcmElement *dcm_element_create_CS(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "CS", value, DCM_CAPACITY_CS);
}


DcmElement *dcm_element_create_CS_multi(DcmError **error,
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "CS", values, vm, DCM_CAPACITY_CS);
}


DcmElement *dcm_element_create_DA(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "DA", value, DCM_CAPACITY_DA);
}


DcmElement *dcm_element_create_DA_multi(DcmError **error,
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "DA", values, vm, DCM_CAPACITY_DA);
}


DcmElement *dcm_element_create_DT(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "DT", value, DCM_CAPACITY_DT);
}


DcmElement *dcm_element_create_DT_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "DT", values, vm, DCM_CAPACITY_DT);
}


DcmElement *dcm_element_create_LO(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "LO", value, DCM_CAPACITY_LO);
}


DcmElement *dcm_element_create_LO_multi(DcmError **error,
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "LO", values, vm, DCM_CAPACITY_LO);
}


DcmElement *dcm_element_create_PN(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "PN", value, DCM_CAPACITY_PN);
}



DcmElement *dcm_element_create_PN_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "PN", values, vm, DCM_CAPACITY_PN);
}


DcmElement *dcm_element_create_SH(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "SH", value, DCM_CAPACITY_SH);
}


DcmElement *dcm_element_create_SH_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "SH", values, vm, DCM_CAPACITY_SH);
}


DcmElement *dcm_element_create_TM(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "TM", value, DCM_CAPACITY_TM);
}


DcmElement *dcm_element_create_TM_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error,
                                    tag, "TM", values, vm, DCM_CAPACITY_TM);
}


DcmElement *dcm_element_create_ST(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "ST", value, DCM_CAPACITY_ST);
}


DcmElement *dcm_element_create_UI(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "UI", value, DCM_CAPACITY_UI);
}


DcmElement *dcm_element_create_UI_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error,
                                    tag, "UI", values, vm, DCM_CAPACITY_UI);
}


static char *get_value_str_multi(const DcmElement *element, uint32_t index)
{
    assert_value_index(element, index);
    return element->value.str_multi[index];
}


const char *dcm_element_get_value_AE(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "AE");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_AS(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "AS");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_AT(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "AT");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_CS(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "CS");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_DA(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "DA");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_DS(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "DS");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_DT(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "DT");
    return get_value_str_multi(element, index);
}


double dcm_element_get_value_FD(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "FD");
    assert_value_index(element, index);
    return element->value.fd_multi[index];
}


float dcm_element_get_value_FL(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "FL");
    assert_value_index(element, index);
    return element->value.fl_multi[index];
}


const char *dcm_element_get_value_IS(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "IS");
    return get_value_str_multi(element, index);
}


const char *dcm_element_get_value_LO(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "LO");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_PN(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "PN");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_SH(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "SH");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_TM(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "TM");
    return get_value_str_multi(element, 0);
}


int16_t dcm_element_get_value_SS(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "SS");
    assert_value_index(element, index);
    return element->value.ss_multi[index];
}


int32_t dcm_element_get_value_SL(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "SL");
    assert_value_index(element, index);
    return element->value.sl_multi[index];
}


int64_t dcm_element_get_value_SV(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "SV");
    assert_value_index(element, index);
    return element->value.sv_multi[index];
}


const char *dcm_element_get_value_ST(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "ST");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_UI(const DcmElement *element, uint32_t index)
{
    assert(element);
    assert_vr(element, "UI");
    return get_value_str_multi(element, index);
}

uint32_t dcm_element_get_value_UL(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "UL");
    assert_value_index(element, index);
    return element->value.ul_multi[index];
}


uint16_t dcm_element_get_value_US(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "US");
    assert_value_index(element, index);
    return element->value.us_multi[index];
}


uint64_t dcm_element_get_value_UV(const DcmElement *element, uint32_t index)
{
    assert_vr(element, "UV");
    assert_value_index(element, index);
    return element->value.uv_multi[index];
}


const char *dcm_element_get_value_UR(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "UR");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_UT(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "UT");
    return get_value_str_multi(element, 0);
}


const char *dcm_element_get_value_OB(const DcmElement *element)
{
    assert_vr(element, "OB");
    return element->value.bytes;
}


const char *dcm_element_get_value_OD(const DcmElement *element)
{
    assert_vr(element, "OD");
    return element->value.bytes;
}


const char *dcm_element_get_value_OF(const DcmElement *element)
{
    assert_vr(element, "OF");
    return element->value.bytes;
}


const char *dcm_element_get_value_OL(const DcmElement *element)
{
    assert_vr(element, "OL");
    return element->value.bytes;
}


const char *dcm_element_get_value_OV(const DcmElement *element)
{
    assert_vr(element, "OV");
    return element->value.bytes;
}


const char *dcm_element_get_value_OW(const DcmElement *element)
{
    assert_vr(element, "OW");
    return element->value.bytes;
}


const char *dcm_element_get_value_UC(const DcmElement *element)
{
    assert_vr(element, "UC");
    return element->value.bytes;
}


const char *dcm_element_get_value_UN(const DcmElement *element)
{
    assert_vr(element, "UN");
    return element->value.bytes;
}


static inline void print_element_value_AE(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_AT(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_AS(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_CS(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_DA(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_DT(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_LO(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_PN(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_SH(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_TM(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


static inline void print_element_value_UI(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}



// Data Elements with numeric Value Representation

DcmElement *dcm_element_create_FD(DcmError **error, uint32_t tag, double value)
{
    uint32_t length = sizeof(double);
    double *values = DCM_NEW_ARRAY(error, 1, double);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "FD", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.fd_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_FD_multi(DcmError **error, 
                                        uint32_t tag,
                                        double *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(double);
    DcmElement *element = create_element(error, tag, "FD", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.fd_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_FL(DcmError **error, uint32_t tag, float value)
{
    uint32_t length = sizeof(double);
    float *values = DCM_NEW_ARRAY(error, 1, float);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "FL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.fl_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_FL_multi(DcmError **error,
                                        uint32_t tag,
                                        float *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(float);
    DcmElement *element = create_element(error, tag, "FL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.fl_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_DS(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "DS", value, DCM_CAPACITY_DS);
}


DcmElement *dcm_element_create_DS_multi(DcmError **error,
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "DS", values, vm, DCM_CAPACITY_DS);
}


DcmElement *dcm_element_create_IS(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "IS", value, DCM_CAPACITY_IS);
}


DcmElement *dcm_element_create_IS_multi(DcmError **error, 
                                        uint32_t tag,
                                        char **values,
                                        uint32_t vm)
{
    return create_element_str_multi(error, 
                                    tag, "IS", values, vm, DCM_CAPACITY_IS);
}


DcmElement *dcm_element_create_SS(DcmError **error, uint32_t tag, int16_t value)
{
    uint32_t length = sizeof(int16_t);
    int16_t *values = DCM_NEW_ARRAY(error, 1, int16_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "SS", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.ss_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SS_multi(DcmError **error, 
                                        uint32_t tag,
                                        int16_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int16_t);
    DcmElement *element = create_element(error, tag, "SS", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.ss_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_SL(DcmError **error,
                                  uint32_t tag, int32_t value)
{
    uint32_t length = sizeof(int32_t);
    int32_t *values = DCM_NEW_ARRAY(error, 1, int32_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "SL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.sl_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SL_multi(DcmError **error,
                                        uint32_t tag,
                                        int32_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int32_t);
    DcmElement *element = create_element(error, tag, "SL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.sl_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_SV(DcmError **error, uint32_t tag, int64_t value)
{
    uint32_t length = sizeof(int64_t);
    int64_t *values = DCM_NEW_ARRAY(error, 1, int64_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "SV", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.sv_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_SV_multi(DcmError **error,
                                        uint32_t tag,
                                        int64_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(int64_t);
    DcmElement *element = create_element(error, tag, "SV", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.sv_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_UL(DcmError **error, 
                                  uint32_t tag, uint32_t value)
{
    uint32_t length = sizeof(uint32_t);
    uint32_t *values = DCM_NEW_ARRAY(error, 1, uint32_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "UL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.ul_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UL_multi(DcmError **error,
                                        uint32_t tag,
                                        uint32_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint32_t);
    DcmElement *element = create_element(error, tag, "UL", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.ul_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_US(DcmError **error, 
                                  uint32_t tag, uint16_t value)
{
    uint32_t length = sizeof(uint16_t);
    uint16_t *values = DCM_NEW_ARRAY(error, 1, uint16_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "US", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.us_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}

DcmElement *dcm_element_create_US_multi(DcmError **error,
                                        uint32_t tag,
                                        uint16_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint16_t);
    DcmElement *element = create_element(error, tag, "US", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.us_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


DcmElement *dcm_element_create_UV(DcmError **error, 
                                  uint32_t tag, uint64_t value)
{
    uint32_t length = sizeof(uint64_t);
    uint64_t *values = DCM_NEW_ARRAY(error, 1, uint64_t);
    if (values == NULL) {
        return NULL;
    }
    values[0] = value;
    DcmElement *element = create_element(error, tag, "UV", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.uv_multi = values;
    element->value_pointer = values;
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UV_multi(DcmError **error,
                                        uint32_t tag,
                                        uint64_t *values,
                                        uint32_t vm)
{
    uint32_t length = vm * sizeof(uint64_t);
    DcmElement *element = create_element(error, tag, "UV", length);
    if (element == NULL) {
        free(values);
        return NULL;
    }
    element->value.uv_multi = values;
    element->value_pointer = values;
    element->vm = vm;
    return element;
}


inline static void print_element_value_DS(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


inline static void print_element_value_FD(const DcmElement *element,
                                          uint32_t index)
{
    printf("%f", element->value.fd_multi[index]);
}


inline static void print_element_value_FL(const DcmElement *element,
                                          uint32_t index)
{
    printf("%f", element->value.fl_multi[index]);
}


inline static void print_element_value_IS(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


inline static void print_element_value_SS(const DcmElement *element,
                                          uint32_t index)
{
    printf("%d", element->value.ss_multi[index]);
}


inline static void print_element_value_SL(const DcmElement *element,
                                          uint32_t index)
{
    printf("%d", element->value.sl_multi[index]);
}


inline static void print_element_value_ST(const DcmElement *element,
                                          uint32_t index)
{
    printf("%s", element->value.str_multi[index]);
}


inline static void print_element_value_SV(const DcmElement *element,
                                          uint32_t index)
{
    // Cast seems necessary for some platforms
    printf("%lld", (long long)element->value.sv_multi[index]);
}


inline static void print_element_value_US(const DcmElement *element,
                                          uint32_t index)
{
    printf("%hu", element->value.us_multi[index]);
}


inline static void print_element_value_UT(const DcmElement *element)
{
    printf("%s", element->value.str_multi[0]);
}


inline static void print_element_value_UL(const DcmElement *element,
                                          uint32_t index)
{
    printf("%u", element->value.ul_multi[index]);
}



// Data Elements with binary Value Representation

static void set_value_bytes(DcmElement *element, char *value)
{
    assert(element);
    assert(value);
    element->value.bytes = value;
    element->value_pointer = value;
}


DcmElement *dcm_element_create_LT(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "LT", value, DCM_CAPACITY_LT);
}


DcmElement *dcm_element_create_OB(DcmError **error, 
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OB", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OD(DcmError **error, 
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OD", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OF(DcmError **error,
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OF", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OL(DcmError **error, 
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OL", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OV(DcmError **error,
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OV", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_OW(DcmError **error,
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "OW", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UC(DcmError **error, 
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "UC", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UN(DcmError **error, 
                                  uint32_t tag, char *value, uint32_t length)
{
    DcmElement *element = create_element(error, tag, "UN", length);
    if (element == NULL) {
        free(value);
        return NULL;
    }
    set_value_bytes(element, value);
    element->vm = 1;
    return element;
}


DcmElement *dcm_element_create_UR(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "UR", value, DCM_CAPACITY_UR);
}


DcmElement *dcm_element_create_UT(DcmError **error, uint32_t tag, char *value)
{
    return create_element_str(error, tag, "UT", value, DCM_CAPACITY_UT);
}


// Sequence Data Element

DcmElement *dcm_element_create_SQ(DcmError **error, 
                                  uint32_t tag, DcmSequence *value)
{
    uint32_t length;
    uint32_t i;
    DcmDataSet *item;
    DcmElement *elem;

    uint32_t seq_length = dcm_sequence_count(value);
    length = 0;
    for (i = 0; i < seq_length; i++) {
        item = dcm_sequence_get(error, value, i);
        if (item == NULL) {
            dcm_sequence_destroy(value);
            return NULL;
        }
        for (elem = item->elements; elem; elem = elem->hh.next) {
            length += elem->length;
        }
    }

    DcmElement *element = create_element(error, tag, "SQ", length);
    if (element == NULL) {
        dcm_sequence_destroy(value);
        return NULL;
    }
    element->value.sq = value;
    element->sequence_pointer = value;
    element->vm = 1;

    return element;
}


DcmSequence *dcm_element_get_value_SQ(const DcmElement *element)
{
    assert(element);
    assert_vr(element, "SQ");
    assert(element->value.sq);
    dcm_sequence_lock(element->value.sq);
    return element->value.sq;
}


void dcm_element_print(const DcmElement *element, uint8_t indentation)
{
    assert(element);
    const uint8_t num_indent = indentation * 2;
    const uint8_t num_indent_next = (indentation + 1) * 2;

    uint32_t i;

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
            } else if (strcmp(element->vr, "ST") == 0) {
                print_element_value_ST(element, i);
            } else if (strcmp(element->vr, "SV") == 0) {
                print_element_value_SV(element, i);
            } else if (strcmp(element->vr, "TM") == 0) {
                print_element_value_TM(element, i);
            } else if (strcmp(element->vr, "UI") == 0) {
                print_element_value_UI(element, i);
            } else if (strcmp(element->vr, "US") == 0) {
                print_element_value_US(element, i);
            } else if (strcmp(element->vr, "UT") == 0) {
                print_element_value_UT(element);
            } else if (strcmp(element->vr, "UL") == 0) {
                print_element_value_UL(element, i);
            } else {
                dcm_log_warning("Encountered unexpected Value Representation.");
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
        DcmSequence *sequence = dcm_element_get_value_SQ(element);
        uint32_t n_items = dcm_sequence_count(sequence);
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
            DcmDataSet *item = dcm_sequence_get(NULL, sequence, i);
            dcm_dataset_print(item, indentation+1);
        }
        printf("%*.*s]\n",
               num_indent,
               num_indent,
               "                                   ");
    }
}


// Datasets

DcmDataSet *dcm_dataset_create(DcmError **error)
{
    dcm_log_debug("Create Data Set.");
    DcmDataSet *dataset = DCM_NEW(error, DcmDataSet);
    if (dataset == NULL) {
        return NULL;
    }
    dataset->elements = NULL;
    dataset->is_locked = false;
    return dataset;
}


DcmDataSet *dcm_dataset_clone(DcmError **error, const DcmDataSet *dataset)
{
    dcm_log_debug("Clone Data Set.");
    DcmDataSet *cloned_dataset = dcm_dataset_create(error);
    if (cloned_dataset == NULL) {
        return NULL;
    }

    DcmElement *element;
    DcmElement *cloned_element;
    for(element = dataset->elements; element; element = element->hh.next) {
        cloned_element = dcm_element_clone(error, element);
        if (cloned_element == NULL) {
            dcm_dataset_destroy(cloned_dataset);
            return NULL;
        }
        if (!dcm_dataset_insert(error, cloned_dataset, cloned_element)) {
            dcm_dataset_destroy(cloned_dataset);
            return NULL;
        }
    }

    return cloned_dataset;
}


bool dcm_dataset_insert(DcmError **error, 
                        DcmDataSet *dataset, DcmElement *element)
{
    assert(dataset);
    assert(element);

    dcm_log_debug("Insert Data Element '%08X' into Data Set.", element->tag);
    if (dataset->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Data Set is locked and cannot be modified",
                      "Inserting Data Element '%08X' into Data Set failed",
                      element->tag);
        dcm_element_destroy(element);
        return false;
    }

    DcmElement *matched_element;
    HASH_FIND_INT(dataset->elements, &element->tag, matched_element);
    if (matched_element) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Element already exists",
                      "Inserting Data Element '%08X' into Data Set failed",
                      element->tag);
        dcm_element_destroy(element);
        return false;
    }

    HASH_ADD_INT(dataset->elements, tag, element);

    return true;
}


bool dcm_dataset_remove(DcmError **error, DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);

    dcm_log_debug("Remove Data Element '%08X' from Data Set.", tag);
    if (dataset->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Data Set is locked and cannot be modified",
                      "Removing Data Element '%08X' from Data Set failed",
                      tag);
        return false;
    }

    DcmElement *matched_element = dcm_dataset_get(error, dataset, tag);
    if (matched_element == NULL) {
        return false;
    }

    HASH_DEL(dataset->elements, matched_element);
    dcm_element_destroy(matched_element);

    return true;
}


DcmElement *dcm_dataset_get_clone(DcmError **error,
                                  const DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);
    DcmElement *element;

    dcm_log_debug("Copy Data Element '%08X' from Data Set.", tag);
    HASH_FIND_INT(dataset->elements, &tag, element);
    if (element == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Could not find Data Element",
                      "Getting Data Element '%08X' from Data Set failed",
                      tag);
        return NULL;
    }
    return dcm_element_clone(error, element);
}


DcmElement *dcm_dataset_get(DcmError **error, 
                            const DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);
    DcmElement *element;

    dcm_log_debug("Get Data Element '%08X' from Data Set.", tag);
    element = dcm_dataset_contains(dataset, tag);
    if (element == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Could not find Data Element",
                      "Getting Data Element '%08X' from Data Set failed",
                      tag);
    }

    return element;
}


void dcm_dataset_foreach(const DcmDataSet *dataset,
                         void (*fn)(const DcmElement *element))
{
    assert(dataset);
    DcmElement *element;

    for(element = dataset->elements; element; element = element->hh.next) {
        fn(element);
    }
}


DcmElement *dcm_dataset_contains(const DcmDataSet *dataset, uint32_t tag)
{
    assert(dataset);

    DcmElement *element;
    HASH_FIND_INT(dataset->elements, &tag, element);

    return element;
}


uint32_t dcm_dataset_count(const DcmDataSet *dataset)
{
    assert(dataset);

    uint32_t num_users = HASH_COUNT(dataset->elements);
    return num_users;
}


void dcm_dataset_copy_tags(const DcmDataSet *dataset, 
                           uint32_t *tags, uint32_t n)
{
    assert(dataset);
    uint32_t i;
    DcmElement *element;

    for(i = 0, element = dataset->elements; 
        element && i < n; 
        element = element->hh.next, i++) {
        tags[i] = element->tag;
    }

    qsort(tags, n, sizeof(uint32_t), compare_tags);
}


void dcm_dataset_print(const DcmDataSet *dataset, uint8_t indentation)
{
    assert(dataset);
    uint32_t i;
    DcmElement *element;

    uint32_t n = dcm_dataset_count(dataset);
    uint32_t *tags = DCM_NEW_ARRAY(NULL, n, uint32_t);
    if (tags == NULL) {
        return;
    }
    dcm_dataset_copy_tags(dataset, tags, n);

    for(i = 0; i < n; i++) {
        element = dcm_dataset_get(NULL, dataset, tags[i]);
        if (element == NULL) {
            dcm_log_warning("Missing tag.");
            free(tags);
            return;
        }
        dcm_element_print(element, indentation);
    }

    free(tags);
}


void dcm_dataset_lock(DcmDataSet *dataset)
{
    dataset->is_locked = true;
}


bool dcm_dataset_is_locked(const DcmDataSet *dataset)
{
    return dataset->is_locked;
}


void dcm_dataset_destroy(DcmDataSet *dataset)
{
    DcmElement *element, *tmp;

    if (dataset) {
        HASH_ITER(hh, dataset->elements, element, tmp) {
            HASH_DEL(dataset->elements, element);
            dcm_element_destroy(element);
        }
        free(dataset);
        dataset = NULL;
    }
}


// Sequences

DcmSequence *dcm_sequence_create(DcmError **error)
{
    DcmSequence *seq = DCM_NEW(error, DcmSequence);
    if (seq == NULL) {
        return NULL;
    }

    UT_array *items;
    utarray_new(items, &sequence_item_icd);
    if (items == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_NOMEM,
                      "Out of memory",
                      "Creation of Sequence failed");
        free(seq);
        return NULL;
    }
    seq->items = items;
    seq->is_locked = false;

    return seq;
}


bool dcm_sequence_append(DcmError **error, DcmSequence *seq, DcmDataSet *item)
{
    assert(seq);
    assert(item);

    dcm_log_debug("Append item to Sequence.");
    if (seq->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Appending item to Sequence failed",
                      "Sequence is locked and cannot be modified.");
        dcm_dataset_destroy(item);
        return false;
    }

    /**
     * The SequenceItem is just a thin wrapper around a DcmDataSet object as a
     * handle for utarray. Under the hood, utarray frees the memory of the
     * DcmDataSet object when the array item gets destroyed. However, utarray
     * does not free the memory of the item handle. Therefore, we need to free
     * the memory of the item handle after the item was added to the array.
     */
    struct SequenceItem *item_handle = create_sequence_item(error, item);
    utarray_push_back(seq->items, item_handle);
    free(item_handle);

    return true;
}


DcmDataSet *dcm_sequence_get(DcmError **error, 
                             const DcmSequence *seq, uint32_t index)
{
    assert(seq);

    dcm_log_debug("Get item #%i of Sequence.", index);
    uint32_t length = utarray_len(seq->items);
    if (index >= length) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Getting item of Sequence failed",
                      "Index %i exceeds length of sequence %i",
                      index, length);
        return NULL;
    }

    struct SequenceItem *item_handle = utarray_eltptr(seq->items, index);
    if (item_handle == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Getting item of Sequence failed",
                      "Getting item #%i of Sequence failed", index);
        return NULL;
    }
    if (item_handle->dataset == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Getting item of Sequence failed",
                      "Getting item #%i of Sequence failed", index);
        return NULL;
    }
    dcm_dataset_lock(item_handle->dataset);

    return item_handle->dataset;
}


void dcm_sequence_foreach(const DcmSequence *seq,
                          void (*fn)(const DcmDataSet *item))
{
    assert(seq);
    uint32_t i;
    struct SequenceItem *item_handle;

    uint32_t length = utarray_len(seq->items);
    for (i = 0; i < length; i++) {
        item_handle = utarray_eltptr(seq->items, i);
        dcm_dataset_lock(item_handle->dataset);
        fn(item_handle->dataset);
    }
}


void dcm_sequence_remove(DcmError **error, DcmSequence *seq, uint32_t index)
{
    assert(seq);
    if (seq->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Removing item from Sequence failed",
                      "Sequence is locked and cannot be modified.");
        return;
    }
    dcm_log_debug("Remove item #%i from Sequence.", index);
    uint32_t length = utarray_len(seq->items);
    if (index >= length) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Removing item from Sequence failed",
                      "Index %i exceeds length of sequence %i",
                      index, length);
        return;
    }
    utarray_erase(seq->items, index, 1);
}


uint32_t dcm_sequence_count(const DcmSequence *seq)
{
    uint32_t length = utarray_len(seq->items);
    return length;
}


void dcm_sequence_lock(DcmSequence *seq)
{
    seq->is_locked = true;
}


bool dcm_sequence_is_locked(const DcmSequence *seq)
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

DcmFrame *dcm_frame_create(DcmError **error, 
                           uint32_t number,
                           const char *data,
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
    if (data == NULL || length == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Frame Item failed",
                      "Pixel data cannot be empty");
        free((char *)data);
        return NULL;
    }
    if (!(bits_allocated == 1 || bits_allocated % 8 == 0)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Frame Item failed",
                      "Wrong number of bits allocated");
        free((char *)data);
        return NULL;
    }
    if (!(bits_stored == 1 || bits_stored % 8 == 0)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Frame Item failed",
                      "Wrong number of bits stored");
        free((char *)data);
        return NULL;
    }
    if (!(pixel_representation == 0 || pixel_representation == 1)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Frame Item failed",
                      "Wrong pixel representation");
        free((char *)data);
        return NULL;
    }
    if (!(planar_configuration == 0 || planar_configuration == 1)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Frame Item failed",
                      "Wrong planar configuration");
        free((char *)data);
        return NULL;
    }

    DcmFrame *frame = DCM_NEW(error, DcmFrame);
    if (frame == NULL) {
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
    frame->photometric_interpretation = photometric_interpretation;
    frame->transfer_syntax_uid = transfer_syntax_uid;

    return frame;
}


uint32_t dcm_frame_get_number(const DcmFrame *frame)
{
    assert(frame);
    return frame->number;
}

uint32_t dcm_frame_get_length(const DcmFrame *frame)
{
    assert(frame);
    return frame->length;
}

uint16_t dcm_frame_get_rows(const DcmFrame *frame)
{
    assert(frame);
    return frame->rows;
}

uint16_t dcm_frame_get_columns(const DcmFrame *frame)
{
    assert(frame);
    return frame->columns;
}

uint16_t dcm_frame_get_samples_per_pixel(const DcmFrame *frame)
{
    assert(frame);
    return frame->samples_per_pixel;
}

uint16_t dcm_frame_get_bits_allocated(const DcmFrame *frame)
{
    assert(frame);
    return frame->bits_allocated;
}

uint16_t dcm_frame_get_bits_stored(const DcmFrame *frame)
{
    assert(frame);
    return frame->bits_stored;
}

uint16_t dcm_frame_get_high_bit(const DcmFrame *frame)
{
    assert(frame);
    return frame->high_bit;
}

uint16_t dcm_frame_get_pixel_representation(const DcmFrame *frame)
{
    assert(frame);
    return frame->pixel_representation;
}

uint16_t dcm_frame_get_planar_configuration(const DcmFrame *frame)
{
    assert(frame);
    return frame->planar_configuration;
}

const char *dcm_frame_get_photometric_interpretation(const DcmFrame *frame)
{
    assert(frame);
    return frame->photometric_interpretation;
}

const char *dcm_frame_get_transfer_syntax_uid(const DcmFrame *frame)
{
    assert(frame);
    return frame->transfer_syntax_uid;
}


const char *dcm_frame_get_value(const DcmFrame *frame)
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
            free((char*)frame->photometric_interpretation);
        }
        if (frame->transfer_syntax_uid) {
            free((char*)frame->transfer_syntax_uid);
        }
        free(frame);
        frame = NULL;
    }
}



// Basic Offset Table

DcmBOT *dcm_bot_create(DcmError **error, ssize_t *offsets, uint32_t num_frames)
{
    if (num_frames == 0) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Basic Offset Table failed",
                      "Expected offsets of %ld Frame Items",
                      num_frames);
        free(offsets);
        return NULL;
    }

    if (offsets == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "Constructing Basic Offset Table failed",
                      "No offsets were provided");
        return NULL;
    }
    DcmBOT *bot = DCM_NEW(error, DcmBOT);
    if (bot == NULL) {
        free(offsets);
        return NULL;
    }
    bot->num_frames = num_frames;
    bot->offsets = offsets;
    return bot;
}


void dcm_bot_print(const DcmBOT *bot)
{
    assert(bot);
    uint32_t i;

    printf("[");
    for(i = 0; i < bot->num_frames; i++) {
        if (i == (bot->num_frames - 1)) {
            printf("%zd]\n", bot->offsets[i]);
        } else {
            printf("%zd,", bot->offsets[i]);
        }
    }
}


uint32_t dcm_bot_get_num_frames(const DcmBOT *bot)
{
    assert(bot);
    return bot->num_frames;
}


ssize_t dcm_bot_get_frame_offset(const DcmBOT *bot, uint32_t number)
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


bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid) 
{
    return 
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1.99") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.2") != 0;
}
