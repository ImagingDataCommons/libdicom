#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "version.h"

#ifndef DCM_INCLUDED
#define DCM_INCLUDED

#if defined(_WIN32) && !defined(__GNUC__)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#ifdef _WIN32
#if DCM_STATIC
#define DCM_EXTERN extern
#elif defined(BUILDING_LIBDICOM)
#define DCM_EXTERN __declspec(dllexport) extern
#else
#define DCM_EXTERN __declspec(dllimport) extern
#endif
#else
#define DCM_EXTERN __attribute__((visibility("default"))) extern
#endif

#ifdef HAS_CONSTRUCTOR
#define DCM_CONSTRUCTOR __attribute__ ((constructor))
#else
#define DCM_CONSTRUCTOR __attribute__ ((constructor))
#endif

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
 * Maximum number of characters in values with Value Representation IS.
 */
#define DCM_CAPACITY_IS 12

/**
 * Maximum number of characters in values with Value Representation LO.
 */
#define DCM_CAPACITY_LO 64

/**
 * Maximum number of characters in values with Value Representation LT.
 */
#define DCM_CAPACITY_LT 10240

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
 * Maximum number of characters in values with Value Representation UR.
 */
#define DCM_CAPACITY_UR 4294967294

/**
 * Maximum number of characters in values with Value Representation UT.
 */
#define DCM_CAPACITY_UT 4294967294

/**
 * Part10 file
 */
typedef struct _DcmFilehandle DcmFilehandle;

/**
 * Data Element
 */
typedef struct _DcmElement DcmElement;

/**
 * Data Set
 */
typedef struct _DcmDataSet DcmDataSet;

/**
 * Sequence of Data Set Items
 */
typedef struct _DcmSequence DcmSequence;

/**
 * Frame Item of Pixel Data Element
 */
typedef struct _DcmFrame DcmFrame;

/**
 * Basic Offset Table (BOT) Item of Pixel Data Element
 */
typedef struct _DcmBOT DcmBOT;

/**
 * Start up libdicom.
 *
 * Call this from the main thread during program startup for libdicom to be
 * threadsafe.
 *
 * If you don't do this, libdicom will attempt to call it for you in a safe
 * way, but cannot guarantee this on all platforms and with all compilers, and
 * therefore cannot guarantee thread safety.
 *
 * This function can be called many times.
 */
DCM_EXTERN
DCM_CONSTRUCTOR
void dcm_init(void);

/**
 * Enumeration of error codes.
 */
enum _DcmErrorCode {
    /** Out of memory */
    DCM_ERROR_CODE_NOMEM = 1,
    /** Invalid parameter */
    DCM_ERROR_CODE_INVALID = 2,
    /** Parse error */
    DCM_ERROR_CODE_PARSE = 3,
    /** IO error */
    DCM_ERROR_CODE_IO = 4,
};

/**
 * Error codes
 */
typedef enum _DcmErrorCode DcmErrorCode;

/**
 * An enum of Value Representations. 
 *
 * Value Representations which are not known to libdicom will be coded as
 * DCM_VR_ERROR (unknown Value Representation).
 *
 * Note to maintainers: this enum must match the table in dicom-dict.c, and
 * the DcmVRTag enum. As the DICOM standard evolves, numbering must be
 * maintained for ABI compatibility.
 */
typedef enum _DcmVR {
    // error value, returned for eg. unknown SR strings
    DCM_VR_ERROR = -1,

    // allowed VRs for DcmElement
    DCM_VR_AE = 0,
    DCM_VR_AS,
    DCM_VR_AT,
    DCM_VR_CS,
    DCM_VR_DA,
    DCM_VR_DS,
    DCM_VR_DT,
    DCM_VR_FL,
    DCM_VR_FD,
    DCM_VR_IS,
    DCM_VR_LO,
    DCM_VR_LT,
    DCM_VR_OB,
    DCM_VR_OD,
    DCM_VR_OF,
    DCM_VR_OW,
    DCM_VR_PN,
    DCM_VR_SH,
    DCM_VR_SL,
    DCM_VR_SQ,
    DCM_VR_SS,
    DCM_VR_ST,
    DCM_VR_TM,
    DCM_VR_UI,
    DCM_VR_UL,
    DCM_VR_UN,
    DCM_VR_US,
    DCM_VR_UT,
    DCM_VR_UR,
    DCM_VR_UC,
    DCM_VR_OL,
    DCM_VR_OV,
    DCM_VR_SV,
    DCM_VR_UV,

    // used to check enums for range errors, add new VRs before this
    DCM_VR_LAST
} DcmVR;

