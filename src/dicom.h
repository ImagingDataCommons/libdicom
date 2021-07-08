#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DCM_INCLUDED
#define DCM_INCLUDED

/**
 * Maximum number of characters in values with Value Representation AE.
 */
#define DCM_CAPACITY_AE 16

/**
 * Maximum number of characters in values with Value Representation AS.
 */
#define DCM_CAPACITY_AS 4

/**
 * Maximum number of characters in values with Value Representation AT.
 */
#define DCM_CAPACITY_AT 4

/**
 * Maximum number of characters in values with Value Representation CS.
 */
#define DCM_CAPACITY_CS 16

/**
 * Maximum number of characters in values with Value Representation DA.
 */
#define DCM_CAPACITY_DA 8

/**
 * Maximum number of characters in values with Value Representation DS.
 */
#define DCM_CAPACITY_DS 16

/**
 * Maximum number of characters in values with Value Representation DT.
 */
#define DCM_CAPACITY_DT 26

/**
 * Maximum number of characters in values with Value Representation LO.
 */
#define DCM_CAPACITY_LO 64

/**
 * Maximum number of characters in values with Value Representation IS.
 */
#define DCM_CAPACITY_IS 12

/**
 * Maximum number of characters in values with Value Representation PN.
 */
#define DCM_CAPACITY_PN 64

/**
 * Maximum number of characters in values with Value Representation SH.
 */
#define DCM_CAPACITY_SH 16

/**
 * Maximum number of characters in values with Value Representation ST.
 */
#define DCM_CAPACITY_ST 1024

/**
 * Maximum number of characters in values with Value Representation TM.
 */
#define DCM_CAPACITY_TM 14

/**
 * Maximum number of characters in values with Value Representation UI.
 */
#define DCM_CAPACITY_UI 64

/**
 * Part10 file
 */
typedef struct dcm_File dcm_file_t;

/**
 * Data Element
 */
typedef struct dcm_Element dcm_element_t;

/**
 * Data Set
 */
typedef struct dcm_DataSet dcm_dataset_t;

/**
 * Sequence of Data Set Items
 */
typedef struct dcm_Sequence dcm_sequence_t;

/**
 * Frame Item of Pixel Data Element
 */
typedef struct dcm_Frame dcm_frame_t;

/**
 * Basic Offset Table (BOT) Item of Pixel Data Element
 */
typedef struct dcm_BOT dcm_bot_t;


/**
 * Enumeration of log levels
 */
enum dcm_LogLevel {
    /** Critical */
    DCM_LOG_CRITICAL = 50,
    /** Error */
    DCM_LOG_ERROR = 40,
    /** Warning */
    DCM_LOG_WARNING = 30,
    /** Info */
    DCM_LOG_INFO = 20,
    /** Debug */
    DCM_LOG_DEBUG = 10,
    /** Not set (no logging) */
    DCM_LOG_NOTSET = 0,
};

/**
 * Log level
 */
typedef enum dcm_LogLevel dcm_log_level_t;

/**
 * Global variable to set log level.
 */
extern dcm_log_level_t dcm_log_level;

/**
 * Write critical log message to stderr stream.
 *
 * :param format: Format string.
 * :param ...: Variable arguments
 */
extern void dcm_log_critical(const char *format, ...);

/**
 * Write error log message to stderr stream.
 *
 * :param format: Format string.
 * :param ...: Variable arguments
 */
extern void dcm_log_error(const char *format, ...);

/**
 * Write warning log message to stderr stream.
 *
 * :param format: Format string.
 * :param ...: Variable arguments
 */
extern void dcm_log_warning(const char *format, ...);

/**
 * Write info log message to stderr stream.
 *
 * :param format: Format string
 * :param ...: Variable arguments
 */
extern void dcm_log_info(const char *format, ...);

/**
 * Write debug log message to stderr stream.
 *
 * :param format: Format string
 * :param ...: Variable arguments
 */
extern void dcm_log_debug(const char *format, ...);

/**
 * Get the version of the library.
 *
 * :return: semantic version string
 */
extern const char *dcm_get_version(void);

/**
 * Look up the Value Representation of an Attribute in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: name of attribute Value Representation
 */
