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
    DcmVR vr;
    uint32_t length;
    uint32_t vm;
    bool assigned;

    // Store values for multiplicity 1 (the most common case)
    // inside the element to reduce malloc/frees during build
    union {
        union {
            float fl;
            double fd;
            int16_t ss;
            int32_t sl;
            int64_t sv;
            uint16_t us;
            uint32_t ul;
            uint64_t uv;

            char *str;

            // Binary value (multiplicity 1)
            char *bytes;

            // Sequence value (multiplicity 1)
            DcmSequence *sq;

        } single;

        union {
            // Numeric value (multiplicity 2-n)
            float *fl;
            double *fd;
            int16_t *ss;
            int32_t *sl;
            int64_t *sv;
            uint16_t *us;
            uint32_t *ul;
            uint64_t *uv;

            // Character string value (multiplicity 2-n)
            char **str;

        } multi;
    } value;

    // Free these on destroy
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


struct SequenceItem {
    DcmDataSet *dataset;
};


static struct SequenceItem *create_sequence_item(DcmError **error,
                                                 DcmDataSet *dataset)
{
    struct SequenceItem *seq_item = DCM_NEW(error, struct SequenceItem);
    if (seq_item == NULL) {
        return NULL;
    }
    seq_item->dataset = dataset;
    dcm_dataset_lock(seq_item->dataset);
    return seq_item;
}


static void copy_sequence_item_icd(void *_dst_item, const void *_src_item)
{
    struct SequenceItem *dst_seq_item = (struct SequenceItem *) _dst_item;
    struct SequenceItem *src_seq_item = (struct SequenceItem *) _src_item;
    dst_seq_item->dataset = src_seq_item->dataset;
    dcm_dataset_lock(dst_seq_item->dataset);
}


static void destroy_sequence_item_icd(void *_item)
{
    if (_item) {
        struct SequenceItem *seq_item = (struct SequenceItem *) _item;
        if (seq_item) {
            if (seq_item->dataset) {
                dcm_dataset_destroy(seq_item->dataset);
                seq_item->dataset = NULL;
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


DcmElement *dcm_element_create(DcmError **error, uint32_t tag, DcmVR vr)
{
    if (!dcm_is_valid_vr_for_tag(vr, tag)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "incorrect tag",
                      "tag %08x does not allow VR %s",
                      tag,
                      dcm_dict_str_from_vr(vr));
        return NULL;
    }

    DcmElement *element = DCM_NEW(error, DcmElement);
    if (element == NULL) {
        return NULL;
    }
    element->tag = tag;
    element->vr = vr;

    return element;
}


void dcm_element_destroy(DcmElement *element)
{
    if (element) {
        dcm_log_debug("destroy Data Element '%08x'", element->tag);
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
    }
}


uint16_t dcm_element_get_group_number(const DcmElement *element)
{
    return element->tag >> 16;
}


uint16_t dcm_element_get_element_number(const DcmElement *element)
{
    return element->tag & 0xffff;
}


uint32_t dcm_element_get_tag(const DcmElement *element)
{
    return element->tag;
}


DcmVR dcm_element_get_vr(const DcmElement *element)
{
    return element->vr;
}


uint32_t dcm_element_get_vm(const DcmElement *element)
{
    return element->vm;
}


bool dcm_element_is_multivalued(const DcmElement *element)
{
    return element->vm > 1;
}


uint32_t dcm_element_get_length(const DcmElement *element)
{
    return element->length;
}


// check, set, get string value representations

static bool element_check_index(DcmError **error,
                                const DcmElement *element, uint32_t index)
{
    if (index >= element->vm) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "data element index out of range",
                      "element tag %08x has VM of %d, index %d is out of range",
                      element->tag,
                      element->vm,
                      index);
        return false;
    }

    return true;
}


static bool element_check_string(DcmError **error,
                                 const DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    if (vr_class != DCM_VR_CLASS_STRING_MULTI &&
        vr_class != DCM_VR_CLASS_STRING_SINGLE) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "data element is not string",
                      "element tag %08x has VR %s with no string value",
                      element->tag,
                      dcm_dict_str_from_vr(element->vr));
        return false;
    }

    return true;
}


static bool element_check_assigned(DcmError **error,
                                   const DcmElement *element)
{
    if (!element->assigned) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "data element not assigned a value",
                      "element tag %08x has not been assigned a value",
                      element->tag);
        return false;
    }

    return true;
}


static bool element_check_not_assigned(DcmError **error,
                                       const DcmElement *element)
{
    if (element->assigned) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "data element assigned twice",
                      "element tag %08x has been previously assigned a value",
                      element->tag);
        return false;
    }

    return true;
}


