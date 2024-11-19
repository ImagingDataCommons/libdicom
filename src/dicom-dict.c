/*
 * Implementation of Part 6 of the DICOM standard: Data Dictionary.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strdup and strcpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uthash.h"

#include <dicom/dicom.h>

#include "pdicom.h"
#include "dicom-dict-lookup.h"

#define LOOKUP(table, field, hash, key, key_len, out) do {		\
        unsigned hash_value;						\
        HASH_VALUE(key, key_len, hash_value);				\
        for (int probe = 0; true; probe++) {				\
            int i = hash ## _dict[(hash_value + probe) % hash ## _len];	\
            if (i == hash ## _empty || probe == LOOKUP_MAX_PROBES) {	\
                (out) = NULL;						\
                break;							\
            } else if (!memcmp(key, &(table)[i].field, key_len)) {	\
                (out) = &(table)[i];					\
                break;							\
            }								\
        }								\
    } while (0)


static const struct _DcmVRTable *vrtable_from_vr(const char *vr)
{
    const struct _DcmVRTable *table;

    LOOKUP(dcm_vr_table, str,
           dcm_vrtable_from_str, vr, strlen(vr),
           table);

    return table;
}


bool dcm_is_valid_vr(const char *str)
{
    const struct _DcmVRTable *table;

    return str &&
        (table = vrtable_from_vr(str)) &&
        table->vr != DCM_VR_ERROR;
}


DcmVR dcm_dict_vr_from_str(const char *str)
{
    const struct _DcmVRTable *table;

    if (str &&
        (table = vrtable_from_vr(str))) {
        return table->vr;
    }

    return DCM_VR_ERROR;
}


const char *dcm_dict_str_from_vr(DcmVR vr)
{
    if (vr >= 0 && vr < DCM_VR_LAST) {
        return dcm_vr_table[(int)vr].str;
    }

    return NULL;
}


DcmVRClass dcm_dict_vr_class(DcmVR vr)
{
    DcmVRTag vr_tag = (DcmVRTag) vr;

    // these are all ints of various sizes and signedness
    if (vr == DCM_VR_TAG_OB_OW ||
        vr == DCM_VR_TAG_US_OW ||
        vr == DCM_VR_TAG_US_SS ||
        vr == DCM_VR_TAG_US_SS_OW) {
        return DCM_VR_CLASS_NUMERIC_INTEGER;
    }

    if (vr >= 0 && vr < DCM_VR_LAST) {
        return dcm_vr_table[(int)vr].vr_class;
    }

    return DCM_VR_CLASS_ERROR;
}


size_t dcm_dict_vr_size(DcmVR vr)
{
    if (vr >= 0 && vr < DCM_VR_LAST) {
        return dcm_vr_table[(int)vr].size;
    }

    return 0;
}


uint32_t dcm_dict_vr_capacity(DcmVR vr)
{
    if (vr >= 0 && vr < DCM_VR_LAST) {
        return dcm_vr_table[(int)vr].capacity;
    }

    return 0;
}


int dcm_dict_vr_header_length(DcmVR vr)
{
    if (vr >= 0 && vr < DCM_VR_LAST) {
        return dcm_vr_table[(int)vr].header_length;
    }

    return 0;
}


static const struct _DcmAttribute *attribute_from_tag(uint32_t tag)
{
    const struct _DcmAttribute *attribute;

    // tags with element number 0 are generic group length tags ... map all of
    // these (except 0000,0000) to tag 0008,0000 (GenericGroupLength)
    if (tag != 0 &&
        (tag & 0xffff) == 0) {
        tag = 0x00080000;
    }

    // private creator elements are (gggg, 0010 - 00ff) where gggg is odd
    if ((tag & (1 << 16)) &&
        (tag & 0xffff) >= 0x0010 &&
        (tag & 0xffff) <= 0x00ff) {
        tag = TAG_PRIVATE_CREATOR;
    }

    // private data elements are (gggg, 1000 - ffff) where gggg is odd
    if ((tag & (1 << 16)) &&
        (tag & 0xffff) >= 0x1000 &&
        (tag & 0xffff) <= 0xffff) {
        tag = TAG_PRIVATE_DATA;
    }

    LOOKUP(dcm_attribute_table, tag,
           dcm_attribute_from_tag, &tag, sizeof(tag),
           attribute);

    return attribute;
}


// this will also fail for unknown or retired public tags
bool dcm_is_public_tag(uint32_t tag)
{
    return !dcm_is_private_tag(tag) &&
        attribute_from_tag(tag) != NULL;
}


bool dcm_is_private_tag(uint32_t tag)
{
    uint16_t group_number = tag >> 16;

    // the spec says groups 1, 3, 5, 7 are not allowed
    if (group_number == 0x1 ||
        group_number == 0x3 ||
        group_number == 0x5 ||
        group_number == 0x7) {
        return false;
    }

    return tag & (1 << 16);
}


bool dcm_is_valid_tag(uint32_t tag)
{
    if (tag == 0x00000000) {
        return false;
    }
    return dcm_is_public_tag(tag) || dcm_is_private_tag(tag);
}


// the set of possible VRs for this tag
DcmVRTag dcm_vr_tag_from_tag(uint32_t tag)
{
    const struct _DcmAttribute *attribute;

    if (!(attribute = attribute_from_tag(tag))) {
        return DCM_VR_ERROR;
    }

    return attribute->vr_tag;
}


// pick a default VR for this tag
DcmVR dcm_vr_from_tag(uint32_t tag)
{
    DcmVRTag vr_tag = dcm_vr_tag_from_tag(tag);

    switch (vr_tag) {
        case DCM_VR_TAG_OB_OW:
            return DCM_VR_OB;

        case DCM_VR_TAG_US_OW:
            return DCM_VR_US;

        case DCM_VR_TAG_US_SS:
            return DCM_VR_US;

        case DCM_VR_TAG_US_SS_OW:
            return DCM_VR_US;

        default:
            return vr_tag;
    }
}


bool dcm_is_valid_vr_for_tag(DcmVR vr, uint32_t tag)
{
    // always fail for illegal VRs
    if (vr < 0 || vr >= DCM_VR_LAST) {
        return false;
    }

    // private tags are unknown to us and can have any legal VR
    if (dcm_is_private_tag(tag)) {
        return true;
    }

    const struct _DcmAttribute *attribute = attribute_from_tag(tag);
    if (attribute == NULL) {
        // unknown public tag ... we don't include retired tags in our
        // dictionary, so we can't check them, but we don't want to fail
        // for them either
        return true;
    } else if (vr == (DcmVR) attribute->vr_tag) {
        // trivially equal
        return true;
    } else switch(attribute->vr_tag) {
        case DCM_VR_TAG_OB_OW:
            return vr == DCM_VR_OB || vr == DCM_VR_OW;

        case DCM_VR_TAG_US_OW:
            return vr == DCM_VR_US || vr == DCM_VR_OW;

        case DCM_VR_TAG_US_SS:
            return vr == DCM_VR_US || vr == DCM_VR_SS;

        case DCM_VR_TAG_US_SS_OW:
            return vr == DCM_VR_US || vr == DCM_VR_SS || vr == DCM_VR_OW;

        default:
            return false;
    }
}


const char *dcm_dict_keyword_from_tag(uint32_t tag)
{
    const struct _DcmAttribute *attribute;

    if ((attribute = attribute_from_tag(tag))) {
        return attribute->keyword;
    }

    return NULL;
}


static const struct _DcmAttribute *attribute_from_keyword(const char *keyword)
{
    const struct _DcmAttribute *attribute;

    LOOKUP(dcm_attribute_table, keyword,
           dcm_attribute_from_keyword, keyword, strlen(keyword),
           attribute);

    return attribute;
}


uint32_t dcm_dict_tag_from_keyword(const char *keyword)
{
    const struct _DcmAttribute *attribute = attribute_from_keyword(keyword);
    if (!attribute) {
        // use this as "bad keyword"
        return 0xffffffff;
    }
    return attribute->tag;
}
