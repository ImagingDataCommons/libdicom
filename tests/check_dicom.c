#include "config.h"

#ifdef HAVE_CHECK

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include "dicom.h"


static char *fixture_path(const char *relpath)
{
    char *path = malloc(strlen(SRCDIR) + strlen(relpath) + 2);
    sprintf(path, "%s/%s", SRCDIR, relpath);
    return path;
}

static size_t compute_length_of_string_value(char *value)
{
    size_t length = strlen(value);
    if (length % 2 != 0) {
        length += 1;  // zero padding
    }
    return length;
}


static size_t compute_length_of_string_value_multi(char **values, uint32_t vm)
{
    uint32_t i;
    size_t length;

    length = 3 * 2;  // separators between values
    for (i = 0; i < vm; i++) {
        length += strlen(values[i]);
    }
    if (length % 2 != 0) {
        length += 1;  // zero padding
    }
    return length;
}


START_TEST(test_log_level)
{
    ck_assert_int_eq(dcm_log_level, DCM_LOG_NOTSET);

    dcm_log_level = DCM_LOG_INFO;
    ck_assert_int_eq(dcm_log_level, DCM_LOG_INFO);
}
END_TEST


START_TEST(test_tag_validity_checks)
{
    ck_assert_int_eq(dcm_is_valid_tag(0x00280008), true);
    ck_assert_int_eq(dcm_is_public_tag(0x00280008), true);

    ck_assert_int_eq(dcm_is_valid_tag(0x00570008), true);
    ck_assert_int_eq(dcm_is_private_tag(0x00570008), true);
    ck_assert_int_eq(dcm_is_public_tag(0x00570008), false);

    ck_assert_int_eq(dcm_is_valid_tag(0x0028000), false);
}
END_TEST


START_TEST(test_vr_validity_checks)
{
    ck_assert_int_eq(dcm_is_valid_vr("SQ"), true);
    ck_assert_int_eq(dcm_is_valid_vr("US"), true);
    ck_assert_int_eq(dcm_is_valid_vr("AE"), true);

    ck_assert_int_eq(dcm_is_valid_vr("A"), false);
    ck_assert_int_eq(dcm_is_valid_vr("ABC"), false);
    ck_assert_int_eq(dcm_is_valid_vr("XY"), false);
}
END_TEST


START_TEST(test_dict_tag_lookups)
{
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00020010), "TransferSyntaxUID");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00080018), "SOPInstanceUID");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00080030), "StudyTime");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00180050), "SliceThickness");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00185100), "PatientPosition");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00200035), "ImageOrientation");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00280008), "NumberOfFrames");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00280010), "Rows");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00400554), "SpecimenUID");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00480003), "ImagedVolumeDepth");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00620021), "TrackingUID");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00660031), "AlgorithmVersion");
    ck_assert_str_eq(dcm_dict_lookup_keyword(0x00701305), "Plane");
}
END_TEST


START_TEST(test_dict_vr_lookups)
{
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00020010), "UI");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00080018), "UI");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00080030), "TM");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00180050), "DS");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00185100), "CS");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00200035), "DS");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00280008), "IS");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00280010), "US");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00400554), "UI");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00480003), "FL");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00620021), "UI");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00660031), "LO");
    ck_assert_str_eq(dcm_dict_lookup_vr(0x00701305), "FD");
}
END_TEST


