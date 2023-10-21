Usage
-----

API overview
++++++++++++

A Filehandle (:c:type:`DcmFilehandle`) enables access of a `DICOM file
<http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_DICOMFile>`_,
which contains an encoded Data Set representing a SOP Instance.
A Filehandle can be created via :c:func:`dcm_filehandle_create_from_file()`
or :c:func:`dcm_filehandle_create_from_memory()` , and destroyed via
:c:func:`dcm_filehandle_destroy()`.  You can make your own load functions
to load from other IO sources, see :c:func:`dcm_filehandle_create()`.

The content of a Part10 file can be read using various functions.

The `File Meta Information
<http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_FileMetaInformation>`_
can be accessed via :c:func:`dcm_filehandle_get_file_meta()`.

The principal metadata of the Data Set can be accessed via
:c:func:`dcm_filehandle_get_metadata_subset()`. This function will stop
read on tags which are likely to take a long time to process.

You can read all metadata and control read stop using a sequence of calls to
:c:func:`dcm_filehandle_read_metadata()`.

In case the Data Set contained in a Part10 file represents an Image instance,
individual frames may be read out with :c:func:`dcm_filehandle_read_frame()`.

Use :c:func:`dcm_filehandle_read_frame_position()` to read the frame at a
certain (column, row) position. This will return NULL and set the error code
`DCM_ERROR_CODE_MISSING_FRAME` if there is no frame at that position.

A `Data Element
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_DataElement>`_
(:c:type:`DcmElement`) is an immutable data container for storing values.

Every data element has a tag indicating its purpose. Tags are 32-bit
unsigned ints with the top 16 bits indicating the group and the bottom 16
the element. They are usually written in hexadecimal, perhaps ``0x00400554``,
meaning element ``0x554`` of group ``0x40``, or as keywords, in this case
``"SpecimenUID"``. You can get the tag from its corresponding keyword with
:c:func:`dcm_dict_tag_from_keyword()`, or find the keyword from a tag with
:c:func:`dcm_dict_keyword_from_tag()`.

Every Data Element has a `Value Representation (VR)
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.2.html>`_,
which specifies the data type and format of the contained value.  VRs can
be conceptually grouped into numbers (integers or floating-point values),
numeric strings (strings of characters encoding numbers using the decimal
or scientific notation), character strings (text of restriction length and
character repertoire), or byte strings (unicode).  Each VR is represented
using a standard C type (e.g,. VR ``"US"`` has type ``uint16_t`` and VR
``"UI"`` has type ``char *``) and additional value constraints may be
checked at runtime (e.g., the maximal capacity of a character string).

The VR must be appropriate for the tag. Use :c:func:`dcm_vr_from_tag()` to
find the set of allowed VRs for a tag. Use :c:func:`dcm_is_valid_vr_for_tag()`
to check if a VR is allowed for a tag.

Depending on the VR, an individual Data
Element may have a `Value Multiplicity (VM)
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.4.html>`_
greater than one, i.e., contain more than one value.  Under the hood,
a Data Element may thus contain an array of values.

A Data Element can be created with :c:func:`dcm_element_create()`, it can have
a value assigned to it with eg.
:c:func:`dcm_element_set_value_integer()`, and it can be destroyed with
:c:func:`dcm_element_destroy()`. See `Memory management <Memory Management_>`_ below for details on
pointer ownership.

An individual value can be retrieved via the getter functions like
(e.g., :c:func:`dcm_element_get_value_integer()`).  Note that in case of
character
string or binary values, the getter function returns the pointer to the
stored character array  (``const char *``) and that pointer is only valid
for the lifetime of the Data Element.  When a Data Element is destroyed,
the memory allocated for contained values is freed and any pointers to the
freed memory area become dangling pointers.

A `Data Set
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_DataSet>`_
(:c:type:`DcmDataSet`) is an ordered collection of
Data Elements (:c:type:`DcmElement`).  A Data Set can be
created via :c:func:`dcm_dataset_create()` and destroyed via
:c:func:`dcm_dataset_destroy()`.  Data Elements can be added to a
Data Set via :c:func:`dcm_dataset_insert()`, removed from a Data Set
via :c:func:`dcm_dataset_remove()`, and retrieved from a Data Set via
:c:func:`dcm_dataset_get()` or :c:func:`dcm_dataset_get_clone()`.

When a Data Element is added to a Data Set, the Data Set takes over ownership
of the memory allocated for contained Data Elements.  When a Data Element
is retrieved from a Data Set, it may either be borrowed with ownership of
the memory allocated for the Data Element remaining with the Data Set in
case of :c:func:`dcm_dataset_get()` or copied with the caller taking on
ownership of the memory newly allocated for the Data Element in case of
:c:func:`dcm_dataset_get_clone()`.

An individual Data Element can only be part of only one Data Set.  When a
Data Element is removed from a Data Set, the memory allocated for the Data
Element is freed.  When a Data Set is destroyed, all contained Data Elements
are also automatically destroyed.

