[![Build Status](https://github.com/hackermd/libdicom/actions/workflows/run_unit_tests_Linux.yml/badge.svg)](https://github.com/hackermd/libdicom/actions)

# libdicom

C library and executable tools for reading DICOM files.

## Getting started

### Building from source

```none
./autogen.sh
make
make install
```
See [the installation documentation](https://libdicom.readthedocs.io/en/latest/installation.html) for build dependencies and installation options.

### Printing the metadata of a DICOM file

```none
dcm-dump data/test_files/sm_image.dcm
```

## Documentation

User and developer guides as well as API documentation can be found at [libdicom.readthedocs.io](https://libdicom.readthedocs.io/en/latest/).
