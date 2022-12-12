Installation
------------

Building
++++++++

The library and executables can be built using `Meson <https://mesonbuild.com/>`_:

.. code:: bash

    meson setup --buildtype release builddir
    meson compile -C builddir
    meson install -C builddir


Build dependencies
++++++++++++++++++

To install build dependencies:

On Debian-based Linux distributions:

.. code:: bash

    sudo apt install build-essential git meson pkg-config

On macOS:

.. code:: bash

    brew install git meson pkg-config


Build configuration
+++++++++++++++++++

Build for development and debugging:

.. code:: bash

    CFLAGS="-DDEBUG" meson setup builddir
    meson compile -C builddir


Optional dependencies
+++++++++++++++++++++

This package uses `check <https://libcheck.github.io/check/>`_ for unit testing and `uthash <https://troydhanson.github.io/uthash/>`_ for data structures.
It will automatically download and build both libraries, or can use system copies.
To install system copies:

On Debian-based Linux distributions:

.. code:: bash

    sudo apt install check uthash-dev

On macOS:

.. code:: bash

    brew install check uthash
