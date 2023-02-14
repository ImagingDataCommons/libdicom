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


#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define USED(x) (void)(x)


void *dcm_calloc(DcmError **error, size_t n, size_t size);

char *dcm_strdup(DcmError **error, const char *str);

void dcm_free_string_array(char **strings, int n);

