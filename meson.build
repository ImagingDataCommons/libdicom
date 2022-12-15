project(
  'libdicom',
  'c',
  default_options : [
    'buildtype=debugoptimized',
    'warning_level=2',
  ],
  license : 'MIT',
  meson_version : '>=0.50',
  version : '0.1.0',
)

# dependencies
cc = meson.get_compiler('c')
found_uthash = cc.check_header(
  'utarray.h',
  required : false
) and cc.check_header(
  'uthash.h',
  required : false
)
if found_uthash
  uthash = declare_dependency()
else
  # fall back to dependency() rather than subproject() so
  # --wrap-mode=nofallback works
  uthash = dependency('uthash')
endif
check = dependency(
  'check',
  default_options : [
    'warning_level=0',
  ],
  version : '>=0.9.6',
)

# options
cfg = configuration_data()
suffixed_version = meson.project_version()
version_suffix = get_option('version_suffix')
if version_suffix != ''
  suffixed_version += '-' + version_suffix
endif
cfg.set_quoted(
  'SRCDIR',
  # escape backslashes for Windows paths without replace(), which doesn't
  # exist in 0.54
  '\\\\'.join(meson.current_source_dir().split('\\')),
  description : 'Define to the path to the source directory.',
)
cfg.set_quoted(
  'SUFFIXED_VERSION',
  suffixed_version,
  description : 'The package version string including any suffix.',
)
configure_file(
  output : 'config.h',
  configuration : cfg,
)

# compiler options
add_project_arguments(
  cc.get_supported_arguments(
    '-Wformat-security',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wstrict-prototypes',
  ),
  language: 'c',
)
library_options = ['-DBUILDING_LIBDICOM'] + cc.get_supported_arguments(
  '-fvisibility=hidden'
)

# libdicom
lib_sources = [
  'src/dicom.c',
  'src/dicom-data.c',
  'src/dicom-dict.c',
  'src/dicom-file.c',
]
install_headers(
  'include/dicom.h',
  subdir : 'dicom',
)
libdicom = library(
  'dicom',
  lib_sources,
  c_args : library_options,
  dependencies: [uthash],
  include_directories : ['include'],
  install : true,
  version : '0.0.0',
)
import('pkgconfig').generate(
  libdicom,
  description : 'C library for reading DICOM files',
  filebase : 'libdicom',
  name : 'libdicom',
  subdirs : 'dicom',
  url : 'https://github.com/hackermd/libdicom',
)
libdicom_dep = declare_dependency(
  include_directories : ['include'],
  link_with : libdicom,
)

# dcm-dump
executable(
  'dcm-dump',
  'tools/dcm-dump.c',
  dependencies : [libdicom_dep],
  install : true,
)
dcm_dump_man = configure_file(
  input : 'tools/dcm-dump.1.in',
  output : 'dcm-dump.1',
  configuration : {
    'SUFFIXED_VERSION': suffixed_version,
  }
)
install_man(dcm_dump_man)

# docs
subdir('doc/env/bin')
custom_target(
  'html',
  command : [sphinx_build, '@SOURCE_ROOT@/doc/source', '@OUTPUT@'],
  input : [
    'doc/source/api.rst',
    'doc/source/conf.py',
    'doc/source/contributing.rst',
    'doc/source/index.rst',
    'doc/source/installation.rst',
    'doc/source/introduction.rst',
    'doc/source/tools.rst',
    'doc/source/usage.rst',
    'include/dicom.h',
  ],
  output : 'html',
)

# tests
check_dicom = executable(
  'check_dicom',
  'tests/check_dicom.c',
  dependencies : [check, libdicom_dep],
)
test('check_dicom', check_dicom)
