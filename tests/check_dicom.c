#include "config.h"

#ifdef _WIN32
// the Windows CRT considers strncpy unsafe
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <dicom/dicom.h>


static char *my_strdup(const char *str)
{
    if (str == NULL) {
        return NULL;
    }

    size_t length = strlen(str);
    char *new_str = malloc(length + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memmove(new_str, str, length + 1);

    return new_str;
}


static char *fixture_path(const char *relpath)
{
    char *path = malloc(strlen(SRCDIR) + strlen(relpath) + 2);
    sprintf(path, "%s/%s", SRCDIR, relpath);
    return path;
}


static size_t compute_length_of_string_value(const char *value)
{
    size_t length = strlen(value);

    if (length % 2 != 0) {
        length += 1;  // zero padding
    }

    return length;
}


static size_t compute_length_of_string_value_multi(char **values,
                                                   uint32_t vm)
{
    size_t length = 0;
    for (uint32_t i = 0; i < vm; i++) {
        length += strlen(values[i]);
    }

    if (vm > 1) {
        // add the separator characters
        length += vm - 1;
    }

    if (length % 2 != 0) {
        length += 1;  // zero padding
    }

    return length;
}


static char *load_file_to_memory(const char *name, int64_t *length_out)
{
    FILE *fp;

    char *full_path = fixture_path(name);
    if (!(fp = fopen(full_path, "rb"))) {
        free(full_path);
        return NULL;
    }
    free(full_path);

    fseek(fp, 0, SEEK_END);
    int64_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (length < 0) {
        fclose(fp);
        return NULL;
    }

    char *result = malloc(length);
    if (result == NULL) {
        fclose(fp);
        return NULL;
    }

    int64_t total_read = 0;
    while (total_read < length) {
        int64_t bytes_read = fread(result + total_read,
                                   1,
                                   length - total_read,
                                   fp);
        total_read += bytes_read;
    }

    fclose(fp);

    *length_out = length;

    return result;
}


START_TEST(test_error)
{
    DcmError *error = NULL;

    DcmFilehandle *filehandle = dcm_filehandle_create_from_file(&error,
                                                                "banana");
    ck_assert_ptr_null(filehandle);
    ck_assert_ptr_nonnull(error);

    ck_assert_int_eq(dcm_error_get_code(error), DCM_ERROR_CODE_IO);
    ck_assert_ptr_nonnull(dcm_error_get_summary(error));
    ck_assert_ptr_nonnull(dcm_error_get_message(error));

    dcm_error_clear(&error);
    ck_assert_ptr_null(error);

}
END_TEST


START_TEST(test_log_level)
{
    DcmLogLevel previous_log_level;

    previous_log_level = dcm_log_set_level(DCM_LOG_INFO);
    ck_assert_int_eq(previous_log_level, DCM_LOG_NOTSET);

    previous_log_level = dcm_log_set_level(DCM_LOG_INFO);
    ck_assert_int_eq(previous_log_level, DCM_LOG_INFO);
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
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00020010), "TransferSyntaxUID");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00080018), "SOPInstanceUID");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00080030), "StudyTime");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00180050), "SliceThickness");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00185100), "PatientPosition");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00200035), "ImageOrientation");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00280008), "NumberOfFrames");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00280010), "Rows");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00400554), "SpecimenUID");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00480003), "ImagedVolumeDepth");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00620021), "TrackingUID");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00660031), "AlgorithmVersion");
    ck_assert_str_eq(dcm_dict_keyword_from_tag(0x00701305), "Plane");

    ck_assert_int_eq(dcm_dict_tag_from_keyword("SpecimenUID"), 0x00400554);
    ck_assert_int_eq(dcm_dict_tag_from_keyword("Banana"), 0xffffffff);
}
END_TEST


