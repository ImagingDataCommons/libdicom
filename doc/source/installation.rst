Installation
------------

Build the library and executables using `GNU Autotools <https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html>`_ (`GNU Autoconf <https://www.gnu.org/software/autoconf/>`_ and `GNU Automake <https://www.gnu.org/software/automake/>`_):

.. code:: bash

    ./configure
    make
    make install

Optional dependencies
+++++++++++++++++++++

This package uses [check](https://libcheck.github.io/check/) for unit testing.

On Ubuntu:

.. code:: bash

    sudo apt install check

On Mac OSX:

.. code:: bash

    brew install check

Build dependencies
++++++++++++++++++

When building directly from the source code in the git repository, first install
autoconf, automake, libtool, and pkg-config.

On Mac OSX:

.. code:: bash

    brew install autoconf automake libtool

On Debian-based Linux:

.. code:: bash

    sudo apt install autogen dh-autoreconf build-essential libtool pkg-config


Then run

.. code:: bash

    ./autogen.sh
    make
    make install


Build configuration
+++++++++++++++++++

Build for development and debugging:

.. code:: bash

    CFLAGS="-O0 -DDEBUG" ./autogen.sh --prefix=/usr/local

Build for production release (the ``NDEBUG`` flag disables ``assert`` macros):

.. code:: bash

    CFLAGS="-O3 -DNDEBUG" ./autogen.sh
