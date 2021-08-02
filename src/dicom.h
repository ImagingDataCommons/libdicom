#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DCM_INCLUDED
#define DCM_INCLUDED

#ifndef NDEBUG
#  define DCM_DEBUG_ONLY( ... ) __VA_ARGS__
#else
#  define DCM_DEBUG_ONLY( ... )
#endif

#define DCM_NEW(TYPE) \
    (TYPE *) dcm_calloc(1, sizeof(TYPE))

#define DCM_ARRAY_ZEROS(N, TYPE) \
    (TYPE *) dcm_calloc(N, sizeof(TYPE))

/**
 * Allocate and initialize a block of memory.
 *
 * :param n: Number of items.
 * :param size: Number of bytes per item.
 *
 * :return: Pointer to allocated memory.
 */
extern void *dcm_calloc(size_t n, size_t size);


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
extern DcmLogLevel dcm_log_level;

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
 * :return: Yes/no answer
 */
extern bool dcm_is_public_tag(uint32_t tag);

/**
 * Determine whether a Tag is private.
 *
 * :param tag: Attribute Tag
 *
 * :return: Yes/no answer
 */
extern bool dcm_is_private_tag(uint32_t tag);

/**
 * Determine whether a Tag is valid.
 *
 * :param tag: Attribute Tag
 *
 * :return: Yes/no answer
 */
extern bool dcm_is_valid_tag(uint32_t tag);

/**
 * Determine whether a Value Representation is valid.
 *
 * :param vr: Attribute Value Representation
 *
 * :return: Yes/no answer
 */
extern bool dcm_is_valid_vr(const char *vr);

/**
 * Determine whether a Transfer Syntax is encapsulated.
 *
 * :param transfer_syntax_uid: Transfer Syntax UID
 *
 * :return: Yes/no answer
 */
extern bool dcm_is_encapsulated_transfer_syntax(const char *transfer_syntax_uid);


/**
 * Data Element
 */

