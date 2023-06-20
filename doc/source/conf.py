# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
from glob import glob
import os
import sys
from hawkmoth.util import readthedocs
from clang.cindex import Config as clang_config

# -- Project information -----------------------------------------------------

project = 'libdicom'
copyright = '2021, Markus D. Herrmann'
author = 'Markus D. Herrmann'


# -- General configuration ---------------------------------------------------

primary_domain = 'c'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'hawkmoth',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# -- Hawkmoth extension ------------------------------------------------------

cautodoc_root = os.path.abspath('../../include')
readthedocs.clang_setup()
if sys.platform == 'darwin':
    lib_search_dirs = [
        '/usr/lib',
        '/usr/local/lib',
        '/Library/Developer/CommandLineTools/usr/lib',
    ]
elif sys.platform == 'windows':
    lib_search_dirs = []
else:
    lib_search_dirs = [
        '/usr/lib',
        '/usr/local/lib',
    ] + glob('/usr/lib/llvm-*/lib')
for lib_dir in lib_search_dirs:
    clang_config.set_library_path(lib_dir)