bool dcm_element_get_value_string(DcmError **error,
                                  const DcmElement *element,
                                  uint32_t index,
                                  const char **value)
{
    if (!element_check_assigned(error, element) ||
        !element_check_string(error, element) ||
        !element_check_index(error, element, index)) {
        return false;
    }

    if (element->vm == 1) {
        *value = element->value.single.str;
    } else {
        *value = element->value.multi.str[index];
    }

    return true;
}


static bool element_check_capacity(DcmError **error,
                                   DcmElement *element, uint32_t capacity)
{
    uint32_t i;

    bool was_assigned = element->assigned;

    // we have to turn on "assigned" for this func so we can read out values
    element->assigned = true;

    for (i = 0; i < element->vm; i++) {
        const char *value;
        if (!dcm_element_get_value_string(error, element, i, &value)) {
            element->assigned = was_assigned;
            return false;
        }

        size_t length = strlen(value);
        if (length > capacity) {
            element->assigned = was_assigned;
            dcm_log_warning("Data Element capacity check failed -- "
                            "Value of Data Element '%08x' exceeds "
                            "maximum length of Value Representation (%d)",
                            element->tag,
                            capacity);
        }
    }

    element->assigned = was_assigned;

    return true;
}


static bool dcm_element_validate(DcmError **error, DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);

    if (!element_check_not_assigned(error, element)) {
        return false;
    }

    if (!dcm_is_valid_vr_for_tag(element->vr, element->tag)) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "data element validation failed",
                      "bad VR for tag %08x, should be %s",
                      element->tag,
                      dcm_dict_str_from_vr(element->vr));
        return false;
    }

    if (vr_class == DCM_VR_CLASS_NUMERIC_DECIMAL ||
        vr_class == DCM_VR_CLASS_NUMERIC_INTEGER) {
        if (element->length != element->vm * dcm_dict_vr_size(element->vr)) {
            dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                          "data element validation failed",
                          "bad length for numeric tag %08x",
                          element->tag);
            return false;
        }
    }

    if (vr_class == DCM_VR_CLASS_STRING_MULTI ||
        vr_class == DCM_VR_CLASS_STRING_SINGLE) {
        uint32_t capacity = dcm_dict_vr_capacity(element->vr);
        if (!element_check_capacity(error, element, capacity)) {
            return false;
        }
    }

    element->assigned = true;

    return true;
}


static void element_set_length(DcmElement *element, uint32_t length)
{
    uint32_t even_length = length % 2 != 0 ? length + 1 : length;

    if (element->length == 0) {
        element->length = even_length;
    }
}


bool dcm_element_set_value_string_multi(DcmError **error,
                                        DcmElement *element,
                                        char **values,
                                        uint32_t vm,
                                        bool steal)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_string(error, element)) {
        return false;
    }

    if (vm == 1) {
        if (steal) {
            element->value.single.str = values[0];
        } else {
            char *value_copy = dcm_strdup(error, values[0]);
            if (value_copy == NULL) {
                return false;
            }

            element->value.single.str = value_copy;
            element->value_pointer = value_copy;
        }
    } else {
        DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
        if (vr_class != DCM_VR_CLASS_STRING_MULTI) {
            dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                          "data element is not multi-valued string",
                          "element tag %08x has VR %s with only a string value",
                          element->tag,
                          dcm_dict_str_from_vr(element->vr));
            return false;
        }

        if (steal) {
            element->value.multi.str = values;
        } else {
            char **values_copy = DCM_NEW_ARRAY(error, vm, char *);
            if (values_copy == NULL) {
                return false;
            }
            element->value.multi.str = values_copy;
            element->value_pointer_array = values_copy;

            for (uint32_t i = 0; i < vm; i++) {
                values_copy[i] = dcm_strdup(error, values[i]);
                if (values_copy[i] == NULL) {
                    return false;
                }
            }
        }
    }

    element->vm = vm;

    uint32_t length = 0;
    for (uint32_t i = 0; i < vm; i++) {
        length += (uint32_t) strlen(values[i]);
    }
    if (vm > 1) {
        // add the separator characters
        length += vm - 1;
    }
    element_set_length(element, length);

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    if (steal) {
        element->value_pointer_array = values;
    }

    return true;
}


static char **dcm_parse_character_string(DcmError **error,
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


bool dcm_element_set_value_string(DcmError **error,
                                  DcmElement *element,
                                  char *value,
                                  bool steal)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_string(error, element)) {
        return false;
    }

    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    if (vr_class == DCM_VR_CLASS_STRING_MULTI) {
        uint32_t vm;
        char **values = dcm_parse_character_string(error, value, &vm);
        if (values == NULL) {
            return false;
        }

        if (!dcm_element_set_value_string_multi(error,
                                                element, values, vm, true)) {
            dcm_free_string_array(values, vm);
            return false;
        }
    } else {
        if (steal) {
            element->value.single.str = value;
        } else {
            char *value_copy = dcm_strdup(error, value);
            if (value_copy == NULL) {
                return false;
            }

            element->value.single.str = value_copy;
            element->value_pointer = value_copy;
        }

        element->vm = 1;
        element_set_length(element, (uint32_t) strlen(value));

        if (!dcm_element_validate(error, element)) {
            return false;
        }
    }

    if (steal) {
        element->value_pointer = value;
    }

    return true;
}


