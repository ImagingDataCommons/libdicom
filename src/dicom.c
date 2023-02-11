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

#include "dicom.h"
#include "pdicom.h"


void *dcm_calloc(DcmError **error, size_t n, size_t size)
{
    void *result = calloc(n, size);
    if (!result) {
        dcm_error_set(error, DCM_ERROR_CODE_NOMEM,
                      "Out of memory",
                      "Failed to allocate %zd bytes", n * size);
        return NULL;
    }
    return result;
}


char *dcm_strdup(DcmError **error, const char *str)
{
    if (str == NULL)
        return NULL;

    size_t length = strlen(str);
    char *new_str = DCM_MALLOC(error, length + 1);
    if (new_str == NULL) {
        return NULL;
    }
    strcpy(new_str, str);

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


DcmErrorCode dcm_error_code(DcmError *error)
{
    return error->code;
}


const char *dcm_error_message(DcmError *error)
{
    return (const char *) error->message;
}


const char *dcm_error_summary(DcmError *error)
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


typedef struct _DcmIOFile {
    int fd;
    char *filename;
} DcmIOFile;

void *dcm_io_open_file(DcmError **error, void *client)
{
    DcmIOFile *io = DCM_NEW(error, DcmIOFile);
    if (io == NULL) {
        return NULL;
    }

    // The "not set" value for fd
    io->fd = -1;

    const char *filename = (const char *) client;
    io->filename = dcm_strdup(error, filename);
    if (io->filename == NULL) {
        dcm_io_file_close(error, io);
        return NULL;
    }

    errno_t open_errno;

#ifdef _WIN32
    int oflag = _O_BINARY | _O_RDONLY | _O_RANDOM;
    int shflag = _SH_DENYWR;
    int pmode = 0;
    open_errno = _sopen_s(&io->fd, io->filename, oflag, sgflag, pmode);
#else
    int flags = O_READ | O_BINARY;
    mode_t mode = 0;
    do
        io->fd = open(io->filename, flags, mode);
    while fd == -1 && errno == EINTR;

    open_errno = errno;
#endif

    if (io->fd == -1) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to open file",
            "Unable to open %s - %s", io->filename, strerror(open_errno));
        dcm_io_file_close(error, io);
        return NULL;
    }

    return io;
}


int dcm_io_close_file(DcmError **error, void *data)
{
    DcmIOFile *io = (DcmIOFile *) data;

    errno_t close_errno = 0;

    if (io->fd != -1) {
        if (close(io->fd)) {
            close_errno = errno;
        }

        io->fd = -1;

        if (close_errno) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unable to close file",
                "Unable to close %s - %s", io->filename, strerror(close_errno));
        }
    }

    free(io->filename);
    free(io);

    return close_errno;
}


int64_t dcm_io_read_file(DcmError **error, void *data, 
    char *buffer, int64_t length)
{
    DcmIOFile *io = (DcmIOFile *) data;

    int64_t bytes_read;

#ifdef _WIN32
    bytes_read = _read(io->fd, buffer, length);
#else
    do {
        bytes_read = read(io->fd, buffer, length);
    } while bytes_read < 0 && errno == EINTR;
#endif

    if (bytes_read < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to read from file",
            "Unable to read %s - %s", io->filename, strerror(errno));
    }

    return bytes_read;
}


int64_t dcm_io_seek_file(DcmError **error void *data, 
    int64_t offset, int whence)
{
    DcmIOFile *io = (DcmIOFile *) data;

    int64_t new_offset;

#ifdef _WIN32
    new_offset = _lseeki64(io->fd, offset, whence);
#else
    new_offset = lseek(io->fd, offset, whence);
#endif

    if (new_offset < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to seek file",
            "Unable to seek %s - %s", io->filename, strerror(errno));
    }

    return new_offset;
}


bool dcm_io_require(DcmError **error, DcmIO *io, void *data, 
    char *buffer, int64_t length, int64_t *position)
{
    do {
        int64_t bytes_read = io->read(error, data, buffer, length);

        if (bytes_read == 0) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "End of file",
                "Needed %zd bytes beyond end of file", length);
            return false;
        }

        buffer += bytes_read;
        length -= bytes_read;
        *position += bytes_read;
    } while length > 0;

    return true;
}

int64_t dcm_io_tell(DcmError **error, DcmIO *io, void *data)
{
    return io->seek(error, data, 0, SEEK_CUR);
}
