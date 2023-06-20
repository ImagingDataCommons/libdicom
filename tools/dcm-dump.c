#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dicom/dicom.h>


static const char usage[] = "usage: dcm-dump [-v] [-V] [-h] FILE_PATH ...";


int main(int argc, char *argv[])
{
    int i;
    int c;

    dcm_init();

    while ((c = dcm_getopt(argc, argv, "h?Vv")) != -1) {
        switch (c) {
            case 'h':
            case '?':
                printf("%s\n", usage);
                return EXIT_SUCCESS;

            case 'v':
                printf("%s\n", dcm_get_version());
                return EXIT_SUCCESS;

            case 'V':
                dcm_log_set_level(DCM_LOG_INFO);
                break;

            case '#':
            default:
                return EXIT_FAILURE;
        }
    }

    for (i = dcm_optind; i < argc; i++) {
        DcmError *error = NULL;
        DcmDataSet *metadata = NULL;
        DcmDataSet *meta = NULL;
        DcmFilehandle *handle = NULL;

        dcm_log_info("Read file '%s'", argv[i]);
        handle = dcm_filehandle_create_from_file(&error, argv[i]);
        if (handle == NULL) {
            dcm_error_print(error);
            dcm_error_clear(&error);
            return EXIT_FAILURE;
        }
        dcm_log_info("Read File Meta Information");
        meta = dcm_filehandle_read_file_meta(&error, handle);
        if (meta == NULL) {
            dcm_error_print(error);
            dcm_error_clear(&error);
            dcm_filehandle_destroy(handle);
            return EXIT_FAILURE;
        }

        printf("===File Meta Information===\n");
        dcm_dataset_print(meta, 0);

        dcm_log_info("Read metadata");
        metadata = dcm_filehandle_read_metadata(&error, handle);
        if (metadata == NULL) {
            dcm_error_print(error);
            dcm_error_clear(&error);
            dcm_dataset_destroy(meta);
            dcm_filehandle_destroy(handle);
            return EXIT_FAILURE;
        }

        printf("===Dataset===\n");
        dcm_dataset_print(metadata, 0);

        dcm_dataset_destroy(meta);
        dcm_dataset_destroy(metadata);
        dcm_filehandle_destroy(handle);
    }

    return EXIT_SUCCESS;
}