// integer numeric types

// use a VR to marshall an int pointer into a int64_t
static int64_t value_to_int64(DcmVR vr, int *value)
{
    uint64_t result = 0;

#define PEEK(TYPE) result = (uint64_t) *((TYPE *) value)
    DCM_SWITCH_NUMERIC(vr, PEEK);
#undef PEEK

    return result;
}


// use a VR to write an int64_t to an int pointer
static void int64_to_value(DcmVR vr, int *result, int64_t value)
{
    *result = 0;
#define POKE(TYPE) *((TYPE *) result) = (TYPE) value;
    DCM_SWITCH_NUMERIC(vr, POKE);
#undef POKE
}


// use a VR to copy any numeric value (not just int as above)
static void value_to_value(DcmVR vr, int *result, int *value)
{
#define COPY(TYPE) *((TYPE *)result) = *((TYPE *)value);
    DCM_SWITCH_NUMERIC(vr, COPY);
#undef COPY
}


static bool element_check_numeric(DcmError **error,
                                  const DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    if (vr_class != DCM_VR_CLASS_NUMERIC_DECIMAL &&
        vr_class != DCM_VR_CLASS_NUMERIC_INTEGER) {
      dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                    "data element is not numeric",
                    "element tag %08x is not numeric",
                    element->tag);
      return false;
    }

    return true;
}


static bool element_check_integer(DcmError **error,
                                  const DcmElement *element)
{
    if (!element_check_numeric(error, element)) {
      return false;
    }
    if (element->vr == DCM_VR_FL || element->vr == DCM_VR_FD) {
      dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                    "data element is not integer",
                    "element tag %08x is not integer",
                    element->tag);
      return false;
    }

    return true;
}


bool dcm_element_get_value_integer(DcmError **error,
                                   const DcmElement *element,
                                   uint32_t index,
                                   int64_t *value)
{
    if (!element_check_assigned(error, element) ||
        !element_check_integer(error, element) ||
        !element_check_index(error, element, index)) {
        return false;
    }

    int *element_value;
    if (element->vm == 1) {
        element_value = (int *) &element->value.single.sl;
    } else {
        size_t size = dcm_dict_vr_size(element->vr);
        unsigned char *base = (unsigned char *) element->value.multi.sl;
        element_value = (int *)(base + size * index);
    }
    *value = value_to_int64(element->vr, element_value);

    return true;
}


bool dcm_element_set_value_integer(DcmError **error,
                                   DcmElement *element,
                                   int64_t value)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_integer(error, element)) {
        return false;
    }

    int *element_value = (int *) &element->value.single.sl;
    int64_to_value(element->vr, element_value, value);
    element->vm = 1;
    element_set_length(element, (uint32_t) dcm_dict_vr_size(element->vr));

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    return true;
}


bool dcm_element_set_value_numeric_multi(DcmError **error,
                                         DcmElement *element,
                                         void *value,
                                         uint32_t vm,
                                         bool steal)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_numeric(error, element)) {
        return false;
    }

    size_t size_in_bytes = vm * dcm_dict_vr_size(element->vr);

    // this will work for all numeric types, since we're just setting a
    // pointer
    if (vm == 1) {
        value_to_value(element->vr, (int *)&element->value.single.sl, value);
    } else {
        if (steal) {
            element->value.multi.sl = (int32_t *)value;
        } else {
            char *value_copy = DCM_NEW_ARRAY(error, size_in_bytes, char);
            if (value_copy == NULL) {
                return false;
            }

            memcpy(value_copy, value, size_in_bytes);

            element->value.multi.sl = (int32_t *)value_copy;
            element->value_pointer = value_copy;
        }
    }

    element->vm = vm;
    element_set_length(element, (uint32_t) size_in_bytes);

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    if (steal) {
        element->value_pointer = value;
    }

    return true;
}


// the float values

// use a VR to marshall a double pointer into a float
static double value_to_double(DcmVR vr, double *value)
{
    double result = 0.0;

#define PEEK(TYPE) result = (double) *((TYPE *) value)
    DCM_SWITCH_NUMERIC(vr, PEEK);
#undef PEEK

    return result;
}


