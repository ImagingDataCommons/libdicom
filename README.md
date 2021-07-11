# libdicom

C library and executable tools for reading DICOM files.

# Dependencies

This package uses `check` for unit tests. On Ubuntu:

```
sudo apt install check
```

# Build from git

```
CFLAGS="-g -Wall" ./autogen.sh --prefix=xxx
make V=0
make install
```

