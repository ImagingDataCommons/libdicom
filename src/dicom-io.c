/*
 * Implementation of subroutines that are independent of the DICOM standard.
 */

#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strdup and strcpy unsafe
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
// and deprecates strdup
#define strdup(v) _strdup(v)
#include <share.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif /*HAVE_IO_H*/
#include <time.h>

#include <dicom/dicom.h>
#include "pdicom.h"

/* The size of the input buffer we use.
 */
#define BUFFER_SIZE (4096)

typedef struct _DcmIOFile {
    int fd;
    char *filehandlename;
    char input_buffer[BUFFER_SIZE];
    int bytes_in_buffer;
    int read_point;
} DcmIOFile;


static int dcm_io_close_filehandle(DcmError **error, void *data)
{
    DcmIOFile *io_filehandle = (DcmIOFile *) data;

    int close_errno = 0;

    if (io_filehandle->fd != -1) {
        if (close(io_filehandle->fd)) {
            close_errno = errno;
        }

        io_filehandle->fd = -1;

        if (close_errno) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unable to close filehandle",
                "Unable to close %s - %s",
                io_filehandle->filehandlename, strerror(close_errno));
        }
    }

    free(io_filehandle->filehandlename);
    free(io_filehandle);

    return close_errno;
}


static void *dcm_io_open_filehandle(DcmError **error, void *client)
{
    DcmIOFile *io_filehandle = DCM_NEW(error, DcmIOFile);
    if (io_filehandle == NULL) {
        return NULL;
    }

    // The "not set" value for fd
    io_filehandle->fd = -1;

    const char *filehandlename = (const char *) client;
    io_filehandle->filehandlename = dcm_strdup(error, filehandlename);
    if (io_filehandle->filehandlename == NULL) {
        dcm_io_close_filehandle(error, io_filehandle);
        return NULL;
    }

    int open_errno;

#ifdef _WIN32
    int oflag = _O_BINARY | _O_RDONLY | _O_RANDOM;
    // some mingw are missing this ... just use the numeric value
    // #define _SH_DENYWR 0x20
    int shflag = 0x20;
    int pmode = 0;
    open_errno = _sopen_s(&io_filehandle->fd, io_filehandle->filehandlename,
                          oflag, shflag, pmode);
#else
    int flags = O_RDONLY;
#ifdef O_BINARY
    flags |= O_BINARY;
#endif
    mode_t mode = 0;
    do
        io_filehandle->fd = open(io_filehandle->filehandlename, flags, mode);
    while (io_filehandle->fd == -1 && errno == EINTR);

    open_errno = errno;
#endif

    if (io_filehandle->fd == -1) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to open filehandle",
            "Unable to open %s - %s", io_filehandle->filehandlename, strerror(open_errno));
        dcm_io_close_filehandle(error, io_filehandle);
        return NULL;
    }

    return io_filehandle;
}


static int64_t read_filehandle(DcmError **error, DcmIOFile *io_filehandle,
    char *buffer, int64_t length)
{
    int64_t bytes_read;

#ifdef _WIN32
    bytes_read = _read(io_filehandle->fd, buffer, length);
#else
    do {
        bytes_read = read(io_filehandle->fd, buffer, length);
    } while (bytes_read < 0 && errno == EINTR);
#endif

    if (bytes_read < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to read from filehandle",
            "Unable to read %s - %s", io_filehandle->filehandlename, strerror(errno));
    }

    return bytes_read;
}


/* Refill the input buffer.
 * -1 on error, 0 on EOF, otherwise bytes read.
 */
static int64_t refill(DcmError **error, DcmIOFile *io_filehandle)
{
    // buffer should be empty coming in
    assert(io_filehandle->bytes_in_buffer - io_filehandle->read_point == 0);

    int64_t bytes_read = read_filehandle(error, io_filehandle,
                                   io_filehandle->input_buffer, BUFFER_SIZE);
    if (bytes_read < 0) {
        return bytes_read;
    }

    io_filehandle->read_point = 0;
    io_filehandle->bytes_in_buffer = bytes_read;

    return bytes_read;
}


static int64_t dcm_io_read_filehandle(DcmError **error, void *data,
    char *buffer, int64_t length)
{
    DcmIOFile *io_filehandle = (DcmIOFile *) data;
    int64_t bytes_read = 0;

    while (length > 0) {
        /* Refill the input buffer if it's empty.
         */
        if (io_filehandle->bytes_in_buffer - io_filehandle->read_point == 0) {
            int64_t refill_bytes = refill(error, io_filehandle);
            if (refill_bytes < 0) {
                return refill_bytes;
            } else if (refill_bytes == 0) {
                // we may be read some bytes in a previous loop
                return bytes_read;
            }
        }

        /* Read what we can from the buffer.
         */
        int bytes_available = io_filehandle->bytes_in_buffer - io_filehandle->read_point;
        int bytes_to_copy = MIN(bytes_available, length);

        memcpy(buffer,
               io_filehandle->input_buffer + io_filehandle->read_point,
               bytes_to_copy);
        length -= bytes_to_copy;
        buffer += bytes_to_copy;

        io_filehandle->read_point += bytes_to_copy;
        bytes_read += bytes_to_copy;
    }

    return bytes_read;
}