extern const char *dcm_dict_lookup_vr(uint32_t tag);

/**
 * Look up the Keyword of an Attribute in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: attribute Keyword
 */
extern const char *dcm_dict_lookup_keyword(uint32_t tag);

/**
 * Determine whether a Tag is public.
 *
 * A Tag is public if it is defined in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: yes/no answer
 */
extern bool dcm_is_public_tag(uint32_t tag);

/**
 * Determine whether a Tag is private.
 *
 * :param tag: Attribute Tag
 *
 * :return: yes/no answer
 */
extern bool dcm_is_private_tag(uint32_t tag);

/**
 * Determine whether a Tag is valid.
 *
 * :param tag: Attribute Tag
 *
 * :return: yes/no answer
 */
extern bool dcm_is_valid_tag(uint32_t tag);

/**
 * Determine whether a Value Representation is valid.
 *
 * :param vr: Attribute Value Representation
 *
 * :return: yes/no answer
 */
extern bool dcm_is_valid_vr(const char *vr);

/**
 * Determine whether a Transfer Syntax is encapsulated.
 *
 * :param transfer_syntax_uid: Transfer Syntax UID
 *
 * :return: yes/no answer
 */
extern bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid);


/**
 * Data Element
 */

/**
 * Create a Data Element with Value Representation AE (Application Entity).
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Data Element
 */
