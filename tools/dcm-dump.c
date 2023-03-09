#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dicom/dicom.h>


static const char usage[] = "usage: dcm-dump [-v] [-V] [-h] FILE_PATH\n";


int main(int argc, char *argv[]) 
{
    int i;
    const char *file_path = NULL;
    DcmError *error = NULL;
    DcmDataSet *metadata = NULL;
    DcmDataSet *file_metadata = NULL;
    DcmFilehandle *filehandle = NULL;

    dcm_log_level = DCM_LOG_ERROR;

    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        switch (argv[i][1]) {
            case 'h':
                printf("%s\n", usage);
                return EXIT_SUCCESS;
            case 'V':
                printf("%s\n", dcm_get_version());
                return EXIT_SUCCESS;
            case 'v':
                dcm_log_level = DCM_LOG_INFO;
                break;
            default:
                fprintf(stderr, "%s\n", usage);
                return EXIT_FAILURE;
        }
    }

    if ((i + 1) != argc) {
        fprintf(stderr, "%s\n", usage);
        return EXIT_FAILURE;
    }
    file_path = argv[i];

    dcm_log_info("Read file '%s'", file_path);
    filehandle = dcm_filehandle_create_from_file(&error, file_path);
    if (filehandle == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        return EXIT_FAILURE;
    }
    dcm_log_info("Read File Meta Information");
    file_metadata = dcm_filehandle_read_file_metadata(&error, filehandle);
    if (file_metadata == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_filehandle_destroy(filehandle);
        return EXIT_FAILURE;
    }

    printf("===File Meta Information===\n");
    dcm_dataset_print(file_metadata, 0);

    dcm_log_info("Read metadata");
    metadata = dcm_filehandle_read_metadata(&error, filehandle);
    if (metadata == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_filehandle_destroy(filehandle);
        dcm_dataset_destroy(file_metadata);
        return EXIT_FAILURE;
    }

    printf("===Dataset===\n");
    dcm_dataset_print(metadata, 0);

    dcm_filehandle_destroy(filehandle);
    dcm_dataset_destroy(file_metadata);
    dcm_dataset_destroy(metadata);

    return EXIT_SUCCESS;
}
