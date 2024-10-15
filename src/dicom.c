/*
 * Implementation of subroutines that are independent of the DICOM standard.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strdup and strcpy unsafe
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
#include <fcntl.h>
#include <time.h>

#include <dicom/dicom.h>
#include "pdicom.h"

// we need a namedspaced free for language bindings
void dcm_free(void *pointer)
{
    free(pointer);
}

void *dcm_calloc(DcmError **error, uint64_t n, uint64_t size)
{
    /* malloc(0) behaviour depends on the platform heap implementation. It can
     * return either a valid pointer that can't be dereferenced, or NULL.
     *
     * We need to be able to support dcm_calloc(0), since VM == 0 is allowed,
     * but we can't return NULL in this case, since that's how we detect
     * out of memory.
     *
     * Instead, force n == 0 to n == 1. This means we will always get a valid
     * pointer from calloc, a NULL return always means out of memory, and we
     * can always free the result.
     */
    void *result = calloc(n == 0 ? 1 : n, size);
    if (!result) {
        dcm_error_set(error, DCM_ERROR_CODE_NOMEM,
                      "out of memory",
                      "failed to allocate %zd bytes", n * size);
        return NULL;
    }
    return result;
}


void *dcm_realloc(DcmError **error, void *ptr, uint64_t size)
{
    void *result = realloc(ptr, size);
    if (!result) {
        dcm_error_set(error, DCM_ERROR_CODE_NOMEM,
                      "out of memory",
                      "failed to allocate %zd bytes", size);
        return NULL;
    }
    return result;
}


char *dcm_strdup(DcmError **error, const char *str)
{
    if (str == NULL) {
        return NULL;
    }

    size_t length = strlen(str);
    char *new_str = DCM_MALLOC(error, length + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memmove(new_str, str, length + 1);

    return new_str;
}


char *dcm_printf_append(char *str, const char *format, ...)
{
    va_list(args);

    // find size required for new text
    va_start(args, format);
    ssize_t n = vsnprintf(NULL, 0, format, args);
    if (n < 0) {
        // some very old libcs will return -1 for truncation
        return NULL;
    }
    va_end(args);

    // size of old text
    if (str == NULL) {
        str = dcm_strdup(NULL, "");
        if (str == NULL) {
            return NULL;
        }
    }
    size_t old_len = strlen(str);

    // new space, copy and render
    char *new_str = dcm_realloc(NULL, str, old_len + n + 1);
    if (new_str == NULL) {
        free(str);
        return NULL;
    }
    va_start(args, format);
    vsnprintf(new_str + old_len, n + 1, format, args);
    va_end(args);

    return new_str;
}


void dcm_free_string_array(char **array, int n)
{
    for (int i = 0; i < n; i++) {
        free(array[i]);
    }
    free(array);
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

        case DCM_ERROR_CODE_PARSE:
            return "Parse error";

        case DCM_ERROR_CODE_IO:
            return "IO error";

        case DCM_ERROR_CODE_MISSING_FRAME:
            return "Missing frame";

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

        case DCM_ERROR_CODE_PARSE:
            return "PARSE";

        case DCM_ERROR_CODE_IO:
            return "IO";

        case DCM_ERROR_CODE_MISSING_FRAME:
            return "MISSING_FRAME";

        default:
            return "UNKNOWN";
    }
}


struct _DcmError {
    DcmErrorCode code;
    char *summary;
    char *message;
};

static void dcm_error_free(DcmError *error)
{
    if (error) {
        free(error->summary);
        error->summary = NULL;
        free(error->message);
        error->message = NULL;
        free(error);
    }
}


static DcmError *dcm_error_newf(DcmErrorCode code,
    const char *summary, const char *format, va_list ap)
{
    DcmError *error;
    char txt[256];

    error = DCM_NEW(NULL, DcmError);
    if (!error) {
        return NULL;
    }
    error->code = code;
    error->summary = dcm_strdup(NULL, summary);
    vsnprintf(txt, sizeof(txt), format, ap);
    error->message = dcm_strdup(NULL, txt);

    return error;
}


void dcm_error_set(DcmError **error, DcmErrorCode code,
    const char *summary, const char *format, ...)
{
    if (error && *error) {
        dcm_log_critical("DcmError set twice");
        return;
    }

    if (error) {
        va_list(ap);

        va_start(ap, format);
        *error = dcm_error_newf(code, summary, format, ap);
        va_end(ap);
    } else {
        /* Log to DEBUG so messages don't get completely lost.
         */
        va_list(ap);

        va_start(ap, format);
        DcmError *local_error = dcm_error_newf(code, summary, format, ap);
        va_end(ap);

        dcm_log_debug("%s: %s - %s",
                      dcm_error_code_str(local_error->code),
                      local_error->summary,
                      local_error->message);

        dcm_error_free(local_error);
    }
}


void dcm_error_clear(DcmError **error)
{
    if (error) {
        dcm_error_free(*error);
        *error = NULL;
    }
}


DcmErrorCode dcm_error_get_code(DcmError *error)
{
    return error->code;
}


const char *dcm_error_get_message(DcmError *error)
{
    return (const char *) error->message;
}


const char *dcm_error_get_summary(DcmError *error)
{
    return (const char *) error->summary;
}


void dcm_error_log(DcmError *error)
{
    if (error) {
        dcm_log_error("%s: %s - %s",
                      dcm_error_code_str(error->code),
                      error->summary,
                      error->message);
    }
}


void dcm_error_print(DcmError *error)
{
    if (error) {
        fprintf(stderr, "%s: %s - %s\n",
                      dcm_error_code_str(error->code),
                      error->summary,
                      error->message);
    }
}


static DcmLogLevel dcm_log_level = DCM_LOG_NOTSET;
static bool dcm_inited;

void dcm_init(void)
{
    if (dcm_inited) {
        return;
    }
    dcm_inited = true;
    if (getenv("DCM_DEBUG")) {
        dcm_log_set_level(DCM_LOG_DEBUG);
    }
}

DcmLogLevel dcm_log_set_level(DcmLogLevel log_level)
{
    DcmLogLevel previous_log_level;

    dcm_init();

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
    if (size >= 26) {
        ctime_r(time, buf);
    }
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
    dcm_init();

    if (dcm_log_level <= DCM_LOG_CRITICAL) {
        va_list(args);
        va_start(args, format);
        dcm_logf("CRITICAL", format, args);
        va_end(args);
    }
}


void dcm_log_error(const char *format, ...)
{
    dcm_init();

    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_ERROR)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("ERROR   ", format, args);
        va_end(args);
    }
}


void dcm_log_warning(const char *format, ...)
{
    dcm_init();

    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_WARNING)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("WARNING ", format, args);
        va_end(args);
    }
}


void dcm_log_info(const char *format, ...)
{
    dcm_init();

    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_INFO)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("INFO    ", format, args);
        va_end(args);
    }
}


void dcm_log_debug(const char *format, ...)
{
    dcm_init();

    if ((dcm_log_level > DCM_LOG_NOTSET) & (dcm_log_level <= DCM_LOG_DEBUG)) {
        va_list(args);
        va_start(args, format);
        dcm_logf("DEBUG   ", format, args);
        va_end(args);
    }
}
