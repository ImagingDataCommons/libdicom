#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "version.h"

#ifndef DCM_INCLUDED
#define DCM_INCLUDED

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
 * Maximum number of characters in values with Value Representation UC.
 */
#define DCM_CAPACITY_UC 4294967294

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

/* We need forward references for these types.
 */
typedef struct _DcmSequence DcmSequence;

/**
 * Start up libdicom.
 *
 * Call this from the main thread during program startup.
 *
 * This function can be called many times.
 *
 * .. deprecated:: 1.1.0
 *    Calling this function is no longer necessary.
 */
#ifndef BUILDING_LIBDICOM
#if defined(_MSC_VER)
__declspec(deprecated("dcm_init() no longer needs to be called"))
#elif defined(__GNUC__)
__attribute__((deprecated("dcm_init() no longer needs to be called")))
#endif
#endif
DCM_EXTERN
void dcm_init(void);

/* Our copy of getopt, since non-glibc platforms are missing this.
 * Used by our tools.
 */
DCM_EXTERN
char *dcm_optarg;
DCM_EXTERN
int dcm_optind, dcm_opterr, dcm_optopt, dcm_optreset;
DCM_EXTERN
int dcm_getopt(int nargc, char * const nargv[], const char *ostr);

/**
 * Error return object.
 */
typedef struct _DcmError DcmError;

/**
 * Enumeration of error codes.
 */
typedef enum _DcmErrorCode {
    /** Out of memory */
    DCM_ERROR_CODE_NOMEM = 1,
    /** Invalid parameter */
    DCM_ERROR_CODE_INVALID = 2,
    /** Parse error */
    DCM_ERROR_CODE_PARSE = 3,
    /** IO error */
    DCM_ERROR_CODE_IO = 4,
    /** Missing frame */
    DCM_ERROR_CODE_MISSING_FRAME = 5,
} DcmErrorCode;

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
 * Print an error message to stderr.
 *
 * :param error: Error object
 */
DCM_EXTERN
void dcm_error_print(DcmError *error);


/**
 * Free an allocated memory area.
 *
 * Any memory allocated by libdicom and returned to the calling program
 * should be freed with this.
 *
 * :param pointer: Memory area to free
 */
DCM_EXTERN
void dcm_free(void *pointer);

/**
 * Allocate and zero an area of memory.
 *
 * Any memory which you pass to libdicom and which you ask libdicom to manage
 * with a "steal" flag should be allcoatyed with one of the libdicom memory
 * allocators.
 *
 * :param error: Pointer to error object
 * :param n: Number of items to allocate
 * :param size: Size of each item in bytes
 *
 * :return: Pointer to memory area
 */
DCM_EXTERN
void *dcm_calloc(DcmError **error, uint64_t n, uint64_t size);


/**
 * Enumeration of log levels
 */
typedef enum _DcmLogLevel {
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
} DcmLogLevel;

/**
 * Set the log level.
 *
 * :param log_level: New log level.
 * :return: previous log level
 */
DCM_EXTERN
DcmLogLevel dcm_log_set_level(DcmLogLevel log_level);

/**
 * Log function. See dcm_log_set_logf().
 */
typedef void (*DcmLogf)(const char *level, const char *format, va_list args);

/**
 * Set the log function.
 *
 * This function will be used to log any error or warning messages from the
 * library. The default DcmLogf function prints messages to stderr. Set to
 * NULL to disable all logging.
 *
 * :param logf: New log function.
 * :return: previous log function
 */
DCM_EXTERN
DcmLogf dcm_log_set_logf(DcmLogf logf);

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
 * The general class of the value associated with a Value Representation.
 *
 * DCM_VR_CLASS_STRING_MULTI -- one or more null-terminated strings, cannot
 * contain backslash
 *
 * DCM_VR_CLASS_STRING_SINGLE -- a single null-terminated string, backslash
 * allowed
 *
 * DCM_VR_CLASS_NUMERIC_DECIMAL -- one or more binary floating point numeric
 * values, other fields give sizeof(type)
 *
 * DCM_VR_CLASS_NUMERIC_INTEGER -- one or more binary integer numeric
 * values, other fields give sizeof(type)
 *
 * DCM_VR_CLASS_BINARY -- an uninterpreted array of bytes, length in the
 * element header
 *
 * DCM_VR_CLASS_SEQUENCE -- Value Representation is a sequence
 */