// use a VR to write a double to a double pointer
static void double_to_value(DcmVR vr, double *result, double value)
{
    *result = 0.0;
#define POKE(TYPE) *((TYPE *) result) = (TYPE) value;
    DCM_SWITCH_NUMERIC(vr, POKE);
#undef POKE
}


static bool element_check_float(DcmError **error,
                                const DcmElement *element)
{
    if (element->vr != DCM_VR_FL && element->vr != DCM_VR_FD) {
      dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                    "data element is not float",
                    "element tag %08x is not one of the float types",
                    element->tag);
      return false;
    }

    return true;
}


bool dcm_element_get_value_decimal(DcmError **error,
                                   const DcmElement *element,
                                   uint32_t index,
                                   double *value)
{
    if (!element_check_assigned(error, element) ||
        !element_check_numeric(error, element) ||
        !element_check_float(error, element) ||
        !element_check_index(error, element, index)) {
        return false;
    }

    double *element_value;
    if (element->vm == 1) {
        element_value = (double *) &element->value.single.fd;
    } else {
        size_t size = dcm_dict_vr_size(element->vr);
        unsigned char *base = (unsigned char *) element->value.multi.fd;
        element_value = (double *)(base + size * index);
    }
    *value = value_to_double(element->vr, element_value);

    return true;
}


bool dcm_element_set_value_decimal(DcmError **error,
                                   DcmElement *element,
                                   double value)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_numeric(error, element) ||
        !element_check_float(error, element)) {
        return false;
    }

    double *element_value = (double *) &element->value.single.fd;
    double_to_value(element->vr, element_value, value);
    element->vm = 1;
    element_set_length(element, (uint32_t) dcm_dict_vr_size(element->vr));

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    return true;
}


// The VRs with binary values

static bool element_check_binary(DcmError **error,
                                 const DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    if (vr_class != DCM_VR_CLASS_BINARY) {
      dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                    "data element is not binary",
                    "element tag %08x does not have a binary value",
                    element->tag);
      return false;
    }

    return true;
}


bool dcm_element_get_value_binary(DcmError **error,
                                  const DcmElement *element,
                                  const void **value)
{
    if (!element_check_assigned(error, element) ||
        !element_check_binary(error, element)) {
        return false;
    }

    *value = element->value.single.bytes;

    return true;
}


bool dcm_element_set_value_binary(DcmError **error,
                                  DcmElement *element,
                                  void *value,
                                  uint32_t length,
                                  bool steal)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_binary(error, element)) {
        return false;
    }

    if (steal) {
        element->value.single.bytes = value;
    } else {
        void *value_copy = DCM_NEW_ARRAY(error, length, char);
        if (value_copy == NULL) {
            return false;
        }

        memcpy(value_copy, value, length);

        element->value.single.bytes = value_copy;
        element->value_pointer = value_copy;
    }

    element->vm = 1;
    element_set_length(element, length);

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    if (steal) {
        element->value_pointer = value;
    }

    return true;
}


/* Set a value from a generic byte buffer. The byte buffer must have been
 * correctly formatted.
 */
bool dcm_element_set_value(DcmError **error,
                           DcmElement *element,
                           char *value,
                           uint32_t length,
                           bool steal)
{
    size_t size;

    switch (dcm_dict_vr_class(element->vr)) {
        case DCM_VR_CLASS_STRING_SINGLE:
        case DCM_VR_CLASS_STRING_MULTI:
            if (!dcm_element_set_value_string(error, element, value, steal)) {
                return false;
            }
            break;

        case DCM_VR_CLASS_NUMERIC_DECIMAL:
        case DCM_VR_CLASS_NUMERIC_INTEGER:
            size = dcm_dict_vr_size(element->vr);
            if (size > 0 &&
                length % size != 0) {
                dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                              "reading of data element failed",
                              "bad byte length for numeric type");
                return false;
            }
            if (!dcm_element_set_value_numeric_multi(error,
                                                     element,
                                                     (int *) value,
                                                     length / (uint32_t) size,
                                                     steal)) {
                return false;
            }
            break;

        case DCM_VR_CLASS_BINARY:
            if (!dcm_element_set_value_binary(error,
                                              element,
                                              value,
                                              length,
                                              steal)) {
                return false;
            }

            break;

        case DCM_VR_CLASS_SEQUENCE:
        default:
            dcm_error_set(error, DCM_ERROR_CODE_PARSE,
                          "reading of data element failed",
                          "data element '%08x' has unexpected "
                          "Value Representation", element->tag);
            return false;
    }

    return true;
}

// Sequence Data Element

static bool element_check_sequence(DcmError **error,
                                   const DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    if (vr_class != DCM_VR_CLASS_SEQUENCE) {
      dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                    "data element is not seeuence",
                    "element tag %08x does not have a seeuence value",
                    element->tag);
      return false;
    }

    return true;
}


