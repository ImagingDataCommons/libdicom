/*
 * Implementation of subroutines that are independent of the DICOM standard.
 */
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"

#include "dicom.h"


void *dcm_calloc(size_t n, size_t size)
{
    void *result = calloc(n, size);
    if(!result) {
        dcm_log_error("Failed to allocate and initialize memory.");
        return NULL;
    }
    return result;
}


const char *dcm_get_version(void)
{
    return SUFFIXED_VERSION;
}


static DcmLogLevel dcm_log_level = DCM_LOG_NOTSET;

DcmLogLevel dcm_log_set_level(DcmLogLevel log_level)
{
    DcmLogLevel previous_log_level;

    previous_log_level = dcm_log_level;

    if ((dcm_log_level >= DCM_LOG_NOTSET) &&
        (dcm_log_level <= DCM_LOG_CRITICAL)) {
        dcm_log_level = log_level;
    }

    return previous_log_level;
}


#ifndef _WIN32
static int ctime_s(char *buf, size_t size, const time_t *time)
{
    assert(size >= 26);
    ctime_r(time, buf);
    return errno;
}
#endif

static void dcm_default_logf(const char *level, const char *format,
    va_list args)
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


static DcmLogf dcm_current_logf = dcm_default_logf;

DcmLogf dcm_log_set_logf(DcmLogf logf)
{
    DcmLogf previous_logf;

    previous_logf = dcm_current_logf;
    dcm_current_logf = logf;

    return previous_logf;
}


static void dcm_logf(const char *level, const char *format, va_list args)
{
    if (dcm_current_logf) {
        dcm_current_logf(level, format, args);
    }
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


DcmError *dcm_error_newf(const char *domain, int code, const char *format,
    va_list args) {
    DcmError *error;

    error = DCM_NEW (DcmError);
    if (error == NULL) {
    }
}


DcmError *dcm_error_new(const char *domain, int code, const char *format, ...) {
}
