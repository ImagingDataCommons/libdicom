project(
  'libdicom',
  'c',
  default_options : [
    'buildtype=debugoptimized',
    'warning_level=2',
  ],
  license : 'MIT',
  meson_version : '>=0.54',
  version : '0.1.0',
)
if not meson.is_subproject()
  meson.add_dist_script(
    'scripts/dist.py'
  )
endif

# project version
version_parts = meson.project_version().split('.')
version_major = version_parts[0]
version_minor = version_parts[1]
version_patch = version_parts[2]

suffixed_version = meson.project_version()
if get_option('version_suffix') != ''
  suffixed_version += '-' + get_option('version_suffix')
endif

# ABI version number.
#
# rules:
#   1. Backward-incompatible ABI change: bump major, reset minor and patch
#   2. Backward-compatible ABI change: bump minor, reset patch
#   3. Other, eg. bugfix: bump patch
abi_version_major = 0
abi_version_minor = 1
abi_version_patch = 0

abi_version = '@0@.@1@.@2@'.format(
  abi_version_major,
  abi_version_minor,
  abi_version_patch,
)
darwin_library_versions = [
  abi_version_major + 1,
  '@0@.@1@'.format(abi_version_major + 1, abi_version_minor)
]

# ready to be pasted into source files
version_data = configuration_data()
version_data.set('DCM_VERSION', meson.project_version())
version_data.set('DCM_SUFFIXED_VERSION', suffixed_version)
version_data.set('DCM_VERSION_MAJOR', version_major)
version_data.set('DCM_VERSION_MINOR', version_minor)
version_data.set('DCM_VERSION_MICRO', version_patch)
version_data.set('DCM_ABI_VERSION_MAJOR', abi_version_major)
version_data.set('DCM_ABI_VERSION_MINOR', abi_version_minor)
version_data.set('DCM_ABI_VERSION_PATCH', abi_version_patch)

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
cfg.set_quoted(
  'SRCDIR',
  # escape backslashes for Windows paths without replace(), which doesn't
  # exist in 0.54
  '\\\\'.join(meson.current_source_dir().split('\\')),
  description : 'Define to the path to the source directory.',
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
  language : 'c',
)

# include
version_header = configure_file(
    input : 'include/dicom/version.h.in',
    output : 'version.h',
    configuration : version_data,
)

install_headers(
  version_header,
  'include/dicom/dicom.h',
  subdir : 'dicom',
)

# src
library_includes = include_directories('include')
library_options = ['-DBUILDING_LIBDICOM']
library_sources = [
  'src/dicom.c',
  'src/dicom-data.c',
  'src/dicom-dict.c',
  'src/dicom-file.c',
]
libdicom = library(
  'dicom',
  library_sources,
  c_args : library_options,
  dependencies : [uthash],
  version : abi_version,
  darwin_versions : darwin_library_versions,
  include_directories : library_includes,
  gnu_symbol_visibility: 'hidden',
  install : true,
)
import('pkgconfig').generate(
  libdicom,
  description : 'C library for reading DICOM files',
  filebase : 'libdicom',
  name : 'libdicom',
  url : 'https://github.com/ImagingDataCommons/libdicom',
)
libdicom_dep = declare_dependency(
  include_directories : library_includes,
  link_with : libdicom,
)
meson.override_dependency('libdicom', libdicom_dep)

# tools
executable(
  'dcm-dump',
  'tools/dcm-dump.c',
  dependencies : [libdicom_dep],
  install : true,
)
dcm_dump_man = configure_file(
  input : 'tools/dcm-dump.1.in',
  output : 'dcm-dump.1',
  configuration : version_data,
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
    'include/dicom/dicom.h',
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
