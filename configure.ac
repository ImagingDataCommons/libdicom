#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dicom], [0.1.0], [hackermd@protonmail.com], [dicom], [https://github.com/hackermd/libdicom])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src])

AM_INIT_AUTOMAKE([foreign subdir-objects 1.11.1 dist-xz])

# Check for cross build
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC([clang gcc])
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD

# Largefile
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# Optional in C99
AC_TYPE_UINTPTR_T

LT_PREREQ([2.2.5])
LT_INIT

# Suffix appended to version string
AC_ARG_WITH([version-suffix],
            AS_HELP_STRING([--with-version-suffix=STRING],
                           [suffix to append to the package version string]))
AC_MSG_CHECKING([version string])
if test "x$with_version_suffix" != "x"; then
  SUFFIXED_VERSION="$VERSION-$with_version_suffix"
else
  SUFFIXED_VERSION="$VERSION"
fi
AC_MSG_RESULT([$SUFFIXED_VERSION])
AC_DEFINE_UNQUOTED([SUFFIXED_VERSION], ["$SUFFIXED_VERSION"],
                   [Define to the package version string including any suffix.])
AC_SUBST([SUFFIXED_VERSION])

PKG_CHECK_MODULES([CHECK], [check >= 0.9.6], [
  AC_DEFINE([HAVE_CHECK], [1], [Define if Check headers are present.])
], [:])

# Mac OS X proc_pidfdinfo()
AC_MSG_CHECKING([for proc_pidfdinfo])
AC_LINK_IFELSE([
  AC_LANG_PROGRAM(
    [#include <libproc.h>],
    [proc_pidfdinfo(0, 0, PROC_PIDFDKQUEUEINFO, NULL, 0)]
  )
], [
  AC_MSG_RESULT([yes])
  AC_DEFINE([HAVE_PROC_PIDFDINFO], [1], [Define to 1 if you have the proc_pidfdinfo function.])
], [
  AC_MSG_RESULT([no])
])

# Compiler warnings
WARN_CFLAGS="-Wall -Werror -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wformat -Wformat-security"
AC_SUBST([WARN_CFLAGS])

AC_SUBST(FEATURE_FLAGS)

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_FILES([
Makefile
tools/dcm-dump.1
])
AC_OUTPUT