START_TEST(test_element_AE)
{
    uint32_t tag = 0x00020016;
    char *value = malloc(DCM_CAPACITY_AE + 1);
    strncpy(value, "Application", 12);
    DcmElement *element = dcm_element_create_AE(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "AE"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(value, dcm_element_get_value_AE(element, 0));

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_AS)
{
    uint32_t tag = 0x00020016;
    char *value = malloc(DCM_CAPACITY_AS + 1);
    strncpy(value, "99", 3);
    DcmElement *element = dcm_element_create_AS(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "AS"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(value, dcm_element_get_value_AS(element, 0));

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_CS_multivalue)
{
    uint32_t tag = 0x00080008;
    uint32_t vm = 4;
    char **values = malloc(vm * sizeof(char *));
    values[0] = malloc(9);
    strcpy(values[0], "ORIGINAL");
    values[1] = malloc(8);
    strcpy(values[1], "PRIMARY");
    values[2] = malloc(7);
    strcpy(values[2], "VOLUME");
    values[3] = malloc(5);
    strcpy(values[3], "NONE");
    DcmElement *element = dcm_element_create_CS_multi(tag, values, vm);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "CS"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value_multi(values, vm));
    ck_assert_int_eq(dcm_element_is_multivalued(element), true);

    for (uint32_t i = 0; i < vm; i++) {
        ck_assert_str_eq(values[i], dcm_element_get_value_CS(element, i));
    }

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_DS)
{
    uint32_t tag = 0x0040072A;
    char *value = malloc(DCM_CAPACITY_DS + 1);
    strncpy(value, "0.0025", 7);
    DcmElement *element = dcm_element_create_DS(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "DS"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(dcm_element_get_value_DS(element, 0), value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_IS)
{
    uint32_t tag = 0x00280008;
    char *value = malloc(DCM_CAPACITY_IS + 1);
    strncpy(value, "10", 3);
    DcmElement *element = dcm_element_create_IS(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "IS"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(dcm_element_get_value_IS(element, 0), value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_ST)
{
    uint32_t tag = 0x00080092;
    char *value = malloc(DCM_CAPACITY_ST + 1);
    strncpy(value, "Random Street, Sometown", 24);
    DcmElement *element = dcm_element_create_ST(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "ST"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(dcm_element_get_value_ST(element), value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_SQ)
{
    char *item_value = malloc(DCM_CAPACITY_DS + 1);
    strncpy(item_value, "0.01", 5);
    DcmElement *item_element = dcm_element_create_DS(0x00180050, item_value);

    DcmDataSet *item = dcm_dataset_create();
    ck_assert_int_eq(dcm_dataset_insert(item, item_element), true);

    uint32_t tag = 0x00289110;
    DcmSequence *value = dcm_sequence_create();
    ck_assert_int_eq(dcm_sequence_append(value, item), true);
    DcmElement *element = dcm_element_create_SQ(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "SQ"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     dcm_element_get_length(item_element));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    DcmSequence *retrieved_value = dcm_element_get_value_SQ(element);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 1);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_SQ_empty)
{
    uint32_t tag = 0x00400555;
    DcmSequence *value = dcm_sequence_create();
    DcmElement *element = dcm_element_create_SQ(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "SQ"), true);
    ck_assert_int_eq(dcm_element_get_length(element), 0);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    DcmSequence *retrieved_value = dcm_element_get_value_SQ(element);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 0);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_UI)
{
    uint32_t tag = 0x00080018;
    char *value = malloc(DCM_CAPACITY_UI + 1);
    strncpy(value, "2.25.1", 7);
    DcmElement *element = dcm_element_create_UI(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "UI"), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_str_eq(dcm_element_get_value_UI(element, 0), value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_US)
{
    uint32_t tag = 0x00280010;
    uint16_t value = 512;
    DcmElement *element = dcm_element_create_US(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, "US"), true);
    ck_assert_int_eq(dcm_element_get_length(element), sizeof(uint16_t));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);
    ck_assert_int_eq(dcm_element_get_value_US(element, 0), value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_sequence)
{
    DcmElement *element;

    DcmDataSet *dataset = dcm_dataset_create();
    DcmDataSet *other_dataset = dcm_dataset_create();

    element = dcm_element_create_US(0x00280010, 256);
    dcm_dataset_insert(dataset, element);
    dcm_dataset_insert(other_dataset, dcm_element_clone(element));
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_dataset_count(other_dataset), 1);

    element = dcm_element_create_US(0x00280011, 512);
    dcm_dataset_insert(dataset, element);
    dcm_dataset_insert(other_dataset, dcm_element_clone(element));
    ck_assert_int_eq(dcm_dataset_count(dataset), 2);
    ck_assert_int_eq(dcm_dataset_count(other_dataset), 2);

    DcmSequence *seq = dcm_sequence_create();

    dcm_sequence_append(seq, dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 1);
    dcm_sequence_append(seq, other_dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 2);

    DcmDataSet *same_dataset = dcm_sequence_get(seq, 1);
    ck_assert_int_eq(dcm_sequence_count(seq), 2);
    ck_assert_int_eq(dcm_dataset_count(dataset),
                     dcm_dataset_count(same_dataset));

    dcm_sequence_remove(seq, 1);
    ck_assert_int_eq(dcm_sequence_count(seq), 1);
    dcm_sequence_remove(seq, 0);
    ck_assert_int_eq(dcm_sequence_count(seq), 0);

    dcm_sequence_destroy(seq);
}
END_TEST


START_TEST(test_dataset)
{
    uint32_t tag = 0x00280010;
    DcmElement *element = dcm_element_create_US(tag, 256);

    uint32_t other_tag = 0x00280011;
    DcmElement *other_element = dcm_element_create_US(other_tag, 512);

    DcmDataSet *dataset = dcm_dataset_create();
    ck_assert_int_eq(dcm_dataset_count(dataset), 0);

    dcm_dataset_insert(dataset, element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_dataset_contains(dataset, tag), true);

    DcmElement *same_element = dcm_dataset_get(dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_element_get_tag(element),
                     dcm_element_get_tag(same_element));

    DcmElement *copied_element = dcm_dataset_get_clone(dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_element_get_tag(element),
                     dcm_element_get_tag(copied_element));

    dcm_dataset_insert(dataset, other_element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 2);
    ck_assert_int_eq(dcm_dataset_contains(dataset, tag), true);
    ck_assert_int_eq(dcm_dataset_contains(dataset, other_tag), true);

    dcm_dataset_remove(dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);

    dcm_dataset_remove(dataset, other_tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 0);

    dcm_dataset_print(dataset, 0);

    dcm_element_destroy(copied_element);
    dcm_dataset_destroy(dataset);
}
END_TEST


START_TEST(test_file_sm_image_file_meta)
{
    uint32_t tag;
    DcmElement *element;

    char *file_path = fixture_path("data/test_files/sm_image.dcm");
    DcmFile *file = dcm_file_create(file_path, 'r');
    free(file_path);

    DcmDataSet *file_meta = dcm_file_read_file_meta(file);

    // Transfer Syntax UID
    tag = 0x00020010;
    element = dcm_dataset_get(file_meta, tag);
    ck_assert_str_eq(dcm_element_get_value_UI(element, 0),
                     "1.2.840.10008.1.2.1");

    // Media Storage SOP Class UID
    tag = 0x00020002;
    element = dcm_dataset_get(file_meta, tag);
    ck_assert_str_eq(dcm_element_get_value_UI(element, 0),
                     "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_print(file_meta, 0);

    dcm_dataset_destroy(file_meta);
    dcm_file_destroy(file);
}
END_TEST


START_TEST(test_file_sm_image_metadata)
{
    char *file_path = fixture_path("data/test_files/sm_image.dcm");
    DcmFile *file = dcm_file_create(file_path, 'r');
    free(file_path);

    DcmDataSet *metadata = dcm_file_read_metadata(file);

    // SOP Class UID
    DcmElement *element = dcm_dataset_get(metadata, 0x00080016);
    ck_assert_str_eq(dcm_element_get_value_UI(element, 0),
                     "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_print(metadata, 0);

    dcm_dataset_destroy(metadata);
    dcm_file_destroy(file);
}
END_TEST


START_TEST(test_file_sm_image_frame)
{
    const uint32_t frame_number = 1;

    char *file_path = fixture_path("data/test_files/sm_image.dcm");
    DcmFile *file = dcm_file_create(file_path, 'r');
    free(file_path);

    DcmDataSet *metadata = dcm_file_read_metadata(file);

    DcmBOT *bot = dcm_file_build_bot(file, metadata);
    ck_assert_uint_eq(dcm_bot_get_num_frames(bot), 25);

    DcmFrame *frame = dcm_file_read_frame(file, metadata, bot, frame_number);
    ck_assert_uint_eq(dcm_frame_get_number(frame), frame_number);
    ck_assert_uint_eq(dcm_frame_get_rows(frame), 10);
    ck_assert_uint_eq(dcm_frame_get_columns(frame), 10);
    ck_assert_uint_eq(dcm_frame_get_samples_per_pixel(frame), 3);
    ck_assert_uint_eq(dcm_frame_get_bits_allocated(frame), 8);
    ck_assert_uint_eq(dcm_frame_get_bits_stored(frame), 8);
    ck_assert_uint_eq(dcm_frame_get_high_bit(frame), 7);
    ck_assert_uint_eq(dcm_frame_get_pixel_representation(frame), 0);
    ck_assert_uint_eq(dcm_frame_get_planar_configuration(frame), 0);
    ck_assert_str_eq(dcm_frame_get_photometric_interpretation(frame), "RGB");
    ck_assert_str_eq(dcm_frame_get_transfer_syntax_uid(frame),
                     "1.2.840.10008.1.2.1");

    dcm_bot_destroy(bot);
    dcm_dataset_destroy(metadata);
    dcm_file_destroy(file);
}
END_TEST


static Suite *create_main_suite(void)
{
    Suite *suite = suite_create("main");

    TCase *log_case = tcase_create("log");
    tcase_add_test(log_case, test_log_level);
    suite_add_tcase(suite, log_case);

    TCase *dict_case = tcase_create("dict");
    tcase_add_test(dict_case, test_tag_validity_checks);
    tcase_add_test(dict_case, test_vr_validity_checks);
    tcase_add_test(dict_case, test_dict_tag_lookups);
    tcase_add_test(dict_case, test_dict_vr_lookups);
    suite_add_tcase(suite, dict_case);

    return suite;
}


static Suite *create_data_suite(void)
{
    Suite *suite = suite_create("data");

    TCase *element_case = tcase_create("element");
    tcase_add_test(element_case, test_element_AE);
    tcase_add_test(element_case, test_element_AS);
    tcase_add_test(element_case, test_element_CS_multivalue);
    tcase_add_test(element_case, test_element_DS);
    tcase_add_test(element_case, test_element_IS);
    tcase_add_test(element_case, test_element_ST);
    tcase_add_test(element_case, test_element_SQ);
    tcase_add_test(element_case, test_element_SQ_empty);
    tcase_add_test(element_case, test_element_UI);
    tcase_add_test(element_case, test_element_US);
    suite_add_tcase(suite, element_case);

    TCase *dataset_case = tcase_create("dataset");
    tcase_add_test(dataset_case, test_dataset);
    suite_add_tcase(suite, dataset_case);

    TCase *sequence_case = tcase_create("sequence");
    tcase_add_test(sequence_case, test_sequence);
    suite_add_tcase(suite, sequence_case);

    return suite;
}


static Suite *create_file_suite(void)
{
    Suite *suite = suite_create("file");

    TCase *file_meta_case = tcase_create("file_meta");
    tcase_add_test(file_meta_case, test_file_sm_image_file_meta);
    suite_add_tcase(suite, file_meta_case);

    TCase *metadata_case = tcase_create("metadata");
    tcase_add_test(metadata_case, test_file_sm_image_metadata);
    suite_add_tcase(suite, metadata_case);

    TCase *frame_case = tcase_create("frame");
    tcase_add_test(frame_case, test_file_sm_image_frame);
    suite_add_tcase(suite, frame_case);

    return suite;
}


int main(void)
{
    SRunner *runner = srunner_create(create_main_suite());
    srunner_add_suite(runner, create_data_suite());
    srunner_add_suite(runner, create_file_suite());
    srunner_run_all(runner, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#else // HAVE_CHECK

#include <stdio.h>

// we're compiled unconditionally because of
// https://github.com/mesonbuild/meson/issues/2518, so ensure compile succeeds
// and we fail at runtime
int main(void)
{
    printf("Error: compiled without Check\n");
    return 1;
}

#endif // HAVE_CHECK
