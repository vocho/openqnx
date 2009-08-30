## Process this file with automake to produce Makefile.in
bin_PROGRAMS = sed

sed_SOURCES = sed.c compile.c execute.c utils.c
noinst_HEADERS = basicdefs.h regex-sed.h sed.h utils.h

INCLUDES = -I$(top_srcdir)/lib

LDADD = ../lib/libsed.a

$(PROGRAMS): $(LDADD)