START_TEST(test_dict_vr_lookups)
{
    ck_assert_int_eq(dcm_vr_from_tag(0x00020010), DCM_VR_UI);
    ck_assert_int_eq(dcm_vr_from_tag(0x00080018), DCM_VR_UI);
    ck_assert_int_eq(dcm_vr_from_tag(0x00080030), DCM_VR_TM);
    ck_assert_int_eq(dcm_vr_from_tag(0x00180050), DCM_VR_DS);
    ck_assert_int_eq(dcm_vr_from_tag(0x00185100), DCM_VR_CS);
    ck_assert_int_eq(dcm_vr_from_tag(0x00200035), DCM_VR_DS);
    ck_assert_int_eq(dcm_vr_from_tag(0x00280008), DCM_VR_IS);
    ck_assert_int_eq(dcm_vr_from_tag(0x00280010), DCM_VR_US);
    ck_assert_int_eq(dcm_vr_from_tag(0x00400554), DCM_VR_UI);
    ck_assert_int_eq(dcm_vr_from_tag(0x00480003), DCM_VR_FL);
    ck_assert_int_eq(dcm_vr_from_tag(0x00620021), DCM_VR_UI);
    ck_assert_int_eq(dcm_vr_from_tag(0x00660031), DCM_VR_LO);
    ck_assert_int_eq(dcm_vr_from_tag(0x00701305), DCM_VR_FD);
}
END_TEST


START_TEST(test_element_AE)
{
    uint32_t tag = 0x00020016;
    char *value = "Application";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_AE);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_AE);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_AS)
{
    uint32_t tag = 0x00101010;
    char *value = "99";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_AS);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_AS);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_CS_multivalue)
{
    uint32_t tag = 0x00080008;
    uint32_t vm = 4;

    char **values = malloc(vm * sizeof(char *));
    values[0] = my_strdup("ORIGINAL");
    values[1] = my_strdup("PRIMARY");
    values[2] = my_strdup("LABEL");
    values[3] = my_strdup("NONE");

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_CS);
    (void) dcm_element_set_value_string_multi(NULL, element, values, vm, true);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_CS);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value_multi(values, vm));
    ck_assert_int_eq(dcm_element_is_multivalued(element), true);

    for (uint32_t i = 0; i < vm; i++) {
        const char *value;
        (void) dcm_element_get_value_string(NULL, element, i, &value);
        ck_assert_str_eq(values[i], value);
    }

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_CS_multivalue_empty)
{
    uint32_t tag = 0x00080008;
    uint32_t vm = 0;

    // since malloc(0) can be NULL on some platforms
    char **values = malloc(sizeof(char *));

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_CS);
    (void) dcm_element_set_value_string_multi(NULL, element, values, vm, true);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_CS);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value_multi(values, vm));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_DS)
{
    uint32_t tag = 0x0040072A;
    char *value = "0.0025";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_DS);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_DS);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_IS)
{
    uint32_t tag = 0x00280008;
    char *value = "10";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_IS);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_IS);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_ST)
{
    uint32_t tag = 0x00080092;
    char *value = "Random Street, Sometown";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_ST);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_ST);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_SQ)
{
    DcmElement *element = dcm_element_create(NULL, 0x00180050, DCM_VR_DS);
    (void) dcm_element_set_value_string(NULL, element, "0.01", false);

    DcmDataSet *dataset = dcm_dataset_create(NULL);
    ck_assert_int_eq(dcm_dataset_insert(NULL, dataset, element), true);

    DcmSequence *sequence = dcm_sequence_create(NULL);
    ck_assert_int_eq(dcm_sequence_append(NULL, sequence, dataset), true);

    uint32_t tag = 0x00289110;
    DcmElement *top = dcm_element_create(NULL, tag, DCM_VR_SQ);
    ck_assert_int_eq(dcm_element_set_value_sequence(NULL, top, sequence), true);

    ck_assert_int_eq(dcm_element_get_tag(top), tag);
    ck_assert_int_eq(dcm_element_get_vr(top), DCM_VR_SQ);
    ck_assert_int_eq(dcm_element_get_length(top),
                     dcm_element_get_length(element));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    DcmSequence *retrieved_value;
    (void) dcm_element_get_value_sequence(NULL, top, &retrieved_value);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 1);

    dcm_element_print(element, 0);

    dcm_element_destroy(top);
}
END_TEST


