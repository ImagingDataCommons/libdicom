Usage
-----

Memory management
+++++++++++++++++

Each data structure (Data Element, Data Set, Sequence, Frame Item, etc.) takes
over ownership of allocated memory for passed arguments upon object creation
(``*_create()`` functions) and deallocates the memory upon object destruction
(``*_destroy()`` functions).  Note that if the creation of a data structure
fails, the memory of the passed arguments will also be freed.


API overview
++++++++++++

A `Data Element
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_DataElement>`_
(:c:type:`DcmElement`) is an immutable data container for
storing values.  Every Data Element has a `Value Representation (VR)
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.2.html>`_,
which specifies the data type and format of the contained value.  VRs can
be conceptually grouped into numbers (integers or floating-point values),
numeric strings (strings of characters encoding numbers using the decimal
or scientific notation), character strings (text of restriction length and
character repertoire), or byte strings (unicode).  Each VR is represented
using a standard C type (e.g,. VR ``"US"`` has type ``uint16_t`` and VR
``"UI"`` has type ``char *``) and additional value constraints may be checked
at runtime (e.g., the maximal capacity of a character string).  Depending
on the VR, an individual Data Element may have a `Value Multiplicity (VM)
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.4.html>`_
greater than one, i.e., contain more than one value.  Under the
hood, a Data Element thus generally contains an array of values.
A Data Element can be created via the VR-specific constructor
function (e.g., :c:func:`dcm_element_create_UI()`) and destroyed
via :c:func:`dcm_element_destroy()`.  Upon creation, the Data Element
takes over ownership of the memory allocated for the contained values.
An individual value can be retrieved via the VR-specific getter function
(e.g., :c:func:`dcm_element_get_value_UI()`).  Note that in case of character
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
:c:func:`dcm_dataset_get()` or :c:func:`dcm_dataset_get_clone()`.  When a
Data Element is added to a Data Set, the Data Set takes over ownership
of the memory allocated for contained Data Elements.  When a Data Element
is retrieved from a Data Set, it may either be borrowed with ownership of
the memory allocated for the Data Element remaining with the Data Set in
case of :c:func:`dcm_dataset_get()` or copied with the caller taking on
ownership of the memory newly allocated for the Data Element in case of
:c:func:`dcm_dataset_get_clone()`.  Furthermore, an individual Data Element
can only be part of only one Data Set.  When a Data Element is removed from a
Data Set, the memory allocated for the Data Element is freed.  When a Data Set
is destroyed, all contained Data Elements are also automatically destroyed.

A `Sequence
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_SequenceOfItems>`_
(:c:type:`DcmSequence`) is an ordered collection of `Items
<http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_Item>`_,
each containing one Data Set.  A Sequence can be created
via :c:func:`dcm_sequence_create()` and destroyed via
:c:func:`dcm_sequence_destroy()`.  Data Sets can be added to a Sequence
via :c:func:`dcm_sequence_append()`, removed from a Sequence via
:c:func:`dcm_sequence_remove()`, and retrieved from a Sequence via
:c:func:`dcm_sequence_get()`.  When a Data Set is added to a sequence,
the sequence takes over ownership of the memory allocated for the Data Set
(and consequently of each contained Data Element).  When a Data Set is
retrieved from a sequence, it is only borrowed and ownership of the memory
allocated for the Data Set remains with the sequence.  Retrieved Data Sets
are immutable (locked).  When a Data Set is removed from a sequence, the
Data Set is destroyed (i.e., the allocated memory is freed).  When a Sequence
is destroyed, all contained Data Sets are also automatically destroyed.

A File (:c:type:`DcmFile`) enables access of a `DICOM file
<http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_DICOMFile>`_,
which contains an encoded Data Set representing a SOP Instance.
A File can be created via :c:func:`dcm_file_open()` and
destroyed via :c:func:`dcm_file_destroy()`, which opens a Part10
file stored on disk and closes it, respectively.  The content of a
Part10 file can be read using various functions.  The `File Meta Information
<http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_FileMetaInformation>`_
can be read via :c:func:`dcm_file_read_file_meta()`.  The metadata of
the Data Set (i.e., all Data Elements with the exception of the Pixel
Data Element) can be read via :c:func:`dcm_file_read_metadata()`.
In case the Data Set contained in a Part10 file represents an Image
instance, individual Frame Items of the Pixel Data Element can be read
via :c:func:`dcm_file_read_frame()` using a Basic Offset Table (BOT) Item.
The BOT Item may either be read from a File via :c:func:`dcm_file_read_bot()`
or built for a File via :c:func:`dcm_file_build_bot()`.

Thread safety
+++++++++++++

Data Elements are immutable and cannot be modified after creation.
Data Sets are generally mutable (i.e., Data Elements can be inserted or
removed), but they can be locked to prevent subsequent modification via
:c:func:`dcm_dataset_lock()`.  A Data Set is automatically locked when
retrieved from a Sequence via :c:func:`dcm_sequence_get()` or read from a
File via :c:func:`dcm_file_read_metadata()`.  Sequences are also mutable
(i.e., Data Sets can be appended or removed), but they can be locked
to prevent subsequent modification via :c:func:`dcm_sequence_lock()`.
A Sequence is automatically locked when used as a value in a Data Element
with Value Representation SQ (Sequence of Items).

Error handling
++++++++++++++

Library functions which can return an error take a double pointer to a
:c:type:`DcmError` struct as a first argument. If an error is detected,
this pointer will be updated to refer to an error object. You can extract
a :c:type:`DcmErrorCode` with :c:func:`dcm_error_code()`, an error summary
with :c:func:`dcm_error_summary()`, and a detailed error message with
:c:func:`dcm_error_message()`. After presenting the error to the user,
call :c:func:`dcm_error_clear()` to clear the error pointer and free any
allocated memory.

You can pass `NULL` instead of an error pointer if you are not interested in
error messages. In this case, any errors will be logged to debug instead, see
:c:func:`dcm_log_debug()`.

For example:

.. code:: c

    #include <stdlib.h>
    #include <dicom.h>

    int main() {
        const char *file_path = "does not exist";
        DcmError *error = NULL;

        DcmFile *file = dcm_file_open(&error, file_path);
        if (file == NULL) {
            printf("error detected: %s\n", dcm_error_code_str(dcm_error_code(error)));
            printf("summary: %s\n", dcm_error_summary(error));
            printf("message: %s\n", dcm_error_message(error));
            dcm_error_clear(&error);
            return 1;
        }

        dcm_file_destroy(file);

        return 0;
    }

Getting started
+++++++++++++++

Below is an example for reading metadata from a DICOM Part10 file and
printing it to standard output:

.. code:: c

    #include <stdlib.h>
    #include <dicom.h>

    int main() {
        const char *file_path = "/path/to/file.dcm";
        DcmError *error = NULL;

        DcmFile *file = dcm_file_open(&error, file_path);
        if (file == NULL) {
            dcm_error_log(error);
            dcm_error_clear(&error);
            return 1;
        }

        DcmDataSet *metadata = dcm_file_read_metadata(&error, file);
        if (metadata == NULL) {
            dcm_error_log(error);
            dcm_error_clear(&error);
            dcm_file_destroy(file);
            return 1;
        }
        dcm_dataset_print(metadata, 0);

        dcm_file_destroy(file);
        dcm_dataset_destroy(metadata);

        return 0;
    }