/**
 * Create a Data Element with Value Representation AE (Application Entity).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AE(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation AE (Application Entity)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AE_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation AS (Age String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AS(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation AS (Age String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AS_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation AT (Attribute Tag).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AT(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation AT (Attribute Tag)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_AT_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation CS (Code String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_CS(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation CS (Code String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_CS_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation DA (Date).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DA(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation DA (Date)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DA_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation DS (Decimal String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DS(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation DS (Decimal String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DS_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation DT (Date Time).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DT(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation DT (Date Time)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_DT_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation FD (Floating Point Double).
 *
 * :param tag: Tag
 * :param value: Floating-point value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_FD(uint32_t tag, double value);

/**
 * Create a Data Element with Value Representation FD (Floating Point Double)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of floating-point values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_FD_multi(uint32_t tag,
                                               double *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation FL (Floating Point Single).
 *
 * :param tag: Tag
 * :param value: Floating-point value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_FL(uint32_t tag, float value);

/**
 * Create a Data Element with Value Representation FL (Floating Point Single)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of floating-point values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_FL_multi(uint32_t tag,
                                               float *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation IS (Integer String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_IS(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation IS (Integer String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_IS_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation LO (Long String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_LO(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation LO (Long String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_LO_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation PN (Person Name).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_PN(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation PN (Person Name)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_PN_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation SH (Short String).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SH(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation SH (Short String)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SH_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation TM (Time).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_TM(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation TM (Time)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_TM_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation UI (Unique Identifier).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UI(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation UI (Unique Identifier)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of character string values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UI_multi(uint32_t tag,
                                               char **values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation SS (Signed Short)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SS(uint32_t tag, int16_t value);

/**
 * Create a Data Element with Value Representation SS (Signed Short).
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SS_multi(uint32_t tag,
                                               int16_t *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation SL (Signed Long).
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SL(uint32_t tag, int32_t value);

/**
 * Create a Data Element with Value Representation SL (Signed Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SL_multi(uint32_t tag,
                                               int32_t *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation SV (Signed Very Long).
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SV(uint32_t tag, int64_t value);

/**
 * Create a Data Element with Value Representation SV (Signed Very Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SV_multi(uint32_t tag,
                                               int64_t *values,
                                               uint32_t vm);


/**
 * Create a Data Element with Value Representation UL (Unsigned Long).
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UL(uint32_t tag, uint32_t value);

/**
 * Create a Data Element with Value Representation UL (Unsigned Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UL_multi(uint32_t tag,
                                               uint32_t *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation US (Unsigned Short).
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_US(uint32_t tag, uint16_t value);

/**
 * Create a Data Element with Value Representation US (Unsigned Short)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_US_multi(uint32_t tag,
                                               uint16_t *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation UV (Unsigned Very Long).
 *
 * :param tag: Tag
 * :param value: Integer value
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UV(uint32_t tag, uint64_t value);

/**
 * Create a Data Element with Value Representation UV (Unsigned Very Long)
 * and Value Multiplicity equal to or greater than one.
 *
 * :param tag: Tag
 * :param values: Array of integer values
 * :param vm: Value Multiplicity
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UV_multi(uint32_t tag,
                                               uint64_t *values,
                                               uint32_t vm);

/**
 * Create a Data Element with Value Representation ST (Short Text).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_ST(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation SQ (Sequence).
 *
 * :param tag: Tag
 * :param value: Sequence value
 *
 * The created object takes over ownership of the memory referenced by `values`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_SQ(uint32_t tag, DcmSequence *value);

/**
 * Create a Data Element with Value Representation LT (Long Text).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_LT(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation OB (Other Byte).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_OB(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation OD (Other Double).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_OD(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation OF (Other Float).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_OF(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation OV (Other Very Long).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_OV(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation OW (Other Word).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_OW(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation UC (Unlimited Characters).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UC(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation UN (Unknown).
 *
 * :param tag: Tag
 * :param value: Byte string value
 * :param length: Number of bytes
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UN(uint32_t tag,
                                         char *value,
                                         uint32_t length);

/**
 * Create a Data Element with Value Representation UR
 * (Universal Resource Identifier).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UR(uint32_t tag, char *value);

/**
 * Create a Data Element with Value Representation UT (Unlimited Text).
 *
 * :param tag: Tag
 * :param value: Character string value
 *
 * The created object takes over ownership of the memory referenced by `value`
 * and frees it when the object is destroyed or if the creation fails.
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_element_create_UT(uint32_t tag, char *value);

/**
 * Get group number (first part of Tag) of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag group number
 */
extern uint16_t dcm_element_get_group_number(DcmElement *element);

/**
 * Get element number (second part of Tag) of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag element number
 */
extern uint16_t dcm_element_get_element_number(DcmElement *element);

/**
 * Get Tag of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Tag
 */
extern uint32_t dcm_element_get_tag(DcmElement *element);

/**
 * Check Value Representation of a Data Element.
 *
 * :param element: Pointer to Data Element
 * :param vr: Value Representation
 *
 * :return: Whether Data Element has the specified Value Representation
 */
extern bool dcm_element_check_vr(DcmElement *element, const char *vr);

/**
 * Get length of the entire value of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Length of value of Data Element
 */
extern uint32_t dcm_element_get_length(DcmElement *element);

/**
 * Get Value Multiplicity of a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Value Multiplicity
 */
extern uint32_t dcm_element_get_vm(DcmElement *element);

/**
 * Determine whether a Data Element has a Value Multiplicity greater than one.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Yes/no answer
 */
extern bool dcm_element_is_multivalued(DcmElement *element);

/**
 * Clone (i.e., create a deep copy of) a Data Element.
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to clone of Data Element
 */