bool dcm_element_get_value_sequence(DcmError **error,
                                    const DcmElement *element,
                                    DcmSequence **value)
{
    if (!element_check_assigned(error, element) ||
        !element_check_sequence(error, element)) {
        return false;
    }

    dcm_sequence_lock(element->value.single.sq);
    *value = element->value.single.sq;

    return true;
}


bool dcm_element_set_value_sequence(DcmError **error,
                                    DcmElement *element,
                                    DcmSequence *value)
{
    if (!element_check_not_assigned(error, element) ||
        !element_check_sequence(error, element)) {
        return false;
    }

    uint32_t seq_count = dcm_sequence_count(value);
    uint32_t length = 0;
    for (uint32_t i = 0; i < seq_count; i++) {
        DcmDataSet *item = dcm_sequence_get(error, value, i);
        if (item == NULL) {
            return false;
        }
        for (DcmElement *element = item->elements;
            element;
            element = element->hh.next) {
            length += element->length;
        }
    }
    element_set_length(element, length);

    element->value.single.sq = value;
    element->vm = 1;

    if (!dcm_element_validate(error, element)) {
        return false;
    }

    element->sequence_pointer = value;

    return true;
}


DcmElement *dcm_element_clone(DcmError **error, const DcmElement *element)
{
    uint32_t i;
    DcmSequence *from_seq;

    dcm_log_debug("clone Data Element '%08x'", element->tag);

    DcmElement *clone = dcm_element_create(error, element->tag, element->vr);
    if (clone == NULL) {
        return NULL;
    }
    clone->length = element->length;

    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    switch (vr_class) {
        case DCM_VR_CLASS_SEQUENCE:
            if (!dcm_element_get_value_sequence(error, element, &from_seq)) {
                dcm_element_destroy(clone);
                return NULL;
            }

            // Copy each data set in sequence
            uint32_t count = dcm_sequence_count(from_seq);
            DcmSequence *seq = dcm_sequence_create(error);

            for (i = 0; i < count; i++) {
                DcmDataSet *item = dcm_sequence_get(error, from_seq, i);
                if (item == NULL) {
                    dcm_sequence_destroy(seq);
                    dcm_element_destroy(clone);
                    return NULL;
                }

                DcmDataSet *cloned_item = dcm_dataset_clone(error, item);
                if (cloned_item == NULL) {
                    dcm_sequence_destroy(seq);
                    dcm_element_destroy(clone);
                    return NULL;
                }

                dcm_sequence_append(error, seq, cloned_item);
            }

            if (!dcm_element_set_value_sequence(error, clone, seq)) {
                dcm_sequence_destroy(seq);
                dcm_element_destroy(clone);
                return NULL;
            }

            break;

        case DCM_VR_CLASS_STRING_MULTI:
        case DCM_VR_CLASS_STRING_SINGLE:
            // all the string types
            if (element->vm == 1 && element->value.single.str) {
                clone->value.single.str = dcm_strdup(error,
                                                     element->value.single.str);
                if (clone->value.single.str == NULL) {
                    dcm_element_destroy(clone);
                    return NULL;
                }
                clone->value_pointer = clone->value.single.str;
                clone->vm = 1;
            } else if (element->vm > 1 && element->value.multi.str) {
                clone->value.multi.str = DCM_NEW_ARRAY(error,
                                                       element->vm, char *);
                if (clone->value.multi.str == NULL) {
                    dcm_element_destroy(clone);
                    return NULL;
                }
                clone->value_pointer_array = clone->value.multi.str;

                for (i = 0; i < element->vm; i++) {
                    clone->value.multi.str[i] = dcm_strdup(error,
                                                           element->
                                                           value.multi.str[i]);
                    if (clone->value.multi.str[i] == NULL) {
                        dcm_element_destroy(clone);
                        return NULL;
                    }
                }
                clone->vm = element->vm;
            }

            break;

        case DCM_VR_CLASS_BINARY:
            if (element->value.single.bytes) {
                clone->value.single.bytes = DCM_MALLOC(error, element->length);
                if (clone->value.single.bytes == NULL) {
                    dcm_element_destroy(clone);
                    return NULL;
                }
                memcpy(clone->value.single.bytes,
                       element->value.single.bytes,
                       element->length);
                clone->value_pointer = clone->value.single.bytes;
                clone->vm = 1;
            }
            break;

        case DCM_VR_CLASS_NUMERIC_DECIMAL:
        case DCM_VR_CLASS_NUMERIC_INTEGER:
            if (element->vm == 1) {
                clone->value = element->value;
                clone->vm = 1;
            } else {
                // some kind of numeric value .. we use the float pointer,
                // but this will do all the numeric array types
                size_t size = dcm_dict_vr_size(element->vr);
                clone->value.multi.fl = dcm_calloc(error, element->vm, size);
                if (clone->value.multi.fl == NULL) {
                    dcm_element_destroy(clone);
                    return NULL;
                }
                memcpy(clone->value.multi.fl,
                       element->value.multi.fl,
                       element->vm * size);
                clone->value_pointer = clone->value.multi.fl;
                clone->vm = element->vm;
            }
            break;

        default:
            break;
    }

    if (!dcm_element_validate(error, clone)) {
        dcm_element_destroy(clone);
        return NULL;
    }

    return clone;
}


