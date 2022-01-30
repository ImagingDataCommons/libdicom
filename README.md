# libdicom

C library and executable tools for reading DICOM files.

## Getting started

### Building from source

For debian/ubuntu you need some prereqs:
```
sudo apt-get install autogen     dh-autoreconf     build-essential     check     dumb-init     libtool     pkg-config     shtool
```

Then:

```none
./autogen.sh
make
make install
```

### Printing the content of a DICOM file

```none
dcm-dump data/test_files/sm_image.dcm
```

## Documentation

User and developer guides as well as API documentation is located under the `doc`.
