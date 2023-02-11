/* Declarations not in the public API.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strncpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
// _close() is the posix-like close()
#define close(v) _close(v)
#endif

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

void *dcm_io_open_file(DcmError **error, void *client);

int dcm_io_close_file(DcmError **error, void *data);

int64_t dcm_io_read_file(DcmError **error, void *data, 
    char *buffer, int64_t length);

int64_t dcm_io_seek_file(DcmError **error void *data, 
    int64_t offset, int whence);

bool dcm_io_require(DcmError **error, DcmIO *io, void *data, 
    char *buffer, int64_t length, int64_t *position)

int64_t dcm_io_tell(DcmError **error, DcmIO *io, void *data);
