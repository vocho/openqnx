## Process this file with automake to produce Makefile.in
noinst_LIBRARIES = libsed.a
noinst_HEADERS = regex-gnu.h getopt.h

libsed_a_SOURCES = getopt1.c getopt.c
EXTRA_DIST = memmove.c regex.c strerror.c

libsed_a_LIBADD = @LIBOBJS@ @ALLOCA@
libsed_a_DEPENDENCIES = $(libsed_a_LIBADD)
