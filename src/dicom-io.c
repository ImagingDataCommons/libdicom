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
    DcmIOMethods *methods;

    // private fields
    int fd;
    char *filename;
    char input_buffer[BUFFER_SIZE];
    int64_t bytes_in_buffer;
    int64_t read_point;
} DcmIOFile;


static void dcm_io_close_file(DcmIO *io)
{
    DcmIOFile *file = (DcmIOFile *) io;

    if (file->fd != -1) {
        (void) close(file->fd);
    }

    free(file->filename);
    free(file);
}


static DcmIO *dcm_io_open_file(DcmError **error, void *client)
{
    DcmIOFile *file = DCM_NEW(error, DcmIOFile);
    if (file == NULL) {
        return NULL;
    }

    // The "not set" value for fd
    file->fd = -1;

    const char *filename = (const char *) client;
    file->filename = dcm_strdup(error, filename);
    if (file->filename == NULL) {
        dcm_io_close_file((DcmIO *)file);
        return NULL;
    }

    int open_errno;

#ifdef _WIN32
    int oflag = _O_BINARY | _O_RDONLY | _O_RANDOM;
    // some mingw are missing this ... just use the numeric value
    // #define _SH_DENYWR 0x20
    int shflag = 0x20;
    int pmode = 0;
    open_errno = _sopen_s(&file->fd, file->filename,
                          oflag, shflag, pmode);
#else
    int flags = O_RDONLY;
#ifdef O_BINARY
    flags |= O_BINARY;
#endif
    mode_t mode = 0;
    do
        file->fd = open(file->filename, flags, mode);
    while (file->fd == -1 && errno == EINTR);

    open_errno = errno;
#endif

    if (file->fd == -1) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "unable to open filehandle",
            "unable to open %s - %s", file->filename, strerror(open_errno));
        dcm_io_close_file((DcmIO *)file);
        return NULL;
    }

    return (DcmIO *)file;
}


static int64_t read_file(DcmError **error, DcmIOFile *file,
    char *buffer, int64_t length)
{
    int64_t bytes_read;

#ifdef _WIN32
    // we'll never read a huge chunk (hopefully)
    bytes_read = _read(file->fd, buffer, (uint32_t) length);
#else
    do {
        bytes_read = read(file->fd, buffer, length);
    } while (bytes_read < 0 && errno == EINTR);
#endif

    if (bytes_read < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "unable to read from file",
            "unable to read %s - %s", file->filename, strerror(errno));
    }

    return bytes_read;
}


/* Refill the input buffer.
 * -1 on error, 0 on EOF, otherwise bytes read.
 */
static int64_t refill(DcmError **error, DcmIOFile *file)
{
    // buffer should be empty coming in
    assert(file->bytes_in_buffer - file->read_point == 0);

    int64_t bytes_read = read_file(error, file,
                                   file->input_buffer, BUFFER_SIZE);
    if (bytes_read < 0) {
        return bytes_read;
    }

    file->read_point = 0;
    file->bytes_in_buffer = bytes_read;

    return bytes_read;
}


static int64_t dcm_io_read_file(DcmError **error, DcmIO *io,
    char *buffer, int64_t length)
{
    DcmIOFile *file = (DcmIOFile *) io;
    int64_t bytes_read = 0;

    while (length > 0) {
        /* Refill the input buffer if it's empty.
         */
        if (file->bytes_in_buffer - file->read_point == 0) {
            int64_t refill_bytes = refill(error, file);
            if (refill_bytes < 0) {
                return refill_bytes;
            } else if (refill_bytes == 0) {
                // we maybe read some bytes in a previous loop
                return bytes_read;
            }
        }

        /* Read what we can from the buffer.
         */
        int64_t bytes_available = file->bytes_in_buffer - file->read_point;
        int64_t bytes_to_copy = MIN(bytes_available, length);

        memcpy(buffer,
               file->input_buffer + file->read_point,
               bytes_to_copy);
        length -= bytes_to_copy;
        buffer += bytes_to_copy;

        file->read_point += bytes_to_copy;
        bytes_read += bytes_to_copy;
    }

    return bytes_read;
}


