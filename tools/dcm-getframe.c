#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dicom.h>


static const char usage[] = "usage: dcm-getframe [-v] [-V] [-h] [-o OUTPUT-FILE] FILE_PATH FRAME_NUMBER\n";


int main(int argc, char *argv[]) 
{
    DcmError *error = NULL;
    char *output_file = NULL;
    int i;

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
            case 'o':
                output_file = argv[i + 1];
                i += 1;
                break;
            default:
                fprintf(stderr, "%s\n", usage);
                return EXIT_FAILURE;
        }
    }

    if ((i + 2) != argc) {
        fprintf(stderr, "%s\n", usage);
        return EXIT_FAILURE;
    }
    const char *file_path = argv[i];

    uint32_t frame_number = atoi(argv[i + 1]);

    dcm_log_info("Read file '%s'", file_path);
    DcmFile *file = dcm_file_open(&error, file_path);
    if (file == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        return EXIT_FAILURE;
    }

    dcm_log_info("Read metadata");
    DcmDataSet *metadata = dcm_file_read_metadata(&error, file);
    if (metadata == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_file_destroy(file);
        return EXIT_FAILURE;
    }

    dcm_log_info("Read BOT");
    DcmBOT *bot = dcm_file_read_bot(&error, file, metadata);
    if (bot == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_dataset_destroy(metadata);
        dcm_file_destroy(file);
        return EXIT_FAILURE;
    }

    uint32_t num_frames = dcm_bot_get_num_frames(bot);
    if (frame_number < 1 || frame_number > num_frames) {
        dcm_error_set(&error, DCM_ERROR_CODE_INVALID,
                      "Bad frame number",
                      "Frame number must be between 1 and %d",
                      num_frames);
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_bot_destroy(bot);
        dcm_dataset_destroy(metadata);
        dcm_file_destroy(file);
        return EXIT_FAILURE;
    }

    dcm_log_info("Read frame %u", frame_number);
    DcmFrame *frame = dcm_file_read_frame(&error, 
                                          file, metadata, bot, frame_number);
    if (frame == NULL) {
        dcm_error_log(error);
        dcm_error_clear(&error);
        dcm_bot_destroy(bot);
        dcm_dataset_destroy(metadata);
        dcm_file_destroy(file);
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
                          "Bad output file name",
                          "Unable to open %s for output", output_file);
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
    dcm_bot_destroy(bot);
    dcm_dataset_destroy(metadata);
    dcm_file_destroy(file);

    return EXIT_SUCCESS;
}