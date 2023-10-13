#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dicom/dicom.h>


static const char usage[] = "usage: "
    "dcm-getframe [-v] [-V] [-h] [-o OUTPUT-FILE] FILE_PATH FRAME_NUMBER";


int main(int argc, char *argv[])
{
    char *output_file = NULL;

    int c;

    while ((c = dcm_getopt(argc, argv, "h?Vvo:")) != -1) {
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

            case 'o':
                output_file = dcm_optarg;
                break;

            case '#':
            default:
                return EXIT_FAILURE;
        }
    }

    DcmError *error = NULL;

    if (dcm_optind + 2 != argc) {
        fprintf(stderr, "%s\n", usage);
        return EXIT_FAILURE;
    }
    const char *input_file = argv[dcm_optind];
    int frame_number = atoi(argv[dcm_optind + 1]);

    dcm_log_info("Read filehandle '%s'", input_file);
    DcmFilehandle *filehandle = dcm_filehandle_create_from_file(&error,
                                                                input_file);
    if (filehandle == NULL) {
        dcm_error_print(error);
        dcm_error_clear(&error);
        return EXIT_FAILURE;
    }

    dcm_log_info("Read frame %u", frame_number);
    DcmFrame *frame = dcm_filehandle_read_frame(&error,
                                                filehandle,
                                                frame_number);
    if (frame == NULL) {
        dcm_error_print(error);
        dcm_error_clear(&error);
        dcm_filehandle_destroy(filehandle);
        return EXIT_FAILURE;
    }

    const char *frame_value = dcm_frame_get_value(frame);
    uint32_t frame_length = dcm_frame_get_length(frame);

    dcm_log_info("frame number = %u", frame_number);
    dcm_log_info("length = %u bytes", frame_length);
    dcm_log_info("rows = %u", dcm_frame_get_rows(frame));
    dcm_log_info("columns = %u", dcm_frame_get_columns(frame));
    dcm_log_info("samples per pixel = %u",
                 dcm_frame_get_samples_per_pixel(frame));
    dcm_log_info("bits allocated = %u", dcm_frame_get_bits_allocated(frame));
    dcm_log_info("bits stored = %u", dcm_frame_get_bits_stored(frame));
    dcm_log_info("high bit = %u", dcm_frame_get_high_bit(frame));
    dcm_log_info("pixel representation = %u",
                 dcm_frame_get_pixel_representation(frame));
    dcm_log_info("planar configuration = %u",
                 dcm_frame_get_planar_configuration(frame));
    dcm_log_info("photometric interpretation = %s",
                 dcm_frame_get_photometric_interpretation(frame));
    dcm_log_info("transfer syntax uid = %s",
                 dcm_frame_get_transfer_syntax_uid(frame));

    FILE *output_fp;
    if (output_file != NULL) {
        output_fp = fopen(output_file, "wb");
        if (output_fp == NULL) {
            dcm_error_set(&error, DCM_ERROR_CODE_INVALID,
                          "Bad output filehandle name",
                          "Unable to open %s for output", output_file);
            dcm_error_print(error);
            dcm_error_clear(&error);
            dcm_frame_destroy(frame);
            dcm_filehandle_destroy(filehandle);
            return EXIT_FAILURE;
        }
    }
    else
        output_fp = stdout;

    fwrite(frame_value, 1, frame_length, output_fp);

    if (output_file != NULL) {
        fclose(output_fp);
        output_fp = NULL;
    }

    dcm_frame_destroy(frame);
    dcm_filehandle_destroy(filehandle);

    return EXIT_SUCCESS;
}