// printing elements

char *dcm_element_value_to_string(const DcmElement *element)
{
    DcmVRClass vr_class = dcm_dict_vr_class(element->vr);
    size_t size = dcm_dict_vr_size(element->vr);

    char *result = NULL;

    double d;
    int64_t i;
    const char *str;
    const void *val;
    uint32_t n;

    if (element->vm > 1) {
        result = dcm_printf_append(result, "[");
    }

    for (uint32_t index = 0; index < element->vm; index++) {
        switch (vr_class) {
            case DCM_VR_CLASS_NUMERIC_DECIMAL:
                (void) dcm_element_get_value_decimal(NULL,
                                                     element,
                                                     index,
                                                     &d);
                result = dcm_printf_append(result, "%g", d);
                break;

            case DCM_VR_CLASS_NUMERIC_INTEGER:
                (void) dcm_element_get_value_integer(NULL,
                                                     element,
                                                     index,
                                                     &i);

                if (element->vr == DCM_VR_UV) {
                    result = dcm_printf_append(result,
                                               "%"PRIu64,
                                               (uint64_t)i);
                } else if (element->vr == DCM_VR_AT) {
                    // a ushort with half of a tag
                    result = dcm_printf_append(result,
                                               "%04x",
                                               i);
                } else {
                    result = dcm_printf_append(result, "%"PRId64, i);
                }
                break;

            case DCM_VR_CLASS_STRING_SINGLE:
            case DCM_VR_CLASS_STRING_MULTI:
                (void) dcm_element_get_value_string(NULL,
                                                    element,
                                                    index,
                                                    &str);
                result = dcm_printf_append(result, "%s", str);
                break;

            case DCM_VR_CLASS_BINARY:
                (void) dcm_element_get_value_binary(NULL, element, &val);
                n = MIN(16, dcm_element_get_length(element));

                for (i = 0; i < n; i++) {
                    result = dcm_printf_append(result, "%02x",
                                               ((unsigned char *) val)[i]);
                    if (size > 0 &&
                        i % size == size - 1) {
                        result = dcm_printf_append(result, " ");
                    }
                }

                if (dcm_element_get_length(element) > 16) {
                    result = dcm_printf_append(result, "...");
                }
                break;

            case DCM_VR_CLASS_SEQUENCE:
                result = dcm_printf_append(result, "<sequence>");
                break;

            default:
                dcm_log_warning("unexpected Value Representation");
        }

        if (element->vm > 1) {
            if (index == element->vm - 1) {
                result = dcm_printf_append(result, "]");
            } else {
                result = dcm_printf_append(result, ", ");
            }
        }
    }

    // AT is a two-element ushort array holding a DICOM tag ... print the tag
    // name if we can
    if (element->vr == DCM_VR_AT && element->vm == 2) {
        int64_t grp;
        int64_t ele;
        (void) dcm_element_get_value_integer(NULL, element, 0, &grp);
        (void) dcm_element_get_value_integer(NULL, element, 1, &ele);

        uint32_t tag = (uint32_t) grp << 16 | (uint32_t) ele;

        const char *keyword = dcm_dict_keyword_from_tag(tag);

        if (keyword) {
            result = dcm_printf_append(result, " (%s)", keyword);
        }
    }

    return result;
}


