#include <stdlib.h>
#include <check.h>

#include "../src/dicom.h"


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


START_TEST(test_element_UI)
{
    uint32_t tag;
    uint32_t vm;
    uint32_t i;
    char vr[3] = "UI";
    uint32_t expected_length;
    dcm_element_t *element = NULL;
    char **values = NULL;
    char *retrieved_value = NULL;

    tag = 0x00080018;
    vm = 1;
    i = 0;
    values = malloc(vm * sizeof(char *));
    values[0] = malloc(7);
    strcpy(values[0], "2.25.1");
    element = dcm_element_create_UI(tag, values, vm);

    expected_length = strlen(values[i]);
    if (expected_length % 2 != 0) {
        expected_length += 1;  // zero padding
    }

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), expected_length);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    retrieved_value = malloc(strlen(values[i]));
    dcm_element_copy_value_UI(element, i, retrieved_value);
    ck_assert_str_eq(values[i], retrieved_value);

    free(retrieved_value);
    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_IS)
{
    uint32_t tag;
    uint32_t vm;
    uint32_t i;
    char vr[3] = "IS";
    uint32_t expected_length;
    dcm_element_t *element = NULL;
    char **values = NULL;
    char *retrieved_value = NULL;

    tag = 0x00280008;
    vm = 1;
    i = 0;
    values = malloc(vm * sizeof(char *));
    values[0] = malloc(3);
    strcpy(values[0], "10");
    element = dcm_element_create_IS(tag, values, vm);

    expected_length = strlen(values[i]);
    if (expected_length % 2 != 0) {
        expected_length += 1;  // zero padding
    }

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), expected_length);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    retrieved_value = malloc(strlen(values[i]));
    dcm_element_copy_value_IS(element, i, retrieved_value);
    ck_assert_str_eq(values[i], retrieved_value);

    free(retrieved_value);
    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_US)
{
    uint32_t tag;
    uint32_t vm;
    uint32_t i;
    char vr[3] = "US";
    dcm_element_t *element = NULL;
    uint16_t *values = NULL;
    uint16_t retrieved_value;

    tag = 0x00280010;
    vm = 1;
    i = 0;
    values = malloc(vm * sizeof(uint16_t));
    values[0] = 512;
    element = dcm_element_create_US(tag, values, vm);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), sizeof(uint16_t));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    dcm_element_copy_value_US(element, i, &retrieved_value);
    ck_assert_int_eq(values[i], retrieved_value);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_CS_multivalue)
{
    uint32_t tag;
    uint32_t vm;
    uint32_t i;
    char vr[3] = "CS";
    dcm_element_t *element = NULL;
    char **values = NULL;
    char *retrieved_value = NULL;
    uint32_t expected_length;

    tag = 0x00080008;
    vm = 4;
    i = 0;
    values = malloc(vm * sizeof(char *));
    values[0] = malloc(9);
    strcpy(values[0], "ORIGINAL");
    values[1] = malloc(8);
    strcpy(values[1], "PRIMARY");
    values[2] = malloc(7);
    strcpy(values[2], "VOLUME");
    values[3] = malloc(5);
    strcpy(values[3], "NONE");
    element = dcm_element_create_CS(tag, values, vm);

    expected_length = 3 * 2;  // separators between values
    for (i = 0; i < vm; i++) {
        expected_length += strlen(values[i]);
    }
    if (expected_length % 2 != 0) {
        expected_length += 1;  // zero padding
    }

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), expected_length);
    ck_assert_int_eq(dcm_element_is_multivalued(element), true);

    retrieved_value = malloc(DCM_CAPACITY_CS + 1);
    for (i = 0; i < vm; i++) {
        dcm_element_copy_value_CS(element, i, retrieved_value);
        ck_assert_str_eq(values[i], retrieved_value);
    }
    free(retrieved_value);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_ST)
{
    uint32_t tag;
    char vr[3] = "ST";
    uint32_t expected_length;
    dcm_element_t *element = NULL;
    char *value = NULL;
    char *retrieved_value = NULL;

    tag = 0x00080092;
    value = malloc(24);
    strcpy(value, "Random Street, Sometown");
    element = dcm_element_create_ST(tag, value);

    expected_length = strlen(value);
    if (expected_length % 2 != 0) {
        expected_length += 1;  // zero padding
    }

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), expected_length);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    retrieved_value = malloc(DCM_CAPACITY_ST + 1);
    dcm_element_copy_value_ST(element, retrieved_value);
    ck_assert_str_eq(value, retrieved_value);
    free(retrieved_value);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_SQ)
{
    uint32_t tag;
    char vr[3] = "SQ";
    dcm_element_t *element = NULL;
    dcm_element_t *item_element = NULL;
    dcm_dataset_t *item = NULL;
    dcm_sequence_t *value = NULL;
    dcm_sequence_t *retrieved_value = NULL;
    char **item_values = NULL;

    item_values = malloc(sizeof(char *));
    item_values[0] = malloc(5);
    strcpy(item_values[0], "0.01");
    item_element = dcm_element_create_DS(0x00180050, item_values, 1);

    item = dcm_dataset_create();
    ck_assert_int_eq(dcm_dataset_insert(item, item_element), true);

    tag = 0x00289110;
    value = dcm_sequence_create();
    ck_assert_int_eq(dcm_sequence_append(value, item), true);
    element = dcm_element_create_SQ(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element),
                     dcm_element_get_length(item_element));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    retrieved_value = dcm_element_get_value_SQ(element);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 1);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_SQ_empty)
{
    uint32_t tag;
    char vr[3] = "SQ";
    dcm_element_t *element = NULL;
    dcm_sequence_t *value = NULL;
    dcm_sequence_t *retrieved_value = NULL;

    tag = 0x00400555;
    value = dcm_sequence_create();
    element = dcm_element_create_SQ(tag, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_check_vr(element, vr), true);
    ck_assert_int_eq(dcm_element_get_length(element), 0);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    retrieved_value = dcm_element_get_value_SQ(element);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_sequence)
{
    uint16_t *values = NULL;
    dcm_element_t *element = NULL;
    dcm_dataset_t *dataset = NULL, *same_dataset = NULL, *other_dataset = NULL;
    dcm_sequence_t *seq = NULL;

    dataset = dcm_dataset_create();
    other_dataset = dcm_dataset_create();

    values = malloc(sizeof(uint16_t));
    values[0] = 256;
    element = dcm_element_create_US(0x00280010, values, 1);
    dcm_dataset_insert(dataset, element);
    dcm_dataset_insert(other_dataset, dcm_element_clone(element));

    values = malloc(sizeof(uint16_t));
    values[0] = 512;
    element = dcm_element_create_US(0x00280011, values, 1);
    dcm_dataset_insert(dataset, element);
    dcm_dataset_insert(other_dataset, dcm_element_clone(element));

    seq = dcm_sequence_create();

    dcm_sequence_append(seq, dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 1);
    dcm_sequence_append(seq, other_dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 2);

    same_dataset = dcm_sequence_get(seq, 1);
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
    uint32_t tag, other_tag;
    uint16_t *values = NULL, *other_values = NULL;
    dcm_element_t *element = NULL, *same_element = NULL, *copied_element = NULL;
    dcm_element_t *other_element = NULL;
    dcm_dataset_t *dataset = NULL;

    tag = 0x00280010;
    values = malloc(sizeof(uint16_t));
    values[0] = 256;
    element = dcm_element_create_US(tag, values, 1);

    other_tag = 0x00280011;
    other_values = malloc(sizeof(uint16_t));
    other_values[0] = 512;
    other_element = dcm_element_create_US(other_tag, other_values, 1);

    dataset = dcm_dataset_create();
    ck_assert_int_eq(dcm_dataset_count(dataset), 0);

    dcm_dataset_insert(dataset, element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_dataset_contains(dataset, tag), true);

    same_element = dcm_dataset_get(dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_element_get_tag(element),
                     dcm_element_get_tag(same_element));

    copied_element = dcm_dataset_get_clone(dataset, tag);
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

    dcm_element_destroy(copied_element);
    dcm_dataset_destroy(dataset);
}
END_TEST


