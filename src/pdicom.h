/* Declarations not in the public API.
 */

#ifndef NDEBUG
#  define DCM_DEBUG_ONLY( ... ) __VA_ARGS__
#else
#  define DCM_DEBUG_ONLY( ... )
#endif

#define DCM_MALLOC(ERROR, SIZE) \
    dcm_calloc(ERROR, 1, SIZE)

#define DCM_NEW(ERROR, TYPE) \
    (TYPE *) dcm_calloc(ERROR, 1, sizeof(TYPE))

#define DCM_NEW_ARRAY(ERROR, N, TYPE) \
    (TYPE *) dcm_calloc(ERROR, N, sizeof(TYPE))

#define DCM_RETURN_VALUE_IF_FAIL(ERROR, CONDITION, RETURN_VALUE) \
    if (!(CONDITION)) { \
        dcm_error_set((ERROR), DCM_ERROR_CODE_INVALID, \
            "Test fail", \
            "%s:%s (%d)" \
            __FILE__, __FUNCTION__, __LINE__); \
        return RETURN_VALUE; \
    }

#define DCM_RETURN_IF_FAIL(ERROR, CONDITION) \
    if (!(CONDITION)) { \
        dcm_error_set((ERROR), DCM_ERROR_CODE_INVALID, \
            "Test fail", \
            "%s:%s (%d)" \
            __FILE__, __FUNCTION__, __LINE__); \
        return; \
    }

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define USED(x) (void)(x)

#define TAG_DIMENSION_INDEX_VALUES                  0x00209157
#define TAG_REFERENCED_IMAGE_NAVIGATION_SEQUENCE    0x00480200
#define TAG_PER_FRAME_FUNCTIONAL_GROUP_SEQUENCE     0x52009230
#define TAG_EXTENDED_OFFSET_TABLE                   0x7FE00001
#define TAG_FLOAT_PIXEL_DATA                        0x7FE00008
#define TAG_DOUBLE_PIXEL_DATA                       0x7FE00009
#define TAG_PIXEL_DATA                              0x7FE00010
#define TAG_TRAILING_PADDING                        0xFFFCFFFC
#define TAG_ITEM                                    0xFFFEE000
#define TAG_ITEM_DELIM                              0xFFFEE00D
#define TAG_SQ_DELIM                                0xFFFEE0DD


void *dcm_calloc(DcmError **error, size_t n, size_t size);

char *dcm_strdup(DcmError **error, const char *str);

void dcm_free_string_array(char **strings, int n);

/* The general class of the value associated with a VR.
 *
 * STRING_MULTI -- one or more null-terminated strings, cannot contain backslash
 *
 * STRING_SINGLE -- a single null-terminated string, backslash allowed
 *
 * NUMERIC -- one or more binary numeric values (float etc.), other fields
 * give sizeof(type)
 *
 * BINARY -- an uninterpreted array of bytes, length in the element header
 */
typedef enum _DcmVRClass {
    DCM_CLASS_ERROR,
    DCM_CLASS_STRING_MULTI,
    DCM_CLASS_STRING_SINGLE,
    DCM_CLASS_NUMERIC,
    DCM_CLASS_BINARY,
    DCM_CLASS_SEQUENCE
} DcmVRClass;

size_t dcm_dict_vr_size(DcmVR vr);
DcmVRClass dcm_dict_vr_class(DcmVR vr);
uint32_t dcm_dict_vr_capacity(DcmVR vr);
int dcm_dict_vr_header_length(DcmVR vr);

#define DCM_SWITCH_NUMERIC(VR, OPERATION) \
    switch (VR) { \
        case DCM_VR_FL: OPERATION(float); break; \
        case DCM_VR_FD: OPERATION(double); break; \
        case DCM_VR_SL: OPERATION(int32_t); break; \
        case DCM_VR_SS: OPERATION(int16_t); break; \
        case DCM_VR_UL: OPERATION(uint32_t); break; \
        case DCM_VR_US: OPERATION(uint16_t); break; \
        case DCM_VR_SV: OPERATION(int64_t); break; \
        case DCM_VR_UV: OPERATION(uint64_t); break; \
        default: break; \
    }

typedef struct _DcmParse {
    bool (*dataset_begin)(DcmError **, void *client);
    bool (*dataset_end)(DcmError **, void *client);

    bool (*sequence_begin)(DcmError **, void *client);
    bool (*sequence_end)(DcmError **, 
                         void *client,
                         uint32_t tag,
                         DcmVR vr,
                         uint32_t length);

    bool (*element_create)(DcmError **, 
                           void *client, 
                           uint32_t tag,
                           DcmVR vr,
                           char *value,
                           uint32_t length);

    bool (*stop)(void *client, 
                 uint32_t tag, 
                 DcmVR vr, 
                 uint32_t length);
} DcmParse;

DCM_EXTERN 
bool dcm_parse_dataset(DcmError **error, 
                       DcmIO *io,
                       bool implicit,
                       bool byteswap,
                       const DcmParse *parse, 
                       void *client);

DCM_EXTERN 
bool dcm_parse_group(DcmError **error, 
                     DcmIO *io,
                     bool implicit,
                     bool byteswap,
                     const DcmParse *parse, 
                     void *client);

bool dcm_parse_pixeldata(DcmError **error,
                         DcmIO *io,
                         bool implicit,
                         bool byteswap,
                         int64_t *first_frame_offset,
                         int64_t *offsets,
                         int num_frames);

struct PixelDescription {
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

char *dcm_parse_frame(DcmError **error,
                      DcmIO *io,
                      bool implicit,
                      bool byteswap,
                      struct PixelDescription *desc,
                      uint32_t *length);
