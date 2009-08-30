# Makefile for Zip, ZipCloak, ZipNote and ZipSplit for
# THEOS ANSI C
# To use, do "make -f theos/makefile"
# This make file uses cl, a C compiler and linker driver written by
# Jean-Michel Dubois. Send a mail to jmdubois@ibcfrance.fr to get it for free.
# MAINWA_BUG    Workaround argument expansion failure
# LOCATE_BUG    Workaround stat, fopen and open failure on relative paths to
#               root dir.

CC=cl
CFLAGS=-Zi -W3 -DDYN_ALLOC -DCRYPT -DMAINWA_BUG -DLOCATE_BUG
LD=cl -o
LDFLAGS=-m -Zi
AS=cc
ASFLAGS=

UTILFLAGS=-DUTIL $(CFLAGS) -Fo

# variables

# object file lists
OBJZ = zip.o zipfile.o zipup.o fileio.o util.o globals.o crypt.o ttyio.o \
       theos.o crc32.o crctab.o _fprintf.o _stat.o _chmod.o _isatty.o \
       _setargv.o _rename.o
OBJI = deflate.o trees.o
OBJA =
OBJU = zipfile_.o fileio_.o util_.o globals.o theos_.o _rename.o _stat.o \
       _chmod.o _fprintf.o
OBJN = zipnote.o  $(OBJU)
OBJC = zipcloak.o $(OBJU) crctab.o crypt_.o ttyio.o
OBJS = zipsplit.o $(OBJU)

ZIPS = zip.command zipnote.command zipsplit.command zipcloak.command

zips:	$(ZIPS)

zip.o:	zip.h theos/osdep.h ziperr.h tailor.h ttyio.h revision.h zip.c
	$(CC) -c $(CFLAGS) $*.c

zipfile.o:	zip.h theos/osdep.h ziperr.h tailor.h zipfile.c
	$(CC) -c $(CFLAGS) $*.c

zipup.o:	zip.h theos/osdep.h ziperr.h tailor.h revision.h zipup.c
	$(CC) -c $(CFLAGS) $*.c

fileio.o:	zip.h theos/osdep.h ziperr.h tailor.h fileio.c
	$(CC) -c $(CFLAGS) $*.c

util.o:	zip.h theos/osdep.h theos/charconv.h ziperr.h tailor.h util.c
	$(CC) -c $(CFLAGS) $*.c

globals.o:	zip.h theos/osdep.h ziperr.h tailor.h globals.c
	$(CC) -c $(CFLAGS) $*.c

crc32.o:	zip.h theos/osdep.h ziperr.h tailor.h crc32.c
	$(CC) -c $(CFLAGS) $*.c

crctab.o:	zip.h theos/osdep.h ziperr.h tailor.h crctab.c
	$(CC) -c $(CFLAGS) $*.c

deflate.o:	zip.h theos/osdep.h ziperr.h tailor.h deflate.c
	$(CC) -c $(CFLAGS) $*.c

trees.o:	zip.h theos/osdep.h ziperr.h tailor.h trees.c
	$(CC) -c $(CFLAGS) $*.c

crypt.o:	zip.h theos/osdep.h ziperr.h tailor.h crypt.c
	$(CC) -c $(CFLAGS) $*.c

theos.o:	zip.h theos/osdep.h ziperr.h tailor.h theos/theos.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/theos.c

_fprintf.o:	theos/_fprintf.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_fprintf.c

_stat.o:	theos/_stat.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_stat.c

_chmod.o:	theos/_chmod.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_chmod.c

_isatty.o:	theos/_isatty.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_isatty.c

_rename.o:	theos/_rename.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_rename.c

_setargv.o:	theos/_setargv.c
	$(CC) -c $(CFLAGS) -Fo$@ theos/_setargv.c

ttyio.o:	zip.h theos/osdep.h ziperr.h tailor.h ttyio.h ttyio.c
	$(CC) -c $(CFLAGS) $*.c

zipcloak.o:	zip.h theos/osdep.h ziperr.h tailor.h ttyio.h revision.h zipcloak.c
	$(CC) -c $(CFLAGS) $*.c

zipnote.o:	zip.h theos/osdep.h ziperr.h tailor.h revision.h zipnote.c
	$(CC) -c $(CFLAGS) $*.c

zipsplit.o:	zipsplit.c zip.h theos/osdep.h ziperr.h tailor.h revision.h
	$(CC) -c $(CFLAGS) $*.c

zipfile_.o:	zipfile.c zip.h theos/osdep.h ziperr.h tailor.h
	$(CC) -c $(UTILFLAGS)$@ zipfile.c

fileio_.o:	fileio.c zip.h theos/osdep.h ziperr.h tailor.h
	$(CC) -c $(UTILFLAGS)$@ fileio.c

theos_.o:	zip.h theos/osdep.h ziperr.h tailor.h theos/theos.c
	$(CC) -c $(UTILFLAGS)$@ theos/theos.c

util_.o:	util.c zip.h theos/osdep.h ziperr.h tailor.h
	$(CC) -c $(UTILFLAGS)$@ util.c

crypt_.o:	zip.h theos/osdep.h ziperr.h tailor.h crypt.c
	$(CC) -c $(UTILFLAGS)$@ crypt.c

zip.command: $(OBJZ) $(OBJI)
	$(LD) $@ $(OBJZ) $(OBJI) $(LDFLAGS)

zipcloak.command: $(OBJC)
	$(LD) $@ $(OBJC) $(LDFLAGS)

zipnote.command: $(OBJN)
	$(LD) $@ $(OBJN) $(LDFLAGS)

zipsplit.command: $(OBJS)
	$(LD) $@ $(OBJS) $(LDFLAGS)

install:	$(ZIPS)
	copy *.command /system.cmd32.=(rep noq not

clean:
	erase *.o(noq not
	erase *.command(noq not