START_TEST(test_element_SQ_empty)
{
    uint32_t tag = 0x00400555;

    DcmSequence *value = dcm_sequence_create(NULL);
    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_SQ);
    dcm_element_set_value_sequence(NULL, element, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_SQ);
    ck_assert_int_eq(dcm_element_get_length(element), 0);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    DcmSequence *retrieved_value;
    (void) dcm_element_get_value_sequence(NULL, element, &retrieved_value);
    ck_assert_int_eq(dcm_sequence_count(retrieved_value), 0);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_UI)
{
    uint32_t tag = 0x00080018;
    char *value = "2.25.1";

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_UI);
    (void) dcm_element_set_value_string(NULL, element, value, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_UI);
    ck_assert_int_eq(dcm_element_get_length(element),
                     compute_length_of_string_value(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    const char *string;
    (void) dcm_element_get_value_string(NULL, element, 0, &string);
    ck_assert_str_eq(value, string);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_US)
{
    uint32_t tag = 0x00280010;
    uint16_t value = 512;

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_US);
    (void) dcm_element_set_value_integer(NULL, element, value);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_US);
    ck_assert_int_eq(dcm_element_get_length(element), sizeof(uint16_t));
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    int64_t integer;
    (void) dcm_element_get_value_integer(NULL, element, 0, &integer);
    ck_assert_int_eq(integer, value);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_US_multivalue)
{
    uint32_t tag = 0x00280010;
    uint16_t value[] = {512, 513, 514, 515};
    uint32_t vm = sizeof(value) / sizeof(value[0]);

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_US);
    (void) dcm_element_set_value_numeric_multi(NULL, element, value, vm, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_US);
    ck_assert_int_eq(dcm_element_get_length(element), sizeof(value));
    ck_assert_int_eq(dcm_element_is_multivalued(element), true);

    for (uint32_t i = 0; i < vm; i++) {
        int64_t integer;
        (void) dcm_element_get_value_integer(NULL, element, i, &integer);
        ck_assert_int_eq(value[i], integer);
    }

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_element_US_multivalue_empty)
{
    uint32_t tag = 0x00280010;
    // msvc hates zero length arrays, so use 1
    uint16_t value[] = {0};

    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_US);
    (void) dcm_element_set_value_numeric_multi(NULL, element, &value, 0, false);

    ck_assert_int_eq(dcm_element_get_tag(element), tag);
    ck_assert_int_eq(dcm_element_get_vr(element), DCM_VR_US);
    ck_assert_int_eq(dcm_element_get_length(element), 0);
    ck_assert_int_eq(dcm_element_is_multivalued(element), false);

    dcm_element_print(element, 0);

    dcm_element_destroy(element);
}
END_TEST


