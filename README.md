[![Build
Status](https://github.com/ImagingDataCommons/libdicom/actions/workflows/run_unit_tests.yml/badge.svg)](https://github.com/ImagingDataCommons/libdicom/actions)

# libdicom

C library and executable tools for reading DICOM files.

## Getting started

### Building from source

```shell
meson setup --buildtype release builddir
meson compile -C builddir
meson install -C builddir
```
See [the installation
documentation](https://libdicom.readthedocs.io/en/latest/installation.html)
for build dependencies and installation options.

### Printing the metadata of a DICOM file

```shell
dcm-dump data/test_files/sm_image.dcm
```

### Fetching a frame from a file

```shell
dcm-getframe -o tile.raw data/test_files/sm_image.dcm 12
```

### From Python

There's a sample Python binding here:

https://github.com/ImagingDataCommons/pywsidicom

## Documentation

User and developer guides as well as API documentation can be found at
[libdicom.readthedocs.io](https://libdicom.readthedocs.io/en/latest/).

# Thanks

Development of this library was supported by [NCI Imaging Data
Commons](https://imaging.datacommons.cancer.gov/), and has been funded in
whole or in part with Federal funds from the National Cancer Institute,
National Institutes of Health, under Task Order No. HHSN26110071 under
Contract No. HHSN261201500003l.

