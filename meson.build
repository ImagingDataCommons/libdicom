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
check = dependency(
  'check',
  required : false,
  version : '>=0.9.6',
)

# options
cfg = configuration_data()
suffixed_version = meson.project_version()
version_suffix = get_option('version_suffix')
if version_suffix != ''
  suffixed_version += '-' + version_suffix
endif
cfg.set(
  'HAVE_CHECK',
  check.found(),
  description : 'Define if the Check package is available.',
)
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
if meson.get_compiler('c').get_argument_syntax() == 'gcc'
  add_project_arguments(
    '-Wformat-security',
    '-Wmissing-declarations',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wstrict-prototypes',
    language: 'c',
  )
endif

# libdicom
lib_sources = [
  'src/dicom.c',
  'src/dicom-data.c',
  'src/dicom-dict.c',
  'src/dicom-file.c',
]
install_headers(
  'src/dicom.h',
  subdir : 'dicom',
)
libdicom = library(
  'dicom',
  lib_sources,
  include_directories : 'lib',
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

# dcm-dump
executable(
  'dcm-dump',
  'tools/dcm-dump.c',
  include_directories : 'src',
  install : true,
  link_with : libdicom,
)
dcm_dump_man = configure_file(
  input : 'tools/dcm-dump.1.in',
  output : 'dcm-dump.1',
  configuration : {
    'SUFFIXED_VERSION': suffixed_version,
  }
)
install_man(dcm_dump_man)

# tests
check_dicom = executable(
  'check_dicom',
  'tests/check_dicom.c',
  dependencies : check,
  include_directories : 'src',
  link_with : libdicom,
)
test('check_dicom', check_dicom)
