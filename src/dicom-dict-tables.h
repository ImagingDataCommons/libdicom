struct _DcmVRTable {
    DcmVR vr;
    char str[3];
    DcmVRClass vr_class;
    size_t size;
    uint32_t capacity;
    int header_length;
};

extern const struct _DcmVRTable dcm_vr_table[];
extern const int dcm_vr_table_len;

/**
 * The VRs and VR alternatives that tags allow. Some tags allow one of several
 * VRs, so this is a superset of DcmVR, meaning you can always cast a DcmVR
 * to a DcmVRTag and you will get a correct result.
 *
 * Note to maintainers: this enum must match DcmVR. The numeric values need
 * to be maintained for ABI compatibility.
 */
typedef enum _DcmVRTag {
    DCM_VR_TAG_error = -1,

    // this part must match DcmVR
    DCM_VR_TAG_AE = 0,
    DCM_VR_TAG_AS,
    DCM_VR_TAG_AT,
    DCM_VR_TAG_CS,
    DCM_VR_TAG_DA,
    DCM_VR_TAG_DS,
    DCM_VR_TAG_DT,
    DCM_VR_TAG_FL,
    DCM_VR_TAG_FD,
    DCM_VR_TAG_IS,
    DCM_VR_TAG_LO,
    DCM_VR_TAG_LT,
    DCM_VR_TAG_OB,
    DCM_VR_TAG_OD,
    DCM_VR_TAG_OF,
    DCM_VR_TAG_OW,
    DCM_VR_TAG_PN,
    DCM_VR_TAG_SH,
    DCM_VR_TAG_SL,
    DCM_VR_TAG_SQ,
    DCM_VR_TAG_SS,
    DCM_VR_TAG_ST,
    DCM_VR_TAG_TM,
    DCM_VR_TAG_UI,
    DCM_VR_TAG_UL,
    DCM_VR_TAG_UN,
    DCM_VR_TAG_US,
    DCM_VR_TAG_UT,
    DCM_VR_TAG_UR,
    DCM_VR_TAG_UC,
    DCM_VR_TAG_OL,
    DCM_VR_TAG_OV,
    DCM_VR_TAG_SV,
    DCM_VR_TAG_UV,

    // we must leave a gap before the combinations start so more VRs can be
    // inserted above
    DCM_VR_TAG_OB_OW = 1000,	// some tags allow several alternative VRs
    DCM_VR_TAG_US_OW,
    DCM_VR_TAG_US_SS,
    DCM_VR_TAG_US_SS_OW
} DcmVRTag;

struct _DcmAttribute {
    uint32_t tag;
    DcmVRTag vr_tag;
    char keyword[63];
};

extern const struct _DcmAttribute dcm_attribute_table[];
extern const int dcm_attribute_table_len;
