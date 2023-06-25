Command line tools
------------------

dcm-dump
++++++++

The ``dcm-dump`` command line tool reads the metadata of a DICOM Data Set
stored in a DICOM Part10 file and prints the metadata to standard output:

.. code:: bash

   dcm-dump /path/to/file.dcm | grep -e Modality -e ImageType

Refer to the man page of the tool for further instructions:

.. code:: bash

    man dcm-dump

dcm-getframe
++++++++++++

The ``dcm-getframe`` command line tool will read numbered frames from a DICOM
file and print then to `stdout`. Use the ``-o`` flag to write to a file
instead. Frames are numbered from 1 in the order they appear in the PixelData
sequence.

.. code:: bash

   dcm-getframe /path/to/file.dcm 12 > x.jpg

Refer to the man page of the tool for further instructions:

.. code:: bash

    man dcm-getframe