void dcm_element_print(const DcmElement *element, int indentation)
{
    const int num_indent = indentation * 2;
    const int num_indent_next = (indentation + 1) * 2;

    uint32_t i;

    if (dcm_is_public_tag(element->tag)) {
        const char *keyword = dcm_dict_keyword_from_tag(element->tag);
        printf("%*.*s(%04x,%04x) %s | %s",
               num_indent,
               num_indent,
               "                                   ",
               dcm_element_get_group_number(element),
               dcm_element_get_element_number(element),
               keyword,
               dcm_dict_str_from_vr(element->vr));
    } else {
        // private tag, or unknown public tag
        // in any case, we can't display the keyword
        printf("%*.*s (%04x,%04x) | %s",
               num_indent,
               num_indent,
               "                                   ",
               dcm_element_get_group_number(element),
               dcm_element_get_element_number(element),
               dcm_dict_str_from_vr(element->vr));
    }

    if (element->vr == DCM_VR_SQ) {
        DcmSequence *sequence;
        (void) dcm_element_get_value_sequence(NULL, element, &sequence);
        uint32_t sequence_count = dcm_sequence_count(sequence);
        if (sequence_count == 0) {
            printf(" | [");
        } else {
            printf(" | [\n");
        }
        for (i = 0; i < sequence_count; i++) {
            printf("%*.*s---Item #%d---\n",
                   num_indent_next,
                   num_indent_next,
                   "                                   ",
                   i + 1);
            DcmDataSet *item = dcm_sequence_get(NULL, sequence, i);
            dcm_dataset_print(item, indentation + 1);
        }
        printf("%*.*s]\n",
               num_indent,
               num_indent,
               "                                   ");
    } else {
        printf(" | %u | %u | ", element->length, element->vm);
        char *str = dcm_element_value_to_string(element);
        if (str != NULL) {
            printf("%s\n", str);
            free(str);
        }
    }
}


// Datasets

DcmDataSet *dcm_dataset_create(DcmError **error)
{
    dcm_log_debug("create Data Set");
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
    dcm_log_debug("clone Data Set");
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
            dcm_element_destroy(cloned_element);
            dcm_dataset_destroy(cloned_dataset);
            return NULL;
        }
    }

    return cloned_dataset;
}


static bool dataset_check_not_locked(DcmError **error, DcmDataSet *dataset)
{
    if (dataset->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID, "data set is locked", "");
        return false;
    }

    return true;
}


DcmElement *dcm_dataset_contains(const DcmDataSet *dataset, uint32_t tag)
{
    DcmElement *element;
    HASH_FIND_INT(dataset->elements, &tag, element);

    return element;
}


bool dcm_dataset_insert(DcmError **error,
                        DcmDataSet *dataset, DcmElement *element)
{
    if (!element_check_assigned(error, element) ||
        !dataset_check_not_locked(error, dataset)) {
        return false;
    }

    DcmElement *matched_element = dcm_dataset_contains(dataset, element->tag);
    if (matched_element) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "element already exists",
                      "inserting data element '%08x' into data set failed",
                      element->tag);
        return false;
    }

    HASH_ADD_INT(dataset->elements, tag, element);

    return true;
}


DcmElement *dcm_dataset_get(DcmError **error,
                            const DcmDataSet *dataset, uint32_t tag)
{
    dcm_log_debug("get Data Element '%08x' from data set", tag);

    DcmElement *element = dcm_dataset_contains(dataset, tag);
    if (element == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "could not find data element",
                      "getting data element '%08x' from data set failed",
                      tag);
    }

    return element;
}


DcmElement *dcm_dataset_get_clone(DcmError **error,
                                  const DcmDataSet *dataset, uint32_t tag)
{
    dcm_log_debug("copy data element '%08x' from data set", tag);

    DcmElement *element = dcm_dataset_get(error, dataset, tag);
    if (element == NULL) {
        return NULL;
    }

    return dcm_element_clone(error, element);
}


bool dcm_dataset_remove(DcmError **error, DcmDataSet *dataset, uint32_t tag)
{
    if (!dataset_check_not_locked(error, dataset)) {
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


bool dcm_dataset_foreach(const DcmDataSet *dataset,
                         bool (*fn)(const DcmElement *element, void *client),
                         void *client)
{
    DcmElement *element;

    for(element = dataset->elements; element; element = element->hh.next) {
        if (!fn(element, client)) {
            return false;
        }
    }

    return true;
}


uint32_t dcm_dataset_count(const DcmDataSet *dataset)
{
    uint32_t num_users = HASH_COUNT(dataset->elements);
    return num_users;
}


void dcm_dataset_copy_tags(const DcmDataSet *dataset,
                           uint32_t *tags, uint32_t n)
{
    uint32_t i;
    DcmElement *element;

    for(i = 0, element = dataset->elements;
        element && i < n;
        element = element->hh.next, i++) {
        tags[i] = element->tag;
    }

    qsort(tags, n, sizeof(uint32_t), compare_tags);
}


void dcm_dataset_print(const DcmDataSet *dataset, int indentation)
{
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
            dcm_log_warning("missing tag");
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
                      "out of memory",
                      "creation of Sequence failed");
        free(seq);
        return NULL;
    }
    seq->items = items;
    seq->is_locked = false;

    return seq;
}


static bool sequence_check_not_locked(DcmError **error, DcmSequence *seq)
{
    if (seq->is_locked) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "sequence is locked", "");
        return false;
    }

    return true;
}


