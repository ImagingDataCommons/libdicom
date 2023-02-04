/*
 * Implementation of subroutines that are independent of the DICOM standard.
 */

#ifdef _WIN32
// the Windows CRT considers strncpy unsafe
#define _CRT_SECURE_NO_WARNINGS
// and deprecates strdup
#define strdup(v) _strdup(v)
#endif

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"

#include "dicom.h"


void *dcm_calloc(DcmError **error, size_t n, size_t size)
{
    void *result = calloc(n, size);
    if (!result) {
        dcm_error_set(error, DCM_ERROR_CODE_NOMEM,
                      "Failed to allocate %zd bytes.", n * size);
        return NULL;
    }
    return result;
}


const char *dcm_get_version(void)
{
    return DCM_SUFFIXED_VERSION;
}


const char *dcm_error_code_str(DcmErrorCode code)
{
    switch (code) {
        case DCM_ERROR_CODE_NOMEM:
            return "Out of memory";

        case DCM_ERROR_CODE_INVALID:
            return "Invalid parameter";

        default:
            return "Unknown error code";
    }
}


const char *dcm_error_code_name(DcmErrorCode code)
{
    switch (code) {
        case DCM_ERROR_CODE_NOMEM:
            return "NOMEM";

        case DCM_ERROR_CODE_INVALID:
            return "INVALID";

        default:
            return "UNKNOWN";
    }
}


struct _DcmError {
    DcmErrorCode code;
    char *message;
};

static void dcm_error_free(DcmError *error)
{
    if (error) {
        free(error->message);
        error->message = NULL;
        free(error);
    }
}


static DcmError *dcm_error_newf(DcmErrorCode code, 
    const char *format, va_list ap)
{
    DcmError *error;
    char txt[256];

    error = DCM_NEW(NULL, DcmError);
    if (!error) {
        return NULL;
    }
    error->code = code;
    vsnprintf(txt, sizeof(txt), format, ap);
    error->message = strdup(txt);

    dcm_error_log(error);

    return error;
}


void dcm_error_set(DcmError **error, 
    DcmErrorCode code, const char *format, ...)
{
    if (error) {
        if (*error) {
            dcm_log_critical("DcmError set twice");
            return;
        }

        va_list(ap);
        va_start(ap, format);
        *error = dcm_error_newf(code, format, ap);
        va_end(ap);
    }
}


void dcm_error_clear(DcmError **error)
{
    if (error) {
        dcm_error_free(*error);
        *error = NULL;
    }
}


DcmErrorCode dcm_error_code(DcmError *error)
{
    return error->code;
}


const char *dcm_error_message(DcmError *error)
{
    return (const char *) error->message;
}


void dcm_error_log(DcmError *error)
{
    if (error) {
        dcm_log_critical("%s (%d): %s", 
                         dcm_error_code_str(error->code),
                         dcm_error_code_name(error->code),
                         error->message);
    }
}


DcmLogLevel dcm_log_level = DCM_LOG_NOTSET;


#ifndef _WIN32
static int ctime_s(char *buf, size_t size, const time_t *time)
{
    assert(size >= 26);
    ctime_r(time, buf);
    return errno;
}
#endif

static void dcm_logf(const char *level, const char *format, va_list args)
{
    time_t now;
    time(&now);
    char datetime[26];
    ctime_s(datetime, sizeof(datetime), &now);
    datetime[strcspn(datetime, "\n")] = '\0';
    fprintf(stderr, "%s [%s] - ", level, datetime);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}


void dcm_log_critical(const char *format, ...)
{
    if (dcm_log_level <= DCM_LOG_CRITICAL) {
        va_list(args);
        va_start(args, format);
        dcm_logf("CRITICAL", format, args);
        va_end(args);
    }
}


void dcm_log_error(const char *format, ...)
{
    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_ERROR)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("ERROR   ", format, args);
        va_end(args);
    }
}


void dcm_log_warning(const char *format, ...)
{
    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_WARNING)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("WARNING ", format, args);
        va_end(args);
    }
}


void dcm_log_info(const char *format, ...)
{
    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_INFO)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("INFO    ", format, args);
        va_end(args);
    }
}


void dcm_log_debug(const char *format, ...)
{
    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_DEBUG)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("DEBUG   ", format, args);
        va_end(args);
    }
}