A `Sequence
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_SequenceOfItems>`_
(:c:type:`DcmSequence`) is an ordered collection of `Items
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_Item>`_,
each containing one Data Set.  A Sequence can be created
via :c:func:`dcm_sequence_create()` and destroyed via
:c:func:`dcm_sequence_destroy()`.  Data Sets can be added to a Sequence
via :c:func:`dcm_sequence_append()`, removed from a Sequence via
:c:func:`dcm_sequence_remove()`, and retrieved from a Sequence via
:c:func:`dcm_sequence_get()`.

When a Data Set is added to a sequence, the sequence takes over ownership of
the memory allocated for the Data Set (and consequently of each contained
Data Element).  When a Data Set is retrieved from a sequence, it is only
borrowed and ownership of the memory allocated for the Data Set remains
with the sequence.  Retrieved Data Sets are immutable (locked).  When a
Data Set is removed from a sequence, the Data Set is destroyed (i.e., the
allocated memory is freed).  When a Sequence is destroyed, all contained
Data Sets are also automatically destroyed.

Thread safety
+++++++++++++

By design, libdicom has no dependencies, not even on a threading
library. This means it can't be threadsafe, since it can't lock any internal
datastructures. However, there are no global structures, so as long as you
don't share a `DcmFilehandle` between threads, you're fine.

You can share `DcmFilehandle` between threads if you lock around calls into
libdicom. The lock only needs to be per-`DcmFilehandle`, you don't need a
global lock.

Error handling
++++++++++++++

Library functions which can return an error take a double pointer to a
:c:type:`DcmError` struct as a first argument. If an error is detected,
this pointer will be updated to refer to an error object. You can extract
a :c:type:`DcmErrorCode` with :c:func:`dcm_error_get_code()`, an error summary
with :c:func:`dcm_error_get_summary()`, and a detailed error message with
:c:func:`dcm_error_get_message()`. After presenting the error to the user,
call :c:func:`dcm_error_clear()` to clear the error pointer and free any
allocated memory.

You can pass `NULL` instead of an error pointer if you are not interested in
error messages. In this case, any errors will be logged to debug instead, see
:c:func:`dcm_log_debug()`.

For example:

.. code:: c

    #include <stdlib.h>
    #include <dicom/dicom.h>

    int main() {
        const char *file_path = "bad-file";
        DcmError *error = NULL;

        DcmFilehandle *filehandle = dcm_filehandle_create_from_file(&error, file_path);
        if (filehandle == NULL) {
            printf("error detected: %s\n", dcm_error_code_str(dcm_error_get_code(error)));
            printf("summary: %s\n", dcm_error_get_summary(error));
            printf("message: %s\n", dcm_error_get_message(error));
            dcm_error_clear(&error);
            return 1;
        }

        dcm_filehandle_destroy(filehandle);

        return 0;
    }

Memory management
+++++++++++++++++

libdicom objects (Data Element, Data Set, Sequence, Frame Item, etc.) can
contain references to other libdicom objects. For example, you can set a
sequence as the value of an element like this:

.. code-block:: c

    if (!dcm_element_set_value_sequence(error, element, sequence)) {
        handle error;
    }

If this function succeeds, ownership of the sequence object passes to the
element, i.e., when the element is destroyed, the sequence will also be
destroyed.

If this function fails, ownership does not transfer.

libdicom objects can also contain references to data structures allocated by
other programs, for example, arrays of numeric values.

.. code-block:: c

    int *values = pointer to array of integers;
    uint32_t vm = number of ints in array;
    if( !dcm_element_set_value_numeric_multi(error, element, values, vm, true)) {
      handle error;
    }

The final parameter, `steal`, sets whether ownership of the pointer to the
array should be "stolen" by libdicom. If it is true, then libdicom will use
:c:func:`free()` to free the array when the element is freed. If it is false,
libdicom will make a copy of the array.

Getting started
+++++++++++++++

Below is an example for reading metadata from a DICOM Part10 file and
printing an element to standard output:

.. code:: c

    #include <stdlib.h>
    #include <dicom/dicom.h>

    int main() {
        const char *file_path = "/path/to/file.dcm";
        DcmError *error = NULL;

        DcmFilehandle *filehandle = dcm_filehandle_create_from_file(&error, file_path);
        if (filehandle == NULL) {
            dcm_error_log(error);
            dcm_error_clear(&error);
            return 1;
        }

        const DcmDataSet *metadata =
            dcm_filehandle_get_metadata_subset(&error, filehandle);
        if (metadata == NULL) {
            dcm_error_log(error);
            dcm_error_clear(&error);
            dcm_filehandle_destroy(filehandle);
            return 1;
        }

        const char *num_frames;
        uint32_t tag = dcm_dict_tag_from_keyword("NumberOfFrames");
        DcmElement *element = dcm_dataset_get(&error, metadata, tag);
        if (element == NULL ||
            !dcm_element_get_value_string(&error, element, 0, &num_frames)) {
            dcm_error_log(error);
            dcm_error_clear(&error);
            dcm_filehandle_destroy(filehandle);
            return 1;
        }

        printf("NumerOfFrames == %s\n", num_frames);

        dcm_filehandle_destroy(filehandle);

        return 0;
    }
