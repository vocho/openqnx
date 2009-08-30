rmtdir=$(DEFAULT_RMT_DIR)
rmt_PROGRAMS = @PU_RMT_PROG@
EXTRA_PROGRAMS = rmt

rmt_SOURCES = rmt.c

INCLUDES = -I$(top_srcdir)/lib -I../ -I../lib

LDADD = ../lib/lib$(PACKAGE).a $(LIBINTL)

rmt_LDADD = $(LDADD) $(LIB_SETSOCKOPT)

rmt.o: ../lib/configmake.h
