EXTRA_DIST = README.md
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = src/libdicom.la

src_libdicom_la_SOURCES = src/dicom.c src/dicom-data.c src/dicom-dict.c src/dicom-file.c lib/uthash.h lib/utarray.h

src_libdicom_la_CFLAGS = -g -Wall -Werror -Wextra
src_libdicom_la_CPPFLAGS = -g -Wall -Wformat -Wformat-security
src_libdicom_la_LDFLAGS = -lm

pkginclude_HEADERS =  src/dicom.h

COMMON_CPPFLAGS = -I$(top_srcdir)/src
COMMON_LDADD = src/libdicom.la

bin_PROGRAMS = tools/dcm-dump
man_MANS = tools/dcm-dump.1
tools_dcm_dump_CPPFLAGS = $(COMMON_CPPFLAGS)
tools_dcm_dump_LDADD = $(COMMON_LDADD)

TESTS = check_dicom
check_PROGRAMS = check_dicom
check_dicom_SOURCES = tests/check_dicom.c $(top_builddir)/src/dicom.h
check_dicom_CFLAGS = @CHECK_CFLAGS@
check_dicom_LDADD = $(top_builddir)/src/libdicom.la @CHECK_LIBS@

dist-hook:
	cd $(distdir)/doc; make html

docs:
	cd doc && make html

EXTRA_DIST += $(man_MANS:=.in)

