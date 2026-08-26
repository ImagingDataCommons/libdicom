/*
 * libdicom fuzz target: parse an arbitrary byte buffer as a DICOM file.
 *
 * Exercises the read paths an application hits when it opens an untrusted
 * DICOM file: File Meta Information, transfer syntax, the metadata data set,
 * and the first PixelData frame.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>

#include <dicom/dicom.h>

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void) argc;
    (void) argv;

    /* Keep the fuzzer quiet; log formatting is not the target here. */
    dcm_log_set_level(DCM_LOG_NOTSET);

    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    DcmError *error = NULL;
    DcmFilehandle *filehandle;
    DcmDataSet *metadata;

    if (size < 1 || size > (1 << 22)) {
        return 0;
    }

    filehandle = dcm_filehandle_create_from_memory(&error,
                                                   (const char *) data,
                                                   (int64_t) size);
    if (filehandle == NULL) {
        dcm_error_clear(&error);
        return 0;
    }

    /* File Meta Information -- the first thing any reader touches.
     *
     * Every call clears the error unconditionally: libdicom can return
     * success with an error still set (see the get_num_frames fix), and a
     * fuzz target must not turn that into a leak report of its own.
     */
    (void) dcm_filehandle_get_file_meta(&error, filehandle);
    dcm_error_clear(&error);

    (void) dcm_filehandle_get_transfer_syntax_uid(filehandle);

    /* Fast metadata subset, then the full data set. */
    (void) dcm_filehandle_get_metadata_subset(&error, filehandle);
    dcm_error_clear(&error);

    metadata = dcm_filehandle_read_metadata(&error, filehandle, NULL);
    dcm_error_clear(&error);
    if (metadata != NULL) {
        dcm_dataset_destroy(metadata);
    }

    /* Frame offset table + first frame: the encapsulated-pixel-data path. */
    if (dcm_filehandle_prepare_read_frame(&error, filehandle)) {
        DcmFrame *frame = dcm_filehandle_read_frame(&error, filehandle, 1);

        if (frame != NULL) {
            dcm_frame_destroy(frame);
        }
    }
    dcm_error_clear(&error);

    dcm_filehandle_destroy(filehandle);

    return 0;
}
