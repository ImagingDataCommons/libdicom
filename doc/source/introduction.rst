Introduction
------------

C library and executable tools for reading and writing DICOM data sets.

Supported parts of the standard
+++++++++++++++++++++++++++++++

* Part 5 - Data Structures and Encoding

* Part 6 - Data Dictionary

* Part 10 - Media Storage and File Format for Media Interchange

Note that the library does not read the Pixel Data element at once, but
instead provides an interface to randomly access individual frame items
of Pixel Data elements.  However, the library does not concern itself with
decoding the values of frame items.

Design goals
++++++++++++

The library aims to:

* Provide a stable application binary interface (ABI)

* Be highly portable and run on Linux, Unix, macOS, and Windows operating
  systems with different architectures

* Be dead simple and free of surprises

* Have no external build or runtime dependencies

* Be easily callable from other languages via a C foreign function interface
  (FFI)

* Be fast to compile and produce small binaries

* Be easty to compile to WebAssembly using Emscripten

