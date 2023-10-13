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
#include "dicom-dict-tables.h"

struct _DcmVRTable_hash_entry {
    struct _DcmVRTable table;
    UT_hash_handle hh;
};

static struct _DcmVRTable_hash_entry *vrtable_from_str_dict = NULL;

struct _DcmAttribute_hash_entry {
    struct _DcmAttribute attr;
    UT_hash_handle hh;
};

static struct _DcmAttribute_hash_entry *attribute_from_tag_dict = NULL;

static struct _DcmAttribute_hash_entry *attribute_from_keyword_dict = NULL;


#ifdef HAS_CONSTRUCTOR
__attribute__ ((constructor))
#endif
void dcm_init(void)
{
    if (!vrtable_from_str_dict) {
        int i;

        for (i = 0; i < dcm_vr_table_len; i++) {
            struct _DcmVRTable_hash_entry *entry;

            HASH_FIND_STR(vrtable_from_str_dict, dcm_vr_table[i].str, entry);
            if (entry) {
                dcm_log_critical("Duplicate VR in VR table %s",
                                 dcm_vr_table[i].str);
                return;
            }

            entry = DCM_NEW(NULL, struct _DcmVRTable_hash_entry);
            entry->table = dcm_vr_table[i];
            HASH_ADD_STR(vrtable_from_str_dict, table.str, entry);
        }
    }

    if (!attribute_from_tag_dict) {
        int i;

        for (i = 0; i < dcm_attribute_table_len; i++) {
            struct _DcmAttribute_hash_entry *entry;

            HASH_FIND_INT(attribute_from_tag_dict,
                          &dcm_attribute_table[i].tag, entry);
            if (entry) {
                dcm_log_critical("Duplicate tag in attribute table -- "
                                 "%8X (%s) registered previously as '%s'",
                                 dcm_attribute_table[i].tag,
                                 dcm_attribute_table[i].keyword,
                                 entry->attr.keyword);
                return;
            }

            entry = DCM_NEW(NULL, struct _DcmAttribute_hash_entry);
            entry->attr = dcm_attribute_table[i];
            HASH_ADD_INT(attribute_from_tag_dict, attr.tag, entry);
        }
    }

    if (!attribute_from_keyword_dict) {
        int i;

        for (i = 0; i < dcm_attribute_table_len; i++) {
            struct _DcmAttribute_hash_entry *entry;

            // The "" keyword appears several times and is used for retired
            // tags ... we can't map this to tags unambiguously, so we skip it
            // in the table
            if (strcmp(dcm_attribute_table[i].keyword, "") == 0) {
                continue;
            }

            HASH_FIND_STR(attribute_from_keyword_dict,
                          dcm_attribute_table[i].keyword, entry);
            if (entry) {
                dcm_log_critical("Duplicate keyword in attribute table '%s'",
                                 dcm_attribute_table[i].keyword);
                return;
            }

            entry = DCM_NEW(NULL, struct _DcmAttribute_hash_entry);
            entry->attr = dcm_attribute_table[i];
            HASH_ADD_STR(attribute_from_keyword_dict, attr.keyword, entry);
        }
    }

    if (getenv("DCM_DEBUG")) {
        dcm_log_set_level(DCM_LOG_DEBUG);
    }
}


static const struct _DcmVRTable *vrtable_from_vr(const char *vr)
{
    struct _DcmVRTable_hash_entry *entry;

    dcm_init();

    HASH_FIND_STR(vrtable_from_str_dict, vr, entry);

    return (const struct _DcmVRTable *) entry;
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
    struct _DcmAttribute_hash_entry *entry;

    dcm_init();

    // tags with element number 0 are generic group length tags ... map all of
    // these (except 0000,0000) to tag 0008,0000 (GenericGroupLength)
    if (tag != 0 &&
        (tag & 0xffff) == 0) {
        tag = 0x00080000;
    }

    HASH_FIND_INT(attribute_from_tag_dict, &tag, entry);

    return (const struct _DcmAttribute *) entry;
}


// this will also fail for unknown or retired public tags
bool dcm_is_public_tag(uint32_t tag)
{
    return attribute_from_tag(tag) != NULL;
}


bool dcm_is_private_tag(uint32_t tag)
{
    return tag & (1 << 16);
}


bool dcm_is_valid_tag(uint32_t tag)
{
    if (tag == 0x00000000) {
        return false;
    }
    return dcm_is_public_tag(tag) || dcm_is_private_tag(tag);
}


DcmVR dcm_vr_from_tag(uint32_t tag)
{
    const struct _DcmAttribute *attribute;

    if (!(attribute = attribute_from_tag(tag))) {
        return DCM_VR_ERROR;
    }

    return (DcmVR) attribute->vr_tag;
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
    struct _DcmAttribute_hash_entry *entry;

    dcm_init();

    HASH_FIND_STR(attribute_from_keyword_dict, keyword, entry);

    return (const struct _DcmAttribute *) entry;
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