START_TEST(test_sequence)
{
    DcmElement *element;

    element = dcm_element_create(NULL, 0x00280010, DCM_VR_US);
    (void) dcm_element_set_value_integer(NULL, element, 256);

    DcmDataSet *dataset = dcm_dataset_create(NULL);
    dcm_dataset_insert(NULL, dataset, element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);

    DcmDataSet *other_dataset = dcm_dataset_create(NULL);
    dcm_dataset_insert(NULL, other_dataset, dcm_element_clone(NULL, element));
    ck_assert_int_eq(dcm_dataset_count(other_dataset), 1);

    element = dcm_element_create(NULL, 0x00280011, DCM_VR_US);
    (void) dcm_element_set_value_integer(NULL, element, 512);

    dcm_dataset_insert(NULL, dataset, element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 2);

    dcm_dataset_insert(NULL, other_dataset, dcm_element_clone(NULL, element));
    ck_assert_int_eq(dcm_dataset_count(other_dataset), 2);

    DcmSequence *seq = dcm_sequence_create(NULL);
    dcm_sequence_append(NULL, seq, dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 1);

    dcm_sequence_append(NULL, seq, other_dataset);
    ck_assert_int_eq(dcm_sequence_count(seq), 2);

    DcmDataSet *same_dataset = dcm_sequence_get(NULL, seq, 1);
    ck_assert_int_eq(dcm_sequence_count(seq), 2);
    ck_assert_int_eq(dcm_dataset_count(dataset),
                     dcm_dataset_count(same_dataset));

    dcm_sequence_remove(NULL, seq, 1);
    ck_assert_int_eq(dcm_sequence_count(seq), 1);

    dcm_sequence_remove(NULL, seq, 0);
    ck_assert_int_eq(dcm_sequence_count(seq), 0);

    dcm_sequence_destroy(seq);
}
END_TEST


