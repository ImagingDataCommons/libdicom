Usage
-----

Memory management
+++++++++++++++++

Each data structure (Data Element, Data Set, Sequence, Frame Item, etc.) takes over ownership of allocated memory for passed arguments upon object creation (``*_create()`` functions) and deallocates the memory upon object destruction (``*_destroy()`` functions).
Note that if the creation of a data structure fails, the memory of the passed arguments will also be freed.


API overview
++++++++++++

A `Data Element <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_DataElement>`_ (:c:type:`dcm_element_t`) is an immutable data container for storing values.
Every Data Element has a `Value Representation (VR) <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.2.html>`_, which specifies the data type and format of the contained value.
VRs can be conceptually grouped into numbers (integers or floating-point values), numeric strings (strings of characters encoding numbers using the decimal or scientific notation), character strings (text of restriction length and character repertoire), or byte strings (unicode).
Each VR is represented using a standard C type (e.g,. VR ``"US"`` has type ``uint16_t`` and VR ``"UI"`` has type ``char *``) and additional value constraints may be checked at runtime (e.g., the maximal capacity of a character string).
Depending on the VR, an individual Data Element may have a `Value Multiplicity (VM) <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/sect_6.4.html>`_ greater than one, i.e., contain more than one value.
Under the hood, a Data Element thus generally contains an array of values.
A Data Element can be created via the VR-specific constructor function (e.g., :c:func:`dcm_element_create_UI()`) and destroyed via :c:func:`dcm_element_destroy()`.
Upon creation, the Data Element takes over ownership of the memory allocated for the contained values.
A copy of the value can be retrieved via the VR-specific getter function (e.g., :c:func:`dcm_element_copy_value_UI()`) using a return attribute.
When a Data Element is destroyed, the memory allocated for contained values is freed.

A `Data Set <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_DataSet>`_ (:c:type:`dcm_dataset_t`) is an ordered collection of Data Elements (:c:type:`dcm_element_t`).
A Data Set can be created via :c:func:`dcm_dataset_create()` and destroyed via :c:func:`dcm_dataset_destroy()`.
Data Elements can be added to a Data Set via :c:func:`dcm_dataset_insert()`, removed from a Data Set via :c:func:`dcm_dataset_remove()`, and retrieved from a Data Set via :c:func:`dcm_dataset_get()` or :c:func:`dcm_dataset_get_clone()`.
When a Data Element is added to a Data Set, the Data Set takes over ownership of the memory allocated for contained Data Elements.
When a Data Element is retrieved from a Data Set, it may either be borrowed with ownership of the memory allocated for the Data Element remaining with the Data Set in case of :c:func:`dcm_dataset_get()` or copied with the caller taking on ownership of the memory newly allocated for the Data Element in case of :c:func:`dcm_dataset_get_clone()`.
Furthermore, an individual Data Element can only be part of only one Data Set.
When a Data Element is removed from a Data Set, the memory allocated for the Data Element is freed.
When a Data Set is destroyed, all contained Data Elements are also automatically destroyed.

A `Sequence <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_SequenceOfItems>`_ (:c:type:`dcm_sequence_t`) is an ordered collection of `Items <http://dicom.nema.org/medical/dicom/current/output/chtml/part05/chapter_3.html#glossentry_Item>`_, each containing one Data Set.
A Sequence can be created via :c:func:`dcm_sequence_create()` and destroyed via :c:func:`dcm_sequence_destroy()`.
Data Sets can be added to a Sequence via :c:func:`dcm_sequence_append()`, removed from a Sequence via :c:func:`dcm_sequence_remove()`, and retrieved from a Sequence via :c:func:`dcm_sequence_get()`.
When a Data Set is added to a sequence, the sequence takes over ownership of the memory allocated for the Data Set (and consequently of each contained Data Element).
When a Data Set is retrieved from a sequence, it is only borrowed and ownership of the memory allocated for the Data Set remains with the sequence.
Retrieved Data Sets are immutable (locked).
When a Data Set is removed from a sequence, the Data Set is destroyed (i.e., the allocated memory is freed).
When a Sequence is destroyed, all contained Data Sets are also automatically destroyed.

A File (:c:type:`dcm_file_t`) enables access of a `DICOM file <http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_DICOMFile>`_, which contains an encoded Data Set representing an SOP Instance.
A File can be created via :c:func:`dcm_file_create()` and destroyed via :c:func:`dcm_file_destroy()`, which opens a Part10 file stored on disk and closes it, respectively.
The content of a Part10 file can be read using various functions.
The `File Meta Information <http://dicom.nema.org/medical/dicom/current/output/chtml/part10/chapter_3.html#glossentry_FileMetaInformation>`_ can be read via :c:func:`dcm_file_read_file_meta()`.
The metadata of the Data Set (i.e., all Data Elements with the exception of the Pixel Data Element) can be read via :c:func:`dcm_file_read_metadata()`.
In case the Data Set contained in a Part10 file represents an Image instance, individual Frame Items of the Pixel Data Element can be read via :c:func:`dcm_file_read_frame()` using a Basic Offset Table (BOT) Item.
The BOT Item may either be read from a File via :c:func:`dcm_file_read_bot()` or built for a File via :c:func:`dcm_file_build_bot()`.

Thread safety
+++++++++++++

Data Elements are immutable and cannot be modified after creation.
Data Sets are generally mutable (i.e., Data Elements can be inserted or removed), but they can be locked to prevent subsequent modification via :c:func:`dcm_dataset_lock()`.
A Data Set is automatically locked when retrieved from a Sequence via :c:func:`dcm_sequence_get()` or read from a File via :c:func:`dcm_file_read_metadata()`.
Sequences are also mutable (i.e., Data Sets can be appended or removed), but they can be locked to prevent subsequent modification via :c:func:`dcm_sequence_lock()`.
A Sequence is automatically locked when used a value in a Data Element with Value Representation SQ (Sequence of Items).

Error handling
++++++++++++++

The library differentiates between two different kinds of errors, which are handled differently:

- **User errors** may occur despite correct usage of the library's API by the programmer and are potentially recoverable by an application (e.g., file not found).
  Functions generally signal that an error of this kind occured to the caller by returning either ``NULL``, ``false`` or ``-1``, depending on whether the function's return value is a pointer, a Boolean (``stdbool.h``), or a signed integer, respectively.
  The caller can then check the return value and take appropriate action.
  Functions that return a value of another type are not supposed to fail due to a user error.

- **Runtime errors** should not occur and reflect incorrect usage of the library's API by the programmer, i.e., a bug in the application code.
  An application will not be able to recover from these kinds of errors.
  Functions generally check for known runtime errors and, in case an error condition is encountered, stop execution and exit the application.

In either case, functions log an error message to the standard error stream when the log level (:c:var:`dcm_log_level`) is set to :c:enumerator:`DCM_LOG_ERROR` or higher.
