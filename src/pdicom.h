/* Declarations not in the public API.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strncpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif


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