START_TEST(test_file_sm_image_file_meta)
{
    uint32_t tag;
    const char *file_path = "./data/test_files/sm_image.dcm";
    char *value = NULL;
    dcm_file_t *file = NULL;
    dcm_dataset_t *file_meta = NULL;
    dcm_element_t *element = NULL;

    file = dcm_file_create(file_path, 'r');

    file_meta = dcm_file_read_file_meta(file);

    // Transfer Syntax UID
    tag = 0x00020010;
    element = dcm_dataset_get(file_meta, tag);
    value = malloc(DCM_CAPACITY_UI + 1);
    dcm_element_copy_value_UI(element, 0, value);
    ck_assert_str_eq(value, "1.2.840.10008.1.2.1");

    // Media Storage SOP Class UID
    tag = 0x00020002;
    element = dcm_dataset_get(file_meta, tag);
    value = malloc(DCM_CAPACITY_UI + 1);
    dcm_element_copy_value_UI(element, 0, value);
    ck_assert_str_eq(value, "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_destroy(file_meta);
    dcm_file_destroy(file);
}
END_TEST


START_TEST(test_file_sm_image_metadata)
{
    uint32_t tag;
    const char *file_path = "./data/test_files/sm_image.dcm";
    char *value = NULL;
    dcm_file_t *file = NULL;
    dcm_dataset_t *metadata = NULL;
    dcm_element_t *element = NULL;

    file = dcm_file_create(file_path, 'r');

    metadata = dcm_file_read_metadata(file);

    // SOP Class UID
    tag = 0x00080016;
    element = dcm_dataset_get(metadata, tag);
    value = malloc(DCM_CAPACITY_UI + 1);
    dcm_element_copy_value_UI(element, 0, value);
    ck_assert_str_eq(value, "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_destroy(metadata);
    dcm_file_destroy(file);
}
END_TEST


START_TEST(test_file_sm_image_frame)
{
    uint32_t frame_number = 1;
    const char *file_path = "./data/test_files/sm_image.dcm";
    dcm_file_t *file = NULL;
    dcm_dataset_t *metadata = NULL;
    dcm_frame_t *frame = NULL;
    dcm_bot_t *bot = NULL;

    file = dcm_file_create(file_path, 'r');

    metadata = dcm_file_read_metadata(file);

    bot = dcm_file_build_bot(file, metadata);
    ck_assert_uint_eq(dcm_bot_get_num_frames(bot), 25);

    frame = dcm_file_read_frame(file, metadata, bot, frame_number);
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


Suite *create_main_suite(void)
{
    Suite *suite = NULL;
    TCase *log_case = NULL, *dict_case = NULL;

    suite = suite_create("main");

    log_case = tcase_create("log");
    tcase_add_test(log_case, test_log_level);
    suite_add_tcase(suite, log_case);

    dict_case = tcase_create("dict");
    tcase_add_test(dict_case, test_tag_validity_checks);
    tcase_add_test(dict_case, test_vr_validity_checks);
    tcase_add_test(dict_case, test_dict_tag_lookups);
    tcase_add_test(dict_case, test_dict_vr_lookups);
    suite_add_tcase(suite, dict_case);

    return suite;
}


Suite *create_data_suite(void)
{
    Suite *suite = NULL;
    TCase *element_case = NULL, *dataset_case = NULL, *sequence_case = NULL;

    suite = suite_create("data");

    element_case = tcase_create("element");
    tcase_add_test(element_case, test_element_UI);
    tcase_add_test(element_case, test_element_IS);
    tcase_add_test(element_case, test_element_US);
    tcase_add_test(element_case, test_element_CS_multivalue);
    tcase_add_test(element_case, test_element_ST);
    tcase_add_test(element_case, test_element_SQ);
    tcase_add_test(element_case, test_element_SQ_empty);
    suite_add_tcase(suite, element_case);

    dataset_case = tcase_create("dataset");
    tcase_add_test(dataset_case, test_dataset);
    suite_add_tcase(suite, dataset_case);

    sequence_case = tcase_create("sequence");
    tcase_add_test(sequence_case, test_sequence);
    suite_add_tcase(suite, sequence_case);

    return suite;
}


Suite *create_file_suite(void)
{
    Suite *suite = NULL;
    TCase *file_meta_case = NULL;
    TCase *metadata_case = NULL;
    TCase *frame_case = NULL;

    suite = suite_create("file");

    file_meta_case = tcase_create("file_meta");
    tcase_add_test(file_meta_case, test_file_sm_image_file_meta);
    suite_add_tcase(suite, file_meta_case);

    metadata_case = tcase_create("metadata");
    tcase_add_test(metadata_case, test_file_sm_image_metadata);
    suite_add_tcase(suite, metadata_case);

    frame_case = tcase_create("frame");
    tcase_add_test(frame_case, test_file_sm_image_frame);
    suite_add_tcase(suite, frame_case);

    return suite;
}


int main(void)
{
    int number_failed;
    SRunner *runner = NULL;

    runner = srunner_create(create_main_suite());
    srunner_add_suite(runner, create_data_suite());
    srunner_add_suite(runner, create_file_suite());
    srunner_run_all(runner, CK_VERBOSE);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