static int64_t dcm_io_seek_filehandle(DcmError **error, void *data,
    int64_t offset, int whence)
{
    DcmIOFile *io_filehandle = (DcmIOFile *) data;

    /* We've read ahead by some number of buffered bytes, so first undo that,
     * then do the seek from the true position.
     */
    int64_t new_offset;

    int64_t bytes_ahead = io_filehandle->bytes_in_buffer - io_filehandle->read_point;
    if (bytes_ahead > 0) {
#ifdef _WIN32
        new_offset = _lseeki64(io_filehandle->fd, -bytes_ahead, SEEK_CUR);
#else
        new_offset = lseek(io_filehandle->fd, -bytes_ahead, SEEK_CUR);
#endif

        if (new_offset < 0) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unable to seek filehandle",
                "Unable to seek %s - %s", io_filehandle->filehandlename, strerror(errno));
        }
    }

#ifdef _WIN32
    new_offset = _lseeki64(io_filehandle->fd, offset, whence);
#else
    new_offset = lseek(io_filehandle->fd, offset, whence);
#endif

    if (new_offset < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to seek filehandle",
            "Unable to seek %s - %s", io_filehandle->filehandlename, strerror(errno));
    }

    /* Empty the buffer, since we may now be at a different position.
     */
    io_filehandle->bytes_in_buffer = 0;
    io_filehandle->read_point = 0;

    return new_offset;
}


DcmFilehandle *dcm_filehandle_create_from_file(DcmError **error,
                                               const char *filehandle_path)
{
    static DcmIO io = {
        dcm_io_open_filehandle,
        dcm_io_close_filehandle,
        dcm_io_read_filehandle,
        dcm_io_seek_filehandle,
    };

    return dcm_filehandle_create(error, &io, (void *) filehandle_path);
}


typedef struct _DcmIOMemory {
    char *buffer;
    int64_t length;
    int64_t read_point;
} DcmIOMemory;


static int dcm_io_close_memory(DcmError **error, void *data)
{
    DcmIOMemory *io_memory = (DcmIOMemory *) data;

    USED(error);
    free(io_memory);

    return 0;
}


static void *dcm_io_open_memory(DcmError **error, void *client)
{
    DcmIOMemory *params = (DcmIOMemory *)client;

    DcmIOMemory *io_memory = DCM_NEW(error, DcmIOMemory);
    if (io_memory == NULL) {
        return NULL;
    }
    io_memory->buffer = params->buffer;
    io_memory->length = params->length;

    return io_memory;
}


static int64_t dcm_io_read_memory(DcmError **error, void *data,
    char *buffer, int64_t length)
{
    DcmIOMemory *io_memory = (DcmIOMemory *) data;

    USED(error);

    int bytes_available = io_memory->length - io_memory->read_point;
    int bytes_to_copy = MIN(bytes_available, length);
    memcpy(buffer,
           io_memory->buffer + io_memory->read_point,
           bytes_to_copy);
    io_memory->read_point += bytes_to_copy;

    return bytes_to_copy;
}


static int64_t dcm_io_seek_memory(DcmError **error, void *data,
    int64_t offset, int whence)
{
    DcmIOMemory *io_memory = (DcmIOMemory *) data;

    int64_t new_offset;

    switch (whence)
    {
        case SEEK_SET:
            new_offset = offset;
            break;

        case SEEK_CUR:
            new_offset = io_memory->read_point + offset;
            break;

        case SEEK_END:
            new_offset = io_memory->length + offset;
            break;

        default:
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unsupported whence",
                "Whence %d not implemented", whence);
            return -1;
    }

    new_offset = MAX(0, MIN(new_offset, io_memory->length));

    return new_offset;
}


DcmFilehandle *dcm_filehandle_create_from_memory(DcmError **error,
                                                 char *buffer, int64_t length)
{
    static DcmIO io = {
        dcm_io_open_memory,
        dcm_io_close_memory,
        dcm_io_read_memory,
        dcm_io_seek_memory,
    };

    DcmIOMemory memory = {
        buffer,
        length,
        0
    };

    return dcm_filehandle_create(error, &io, &memory);
}
