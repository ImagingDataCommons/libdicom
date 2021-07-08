Installation
------------

Build the library and executables using `GNU Autotools <https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html>`_ (`GNU Autoconf <https://www.gnu.org/software/autoconf/>`_ and `GNU Automake <https://www.gnu.org/software/automake/>`_):

.. code:: bash

    ./configure
    make
    make install

Build dependencies
++++++++++++++++++

When building directly from the source code in the git repository, first install
autoconf, automake, libtool, and pkg-config and then run

.. code:: bash

    autoreconf -i

This will create the ``configure`` script required for building library and executable tools.


Build configuration
+++++++++++++++++++

Build for development and debugging:

.. code:: bash

    ./configure CFLAGS="-O0 -DDEBUG"

Build for production release (the ``NDEBUG`` flag disables ``assert`` macros):

.. code:: bash

    ./configure CFLAGS="-O3 -DNDEBUG"
