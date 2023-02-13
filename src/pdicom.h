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


void *dcm_calloc(DcmError **error, size_t n, size_t size);

char *dcm_strdup(DcmError **error, const char *str);

void dcm_free_string_array(char **strings, int n);