extern DcmElement *dcm_element_clone(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation AE
 * (Appllication Entity).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_AE(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation AS (Age String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_AS(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation AT (Attribute Tag).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_AT(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation CS (Code String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_CS(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation DA (Date).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_DA(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation DS (Decimal String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_DS(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation DT (Date Time).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_DT(DcmElement *element,
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
extern double dcm_element_get_value_FD(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation FL (Floating Point).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern float dcm_element_get_value_FL(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation IS (Integer String).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_IS(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation LO (Long String).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_LO(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation PN (Person Name).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_PN(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation SH (Short String).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_SH(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation TM (Time).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_TM(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation SL (Signed Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern int32_t dcm_element_get_value_SL(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation SS (Signed Short).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern int16_t dcm_element_get_value_SS(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation SV (Signed Very Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern int64_t dcm_element_get_value_SV(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation ST (Short text).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_ST(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UI (Unique Identifier).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_UI(DcmElement *element,
                                            uint32_t index);

/**
 * Get value of a Data Element with Value Representation UL (Unsigned Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern uint32_t dcm_element_get_value_UL(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation US (Unsigned Short).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern uint16_t dcm_element_get_value_US(DcmElement *element, uint32_t index);

/**
 * Get value of a Data Element with Value Representation UV (Unsigned Very Long).
 *
 * :param element: Pointer to Data Element
 * :param index: Zero-based index of value within the Data Element
 *
 * :return: Value
 */
extern uint64_t dcm_element_get_value_UV(DcmElement *element, uint32_t index);


/**
 * Get value of a Data Element with Value Representation OB (Other Byte).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_OB(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OD (Other Double).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_OD(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OF (Other Float).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_OF(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OV (Other Very Long).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_OV(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation OW (Other Word).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_OW(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UC (Unlimited Characters).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_UC(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UN (Unknown).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_UN(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UR
 * (Universal Resource Identifier).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_get_value_UR(DcmElement *element);

/**
 * Get value of a Data Element with Value Representation UT (Unlimited Text).
 *
 * :param element: Pointer to Data Element
 *
 * :return: Pointer to memory location where value is stored
 */
extern const char *dcm_element_copy_value_UT(DcmElement *element);


extern DcmSequence *dcm_element_get_value_SQ(DcmElement *element);

/**
 * Print a Data Element.
 *
 * :param element: Pointer to Data Element
 * :param indentation: Number of white spaces before text
 */
extern void dcm_element_print(DcmElement *element, uint8_t indentation);

/**
 * Destroy a Data Element.
 *
 * :param element: Pointer to Data Element
 */
extern void dcm_element_destroy(DcmElement *element);


/**
 * Data Set
 */

/**
 * Create an empty Data Set.
 */
extern DcmDataSet *dcm_dataset_create(void);

/**
 * Clone (i.e., create a deep copy of) a Data Set.
 *
 * :param dataset: Pointer to Data Set
 *
 * :return: Pointer to clone of Data Set
 */
extern DcmDataSet *dcm_dataset_clone(DcmDataSet *dataset);

/**
 * Insert a Data Element into a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param element: Pointer to Data Element
 *
 * The object takes over ownership of the memory referenced by `element`
 * and frees it when the object is destroyed or if the insert operation fails.
 *
 * :return: Whether insert operation was successful
 */
extern bool dcm_dataset_insert(DcmDataSet *dataset, DcmElement *element);

/**
 * Remove a Data Element from a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Whether remove operation was successful
 */
extern bool dcm_dataset_remove(DcmDataSet *dataset, uint32_t tag);

/**
 * Get a Data Element from a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Pointer to Data Element
 */
extern DcmElement *dcm_dataset_get(DcmDataSet *dataset, uint32_t tag);

/**
 * Get a clone (deep copy) of a Data Element from a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Pointer to clone of Data Element
 */
extern DcmElement *dcm_dataset_get_clone(DcmDataSet *dataset, uint32_t tag);

/**
 * Iterate over Data Elements in a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param fn: Function that should be called for each Data Element
 */
extern void dcm_dataset_foreach(DcmDataSet *dataset,
                                void (*fn)(DcmElement *element));

/**
 * Determine whether a Data Element is contained in a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param tag: Attribute Tag of a Data Element
 *
 * :return: Yes/no answer
 */
extern bool dcm_dataset_contains(DcmDataSet *dataset, uint32_t tag);

/**
 * Count the number of Data Elements in a Data Set.
 *
 * :param dataset: Pointer to Data Set
 *
 * :return: Number of Data Elements
 */
extern uint32_t dcm_dataset_count(DcmDataSet *dataset);

/**
 * Obtain a copy of the Tag of each Data Element in a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param tags: Pointer to memory location to which to copy tags
 */
extern void dcm_dataset_copy_tags(DcmDataSet *dataset, uint32_t *tags);

/**
 * Lock a Data Set to prevent modification.
 *
 * :param dataset: Pointer to Data Set
 */
extern void dcm_dataset_lock(DcmDataSet *dataset);

/**
 * Check whether a Data Set is locked.
 *
 * :param dataset: Pointer to Data Set
 *
 * :return: Yes/no answer
 */
extern bool dcm_dataset_is_locked(DcmDataSet *dataset);

/**
 * Print a Data Set.
 *
 * :param dataset: Pointer to Data Set
 * :param indentation: Number of white spaces before text
 */
extern void dcm_dataset_print(DcmDataSet *dataset, uint8_t indentation);

/**
 * Destroy a Data Set.
 *
 * :param dataset: Pointer to Data Set
 */
extern void dcm_dataset_destroy(DcmDataSet *dataset);


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
 * :return: Pointer to Sequence
 */
extern DcmSequence *dcm_sequence_create(void);

/**
 * Append a Data Set item to a Sequence.
 *
 * :param seq: Pointer to Sequence
 * :param item: Data Set item
 *
 * The object takes over ownership of the memory referenced by `item`
 * and frees it when the object is destroyed or if the append operation fails.
 *
 * :return: Whether append operation was successful
 */
extern bool dcm_sequence_append(DcmSequence *seq, DcmDataSet *item);

/**
 * Get a Data Set item from a Sequence.
 *
 * :param seq: Pointer to Sequence
 * :param index: Zero-based index of the Data Set item in the Sequence
 *
 * :return: Pointer to Data Set item
 */
extern DcmDataSet *dcm_sequence_get(DcmSequence *seq, uint32_t index);

/**
 * Iterate over Data Set items in a Sequence.
 *
 * :param seq: Pointer to Sequence
 * :param fn: Function that should be called for each Data Set item
 */
extern void dcm_sequence_foreach(DcmSequence *seq,
                                 void (*fn)(DcmDataSet *item));

/**
 * Remove a Data Set item from a Sequence.
 *
 * :param seq: Pointer to Sequence
 * :param index: Zero-based index of the Data Set item in the Sequence
 */
extern void dcm_sequence_remove(DcmSequence *seq, uint32_t index);

/**
 * Count the number of Data Set items in a Sequence.
 *
 * :param seq: Pointer to Sequence
 *
 * :return: number of Data Set items
 */
extern uint32_t dcm_sequence_count(DcmSequence *seq);

/**
 * Lock a Sequence to prevent modification.
 *
 * :param seq: Pointer to Sequence
 */
extern void dcm_sequence_lock(DcmSequence *seq);

/**
 * Check whether a Sequence is locked.
 *
 * :param seq: Pointer to Sequence
 *
 * :return: Yes/no answer
 */
extern bool dcm_sequence_is_locked(DcmSequence *seq);

/**
 * Destroy a Sequence.
 *
 * :param seq: Pointer to Sequence
 */
extern void dcm_sequence_destroy(DcmSequence *seq);


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
extern DcmFrame *dcm_frame_create(uint32_t number,
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
extern uint32_t dcm_frame_get_number(DcmFrame *frame);

/**
 * Get length of a Frame Item.
 *
 * :param frame: Frame
 *
 * :return: number of bytes
 */
extern uint32_t dcm_frame_get_length(DcmFrame *frame);

/**
 * Get Rows of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of rows in pixel matrix
 */
extern uint16_t dcm_frame_get_rows(DcmFrame *frame);

/**
 * Get Columns of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of columns in pixel matrix
 */
extern uint16_t dcm_frame_get_columns(DcmFrame *frame);

/**
 * Get Samples per Pixel of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of samples (color channels) per pixel
 */
extern uint16_t dcm_frame_get_samples_per_pixel(DcmFrame *frame);

/**
 * Get Bits Allocated of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits allocated per pixel
 */
extern uint16_t dcm_frame_get_bits_allocated(DcmFrame *frame);

/**
 * Get Bits Stored of a Frame.
 *
 * :param frame: Frame
 *
 * :return: number of bits stored per pixel
 */
extern uint16_t dcm_frame_get_bits_stored(DcmFrame *frame);

/**
 * Get High Bit of a Frame.
 *
 * :param frame: Frame
 *
 * :return: most significant bit of pixels
 */
extern uint16_t dcm_frame_get_high_bit(DcmFrame *frame);

/**
 * Get Pixel Representation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: representation of pixels (unsigned integers or 2's complement)
 */
extern uint16_t dcm_frame_get_pixel_representation(DcmFrame *frame);

/**
 * Get Planar Configuration of a Frame.
 *
 * :param frame: Frame
 *
 * :return: configuration of samples (color-by-plane or color-by-pixel)
 */
extern uint16_t dcm_frame_get_planar_configuration(DcmFrame *frame);

/**
 * Get Photometric Interpretation of a Frame.
 *
 * :param frame: Frame
 *
 * :return: interpretation of pixels (monochrome, RGB, etc.)
 */
extern const char *dcm_frame_get_photometric_interpretation(DcmFrame *frame);

/**
 * Get Transfer Syntax UID for a Frame.
 *
 * :param frame: Frame
 *
 * :return: UID of the transfer syntax in which frame is encoded
 */
extern const char *dcm_frame_get_transfer_syntax_uid(DcmFrame *frame);

/**
 * Get pixel data of a Frame.
 *
 * :param frame: Frame
 *
 * :return: pixel data
 */
extern const char *dcm_frame_get_value(DcmFrame *frame);

/**
 * Destroy a Frame.
 *
 * :param frame: Frame
 */
extern void dcm_frame_destroy(DcmFrame *frame);


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
extern DcmBOT *dcm_bot_create(ssize_t *offsets, uint32_t num_frames);

/**
 * Get number of Frame offsets in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 *
 * :return: number of frames
 */
extern uint32_t dcm_bot_get_num_frames(DcmBOT *bot);

/**
 * Get Frame offset in the Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 * :param index: Zero-based index of Frame in the Pixel Data Element
 *
 * :return: offset
 */
extern ssize_t dcm_bot_get_frame_offset(DcmBOT *bot, uint32_t index);

/**
 * Print a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
extern void dcm_bot_print(DcmBOT *bot);

/**
 * Destroy a Basic Offset Table.
 *
 * :param bot: Basic Offset Table
 */
extern void dcm_bot_destroy(DcmBOT *bot);


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
extern DcmFile *dcm_file_create(const char *file_path, char mode);

/**
 * Read File Metainformation from a File.
 *
 * :param file: File
 *
 * :return: File Metainformation
 */
extern DcmDataSet *dcm_file_read_file_meta(DcmFile *file);

/**
 * Read metadata from a File.
 *
 * :param file: File
 *
 * :return: metadata
 */
extern DcmDataSet *dcm_file_read_metadata(DcmFile *file);

/**
 * Read Basic Offset Table from a File.
 *
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
extern DcmBOT *dcm_file_read_bot(DcmFile *file, DcmDataSet *metadata);

/**
 * Build Basic Offset Table for a File.
 *
 * :param file: File
 * :param metadata: Metadata
 *
 * :return: Basic Offset Table
 */
extern DcmBOT *dcm_file_build_bot(DcmFile *file, DcmDataSet *metadata);

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
extern DcmFrame *dcm_file_read_frame(DcmFile *file,
                                     DcmDataSet *metadata,
                                     DcmBOT *bot,
                                     uint32_t index);

/**
 * Destroy a File.
 *
 * :param file: File
 */
extern void dcm_file_destroy(DcmFile *file);

#endif