static int64_t dcm_io_seek_file(DcmError **error, DcmIO *io,
    int64_t offset, int whence)
{
    DcmIOFile *file = (DcmIOFile *) io;

    /* We've read ahead by some number of buffered bytes, so first undo that,
     * then do the seek from the true position.
     */
    int64_t new_offset;

    int64_t bytes_ahead = file->bytes_in_buffer - file->read_point;
    if (bytes_ahead > 0) {
#ifdef _WIN32
        new_offset = _lseeki64(file->fd, -bytes_ahead, SEEK_CUR);
#else
        new_offset = lseek(file->fd, -bytes_ahead, SEEK_CUR);
#endif

        if (new_offset < 0) {
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "unable to seek file",
                "unable to seek %s - %s", file->filename, strerror(errno));
        }
    }

#ifdef _WIN32
    new_offset = _lseeki64(file->fd, offset, whence);
#else
    new_offset = lseek(file->fd, offset, whence);
#endif

    if (new_offset < 0) {
        dcm_error_set(error, DCM_ERROR_CODE_IO,
            "unable to seek file",
            "unable to seek %s - %s", file->filename, strerror(errno));
    }

    /* Empty the buffer, since we may now be at a different position.
     */
    file->bytes_in_buffer = 0;
    file->read_point = 0;

    return new_offset;
}


DcmIO *dcm_io_create(DcmError **error,
                     const DcmIOMethods *methods,
                     void *client)
{
    DcmIO *io = methods->open(error, client);
    if (io == NULL) {
        return NULL;
    }
    io->methods = methods;

    return io;
}


DcmIO *dcm_io_create_from_file(DcmError **error, const char *filename)
{
    static DcmIOMethods methods = {
        dcm_io_open_file,
        dcm_io_close_file,
        dcm_io_read_file,
        dcm_io_seek_file,
    };

    return dcm_io_create(error, &methods, (void *) filename);
}


typedef struct _DcmIOMemory {
    DcmIOMethods *methods;

    // private fields
    const char *buffer;
    int64_t length;
    int64_t read_point;
} DcmIOMemory;


static void dcm_io_close_memory(DcmIO *io)
{
    DcmIOMemory *memory = (DcmIOMemory *) io;

    free(memory);
}


static DcmIO *dcm_io_open_memory(DcmError **error, void *client)
{
    DcmIOMemory *params = (DcmIOMemory *)client;

    DcmIOMemory *memory = DCM_NEW(error, DcmIOMemory);
    if (memory == NULL) {
        return NULL;
    }
    memory->buffer = params->buffer;
    memory->length = params->length;

    return (DcmIO *) memory;
}


static int64_t dcm_io_read_memory(DcmError **error, DcmIO *io,
    char *buffer, int64_t length)
{
    DcmIOMemory *memory = (DcmIOMemory *) io;

    USED(error);

    int64_t bytes_available = memory->length - memory->read_point;
    int64_t bytes_to_copy = MIN(bytes_available, length);
    memcpy(buffer,
           memory->buffer + memory->read_point,
           bytes_to_copy);
    memory->read_point += bytes_to_copy;

    return bytes_to_copy;
}


static int64_t dcm_io_seek_memory(DcmError **error, DcmIO *io,
    int64_t offset, int whence)
{
    DcmIOMemory *memory = (DcmIOMemory *) io;

    int64_t new_offset;

    switch (whence)
    {
        case SEEK_SET:
            new_offset = offset;
            break;

        case SEEK_CUR:
            new_offset = memory->read_point + offset;
            break;

        case SEEK_END:
            new_offset = memory->length + offset;
            break;

        default:
            dcm_error_set(error, DCM_ERROR_CODE_IO,
                "unsupported whence",
                "whence %d not implemented", whence);
            return -1;
    }

    memory->read_point = MAX(0, MIN(new_offset, memory->length));

    return memory->read_point;
}


DcmIO *dcm_io_create_from_memory(DcmError **error,
                                 const char *buffer,
                                 int64_t length)
{
    static DcmIOMethods methods = {
        dcm_io_open_memory,
        dcm_io_close_memory,
        dcm_io_read_memory,
        dcm_io_seek_memory,
    };

    DcmIOMemory memory = {
        &methods,
        buffer,
        length,
        0
    };

    return dcm_io_create(error, &methods, &memory);
}


void dcm_io_close(DcmIO *io)
{
    io->methods->close(io);
}


int64_t dcm_io_read(DcmError **error,
                    DcmIO *io,
                    char *buffer,
                    int64_t length)
{
    return io->methods->read(error, io, buffer, length);
}


int64_t dcm_io_seek(DcmError **error,
                    DcmIO *io,
                    int64_t offset,
                    int whence)
{
    return io->methods->seek(error, io, offset, whence);
}