bool dcm_sequence_append(DcmError **error, DcmSequence *seq, DcmDataSet *item)
{
    if (!sequence_check_not_locked(error, seq)) {
        return false;
    }

    dcm_log_debug("append item to sequence");

    /**
     * The SequenceItem is just a thin wrapper around a DcmDataSet object as a
     * handle for utarray. Under the hood, utarray frees the memory of the
     * DcmDataSet object when the array item gets destroyed. However, utarray
     * does not free the memory of the item handle. Therefore, we need to free
     * the memory of the item handle after the item was added to the array.
     */
    struct SequenceItem *seq_item = create_sequence_item(error, item);
    utarray_push_back(seq->items, seq_item);
    free(seq_item);

    return true;
}


static struct SequenceItem *sequence_get_index(const DcmSequence *seq,
                                               uint32_t index)
{
    return (struct SequenceItem*)utarray_eltptr(seq->items, index);
}


static struct SequenceItem *sequence_check_index(DcmError **error,
                                                 const DcmSequence *seq,
                                                 uint32_t index)
{
    uint32_t length = utarray_len(seq->items);
    if (index >= length) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "item of sequence invalid",
                      "index %i exceeds length of sequence %i",
                      index, length);
        return NULL;
    }

    struct SequenceItem *seq_item = sequence_get_index(seq, index);
    if (seq_item == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "item of sequence invalid",
                      "getting item #%i of Sequence failed", index);
        return NULL;
    }
    if (seq_item->dataset == NULL) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "item of sequence invalid",
                      "getting item #%i of sequence failed", index);
        return NULL;
    }

    return seq_item;
}


DcmDataSet *dcm_sequence_get(DcmError **error,
                             const DcmSequence *seq, uint32_t index)
{
    struct SequenceItem *seq_item = sequence_check_index(error, seq, index);
    if (seq_item == NULL) {
        return NULL;
    }

    dcm_dataset_lock(seq_item->dataset);

    return seq_item->dataset;
}


DcmDataSet *dcm_sequence_steal(DcmError **error,
                               const DcmSequence *seq, uint32_t index)
{
    struct SequenceItem *seq_item = sequence_check_index(error, seq, index);
    if (seq_item == NULL) {
        return NULL;
    }

    DcmDataSet *result = seq_item->dataset;
    seq_item->dataset = NULL;
    // this will free the SequenceItem
    utarray_erase(seq->items, index, 1);

    return result;
}


bool dcm_sequence_foreach(const DcmSequence *seq,
                          bool (*fn)(const DcmDataSet *item,
                                     uint32_t index,
                                     void *client),
                          void *client)
{
    uint32_t length = utarray_len(seq->items);
    for (uint32_t index = 0; index < length; index++) {
        struct SequenceItem *seq_item = sequence_get_index(seq, index);
        DcmDataSet *dataset = seq_item->dataset;

        dcm_dataset_lock(dataset);

        if (!fn(dataset, index, client)) {
            return false;
        }
    }

    return true;
}


bool dcm_sequence_remove(DcmError **error, DcmSequence *seq, uint32_t index)
{
    if (!sequence_check_not_locked(error, seq) ||
        sequence_check_index(error, seq, index) == NULL) {
        return false;
    }

    dcm_log_debug("remove item #%i from Sequence", index);

    utarray_erase(seq->items, index, 1);

    return true;
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
                      "constructing frame item failed",
                      "PixelData cannot be empty");
        return NULL;
    }

    if (bits_allocated != 1 && bits_allocated % 8 != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "constructing frame item failed",
                      "wrong number of bits allocated");
        return NULL;
    }

    if (bits_stored != 1 && bits_stored % 8 != 0) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "constructing frame item failed",
                      "wrong number of bits stored");
        return NULL;
    }

    if (pixel_representation != 0 && pixel_representation != 1) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "constructing frame item failed",
                      "wrong pixel representation");
        return NULL;
    }

    if (planar_configuration != 0 && planar_configuration != 1) {
        dcm_error_set(error, DCM_ERROR_CODE_INVALID,
                      "constructing frame item failed",
                      "wrong planar configuration");
        return NULL;
    }

    DcmFrame *frame = DCM_NEW(error, DcmFrame);
    if (frame == NULL) {
        return NULL;
    }

    frame->photometric_interpretation = dcm_strdup(error,
                                                   photometric_interpretation);
    if (frame->photometric_interpretation == NULL) {
        dcm_frame_destroy(frame);
        return NULL;
    }

    frame->transfer_syntax_uid = dcm_strdup(error, transfer_syntax_uid);
    if (frame->transfer_syntax_uid == NULL) {
        dcm_frame_destroy(frame);
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


bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid)
{
    return
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.1.99") != 0 &&
        strcmp(transfer_syntax_uid, "1.2.840.10008.1.2.2") != 0;
}