START_TEST(test_dataset)
{
    uint32_t tag = 0x00280010;
    DcmElement *element = dcm_element_create(NULL, tag, DCM_VR_US);
    (void) dcm_element_set_value_integer(NULL, element, 256);

    uint32_t other_tag = 0x00280011;
    DcmElement *other_element = dcm_element_create(NULL, other_tag, DCM_VR_US);
    (void) dcm_element_set_value_integer(NULL, other_element, 512);

    DcmDataSet *dataset = dcm_dataset_create(NULL);
    ck_assert_int_eq(dcm_dataset_count(dataset), 0);

    dcm_dataset_insert(NULL, dataset, element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_ptr_nonnull(dcm_dataset_contains(dataset, tag));

    DcmElement *same_element = dcm_dataset_get(NULL, dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_element_get_tag(element),
                     dcm_element_get_tag(same_element));

    DcmElement *copied_element = dcm_dataset_get_clone(NULL, dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);
    ck_assert_int_eq(dcm_element_get_tag(element),
                     dcm_element_get_tag(copied_element));

    dcm_dataset_insert(NULL, dataset, other_element);
    ck_assert_int_eq(dcm_dataset_count(dataset), 2);
    ck_assert_ptr_nonnull(dcm_dataset_contains(dataset, tag));
    ck_assert_ptr_nonnull(dcm_dataset_contains(dataset, other_tag));

    dcm_dataset_remove(NULL, dataset, tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 1);

    dcm_dataset_remove(NULL, dataset, other_tag);
    ck_assert_int_eq(dcm_dataset_count(dataset), 0);

    dcm_dataset_print(dataset, 0);

    dcm_element_destroy(copied_element);
    dcm_dataset_destroy(dataset);
}
END_TEST


START_TEST(test_file_sm_image_file_meta)
{
    const char *value;
    DcmElement *element;

    char *file_path = fixture_path("data/test_files/sm_image.dcm");
    DcmFilehandle *filehandle =
        dcm_filehandle_create_from_file(NULL, file_path);
    free(file_path);
    ck_assert_ptr_nonnull(filehandle);

    const DcmDataSet *meta = dcm_filehandle_get_file_meta(NULL, filehandle);
    ck_assert_ptr_nonnull(meta);

    // Transfer Syntax UID
    element = dcm_dataset_get(NULL, meta, 0x00020010);
    (void) dcm_element_get_value_string(NULL, element, 0, &value);
    ck_assert_str_eq(value, "1.2.840.10008.1.2.1");

    // Media Storage SOP Class UID
    element = dcm_dataset_get(NULL, meta, 0x00020002);
    (void) dcm_element_get_value_string(NULL, element, 0, &value);
    ck_assert_str_eq(value, "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_print(meta, 0);

    dcm_filehandle_destroy(filehandle);
}
END_TEST


START_TEST(test_file_sm_image_metadata)
{
    char *file_path = fixture_path("data/test_files/sm_image.dcm");

    DcmFilehandle *filehandle =
        dcm_filehandle_create_from_file(NULL, file_path);
    free(file_path);
    ck_assert_ptr_nonnull(filehandle);

    const DcmDataSet *metadata =
        dcm_filehandle_get_metadata_subset(NULL, filehandle);
    ck_assert_ptr_nonnull(metadata);

    // SOP Class UID
    DcmElement *element = dcm_dataset_get(NULL, metadata, 0x00080016);
    const char *value;
    (void) dcm_element_get_value_string(NULL, element, 0, &value);
    ck_assert_str_eq(value, "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_print(metadata, 0);

    dcm_filehandle_destroy(filehandle);
}
END_TEST


START_TEST(test_file_sm_image_frame)
{
    const uint32_t frame_number = 1;

    char *file_path = fixture_path("data/test_files/sm_image.dcm");
    DcmFilehandle *filehandle =
        dcm_filehandle_create_from_file(NULL, file_path);
    free(file_path);
    ck_assert_ptr_nonnull(filehandle);

    const DcmDataSet *metadata =
        dcm_filehandle_get_metadata_subset(NULL, filehandle);
    ck_assert_ptr_nonnull(metadata);

    ck_assert_int_ne(dcm_filehandle_prepare_read_frame(NULL, filehandle), 0);

    DcmFrame *frame = dcm_filehandle_read_frame(NULL,
                                                filehandle,
                                                frame_number);
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

    dcm_frame_destroy(frame);
    dcm_filehandle_destroy(filehandle);
}
END_TEST


START_TEST(test_file_sm_image_file_meta_memory)
{
    DcmElement *element;
    const char *value;

    int64_t length;
    char *memory = load_file_to_memory("data/test_files/sm_image.dcm", &length);
    ck_assert_ptr_nonnull(memory);

    DcmFilehandle *filehandle =
        dcm_filehandle_create_from_memory(NULL, memory, length);
    ck_assert_ptr_nonnull(filehandle);

    const DcmDataSet *meta = dcm_filehandle_get_file_meta(NULL, filehandle);

    // Transfer Syntax UID
    element = dcm_dataset_get(NULL, meta, 0x00020010);
    (void) dcm_element_get_value_string(NULL, element, 0, &value);
    ck_assert_str_eq(value, "1.2.840.10008.1.2.1");

    // Media Storage SOP Class UID
    element = dcm_dataset_get(NULL, meta, 0x00020002);
    (void) dcm_element_get_value_string(NULL, element, 0, &value);
    ck_assert_str_eq(value, "1.2.840.10008.5.1.4.1.1.77.1.6");

    dcm_dataset_print(meta, 0);

    dcm_filehandle_destroy(filehandle);
    free(memory);
}
END_TEST


static Suite *create_main_suite(void)
{
    Suite *suite = suite_create("main");

    TCase *error_case = tcase_create("error");
    tcase_add_test(error_case, test_error);
    suite_add_tcase(suite, error_case);

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
    tcase_add_test(element_case, test_element_CS_multivalue_empty);
    tcase_add_test(element_case, test_element_DS);
    tcase_add_test(element_case, test_element_IS);
    tcase_add_test(element_case, test_element_ST);
    tcase_add_test(element_case, test_element_SQ);
    tcase_add_test(element_case, test_element_SQ_empty);
    tcase_add_test(element_case, test_element_UI);
    tcase_add_test(element_case, test_element_US);
    tcase_add_test(element_case, test_element_US_multivalue);
    tcase_add_test(element_case, test_element_US_multivalue_empty);
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

    TCase *memory_case = tcase_create("memory");
    tcase_add_test(memory_case, test_file_sm_image_file_meta_memory);
    suite_add_tcase(suite, memory_case);

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
