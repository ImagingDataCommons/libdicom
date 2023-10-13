#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dicom/dicom.h>


static const char usage[] = "usage: dcm-dump [-v] [-V] [-h] FILE_PATH ...";


int main(int argc, char *argv[])
{
    int i, c;

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
        DcmFilehandle *filehandle = NULL;

        dcm_log_info("Read file '%s'", argv[i]);
        filehandle = dcm_filehandle_create_from_file(&error, argv[i]);
        if (filehandle == NULL) {
            dcm_error_print(error);
            dcm_error_clear(&error);
            return EXIT_FAILURE;
        }  
        
        if (!dcm_filehandle_print(&error, filehandle)) {
            dcm_error_print(error);
            dcm_error_clear(&error);
            dcm_filehandle_destroy(filehandle);
            return EXIT_FAILURE;
        }

        dcm_filehandle_destroy(filehandle);
    }

    return EXIT_SUCCESS;
}