extern dcm_element_t *dcm_element_create_AE(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation AS (Age String).
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Data Element
 */
extern dcm_element_t *dcm_element_create_AS(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation AT (Attribute Tag).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_AT(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation CS (Code String).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_CS(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation DA (Date).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_DA(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation DS (Decimal String).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_DS(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation DT (Date Time).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_DT(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation FD (Floating Point Double).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_FD(uint32_t tag,
                                            double *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation FL (Floating Point Single).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_FL(uint32_t tag,
                                            float *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation IS (Integer String).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_IS(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation LO (Long String).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_LO(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation PN (Person Name).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_PN(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation SH (Short String).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_SH(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation TM (Time).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_TM(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation UI (Unique Identifier).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UI(uint32_t tag,
                                            char **values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation SS (Signed Short).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_SS(uint32_t tag,
                                            int16_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation SL (Signed Long).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_SL(uint32_t tag,
                                            int32_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation SV (Signed Very Long).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_SV(uint32_t tag,
                                            int64_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation UL (Unsigned Long).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UL(uint32_t tag,
                                            uint32_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation US (Unsigned Short).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_US(uint32_t tag,
                                            uint16_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation UV (Unsigned Very Long).
 *
 * :param tag: Tag
 * :param values: Array of values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UV(uint32_t tag,
                                            uint64_t *values,
                                            uint32_t vm);

/**
 * Create a Data Element with Value Representation ST (Short Text).
 *
 * :param tag: Tag
 * :param value: Value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_ST(uint32_t tag,
                                            char *value);

/**
 * Create a Data Element with Value Representation SQ (Sequence).
 *
 * :param tag: Tag
 * :param value: Value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_SQ(uint32_t tag,
                                            dcm_sequence_t *value);

/**
 * Create a Data Element with Value Representation LT (Long Text).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_LT(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation OB (Other Byte).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_OB(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation OD (Other Double).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_OD(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation OF (Other Float).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_OF(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation OV (Other Very Long).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_OV(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation OW (Other Word).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_OW(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation UC (Unlimited Characters).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UC(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation UN (Unknown).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UN(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation UR (Universal Resource Identifier).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UR(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Create a Data Element with Value Representation UT (Unlimited Text).
 *
 * :param tag: Tag
 * :param value: Value
 * :param length: Length of value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: pointer to Data Element
 */
extern dcm_element_t *dcm_element_create_UT(uint32_t tag,
                                            char *value,
                                            uint32_t length);

/**
 * Get group number (first part of Tag) of a Data Element.
 *
 * :param element: Data Element
 *
 * :return: Tag group number
 */
extern uint16_t dcm_element_get_group_number(dcm_element_t *element);

/**
 * Get element number (second part of Tag) of a Data Element.
 *
 * :param element: Data Element
 *
 * :return: Tag element number
 */
extern uint16_t dcm_element_get_element_number(dcm_element_t *element);

/**
 * Get Tag of a Data Element.
 *
 * :param element: Data Element
 *
 * :return: Tag
 */
extern uint32_t dcm_element_get_tag(dcm_element_t *element);

/**
 * Check Value Representation of a Data Element.
 *
 * :param element: Data Element
 * :param vr: Value Representation
 *
 * :return: whether Data Element has the specified Value Representation
 */
extern bool dcm_element_check_vr(dcm_element_t *element, const char *vr);

/**
 * Get length of a Data Element.
 *
 * :param element: Data Element
 *
 * :return: length
 */
extern uint32_t dcm_element_get_length(dcm_element_t *element);

/**
 * Get Value Multiplicity of a Data Element.
 *
 * :param element: Data Element
 *
 * :return: Value Multiplicity
 */
extern uint32_t dcm_element_get_vm(dcm_element_t *element);

/**
 * Determine whether a Data Element has a Value Multiplicity greater than one.
 *
 * :param element: Data Element
 *
 * :return: yes/no answer
 */
extern bool dcm_element_is_multivalued(dcm_element_t *element);

/**
 * Clone (i.e., create a deep copy of) a Data Element.
 *
 * :param element: Data Element
 *
 * :return: Data Element
 */
extern dcm_element_t *dcm_element_clone(dcm_element_t *element);

/**
 * Copy value of a Data Element with Value Representation CS (Code String).
 *
 * :param element: Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to memory location into which to copy value
 */
extern void dcm_element_copy_value_CS(dcm_element_t *element,
                                      uint32_t index,
                                      char *value);

/**
 * Copy value of a Data Element with Value Representation IS (Integer String).
 *
 * :param element: Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to memory location into which to copy value
 */
extern void dcm_element_copy_value_IS(dcm_element_t *element,
                                      uint32_t index,
                                      char *value);

/**
 * Copy value of a Data Element with Value Representation FD (Floating Point Double).
 *
 * :param element: Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to memory location into which to copy value
 */
extern void dcm_element_copy_value_FD(dcm_element_t *element,
                                      uint32_t index,
                                      double *value);

extern void dcm_element_copy_value_FL(dcm_element_t *element,
                                      uint32_t index,
                                      float *value);

extern void dcm_element_copy_value_SL(dcm_element_t *element,
                                      uint32_t index,
                                      int32_t *value);

extern void dcm_element_copy_value_ST(dcm_element_t *element,
                                      char *value);

extern void dcm_element_copy_value_UI(dcm_element_t *element,
                                      uint32_t index,
                                      char *value);

extern void dcm_element_copy_value_UL(dcm_element_t *element,
                                      uint32_t index,
                                      uint32_t *value);

extern void dcm_element_copy_value_US(dcm_element_t *element,
                                      uint32_t index,
                                      uint16_t *value);

extern void dcm_element_copy_value_UV(dcm_element_t *element,
                                      uint32_t index,
                                      uint16_t *value);

extern void dcm_element_copy_value_SS(dcm_element_t *element,
                                      uint32_t index,
                                      int16_t *value);

extern void dcm_element_copy_value_SV(dcm_element_t *element,
                                      uint32_t index,
                                      int64_t *value);

extern void dcm_element_copy_value_ST(dcm_element_t *element,
                                      char *value);

extern void dcm_element_copy_value_OB(dcm_element_t *element,
                                      char *value,
                                      ssize_t n);

extern dcm_sequence_t *dcm_element_get_value_SQ(dcm_element_t *element);

/**
 * Print a Data Element.
 *
 * :param element: Data Element
 * :param indentation: Number of white spaces before text
 */
extern void dcm_element_print(dcm_element_t *element, uint8_t indentation);

/**
 * Destroy a Data Element.
 *
 * :param element: Data Element
 */
extern void dcm_element_destroy(dcm_element_t *element);


/**
 * Dataset
 */

/**
 * Create an empty Dataset.
 */
extern dcm_dataset_t *dcm_dataset_create(void);

/**
 * Insert a Data Element into a Dataset.
 *
 * :param dataset: Dataset
 * :param element: Data Element
 *
 * The object takes over ownership of the memory referenced by `element`
 * and frees it when the object is destroyed or if the insert operation fails.
 *
 * :return: whether insert operation was successful
 */
extern bool dcm_dataset_insert(dcm_dataset_t *dataset, dcm_element_t *element);

/**
 * Remove a Data Element from a Dataset.
 *
 * :param dataset: Dataset
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: whether remove operation was successful
 */
extern bool dcm_dataset_remove(dcm_dataset_t *dataset, uint32_t tag);

/**
 * Get a Data Element from a Dataset.
 *
 * :param dataset: Dataset
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Data Element
 */
extern dcm_element_t *dcm_dataset_get(dcm_dataset_t *dataset, uint32_t tag);

/**
 * Get a clone (deep copy) of a Data Element from a Dataset.
 *
 * :param dataset: Dataset
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: clone of Data Element
 */
extern dcm_element_t *dcm_dataset_get_clone(dcm_dataset_t *dataset,
                                            uint32_t tag);

/**
 * Iterate over Data Elements in a Dataset.
 *
 * :param dataset: Dataset
 * :param fn: Function that should be called for each Data Element
 */
extern void dcm_dataset_foreach(dcm_dataset_t *dataset,
                                void (*fn)(dcm_element_t *element));

/**
 * Determine whether a Data Element is contained in a Dataset.
 *
 * :param dataset: Dataset
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: yes/no answer
 */
extern bool dcm_dataset_contains(dcm_dataset_t *dataset, uint32_t tag);

/**
 * Count the number of Data Elements in a Dataset.
 *
 * :param dataset: Dataset
 *
 * :return: number of Data Elements
 */
extern uint32_t dcm_dataset_count(dcm_dataset_t *dataset);

/**
 * Obtain a copy of the Tag of each Data Element in a Dataset.
 *
 * :param dataset: Dataset
 * :param tags: Pointer to memory location to which to copy tags
 */
extern void dcm_dataset_copy_tags(dcm_dataset_t *dataset, uint32_t *tags);

/**
 * Lock a Dataset to prevent modification.
 *
 * :param dataset: Dataset
 */
extern void dcm_dataset_lock(dcm_dataset_t *dataset);

/**
 * Check whether a Dataset is locked.
 *
 * :param dataset: Dataset
 *
 * :return: yes/no answer
 */
extern bool dcm_dataset_is_locked(dcm_dataset_t *dataset);

/**
 * Print a Dataset.
 *
 * :param dataset: Dataset
 * :param indentation: Number of white spaces before text
 */
extern void dcm_dataset_print(dcm_dataset_t *dataset, uint8_t indentation);

/**
 * Destroy a Dataset.
 *
 * :param dataset: Dataset
 */
extern void dcm_dataset_destroy(dcm_dataset_t *dataset);


/**
 * Sequence
 */

/**
 * Create a Sequence, i.e., a collection of Dataset items that represent the
 * value of a Data Element with Value Representation SQ (Sequence).
 *
 * Note that created object represents the value of a Data Element rather
 * than a Data Element itself.
 *
 * :return: Sequence
 */
extern dcm_sequence_t *dcm_sequence_create(void);

/**
 * Append a Dataset item to a Sequence.
 *
 * :param seq: Sequence
 * :param item: Dataset item
 *
 * The object takes over ownership of the memory referenced by `item`
 * and frees it when the object is destroyed or if the append operation fails.
 *
 * :return: whether append operation was successful
 */
extern bool dcm_sequence_append(dcm_sequence_t *seq, dcm_dataset_t *item);

/**
 * Get a Dataset item from a Sequence.
 *
 * :param seq: Sequence
 * :param index: Zero-based index of the Dataset item in the Sequence
 *
 * :return: Dataset item
 */
extern dcm_dataset_t *dcm_sequence_get(dcm_sequence_t *seq, uint32_t index);

/**
 * Iterate over Dataset items in a Sequence.
 *
 * :param seq: Sequence
 * :param fn: Function that should be called for each Dataset item
 */
extern void dcm_sequence_foreach(dcm_sequence_t *seq,
                                 void (*fn)(dcm_dataset_t *item));

/**
 * Remove a Dataset item from a Sequence.
 *
 * :param seq: Sequence
 * :param index: Zero-based index of the Dataset item in the Sequence
 */
extern void dcm_sequence_remove(dcm_sequence_t *seq, uint32_t index);

/**
 * Count the number of Dataset items in a Sequence.
 *
 * :param seq: Sequence
 *
 * :return: number of Dataset items
 */
extern uint32_t dcm_sequence_count(dcm_sequence_t *seq);

/**
 * Lock a Sequence to prevent modification.
 *
 * :param sequence: Sequence
 */
extern void dcm_sequence_lock(dcm_sequence_t *seq);

/**
 * Check whether a Sequence is locked.
 *
 * :param sequence: Sequence
 *
 * :return: yes/no answer
 */
extern bool dcm_sequence_is_locked(dcm_sequence_t *seq);

/**
 * Destroy a Sequence.
 *
 * :param seq: Sequence
 */
extern void dcm_sequence_destroy(dcm_sequence_t *seq);


/**
 * Frame
 *
 * Encoded pixels of an individual pixel matrix and associated
 * descriptive metadata.
 */

/**
 * Create a Frame.
 *
 * :param index: Index of the Frame within the Pixel Data Element
 * :param data: Pixel data of the Frame
 * :param length: Size of the Frame (number of bytes)
 * :param rows: Number of rows in pixel matrix
 * :param columns: Number of columns in pixel matrix
 * :param samples_per_pixel: Number of samples per pixel
 * :param bits_allocated: Number of bits allocated per pixel
 * :param bits_stored: Number of bits stored per pixel
 * :param pixel_representation: Representation of pixels
 *                              (unsigned integers or 2's complement)
 * :param planar_configuration: Configuration of samples
 *                              (color-by-plane or color-by-pixel)
 * :param photometric_interpretation: Interpretation of pixels
 *                                    (monochrome, RGB, etc.)
 * :param transfer_syntax_uid: UID of transfer syntax in which data is encoded
 *
 * The object takes over ownership of the memory referenced by `data`,
 * `photometric_interpretation`, and `transfer_syntax_uid`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Frame Item
 */
extern dcm_frame_t *dcm_frame_create(uint32_t number,
                                     char *data,
                                     uint32_t length,
                                     uint16_t rows,
                                     uint16_t columns,
                                     uint16_t samples_per_pixel,
                                     uint16_t bits_allocated,
                                     uint16_t bits_stored,
                                     uint16_t pixel_representation,
                                     uint16_t planar_configuration,
                                     const char *photometric_interpretation,
                                     const char *transfer_syntax_uid);

/**
 * Get number of a Frame Item within the Pixel Data Element.
 *
 * :param frame: Frame
 *
 * :return: number (one-based index)
 */
extern uint32_t dcm_frame_get_number(dcm_frame_t *frame);

/**
 * Get length of a Frame Item.
 *
 * :param frame: Frame
 *
 * :return: number of bytes
 */
extern uint32_t dcm_frame_get_length(dcm_frame_t *frame);

/**
 * Get Rows of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of rows in pixel matrix
 */
extern uint16_t dcm_frame_get_rows(dcm_frame_t *frame);

/**
 * Get Columns of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of columns in pixel matrix
 */
extern uint16_t dcm_frame_get_columns(dcm_frame_t *frame);

/**
 * Get Samples per Pixel of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of samples (color channels) per pixel
 */
extern uint16_t dcm_frame_get_samples_per_pixel(dcm_frame_t *frame);

/**
 * Get Bits Allocated of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits allocated per pixel
 */
extern uint16_t dcm_frame_get_bits_allocated(dcm_frame_t *frame);

/**
 * Get Bits Stored of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits stored per pixel
 */
extern uint16_t dcm_frame_get_bits_stored(dcm_frame_t *frame);

/**
 * Get High Bit of a Frame.
 *
 * :param frame: Frame
 *
 * :return: most significant bit of pixels
 */
extern uint16_t dcm_frame_get_high_bit(dcm_frame_t *frame);

/**
 * Get Pixel Representation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: representation of pixels (unsigned integers or 2's complement)
 */
extern uint16_t dcm_frame_get_pixel_representation(dcm_frame_t *frame);

/**
 * Get Planar Configuration of a Frame.
 *
 * :param frame: Frame
 *
 * :return: configuration of samples (color-by-plane or color-by-pixel)
 */
extern uint16_t dcm_frame_get_planar_configuration(dcm_frame_t *frame);

/**
 * Get Photometric Interpretation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: interpretation of pixels (monochrome, RGB, etc.)
 */
extern const char *dcm_frame_get_photometric_interpretation(dcm_frame_t *frame);

/**
 * Get Transfer Syntax UID for a Frame.
 *
 * :param frame: Frame
 *
 * :return: UID of the transfer syntax in which frame is encoded
 */
extern const char *dcm_frame_get_transfer_syntax_uid(dcm_frame_t *frame);

/**
 * Get pixel data of a Frame.
 *
 * :param frame: Frame
 *
 * :return: pixel data
 */
extern const char *dcm_frame_get_value(dcm_frame_t *frame);

/**
 * Destroy a Frame.
 *
 * :param frame: Frame
 */
extern void dcm_frame_destroy(dcm_frame_t *frame);


/**
 * Basic Offset Table (BOT).
 */

/**
 * Create a Basic Offset Table.
 *
 * :param offsets: Offset of each Frame in the Pixel Data Element
 *                 (measured from the first byte of the first Frame).
 * :param num_frames: Number of Frames in the Pixel Data Element
 *                    (measured from the first byte of the File).
 *
 * The created object takes over ownership of the memory referenced by `offsets`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Basic Offset Table
 */
extern dcm_bot_t *dcm_bot_create(ssize_t *offsets, uint32_t num_frames);

/**
 * Get number of Frame offsets in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 *
 * :return: number of frames
 */
extern uint32_t dcm_bot_get_num_frames(dcm_bot_t *bot);

/**
 * Get Frame offset in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 * :param index: Zero-based index of Frame in the Pixel Data Element
 *
 * :return: offset
 */
extern ssize_t dcm_bot_get_frame_offset(dcm_bot_t *bot, uint32_t index);

/**
 * Print a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
extern void dcm_bot_print(dcm_bot_t *bot);

/**
 * Destroy a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
extern void dcm_bot_destroy(dcm_bot_t *bot);


/**
 * Part 10 File
 */

/**
 * Create a File
 *
 * :param file_path: Path to the file on disk.
 * :param mode: File Mode to use when opening the file.
 *
 * :return: file
 */
extern dcm_file_t *dcm_file_create(const char *file_path, char mode);

/**
 * Read File Metainformation from a File.
 *
 * :param file: File
 *
 * :return: File Metainformation
 */
extern dcm_dataset_t *dcm_file_read_file_meta(dcm_file_t *file);

/**
 * Read metadata from a File.
 *
 * :param file: File
 *
 * :return: metadata
 */
extern dcm_dataset_t *dcm_file_read_metadata(dcm_file_t *file);

/**
 * Read Basic Offset Table from a File.
 *
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
extern dcm_bot_t *dcm_file_read_bot(dcm_file_t *file, dcm_dataset_t *metadata);

/**
 * Build Basic Offset Table for a File.
 *
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
extern dcm_bot_t *dcm_file_build_bot(dcm_file_t *file, dcm_dataset_t *metadata);

/**
 * Read an individual Frame from a File.
 *
 * :param file: File
 * :param metadata: Metadata
 * :param bot: Basic Offset Table
 * :param index: Zero-based offset of the Frame in the Pixel Data Element
 *
 * :return: Frame
 */
extern dcm_frame_t *dcm_file_read_frame(dcm_file_t *file,
                                        dcm_dataset_t *metadata,
                                        dcm_bot_t *bot,
                                        uint32_t index);

/**
 * Destroy a File.
 *
 * :param file: File
 */
extern void dcm_file_destroy(dcm_file_t *file);

#endif
