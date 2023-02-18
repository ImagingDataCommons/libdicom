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
#ifdef BUILDING_LIBDICOM
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
typedef struct _DcmFile DcmFile;

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
 * If you don't do this, libdidom will attempt to call it for you in a safe
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
 * :param format: Format string
 * :param ...: Variable arguments
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
 * Get the summary from a DcmError object.
 *
 * Do not free this result. The pointer will be valid as long as error is
 * valid.
 *
 * :param error: DcmError to read the error from
 *
 * :return: Summary stored in a dcm error object
 */
DCM_EXTERN
const char *dcm_error_summary(DcmError *error);

/**
 * Get the message from a DcmError object.
 *
 * Do not free this result. The pointer will be valid as long as error is
 * valid.
 *
 * :param error: DcmError to read the error from
 *
 * :return: Message stored in a dcm error object
 */
DCM_EXTERN
const char *dcm_error_message(DcmError *error);

/**
 * Get the code from a DcmError object.
 *
 * :param error: DcmError to read the error from
 *
 * :return: Error code
 */
DCM_EXTERN
DcmErrorCode dcm_error_code(DcmError *error);

/**
 * Get a symbolic name for a DcmErrorCode.
 *
 * :param code: DcmError to read the error from
 *
 * :return: Error code
 */
DCM_EXTERN
const char *dcm_error_code_name(DcmErrorCode code);

/**
 * Add an error message to the log for a DcmError.
 *
 * :param error: DcmError to read the error from
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
 * :param format: Format string.
 * :param ...: Variable arguments
 */
DCM_EXTERN
void dcm_log_critical(const char *format, ...);

/**
 * Write error log message to stderr stream.
 *
 * :param format: Format string.
 * :param ...: Variable arguments
 */
DCM_EXTERN
void dcm_log_error(const char *format, ...);

/**
 * Write warning log message to stderr stream.
 *
 * :param format: Format string.
 * :param ...: Variable arguments
 */
DCM_EXTERN
void dcm_log_warning(const char *format, ...);

/**
 * Write info log message to stderr stream.
 *
 * :param format: Format string
 * :param ...: Variable arguments
 */
DCM_EXTERN
void dcm_log_info(const char *format, ...);

/**
 * Write debug log message to stderr stream.
 *
 * :param format: Format string
 * :param ...: Variable arguments
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
 * Look up the Value Representation of an Attribute in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: name of attribute Value Representation
 */
DCM_EXTERN
const char *dcm_dict_lookup_vr(uint32_t tag);

/**
 * Look up the Keyword of an Attribute in the Dictionary.
 *
 * :param tag: Attribute Tag
 *
 * :return: attribute Keyword
 */