typedef enum _DcmVRClass {
    DCM_VR_CLASS_ERROR,
    DCM_VR_CLASS_STRING_MULTI,
    DCM_VR_CLASS_STRING_SINGLE,
    DCM_VR_CLASS_NUMERIC_DECIMAL,
    DCM_VR_CLASS_NUMERIC_INTEGER,
    DCM_VR_CLASS_BINARY,
    DCM_VR_CLASS_SEQUENCE
} DcmVRClass;

/**
 * Find the general class for a particular Value Representation.
 *
 * :param vr: The Value Representation
 *
 * :return: The general class of that Value Representation
 */
DCM_EXTERN
DcmVRClass dcm_dict_vr_class(DcmVR vr);

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
 * Data Element.
 */
typedef struct _DcmElement DcmElement;

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
bool dcm_element_get_value_decimal(DcmError **error,
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
bool dcm_element_set_value_decimal(DcmError **error,
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
                                  const void **value);

/**
 * Set the value of a Data Element to binary data.
 *
 * The Data Element must have a Tag that allows for a binary Value
 * Representation. If that is not the case, the function will fail.
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
                                  void *value,
                                  uint32_t length,
                                  bool steal);

/* Set a value for an Element from a generic byte buffer. The byte buffer must
 * have been correctly formatted for the VR of this Element.
 *
 * :param error: Pointer to error object
 * :param element: Pointer to Data Element
 * :param value: Pointer to value
 * :param length: Length in bytes of the value
 * :param steal: if true, ownership of the value passes to element
 *
 * :return: true on success
 */
bool dcm_element_set_value(DcmError **error,
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
 * Make a string suitable for display to a user from the value of an element.
 *
 * The return result must be freed with free(). The result may be NULL.
 *
 * :return: string to display
 */
DCM_EXTERN
char *dcm_element_value_to_string(const DcmElement *element);

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
typedef struct _DcmDataSet DcmDataSet;

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
 * On success, the dataset takes over ownership of `element`
 * and frees it when the dataset is destroyed.
 *
 * If the insert operation fails, ownership does not pass and the caller is
 * responsible for freeing `element`.
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
 * The user function should return true to continue looping, or false to
 * terminate the loop early.
 *
 * The result is true if the whole Data Set returned true, or false if one
 * call requested early termination.
 *
 * The function must not modify the Data Set.
 *
 * :param seq: Pointer to Data Set
 * :param fn: Pointer to function that should be called for each Data Element
 * :param client: Client data for function
 *
 * :return: true if all functions return true
 */
DCM_EXTERN
bool dcm_dataset_foreach(const DcmDataSet *dataset,
                         bool (*fn)(const DcmElement *element, void *client),
                         void *client);

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
void dcm_dataset_copy_tags(const DcmDataSet *dataset,
                           uint32_t *tags,
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
 * Create a Sequence, i.e., an ordered list of Data Set items that represent
 * the value of a Data Element with Value Representation SQ (Sequence).
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
 * On success, the sequence takes over ownership of `item`
 * and frees it when the sequence is destroyed.
 *
 * If the append fails, ownership does not pass and the caller is
 * responsible for freeing `item`.
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
 * Iterate over Data Sets in a Sequence.
 *
 * The user function should return true to continue looping, or false to
 * terminate the loop early.
 *
 * The result is true if the whole sequence returned true, or false if one
 * call requested early termination.
 *
 * The function must not modify the seqeucence.
 *
 * :param seq: Pointer to Sequence
 * :param fn: Pointer to function that should be called for each Data Set
 * :param client: Client data for function
 *
 * :return: Pointer to Data Set item
 */
DCM_EXTERN
bool dcm_sequence_foreach(const DcmSequence *seq,
                          bool (*fn)(const DcmDataSet *dataset,
                              uint32_t index, void *client),
                          void *client);

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
 * Frame Item of Pixel Data Element
 *
 * Encoded pixels of an individual pixel matrix and associated
 * descriptive metadata.
 */
typedef struct _DcmFrame DcmFrame;

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
 * Part 10 File
 */
typedef struct _DcmFilehandle DcmFilehandle;

typedef struct _DcmIOMethods DcmIOMethods;

/**
 * An object we can read from.
 */
typedef struct _DcmIO {
        const DcmIOMethods *methods;
        // more private fields follow
} DcmIO;

/**
 * A set of IO methods, see dcm_io_create().
 */
struct _DcmIOMethods {
    /** Open an IO object */
    DcmIO *(*open)(DcmError **error, void *client);

    /** Close an IO object */
    void (*close)(DcmIO *io);

    /** Read from an IO object, semantics as POSIX read() */
    int64_t (*read)(DcmError **error, DcmIO *io, char *buffer, int64_t length);

    /** Seek an IO object, semantics as POSIX seek() */
    int64_t (*seek)(DcmError **error, DcmIO *io, int64_t offset, int whence);
};

/**
 * Create an IO object using a set of IO methods.
 *
 * :param error: Error structure pointer
 * :param io: Set of read methods
 * :param client: Client data for read methods
 *
 * :return: IO object
 */
DCM_EXTERN
DcmIO *dcm_io_create(DcmError **error,
                     const DcmIOMethods *methods,
                     void *client);

/**
 * Open a file on disk for IO.
 *
 * :param error: Error structure pointer
 * :param filename: Path to the file on disk
 *
 * :return: IO object
 */
DCM_EXTERN
DcmIO *dcm_io_create_from_file(DcmError **error, const char *filename);

/**
 * Open an area of memory for IO.
 *
 * :param error: Error structure pointer
 * :param buffer: Pointer to memory area
 * :param length: Length of memory area in bytes
 *
 * :return: IO object
 */
DCM_EXTERN
DcmIO *dcm_io_create_from_memory(DcmError **error, const char *buffer,
                                 int64_t length);

/**
 * Close an IO object.
 *
 * :param io: Pointer to IO object
 */
DCM_EXTERN
void dcm_io_close(DcmIO *io);

/**
 * Read from an IO object.
 *
 * Read up to length bytes from the IO object. Returns the number of bytes
 * read, or -1 for an error. A return of 0 indicates end of file.
 *
 * :param error: Pointer to error object
 * :param io: Pointer to IO object
 * :param buffer: Memory area to read to
 * :param length: Size of memory area
 *
 * :return: Number of bytes read
 */
DCM_EXTERN
int64_t dcm_io_read(DcmError **error, DcmIO *io, char *buffer, int64_t length);

/**
 * Seek an IO object.
 *
 * Set whence to `SEEK_CUR` to seek relative to the current file position,
 * `SEEK_END` to seek relative to the end of the file, or `SEEK_SET` to seek
 * relative to the start.
 *
 * Returns the new absolute read position, or -1 for IO error.
 *
 * :param error: Error structure pointer
 * :param io: Pointer to IO object
 * :param offset: Seek offset
 * :param whence: Seek mode
 *
 * :return: New read position
 */
DCM_EXTERN
int64_t dcm_io_seek(DcmError **error, DcmIO *io, int64_t offset, int whence);

/**
 * Create a representation of a DICOM File using an IO object.
 *
 * The File object tracks information like the transfer syntax and the byte
 * ordering.
 *
 * :param error: Error structure pointer
 * :param io: IO object to read from
 *
 * :return: filehandle
 */
DCM_EXTERN
DcmFilehandle *dcm_filehandle_create(DcmError **error, DcmIO *io);

/**
 * Open a file on disk as a DcmFilehandle.
 *
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
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
 * Destroy a Filehandle.
 *
 * :param filehandle: File
 */
DCM_EXTERN
void dcm_filehandle_destroy(DcmFilehandle *filehandle);

/**
 * Get File Meta Information from a File.
 *
 * Reads the File Meta Information and saves it in the File handle. Returns a
 * reference to this internal copy of the File Meta Information.
 *
 * The return result must not be destroyed. Make a clone of it with
 * :c:func:`dcm_dataset_clone()` if you need it to remain valid after
 * closing the File handle.
 *
 * After calling this function, the filehandle read point is always
 * positioned at the start of the File metadata.
 *
 * It is safe to call this function many times.
 *
 * :param error: Pointer to error object
 * :param filehandle: Pointer to file handle
 *
 * :return: File Meta Information
 */
DCM_EXTERN
const DcmDataSet *dcm_filehandle_get_file_meta(DcmError **error,
                                               DcmFilehandle *filehandle);

/**
 * Get Transfer Syntax UID for a fileahndle.
 *
 * :param filehandle: File
 *
 * :return: UID of the transfer syntax for this File.
 */
DCM_EXTERN
const char *dcm_filehandle_get_transfer_syntax_uid(const DcmFilehandle *filehandle);

/**
 * Read metadata from a File.
 *
 * Read slide metadata, stopping when one of the tags in the stop list is
 * seen. If the stop list pointer is NULL, it will stop on any of the pixel
 * data tags.
 *
 * The return result must be destroyed with :c:func:`dcm_dataset_destroy()`.
 *
 * After calling this function, the filehandle read point is always
 * positioned at the tag that stopped the read. You can call this function
 * again with a different stop set to read more of the metadata.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param stop_tags: NULL, or Zero-terminated array of tags to stop on
 *
 * :return: metadata
 */
DCM_EXTERN
DcmDataSet *dcm_filehandle_read_metadata(DcmError **error,
                                         DcmFilehandle *filehandle,
                                         const uint32_t *stop_tags);

/**
 * Get a fast subset of metadata from a File.
 *
 * Gets a subset of the File's metadata and saves it in the File handle.
 * Returns a reference to this internal copy of the File metadata.
 *
 * The subset is the part of the DICOM metadata that can be read quickly. It
 * is missing tags such as PerFrameFunctionalGroupSequence. Use
 * dcm_filehandle_read_metadata() if you need all file metadata.
 *
 * The return result must not be destroyed. Make a clone of it with
 * :c:func:`dcm_dataset_clone()` if you need it to remain valid after
 * closing the File handle.
 *
 * After calling this function, the filehandle read point is always
 * positioned at the tag that stopped the read.
 *
 * It is safe to call this function many times.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 *
 * :return: metadata
 */
DCM_EXTERN
const DcmDataSet *dcm_filehandle_get_metadata_subset(DcmError **error,
                                                     DcmFilehandle *filehandle);

/**
 * Read everything necessary to fetch frames from the file.
 *
 * Scans the PixelData sequence and loads the PerFrameFunctionalGroupSequence,
 * if present.
 *
 * This function will be called automatically on the first call to
 * :c:func:`dcm_filehandle_read_frame_position()` or
 * :c:func:`dcm_filehandle_read_frame()`. It can take some time to execute,
 * so it is available as a separate function call in case this delay needs
 * to be managed.
 *
 * After calling this function, the filehandle read point is always
 * positioned at the PixelData tag.
 *
 * It is safe to call this function many times.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 *
 * :return: true on success
 */
DCM_EXTERN
bool dcm_filehandle_prepare_read_frame(DcmError **error,
                                       DcmFilehandle *filehandle);

/**
 * Read an individual Frame from a File.
 *
 * Frames are numbered from 1 in the order they appear in the PixelData element.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param frame_number: One-based frame number
 *
 * :return: Frame
 */
DCM_EXTERN
DcmFrame *dcm_filehandle_read_frame(DcmError **error,
                                    DcmFilehandle *filehandle,
                                    uint32_t frame_number);

/**
 * Get the frame number at a position.
 *
 * Given a tile row and column, get the number of the frame that should be
 * displayed at that position, taking into account any frame-positioning
 * metadata.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param column: Column number, from 0
 * :param row: Row number, from 0
 * :param frame_number: Return one-based frame number
 *
 * :return: true on success, false for no frame available
 */
DCM_EXTERN
bool dcm_filehandle_get_frame_number(DcmError **error,
                                     DcmFilehandle *filehandle,
                                     uint32_t column,
                                     uint32_t row,
                                     uint32_t *frame_number);

/**
 * Read the frame at a position in a File.
 *
 * Read a frame from a File at a specified (column, row), numbered from zero.
 * This takes account of any frame positioning given in
 * PerFrameFunctionalGroupSequence.
 *
 * If the frame is missing, perhaps because this is a sparse file, this
 * function returns NULL and sets the error
 * :c:enum:`DCM_ERROR_CODE_MISSING_FRAME`. Applications can detect
 * this and render a background image.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 * :param column: Column number, from 0
 * :param row: Row number, from 0
 *
 * :return: Frame
 */
DCM_EXTERN
DcmFrame *dcm_filehandle_read_frame_position(DcmError **error,
                                             DcmFilehandle *filehandle,
                                             uint32_t column,
                                             uint32_t row);

/**
 * Scan a file and print the entire structure to stdout.
 *
 * :param error: Pointer to error object
 * :param filehandle: File
 *
 * :return: true on successful parse, false otherwise.
 */
DCM_EXTERN
bool dcm_filehandle_print(DcmError **error,
                          DcmFilehandle *filehandle);

#endif
