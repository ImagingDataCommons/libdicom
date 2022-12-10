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

    sudo apt install build-essential meson pkg-config

On macOS:

.. code:: bash

    brew install meson pkg-config


Build configuration
+++++++++++++++++++

Build for development and debugging:

.. code:: bash

    CFLAGS="-DDEBUG" meson setup builddir
    meson compile -C builddir


Optional dependencies
+++++++++++++++++++++

This package uses `check <https://libcheck.github.io/check/>`_ for unit testing.

On Debian-based Linux distributions:

.. code:: bash

    sudo apt install check

On macOS:

.. code:: bash

    brew install check
