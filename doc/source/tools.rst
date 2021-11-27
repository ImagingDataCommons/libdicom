Command line tools
------------------

dcm-dump
++++++++

The ``dcm-dump`` command line tool reads the metadata of a DICOM Data Set stored in a DICOM Part10 file and prints the metadata to standard output:

.. code:: bash

   dcm-dump /path/to/file.dcm | grep -e Modality -e ImageType


Refer to the man page of the tool for further instructions:

.. code:: bash

    man dcm-dump
