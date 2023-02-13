/*
 * Implementation of subroutines that are independent of the DICOM standard.
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifndef _WIN32
// needed for close/read/lseek on posix
#include <unistd.h>
#endif
#include <time.h>

#include "dicom.h"
#include "pdicom.h"

/* The size of the input buffer we use.
 */
#define BUFFER_SIZE (4096)

typedef struct _DcmIOFile {
    int fd;
    char *filename;
    char input_buffer[BUFFER_SIZE];
    int bytes_in_buffer;
    int read_point;
} DcmIOFile;


static int dcm_io_close_file(DcmError **error, void *data)
{
    DcmIOFile *io_file = (DcmIOFile *) data;

    int close_errno = 0;

    if (io_file->fd != -1) {
        if (close(io_file->fd)) {
            close_errno = errno;
        }

        io_file->fd = -1;

        if (close_errno) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unable to close file",
                "Unable to close %s - %s", 
                io_file->filename, strerror(close_errno));
        }
    }

    free(io_file->filename);
    free(io_file);

    return close_errno;
}


static void *dcm_io_open_file(DcmError **error, void *client)
{
    DcmIOFile *io_file = DCM_NEW(error, DcmIOFile);
    if (io_file == NULL) {
        return NULL;
    }

    // The "not set" value for fd
    io_file->fd = -1;

    const char *filename = (const char *) client;
    io_file->filename = dcm_strdup(error, filename);
    if (io_file->filename == NULL) {
        dcm_io_close_file(error, io_file);
        return NULL;
    }

    int open_errno;

#ifdef _WIN32
    int oflag = _O_BINARY | _O_RDONLY | _O_RANDOM;
    int shflag = _SH_DENYWR;
    int pmode = 0;
    open_errno = _sopen_s(&io_file->fd, io_file->filename, 
                          oflag, sgflag, pmode);
#else
    int flags = O_RDONLY;
#ifdef O_BINARY
    flags |= O_BINARY;
#endif
    mode_t mode = 0;
    do
        io_file->fd = open(io_file->filename, flags, mode);
    while (io_file->fd == -1 && errno == EINTR);

    open_errno = errno;
#endif

    if (io_file->fd == -1) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to open file",
            "Unable to open %s - %s", io_file->filename, strerror(open_errno));
        dcm_io_close_file(error, io_file);
        return NULL;
    }

    return io_file;
}


static int64_t read_file(DcmError **error, DcmIOFile *io_file,
    char *buffer, int64_t length)
{
    int64_t bytes_read;

#ifdef _WIN32
    bytes_read = _read(io_file->fd, buffer, length);
#else
    do {
        bytes_read = read(io_file->fd, buffer, length);
    } while (bytes_read < 0 && errno == EINTR);
#endif

    if (bytes_read < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to read from file",
            "Unable to read %s - %s", io_file->filename, strerror(errno));
    }

    return bytes_read;
}


/* Refill the input buffer. 
 * -1 on error, 0 on EOF, otherwise bytes read.
 */
static int64_t refill(DcmError **error, DcmIOFile *io_file)
{
    // buffer should be empty coming in
    assert(io_file->bytes_in_buffer - io_file->read_point == 0);

    int64_t bytes_read = read_file(error, io_file, 
                                   io_file->input_buffer, BUFFER_SIZE); 
    if (bytes_read < 0) {
        return bytes_read;
    }

    io_file->read_point = 0;
    io_file->bytes_in_buffer = bytes_read;

    return bytes_read;
}


static int64_t dcm_io_read_file(DcmError **error, void *data, 
    char *buffer, int64_t length)
{
    DcmIOFile *io_file = (DcmIOFile *) data;
    int64_t bytes_read = 0;

    while (length > 0) {
        /* Refill the input buffer if it's empty.
         */
        if (io_file->bytes_in_buffer - io_file->read_point == 0) {
            int64_t refill_bytes = refill(error, io_file);
            if (refill_bytes < 0) {
                return refill_bytes;
            } else if (refill_bytes == 0) {
                // we may be read some bytes in a previous loop
                return bytes_read;
            }
        }

        /* Read what we can from the buffer.
         */
        int bytes_available = io_file->bytes_in_buffer - io_file->read_point;
        int bytes_to_copy = MIN(bytes_available, length);

        memcpy(buffer, 
               io_file->input_buffer + io_file->read_point, 
               bytes_to_copy);
        length -= bytes_to_copy;
        buffer += bytes_to_copy;

        io_file->read_point += bytes_to_copy;
        bytes_read += bytes_to_copy;
    }

    return bytes_read;
}


static int64_t dcm_io_seek_file(DcmError **error, void *data, 
    int64_t offset, int whence)
{
    DcmIOFile *io_file = (DcmIOFile *) data;

    /* We've read ahead by some number of buffered bytes, so first undo that,
     * then do the seek from the true position.
     */
    int64_t new_offset;

    int64_t bytes_ahead = io_file->bytes_in_buffer - io_file->read_point;
    if (bytes_ahead > 0) {
#ifdef _WIN32
        new_offset = _lseeki64(io_file->fd, -bytes_ahead, SEEK_CUR);
#else
        new_offset = lseek(io_file->fd, -bytes_ahead, SEEK_CUR);
#endif

        if (new_offset < 0) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "Unable to seek file",
                "Unable to seek %s - %s", io_file->filename, strerror(errno));
        }
    }

#ifdef _WIN32
    new_offset = _lseeki64(io_file->fd, offset, whence);
#else
    new_offset = lseek(io_file->fd, offset, whence);
#endif

    if (new_offset < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "Unable to seek file",
            "Unable to seek %s - %s", io_file->filename, strerror(errno));
    }

    /* Empty the buffer, since we may now be at a different position.
     */
    io_file->bytes_in_buffer = 0;
    io_file->read_point = 0;

    return new_offset;
}


DcmFile *dcm_file_open(DcmError **error, const char *file_path)
{
    static DcmIO io = {
        dcm_io_open_file,
        dcm_io_close_file,
        dcm_io_read_file,
        dcm_io_seek_file,
    };

    DcmFile *file = dcm_file_create_io(error, &io, (void *) file_path);
    if (file == NULL) {
        return NULL;
    }

    return file;
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


DcmFile *dcm_file_memory(DcmError **error, char *buffer, int64_t length)
{
    static DcmIO io = {
        dcm_io_open_memory,
        dcm_io_close_memory,
        dcm_io_read_memory,
        dcm_io_seek_memory,
    };

    DcmIOMemory *memory = DCM_NEW(error, DcmIOMemory);
    if (memory == NULL) {
        return NULL;
    }
    memory->buffer = buffer;
    memory->length = length;

    DcmFile *file = dcm_file_create_io(error, &io, memory);
    free(memory);
    if (file == NULL) {
        return NULL;
    }

    return file;
}