DCM_EXTERN
const char *dcm_dict_lookup_keyword(uint32_t tag);

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
 * Create a Data Element with Value Representation AE (Application Entity).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AE(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation AE (Application Entity)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AE_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation AS (Age String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AS(DcmError **error, 
                                  uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation AS (Age String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param error: Error structure pointer
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AS_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation AT (Attribute Tag).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AT(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation AT (Attribute Tag)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_AT_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation CS (Code String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_CS(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation CS (Code String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_CS_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation DA (Date).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DA(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation DA (Date)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DA_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation DS (Decimal String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DS(DcmError **error, 
                                  uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation DS (Decimal String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DS_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation DT (Date Time).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DT(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation DT (Date Time)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_DT_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation FD (Floating Point Double).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Floating-point value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_FD(DcmError **error,
                                  uint32_t tag, 
                                  double value);

/**
 * Create a Data Element with Value Representation FD (Floating Point Double)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of floating-point values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_FD_multi(DcmError **error,
                                        uint32_t tag, 
                                        double *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation FL (Floating Point Single).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Floating-point value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_FL(DcmError **error,
                                  uint32_t tag, 
                                  float value);

/**
 * Create a Data Element with Value Representation FL (Floating Point Single)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of floating-point values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_FL_multi(DcmError **error,
                                        uint32_t tag, 
                                        float *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation IS (Integer String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_IS(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation IS (Integer String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_IS_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation LO (Long String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_LO(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation LO (Long String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_LO_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation PN (Person Name).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_PN(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation PN (Person Name)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_PN_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation SH (Short String).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SH(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation SH (Short String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SH_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation TM (Time).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_TM(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation TM (Time)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_TM_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation UI (Unique Identifier).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UI(DcmError **error,
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation UI (Unique Identifier)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UI_multi(DcmError **error,
                                        uint32_t tag, 
                                        char **values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation SS (Signed Short)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SS(DcmError **error,
                                  uint32_t tag, 
                                  int16_t value);

/**
 * Create a Data Element with Value Representation SS (Signed Short).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SS_multi(DcmError **error,
                                        uint32_t tag, 
                                        int16_t *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation SL (Signed Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SL(DcmError **error,
                                  uint32_t tag, 
                                  int32_t value);

/**
 * Create a Data Element with Value Representation SL (Signed Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SL_multi(DcmError **error,
                                        uint32_t tag, 
                                        int32_t *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation SV (Signed Very Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SV(DcmError **error,
                                  uint32_t tag, 
                                  int64_t value);

/**
 * Create a Data Element with Value Representation SV (Signed Very Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SV_multi(DcmError **error,
                                        uint32_t tag, 
                                        int64_t *values,
                                        uint32_t vm);


/**
 * Create a Data Element with Value Representation UL (Unsigned Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UL(DcmError **error,
                                  uint32_t tag, 
                                  uint32_t value);

/**
 * Create a Data Element with Value Representation UL (Unsigned Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UL_multi(DcmError **error,
                                        uint32_t tag, 
                                        uint32_t *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation US (Unsigned Short).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_US(DcmError **error, 
                                  uint32_t tag, 
                                  uint16_t value);

/**
 * Create a Data Element with Value Representation US (Unsigned Short)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_US_multi(DcmError **error,
                                        uint32_t tag, 
                                        uint16_t *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation UV (Unsigned Very Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UV(DcmError **error,
                                  uint32_t tag, 
                                  uint64_t value);

/**
 * Create a Data Element with Value Representation UV (Unsigned Very Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UV_multi(DcmError **error,
                                        uint32_t tag,
                                        uint64_t *values,
                                        uint32_t vm);

/**
 * Create a Data Element with Value Representation ST (Short Text).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_ST(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation SQ (Sequence).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Sequence value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_SQ(DcmError **error,
                                  uint32_t tag, 
                                  DcmSequence *value);

/**
 * Create a Data Element with Value Representation LT (Long Text).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_LT(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation OB (Other Byte).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OB(DcmError **error, 
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation OD (Other Double).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OD(DcmError **error, 
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation OF (Other Float).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OF(DcmError **error,
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation OL (Other Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OL(DcmError **error,
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation OV (Other Very Long).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OV(DcmError **error, 
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation OW (Other Word).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_OW(DcmError **error,
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation UC (Unlimited Characters).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UC(DcmError **error,
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation UN (Unknown).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UN(DcmError **error,
                                  uint32_t tag, 
                                  char *value, 
                                  uint32_t length);

/**
 * Create a Data Element with Value Representation UR
 * (Universal Resource Identifier).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UR(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

/**
 * Create a Data Element with Value Representation UT (Unlimited Text).
 *
 * :param error: Error structure pointer
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_create_UT(DcmError **error, 
                                  uint32_t tag, 
                                  char *value);

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
 * Get element number (second part of Tag) of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag element number
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
 * Check Value Representation of a Data Element.
 *
 * :param element: Pointer to Data Element
 * :param vr: Value Representation
 *
 * :return: Whether Data Element has the specified Value Representation
 */
DCM_EXTERN
bool dcm_element_check_vr(const DcmElement *element, const char *vr);

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
 * :param error: Error structure pointer
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to clone of Data Element
 */
DCM_EXTERN
DcmElement *dcm_element_clone(DcmError **error, const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation AE
 * (Appllication Entity).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_AE(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation AS (Age String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_AS(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation AT (Attribute Tag).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_AT(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation CS (Code String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_CS(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation DA (Date).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_DA(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation DS (Decimal String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_DS(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation DT (Date Time).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_DT(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation FD
 * (Floating Point Double).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
double dcm_element_get_value_FD(const DcmElement *element,
                                uint32_t index);

/**
 * Get value of a Data Element with Value Representation FL (Floating Point).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
float dcm_element_get_value_FL(const DcmElement *element,
                               uint32_t index);

/**
 * Get value of a Data Element with Value Representation IS (Integer String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_IS(const DcmElement *element,
                                     uint32_t index);

/**
 * Get value of a Data Element with Value Representation LO (Long String).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_LO(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation PN (Person Name).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_PN(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation SH (Short String).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_SH(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation TM (Time).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_TM(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation SL (Signed Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
int32_t dcm_element_get_value_SL(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation SS (Signed Short).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
int16_t dcm_element_get_value_SS(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation SV (Signed Very Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
int64_t dcm_element_get_value_SV(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation ST (Short text).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_ST(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UI (Unique Identifier).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_UI(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation UL (Unsigned Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
uint32_t dcm_element_get_value_UL(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation US (Unsigned Short).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
uint16_t dcm_element_get_value_US(const DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation UV (Unsigned Very Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
DCM_EXTERN
uint64_t dcm_element_get_value_UV(const DcmElement *element, uint32_t index);


/**
 * Get value of a Data Element with Value Representation OB (Other Byte).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OB(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OD (Other Double).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OD(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OF (Other Float).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OF(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OL (Other Long).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OL(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OV (Other Very Long).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OV(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OW (Other Word).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_OW(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UC (Unlimited Characters).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_UC(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UN (Unknown).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_UN(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UR
 * (Universal Resource Identifier).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_UR(const DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UT (Unlimited Text).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
DCM_EXTERN
const char *dcm_element_get_value_UT(const DcmElement *element);


DCM_EXTERN
DcmSequence *dcm_element_get_value_SQ(const DcmElement *element);

/**
 * Print a Data Element.
 *
 * :param element: Pointer to Data Element
 * :param indentation: Number of white spaces before text
 */
DCM_EXTERN
void dcm_element_print(const DcmElement *element, uint8_t indentation);

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
 * :param error: Error structure pointer
 */
DCM_EXTERN
DcmDataSet *dcm_dataset_create(DcmError **error);

/**
 * Clone (i.e., create a deep copy of) a Data Set.
 *
 * :param error: Error structure pointer
 * :param dataset: Pointer to Data Set
 *
 * :return: Pointer to clone of Data Set
 */
DCM_EXTERN
DcmDataSet *dcm_dataset_clone(DcmError **error, const DcmDataSet *dataset);

/**
 * Insert a Data Element into a Data Set.
 *
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
 * :param dataset: Pointer to Data Set
 * :param indentation: Number of white spaces before text
 */
DCM_EXTERN
void dcm_dataset_print(const DcmDataSet *dataset, uint8_t indentation);

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
 * :param error: Error structure pointer
 * :return: Pointer to Sequence
 */
DCM_EXTERN
DcmSequence *dcm_sequence_create(DcmError **error);

/**
 * Append a Data Set item to a Sequence.
 *
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
 * :param seq: Pointer to Sequence
 * :param index: Zero-based index of the Data Set item in the Sequence
 */
DCM_EXTERN
void dcm_sequence_remove(DcmError **error, DcmSequence *seq, uint32_t index);

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
 * :param error: Error structure pointer
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
 * :param error: Error structure pointer
 * :param offsets: Offset of each Frame in the Pixel Data Element
 *                 (measured from the first byte of the first Frame).
 * :param num_frames: Number of Frames in the Pixel Data Element
 * :first_frame_offset: Offset from pixel_data_offset to the first byte of the
 * 			first frame
 *
 * The created object takes over ownership of the memory referenced by `offsets`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_bot_create(DcmError **error,
                       ssize_t *offsets, uint32_t num_frames, 
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
 * :param index: Zero-based index of Frame in the Pixel Data Element
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
 * A set of IO functions, see dcm_file_create_io().
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
 * Create a File that reads using a set of DcmIO functions.
 *
 * :param io: Set of read functions for this DcmFile
 * :param client: Client data for read functions
 *
 * :return: file
 */
DCM_EXTERN
DcmFile *dcm_file_create_io(DcmError **error, DcmIO *io, void *client); 

/**
 * Open a file on disk as a DcmFile.
 *
 * :param file_path: Path to the file on disk
 *
 * :return: file
 */
DCM_EXTERN
DcmFile *dcm_file_open(DcmError **error, const char *file_path);

/**
 * Open an area of memory as a DcmFile.
 *
 * :param buffer: Pointer to memory area
 * :param length: Length of memory area in bytes
 *
 * :return: file
 */
DCM_EXTERN
DcmFile *dcm_file_memory(DcmError **error, char *buffer, int64_t length);

/**
 * Read File Metainformation from a File.
 *
 * Keeps track of the offset of the Data Set relative to the beginning of the
 * file to speed up subsequent access and determines the transfer syntax in
 * which the contained Data Set is encoded.
 *
 * :param error: Error structure pointer
 * :param file: File
 *
 * :return: File Metainformation
 */
DCM_EXTERN
DcmDataSet *dcm_file_read_file_meta(DcmError **error, DcmFile *file);

/**
 * Read metadata from a File.
 *
 * Keeps track of the offset of the Pixel Data Element relative to the 
 * beginning of the file to speed up subsequent access to individual 
 * Frame items.
 *
 * :param error: Error structure pointer
 * :param file: File
 *
 * :return: metadata
 */
DCM_EXTERN
DcmDataSet *dcm_file_read_metadata(DcmError **error, DcmFile *file);

/**
 * Read Basic Offset Table from a File.
 *
 * In case the Pixel Data element does not contain a Basic Offset Table item,
 * but contains an Extended Offset Table element, the value of the Extended
 * Offset Table element will be read instead.
 *
 * :param error: Error structure pointer
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_file_read_bot(DcmError **error, DcmFile *file,
                          DcmDataSet *metadata);

/**
 * Build Basic Offset Table for a File.
 *
 * :param error: Error structure pointer
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
DCM_EXTERN
DcmBOT *dcm_file_build_bot(DcmError **error, DcmFile *file, 
			   DcmDataSet *metadata);

/**
 * Read an individual Frame from a File.
 *
 * :param error: Error structure pointer
 * :param file: File
 * :param metadata: Metadata
 * :param bot: Basic Offset Table
 * :param index: Zero-based offset of the Frame in the Pixel Data Element
 *
 * :return: Frame
 */
DCM_EXTERN
DcmFrame *dcm_file_read_frame(DcmError **error,
                              DcmFile *file,
                              DcmDataSet *metadata,
                              DcmBOT *bot,
                              uint32_t index);

/**
 * Destroy a File.
 *
 * :param file: File
 */
DCM_EXTERN
void dcm_file_destroy(DcmFile *file);

#endif