/**
 * Convert an error code to a human-readable string.
 *
 * :param code: The error code
 *
 * :return: A string that can be displayed to users
 */
DCM_EXTERN
const char *dcm_error_code_str(DcmErrorCode code);

/**
 * Get a symbolic name for a DcmErrorCode.
 *
 * :param code: The error code
 *
 * :return: A symbolic name for the code.
 */
DCM_EXTERN
const char *dcm_error_code_name(DcmErrorCode code);

/**
 * Error return object.
 */
typedef struct _DcmError DcmError;

/**
 * Set an error.
 *
 * Create a new DcmError object and store the pointer in error.
 *
 * You can't set error twice -- always check the error state and return
 * immediately if set.
 *
 * :param error: Pointer to store the new error object in
 * :param code: Numeric error code
 * :param summary: Summary of error
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_error_set(DcmError **error, DcmErrorCode code,
    const char *summary, const char *format, ...);

/**
 * Clear an error, if set.
 *
 * :param error: Pointer holding the error object
 */
DCM_EXTERN
void dcm_error_clear(DcmError **error);

/**
* Get a summary of the error.
 *
 * Do not free this result. The pointer will be valid as long as error is
 * valid.
 *
 * :param error: DcmError to read the error from
 *
 * :return: Short description of the error
 */
DCM_EXTERN
const char *dcm_error_get_summary(DcmError *error);

/**
 * Get the error message.
 *
 * Do not free this result. The pointer will be valid as long as error is
 * valid.
 *
 * :param error: Error object
 *
 * :return: Message stored in a error object
 */
DCM_EXTERN
const char *dcm_error_get_message(DcmError *error);

/**
 * Get the error code.
 *
 * :param error: Error object
 *
 * :return: Error code
 */
DCM_EXTERN
DcmErrorCode dcm_error_get_code(DcmError *error);

/**
 * Log an error message using information stored on the error object.
 *
 * :param error: Error object
 */
DCM_EXTERN
void dcm_error_log(DcmError *error);

/**
 * Enumeration of log levels
 */
