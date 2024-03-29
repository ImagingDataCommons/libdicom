Contributing
------------

Coding style
++++++++++++

Source code should be written following the `K&R (Kernighan & Ritchie)
style <https://en.wikipedia.org/wiki/Indentation_style#K&R_style>`_ with
a few modifications.

* Line length: max 80 characters

* Indentation: 4 spaces (no tabs)

* Braces:

  - Functions: opening brace at next line

  - Control statements (mandatory): opening brace at same line

* Spacing:

  - 2 lines between function definitions

  - 1 line between logical blocks within functions (and between variable
    declarations and definitions)

* Comments:

  - Documentation of functions and other symbols: balanced,
    multi-line ``/** ... */`` comments in `reStructuredText
    <https://docutils.sourceforge.io/rst.html>`_ format using `field lists
    <https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html#field-lists>`_
    ``:param:`` and ``:return:`` to document function parameters and return
    values, respectively, and ``:c:member:``, ``:c:func:``, ``:c:macro:``,
    ``:c:struct:``, ``:c:union:``, ``:c:enum:``, ``:c:enumerator:``,
    ``:c:type:``, ``:c:expr:``, ``:c:var:`` from the `Sphinx C domain
    <https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#the-c-domain>`_
    directive to `cross-reference other C language constructs
    <https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#cross-referencing-c-constructs>`_
    or to insert a C expression as inline code.

  - Inline comments in function body: single-line ``//`` C++ style comments

* Naming conventions:

  - Data structures (``struct`` or ``enum``) and types are named using
    upper camel case (e.g., ``DcmDataSet``), while functions are named using
    all lower case with underscores (e.g., ``dcm_dataset_create()``).

  - Names of ``external`` functions, data structures, and types that are
    declared in the ``dicom.h`` header file are prefixed with ``dcm_`` or
    ``Dcm``. Names of ``static`` functions, types, or data structures declared
    in ``*.c`` files are never prefixed.


Interface
+++++++++

The library exposes an "object-oriented" application programming interface
(API), which provides data structures and functions to store, access,
and manipulate the data.

To facilitate portability, the ``dicom.h`` header file is restricted to

* C99 version of the standard (C89 + Boolean type from ``stdbool.h`` +
  fixed-width integer types from ``stdint.h``/``inttypes.h``)

* Opaque data types

* Clear, exact-width integer types (``int16_t``, ``int32_t``, ``int64_t``,
  ``uint16_t``, ``uint32_t``, and ``uint64_t``)

* Minimal use of enums


Implementation
++++++++++++++

The ``dicom-data.c`` (Part 5), ``dicom-dict.c`` (Part 6), and ``dicom-file.c``
 and ``dicom-parse.c`` (Part 10) are implemented based on the C11 version
 of the standard.

The Data Set and Sequence data structures are implemented using the
battletested `uthash <https://troydhanson.github.io/uthash>`_ headers.


Documentation
+++++++++++++

Documentation is written in `reStructuredText
<https://docutils.sourceforge.io/rst.html>`_ format and HTML documents
are autogenerated using `Sphinx <https://www.sphinx-doc.org/en/master/>`_.
API documentation is automatically extracted from the comments in the source
code in the ``dicom.h`` header file via the `Hawkmoth Sphinx C Autodoc
<https://hawkmoth.readthedocs.io/en/latest/index.html>`_ extension, which
relies on `Clang <https://libclang.readthedocs.io/en/latest/index.html>`_
to parse C code.

Documentation files are located under the ``doc/source`` directory of the
repository.  To build the documentation, install ``libclang`` development
headers and the Python ``venv`` module, then build with ``meson``:

    meson compile -C builddir html

The generated documentation files will then be located under the
``builddir/html`` directory.  The ``builddir/html/index.html`` HTML document
can be rendered in the web browser.


Testing
+++++++

Unit test cases are defined and run using `check
<https://github.com/libcheck/check>`_.

Test files are located under ``/tests`` and can be built and run using
``meson``::

    meson test -C builddir

Dynamic analysis
++++++++++++++++

The source code can be analysed using `valgrind <https://www.valgrind.org/>`_.

For example::

    valgrind --leak-check=full dcm-dump data/test_files/sm_image.dcm

Unit testing and dynamic analysis can also be performed using the provided
`Dockerfile` (located in the root of the repository)::

    docker build -t dcm-testing .
    docker run dcm-testing