enum _DcmLogLevel {
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
typedef enum _DcmLogLevel DcmLogLevel;

/**
 * Global variable to set log level.
 */
DCM_EXTERN
DcmLogLevel dcm_log_level;

/**
 * Write critical log message to stderr stream.
 *
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_log_critical(const char *format, ...);

/**
 * Write error log message to stderr stream.
 *
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_log_error(const char *format, ...);

/**
 * Write warning log message to stderr stream.
 *
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_log_warning(const char *format, ...);

/**
 * Write info log message to stderr stream.
 *
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_log_info(const char *format, ...);

/**
 * Write debug log message to stderr stream.
 *
 * :param format: printf-style format string
 * :param ...: Format string arguments
 */
DCM_EXTERN
void dcm_log_debug(const char *format, ...);

/**
 * Get the version of the library.
 *
 * :return: semantic version string
 */
DCM_EXTERN
const char *dcm_get_version(void);

/**
 * Turn a string Value Representation into an enum value.
 *
 * :param vr: The Value Representation as a two character string.
 *
 * :return: the enum for that Value Representation
 */
DCM_EXTERN
DcmVR dcm_dict_vr_from_str(const char *vr);

/**
 * Turn an enum Value Representation into a character string.
 *
 * :param vr: The Value Representation as an enum value.
 *
 * :return: the string representation of that Value Representation, or NULL
 */
DCM_EXTERN
const char *dcm_dict_str_from_vr(DcmVR vr);

/**
 * Look up the Keyword of an Attribute in the Dictionary.
 * 
 * Returns NULL if the tag is not recognised.
 *
 * :param tag: Attribute Tag
 *
 * :return: attribute Keyword
 */
DCM_EXTERN
const char *dcm_dict_keyword_from_tag(uint32_t tag);

/**
 * Look up the tag of an Attribute in the Dictionary.
 *
 * Returns ``0xffffffff`` if the keyword is not recognised.
 *
 * :param tag: Attribute keyword
 *
 * :return: attribute tag
 */
DCM_EXTERN
uint32_t dcm_dict_tag_from_keyword(const char *keyword);

/**
 * Find the Value Representation for a tag. 
 *
 * This will return DCM_VR_ERROR if the tag is unknown, or does not have a 
 * unique Value Representation.
 *
 * :param tag: Attribute Tag
 *
 * :return: the unique Value Representation for this tag, or DCM_VR_ERROR
 */
DCM_EXTERN
DcmVR dcm_vr_from_tag(uint32_t tag);

/**
 * Determine whether a Tag is public.
 *
 * A Tag is public if it is defined in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_public_tag(uint32_t tag);

/**
 * Determine whether a Tag is private.
 *
 * :param tag: Attribute Tag
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_private_tag(uint32_t tag);

/**
 * Determine whether a Tag is valid.
 *
 * :param tag: Attribute Tag
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_valid_tag(uint32_t tag);

/**
 * Determine whether a Value Representation is valid.
 *
 * :param vr: Attribute Value Representation
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_valid_vr(const char *vr);

/**
 * Determine whether a Value Representation is valid.
 *
 * :param vr: Attribute Value Representation
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_valid_vr_for_tag(DcmVR vr, uint32_t tag);

/**
 * Determine whether a Transfer Syntax is encapsulated.
 *
 * :param transfer_syntax_uid: Transfer Syntax UID
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid);


/**
 * Data Element
 */

/**
 * Create a Data Element for a tag.
 *
 * After creating a Data Element, you must
 * attach an appropriate value using one of the setting functions.
 * See for example :c:func:`dcm_element_set_value_string`.
 *
 * :param error: Pointer to error object
 * :param tag: Tag
 * :param vr: The Value Representation for this Data Element
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create(DcmError **error, uint32_t tag, DcmVR vr);

/**
 * Get group number (first part of Tag) of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag group number
 */
DCM_EXTERN
uint16_t dcm_element_get_group_number(const DcmElement *element);

/**
 * Get Element Number (second part of Tag) of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag Element Number
 */
DCM_EXTERN
uint16_t dcm_element_get_element_number(const DcmElement *element);

/**
 * Get Tag of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag
 */
DCM_EXTERN
uint32_t dcm_element_get_tag(const DcmElement *element);

/**
 * Get the Value Representation of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Value Representation
 */
DCM_EXTERN
DcmVR dcm_element_get_vr(const DcmElement *element);

/**
 * Get length of the entire value of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Length of value of Data Element
 */
DCM_EXTERN
uint32_t dcm_element_get_length(const DcmElement *element);

/**
 * Get Value Multiplicity of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Value Multiplicity
 */
DCM_EXTERN
uint32_t dcm_element_get_vm(const DcmElement *element);

/**
 * Determine whether a Data Element has a Value Multiplicity greater than one.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_element_is_multivalued(const DcmElement *element);

/**
 * Clone (i.e., create a deep copy of) a Data Element.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to clone of Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_clone(DcmError **error, const DcmElement *element);

/**
 * Get a string from a string-valued Data Element.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to return location for value 
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_get_value_string(DcmError **error,
                                  const DcmElement *element,
                                  uint32_t index,
                                  const char **value);

/**
 * Set the value of a Data Element to a character string.
 *
 * The Data Element must have a Tag that allows for a
 * character string Value Representation.
 * If that is not the case, the function will fail.
 *
 * On success, if `steal` is true, ownership of `value` passes to
 * `element`, i.e. it will be freed when `element` is destroyed. If `steal` is
 * false, then a copy is made of `value` and ownership is not transferred.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: String value
 * :param steal: if true, ownership of value passes to element
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_string(DcmError **error,
                                  DcmElement *element,
                                  char *value,
                                  bool steal);

/**
* Set the value of a Data Element to an array of character strings.
 *
 * The Data Element must have a Tag that allows for a
 * character string Value Representation and for a
 * Value Multiplicity greater than one.
 * If that is not the case, the function will fail.
 *
 * On success, if `steal` is true, ownership of `value` passes to
 * `element`, i.e. it will be freed when `element` is destroyed. If `steal` is
 * false, then a copy is made of `value` and ownership is not transferred.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param values: Pointer to memory location where values are written to
 * :param vm: Number of values
 * :param steal: if true, ownership of values passes to element
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_string_multi(DcmError **error,
                                        DcmElement *element,
                                        char **values,
                                        uint32_t vm,
                                        bool steal);

/**
 * Get an integer from a 16, 32 or 64-bit integer-valued Data Element.
 *
 * The integer held in the Element will be cast to int64_t for return.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to return location for value 
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_get_value_integer(DcmError **error,
                                   const DcmElement *element,
                                   uint32_t index,
                                   int64_t *value);

/**
 * Set the value of a Data Element to an integer.
 * 
 * The Data Element must have a Tag that allows for a
 * integer Value Representation.
 * If that is not the case, the function will fail.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Integer value
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_integer(DcmError **error,
                                   DcmElement *element,
                                   int64_t value);

/**
 * Set the value of a Data Element to a number.
 * 
 * The Data Element must have a Tag that allows for a
 * numeric Value Representation.
 * If that is not the case, the function will fail.
 *
 * Although the value passed is `void*`, it should
 * be a pointer to an array of 16- to 64-bit numeric values of the
 * appropriate type for the Data Element Value Representation.
 *
 * On success, if `steal` is true, ownership of `values` passes to
 * `element`, i.e. it will be freed when `element` is destroyed. If `steal` is
 * false, then a copy is made of `values` and ownership is not transferred.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param values: Array of values
 * :param vm: Number of values
 * :param steal: if true, ownership of values passes to element
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_numeric_multi(DcmError **error,
                                         DcmElement *element,
                                         void *values,
                                         uint32_t vm,
                                         bool steal);

/**
 * Get a floating-point value from a Data Element.
 *
 * The Data Element Value Reepresentation may be either single- or 
 * double-precision floating point.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 * :param value: Pointer to return location for value 
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_get_value_floatingpoint(DcmError **error,
                                         const DcmElement *element,
                                         uint32_t index,
                                         double *value);

/**
 * Set the value of a Data Element to a floating-point.
 * 
 * The Data Element must have a Tag that allows for a
 * floating-point Value Representation.
 * If that is not the case, the function will fail.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Floating point value
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_floatingpoint(DcmError **error,
                                         DcmElement *element,
                                         double value);

/**
 * Get a binary value from a Data Element.
 *
 * Use :c:func:`dcm_element_length` to get the length of the binary value.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Pointer to return location for value 
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_get_value_binary(DcmError **error,
                                  const DcmElement *element,
                                  const char **value);

/**
 * Set the value of a Data Element to binary data.
 * 
 * The Data Element must have a Tag that allows for a
 * binary Value Representation.
 * If that is not the case, the function will fail.
 *
 * On success, if `steal` is true, ownership of `value` passes to
 * `element`, i.e. it will be freed when `element` is destroyed. If `steal` is
 * false, then a copy is made of `value` and ownership is not transferred.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Pointer to binary value
 * :param length: Length in bytes of the binary value
 * :param steal: if true, ownership of the value passes to element
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_binary(DcmError **error,
                                  DcmElement *element,
                                  char *value,
                                  uint32_t length,
                                  bool steal);

/**
 * Get a sequence value from a Data Element.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Pointer to return location for value 
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_get_value_sequence(DcmError **error,
                                    const DcmElement *element,
                                    DcmSequence **value);


/**
 * Set the value of a Data Element to a Sequence.
 * 
 * The Data Element must have a Tag that allows for
 * Value Representation ``"SQ"``.
 * If that is not the case, the function will fail.
 *
 * The Data Element takes ownership of the value pointer on success.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Pointer to Sequence
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_element_set_value_sequence(DcmError **error,
                                    DcmElement *element,
                                    DcmSequence *value);

/**
 * Print a Data Element.
 *
 * :param element: Pointer to Data Element
 * :param indentation: Number of white spaces before text
 */
DCM_EXTERN
void dcm_element_print(const DcmElement *element, int indentation);

/**
 * Destroy a Data Element.
 *
 * :param element: Pointer to Data Element
 */
DCM_EXTERN
void dcm_element_destroy(DcmElement *element);


/**
 * Data Set
 */

/**
 * Create an empty Data Set.
 *
 * :param error: Pointer to error object
 */
DCM_EXTERN
DcmDataSet *dcm_dataset_create(DcmError **error);

/**
 * Clone (i.e., create a deep copy of) a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 *
 * :return: Pointer to clone of Data Set
 */
DCM_EXTERN
DcmDataSet *dcm_dataset_clone(DcmError **error, const DcmDataSet *dataset);

/**
 * Insert a Data Element into a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 * :param element: Pointer to Data Element
 *
 * The object takes over ownership of the memory referenced by `element`
 * and frees it when the object is destroyed or if the insert operation fails.
 *
 * :return: Whether insert operation was successful
 */
DCM_EXTERN
bool dcm_dataset_insert(DcmError **error,
                        DcmDataSet *dataset, DcmElement *element);

/**
 * Remove a Data Element from a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Whether remove operation was successful
 */
DCM_EXTERN
bool dcm_dataset_remove(DcmError **error, DcmDataSet *dataset, uint32_t tag);

/**
 * Get a Data Element from a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_dataset_get(DcmError **error,
                            const DcmDataSet *dataset, uint32_t tag);

/**
 * Get a clone (deep copy) of a Data Element from a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Pointer to clone of Data Element
 */
DCM_EXTERN
DcmElement *dcm_dataset_get_clone(DcmError **error,
                                  const DcmDataSet *dataset, uint32_t tag);

/**
 * Iterate over Data Elements in a Data Set.
 *
 * Does not sort Data Elements, but iterates over them in the order in which
 * they were originally inserted into the Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param fn: Pointer to function that should be called for each Data Element
 */
DCM_EXTERN
void dcm_dataset_foreach(const DcmDataSet *dataset,
                         void (*fn)(const DcmElement *element));

/**
 * Fetch a Data Element from a Data Set, or NULL if not present.
 *
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Data Element, or NULL if not present
 */
DCM_EXTERN
DcmElement *dcm_dataset_contains(const DcmDataSet *dataset, uint32_t tag);

/**
 * Count the number of Data Elements in a Data Set.
 *
 * :param dataset: Pointer to Data Set
 *
 * :return: Number of Data Elements
 */
DCM_EXTERN
uint32_t dcm_dataset_count(const DcmDataSet *dataset);

/**
 * Obtain a copy of the Tag of each Data Element in a Data Set.
 *
 * The tags will be sorted in ascending order.
 *
 * :param dataset: Pointer to Data Set
 * :param tags: Pointer to memory location to of the array into which to copy
 *              tags. Number of items in the array must match the number of
 *              Data Elements in the Data Set as determined by
 *              :c:func:`dcm_dataset_count`.
 * :param n: Number of items in the array.
 *
 * Ownership of the memory allocated for `tags` remains with the caller.
 * Specifically, the function does not free the memory allocated for `tags` if
 * the copy operation fails.
 */
DCM_EXTERN
void dcm_dataset_copy_tags(const DcmDataSet *dataset, uint32_t *tags,
                           uint32_t n);

/**
 * Lock a Data Set to prevent modification.
 *
 * :param dataset: Pointer to Data Set
 */
DCM_EXTERN
void dcm_dataset_lock(DcmDataSet *dataset);

/**
 * Check whether a Data Set is locked.
 *
 * :param dataset: Pointer to Data Set
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_dataset_is_locked(const DcmDataSet *dataset);

/**
 * Print a Data Set.
 *
 * :param error: Pointer to error object
 * :param dataset: Pointer to Data Set
 * :param indentation: Number of white spaces before text
 */
DCM_EXTERN
void dcm_dataset_print(const DcmDataSet *dataset, int indentation);

/**
 * Destroy a Data Set.
 *
 * :param dataset: Pointer to Data Set
 */
DCM_EXTERN
void dcm_dataset_destroy(DcmDataSet *dataset);


/**
 * Sequence
 */

/**
 * Create a Sequence, i.e., a collection of Data Set items that represent the
 * value of a Data Element with Value Representation SQ (Sequence).
 *
 * Note that created object represents the value of a Data Element rather
 * than a Data Element itself.
 *
 * :param error: Pointer to error object
 * :return: Pointer to Sequence
 */
DCM_EXTERN
DcmSequence *dcm_sequence_create(DcmError **error);

/**
 * Append a Data Set item to a Sequence.
 *
 * :param error: Pointer to error object
 * :param seq: Pointer to Sequence
 * :param item: Data Set item
 *
 * The object takes over ownership of the memory referenced by `item`
 * and frees it when the object is destroyed or if the append operation fails.
 *
 * :return: Whether append operation was successful
 */
DCM_EXTERN
bool dcm_sequence_append(DcmError **error,
                         DcmSequence *seq, DcmDataSet *item);

/**
 * Get a Data Set item from a Sequence.
 *
 * :param error: Pointer to error object
 * :param seq: Pointer to Sequence
 * :param index: Zero-based index of the Data Set item in the Sequence
 *
 * :return: Pointer to Data Set item
 */
DCM_EXTERN
DcmDataSet *dcm_sequence_get(DcmError **error,
                             const DcmSequence *seq, uint32_t index);

/**
 * Iterate over Data Set items in a Sequence.
 *
 * :param seq: Pointer to Sequence
 * :param fn: Pointer to function that should be called for each Data Set item
 */
DCM_EXTERN
void dcm_sequence_foreach(const DcmSequence *seq,
                          void (*fn)(const DcmDataSet *item));

/**
 * Remove a Data Set item from a Sequence.
 *
 * :param error: Pointer to error object
 * :param seq: Pointer to Sequence
 * :param index: Zero-based index of the Data Set item in the Sequence
 *
 * :return: Whether remove operation was successful
 */
DCM_EXTERN
bool dcm_sequence_remove(DcmError **error, DcmSequence *seq, uint32_t index);

/**
 * Count the number of Data Set items in a Sequence.
 *
 * :param seq: Pointer to Sequence
 *
 * :return: number of Data Set items
 */
DCM_EXTERN
uint32_t dcm_sequence_count(const DcmSequence *seq);

/**
 * Lock a Sequence to prevent modification.
 *
 * :param seq: Pointer to Sequence
 */
DCM_EXTERN
void dcm_sequence_lock(DcmSequence *seq);

/**
 * Check whether a Sequence is locked.
 *
 * :param seq: Pointer to Sequence
 *
 * :return: Yes/no answer
 */
DCM_EXTERN
bool dcm_sequence_is_locked(const DcmSequence *seq);

/**
 * Destroy a Sequence.
 *
 * :param seq: Pointer to Sequence
 */
DCM_EXTERN
void dcm_sequence_destroy(DcmSequence *seq);


/**
 * Frame
 *
 * Encoded pixels of an individual pixel matrix and associated
 * descriptive metadata.
 */

/**
 * Create a Frame.
 *
 * :param error: Pointer to error object
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
DCM_EXTERN
DcmFrame *dcm_frame_create(DcmError **error,
                           uint32_t number,
                           const char *data,
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
DCM_EXTERN
uint32_t dcm_frame_get_number(const DcmFrame *frame);

/**
 * Get length of a Frame Item.
 *
 * :param frame: Frame
 *
 * :return: number of bytes
 */
DCM_EXTERN
uint32_t dcm_frame_get_length(const DcmFrame *frame);

/**
 * Get Rows of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of rows in pixel matrix
 */
DCM_EXTERN
uint16_t dcm_frame_get_rows(const DcmFrame *frame);

/**
 * Get Columns of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of columns in pixel matrix
 */
DCM_EXTERN
uint16_t dcm_frame_get_columns(const DcmFrame *frame);

/**
 * Get Samples per Pixel of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of samples (color channels) per pixel
 */
DCM_EXTERN
uint16_t dcm_frame_get_samples_per_pixel(const DcmFrame *frame);

/**
 * Get Bits Allocated of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits allocated per pixel
 */
DCM_EXTERN
uint16_t dcm_frame_get_bits_allocated(const DcmFrame *frame);

/**
 * Get Bits Stored of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits stored per pixel
 */
DCM_EXTERN
uint16_t dcm_frame_get_bits_stored(const DcmFrame *frame);

/**
 * Get High Bit of a Frame.
 *
 * :param frame: Frame
 *
 * :return: most significant bit of pixels
 */
DCM_EXTERN
uint16_t dcm_frame_get_high_bit(const DcmFrame *frame);

/**
 * Get Pixel Representation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: representation of pixels (unsigned integers or 2's complement)
 */
DCM_EXTERN
uint16_t dcm_frame_get_pixel_representation(const DcmFrame *frame);

/**
 * Get Planar Configuration of a Frame.
 *
 * :param frame: Frame
 *
 * :return: configuration of samples (color-by-plane or color-by-pixel)
 */
DCM_EXTERN
uint16_t dcm_frame_get_planar_configuration(const DcmFrame *frame);

/**
 * Get Photometric Interpretation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: interpretation of pixels (monochrome, RGB, etc.)
 */
DCM_EXTERN
const char *dcm_frame_get_photometric_interpretation(const DcmFrame *frame);

/**
 * Get Transfer Syntax UID for a Frame.
 *
 * :param frame: Frame
 *
 * :return: UID of the transfer syntax in which frame is encoded
 */
DCM_EXTERN
const char *dcm_frame_get_transfer_syntax_uid(const DcmFrame *frame);

/**
 * Get pixel data of a Frame.
 *
 * :param frame: Frame
 *
 * :return: pixel data
 */
DCM_EXTERN
const char *dcm_frame_get_value(const DcmFrame *frame);

/**
 * Destroy a Frame.
 *
 * :param frame: Frame
 */
DCM_EXTERN
void dcm_frame_destroy(DcmFrame *frame);


/**
 * Basic Offset Table (BOT).
 */

/**
 * Create a Basic Offset Table.
 *
 * :param error: Pointer to error object
 * :param offsets: Offset of each Frame in the Pixel Data Element
 *                 (measured from the first byte of the first Frame).
 * :param num_frames: Number of Frames in the Pixel Data Element
 * :first_frame_offset: Offset from pixel_data_offset to the first byte of the
 *                      first frame
 *
 * The created object takes over ownership of the memory referenced by `offsets`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_bot_create(DcmError **error,
                       ssize_t *offsets,
		       uint32_t num_frames,
                       ssize_t first_frame_offset);

/**
 * Get number of Frame offsets in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 *
 * :return: number of frames
 */
DCM_EXTERN
uint32_t dcm_bot_get_num_frames(const DcmBOT *bot);

/**
 * Get Frame offset in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 * :param index: One-based index of Frame in the Pixel Data Element
 *
 * :return: offset from pixel_data_offset
 */
DCM_EXTERN
ssize_t dcm_bot_get_frame_offset(const DcmBOT *bot, uint32_t index);

/**
 * Print a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
DCM_EXTERN
void dcm_bot_print(const DcmBOT *bot);

/**
 * Destroy a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
DCM_EXTERN
void dcm_bot_destroy(DcmBOT *bot);


/**
 * Part 10 File
 */

/**
 * A set of IO functions, see dcm_filehandle_create().
 */
typedef struct _DcmIO {
    /** Open an IO object */
    void *(*open)(DcmError **error, void *client);
    /** Close an IO object */
    int (*close)(DcmError **error, void *data);
    /** Read from an IO object, semantics as POSIX read() */
    int64_t (*read)(DcmError **error, void *data, char *buffer, int64_t length);
    /** Seek an IO object, semantics as POSIX seek() */
    int64_t (*seek)(DcmError **error, void *data, int64_t offset, int whence);
} DcmIO;

/**
 * Create a filehandle that reads using a set of DcmIO functions.
 *
 * :param error: Pointer to error object
 * :param io: Set of read functions for this DcmFilehandle
 * :param client: Client data for read functions
 *
 * :return: filehandle
 */
DCM_EXTERN
DcmFilehandle *dcm_filehandle_create(DcmError **error,
				     const DcmIO *io,
				     void *client);

/**
 * Open a file on disk as a DcmFilehandle.
 *
 * :param error: Pointer to error object
 * :param filepath: Path to the file on disk
 *
 * :return: filehandle
 */
DCM_EXTERN
DcmFilehandle *dcm_filehandle_create_from_file(DcmError **error,
                                               const char *filepath);

/**
 * Open an area of memory as a DcmFilehandle.
 *
 * :param error: Pointer to error object
 * :param buffer: Pointer to memory area
 * :param length: Length of memory area in bytes
 *
 * :return: filehandle
 */
DCM_EXTERN
DcmFilehandle *dcm_filehandle_create_from_memory(DcmError **error,
                                                 const char *buffer,
                                                 int64_t length);

/**
 * Read File Meta Information from a File.
 *
 * Keeps track of the offset of the Data Set relative to the beginning of the
 * filehandle to speed up subsequent access, and determines the Transfer
 * Syntax in which the contained Data Set is encoded.
 *
 * :param error: Pointer to error object
 * :param filehandle: Pointer to file handle
 *
 * :return: File Meta Information
 */
DCM_EXTERN
DcmDataSet *dcm_filehandle_read_file_meta(DcmError **error,
                                          DcmFilehandle *filehandle);

/**
 * Read metadata from a File.
 *
 * Keeps track of the offset of the Pixel Data Element relative to the
 * beginning of the filehandle to speed up subsequent access to individual
 * Frame items.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 *
 * :return: metadata
 */
DCM_EXTERN
DcmDataSet *dcm_filehandle_read_metadata(DcmError **error,
                                         DcmFilehandle *filehandle);

/**
 * Read Basic Offset Table from a File.
 *
 * In case the Pixel Data Element does not contain a Basic Offset Table item,
 * but contains an Extended Offset Table element, the value of the Extended
 * Offset Table element will be read instead.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_filehandle_read_bot(DcmError **error, DcmFilehandle *filehandle,
                                DcmDataSet *metadata);

/**
 * Build Basic Offset Table for a File.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_filehandle_build_bot(DcmError **error, DcmFilehandle *filehandle,
                                 DcmDataSet *metadata);

/**
 * Read an individual Frame from a File.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param metadata: Metadata
 * :param bot: Basic Offset Table
 * :param index: One-based offset of the Frame in the Pixel Data Element
 *
 * :return: Frame
 */
DCM_EXTERN
DcmFrame *dcm_filehandle_read_frame(DcmError **error,
                                    DcmFilehandle *filehandle,
                                    DcmDataSet *metadata,
                                    DcmBOT *bot,
                                    uint32_t index);

/**
 * Destroy a File.
 *
 * :param filehandle: File
 */
DCM_EXTERN
void dcm_filehandle_destroy(DcmFilehandle *filehandle);

#endif
