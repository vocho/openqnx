#==============================================================================
# Makefile for UnZip, UnZipSFX and fUnZip:  Unix and MS-DOS ("real" makes only)
# Version:  5.52                                               27 February 2005
#==============================================================================


# INSTRUCTIONS (such as they are):
#
# "make vax"	-- makes UnZip on a generic Unix VAX in the current directory
# "make list"	-- lists all supported systems (targets)
# "make help"	-- provides pointers on what targets to try if problems occur
# "make wombat" -- chokes and dies if you haven't added the specifics for your
#		    Wombat 68000 (or whatever) to the systems list
#
# CF are flags for the C compiler.  LF are flags for the loader.  LF2 are more
# flags for the loader, if they need to be at the end of the line instead of at
# the beginning (for example, some libraries).  FL and FL2 are the corre-
# sponding flags for fUnZip.  LOCAL_UNZIP is an environment variable that can
# be used to add default C flags to your compile without editing the Makefile
# (e.g., -DDEBUG_STRUC, or -FPi87 on PCs using Microsoft C).
#
# Some versions of make do not define the macro "$(MAKE)"; this is rare, but
# if things don't work, try using "make" instead of "$(MAKE)" in your system's
# makerule.  Or try adding the following line to your .login file:
#	setenv MAKE "make"
# (That never works--makes which are too stupid to define MAKE are also too
# stupid to look in the environment--but try it anyway for kicks. :-) )
#
# Memcpy and memset are provided for those systems that don't have them; they
# are in file_io.c and will be used if -DZMEM is included in CF.  These days
# almost all systems have them.
#
# Be sure to test your new UnZip (and UnZipSFX and fUnZip); successful compila-
# tion does not always imply a working program.


#####################
# MACRO DEFINITIONS #
#####################

# Defaults most systems use (use LOCAL_UNZIP in environment to add flags,
# such as -DDOSWILD).

# UnZip flags
CC = cl
LD = cl
LOC = $(LOCAL_UNZIP)
CF = -c -Zi -W2 -DCRYPT -DOLD_THEOS_EXTRA $(LOCAL_UNZIP)
LF = -Zi -o
LF2 = -m

# defaults for crc32 stuff and system dependent headers
CRC32 = crc32
OSDEP_H =

# general-purpose stuff
CP = copyfile
LN = copyfile
RM = erase

.c.o:
	$(CC) $(CF) $*.c

# object files
OBJS1 = unzip.o $(CRC32).o crctab.o crypt.o envargs.o explode.o
OBJS2 = extract.o fileio.o globals.o inflate.o list.o match.o
OBJS3 = process.o ttyio.o unreduce.o unshrink.o zipinfo.o
OBJS4 = _sprintf.o _fprintf.o _isatty.o _stat.o _setargv.o
OBJS = $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) theos.o
#LOBJS = $(OBJS)
LOBJS = $(OBJS1) $(OBJS2) $(OBJS3) theos.o
OBJX = unzipsfx.o $(CRC32).o crctab_.o crypt_.o extract_.o fileio_.o \
	globals_.o inflate_.o match_.o process_.o ttyio_.o theos_.o
LOBJX = $(OBJX)
OBJF = funzip.o $(CRC32).o cryptf.o globalsf.o inflatef.o ttyiof.o _sprintf.o \
       _fprintf.o _isatty.o
UNZIP_H = unzip.h unzpriv.h globals.h $(OSDEP_H)
EBCDIC_H = ebcdic.h theos/charconv.h

UNZIPS = unzip.command funzip.command unzipsfx.command zipinfo.command

unzips:	$(UNZIPS)


crc32.o:	crc32.c $(UNZIP_H) zip.h
crctab.o:	crctab.c $(UNZIP_H) zip.h
crypt.o:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h
envargs.o:	envargs.c $(UNZIP_H)
explode.o:	explode.c $(UNZIP_H)
extract.o:	extract.c $(UNZIP_H) crypt.h
fileio.o:	fileio.c $(UNZIP_H) crypt.h ttyio.h EBCDIC_H
funzip.o:	funzip.c $(UNZIP_H) crypt.h ttyio.h tables.h
globals.o:	globals.c $(UNZIP_H)
inflate.o:	inflate.c inflate.h $(UNZIP_H)
list.o:		list.c $(UNZIP_H)
match.o:	match.c $(UNZIP_H)
process.o:	process.c $(UNZIP_H)
ttyio.o:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h
unreduce.o:	unreduce.c $(UNZIP_H)
unshrink.o:	unshrink.c $(UNZIP_H)
unzip.o:	unzip.c $(UNZIP_H) crypt.h unzvers.h consts.h
zipinfo.o:	zipinfo.c $(UNZIP_H)

crctab_.o:	crctab.c $(UNZIP_H) zip.h			# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ crctab.c

crypt_.o:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h	# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ crypt.c

extract_.o:	extract.c $(UNZIP_H) crypt.h			# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ extract.c

fileio_.o:	fileio.c $(UNZIP_H) crypt.h ttyio.h EBCDIC_H	# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ fileio.c

globals_.o:	globals.c $(UNZIP_H)				# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ globals.c

inflate_.o:	inflate.c inflate.h $(UNZIP_H) crypt.h		# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ inflate.c

match_.o:	match.c $(UNZIP_H)				# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ match.c

process_.o:	process.c $(UNZIP_H)				# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ process.c

ttyio_.o:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h	# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ ttyio.c

unzipsfx.o:	unzip.c $(UNZIP_H) crypt.h unzvers.h consts.h	# unzipsfx only
	$(CC) $(CF) -DSFX -Fo$@ unzip.c

cryptf.o:	crypt.c $(UNZIP_H) zip.h crypt.h ttyio.h	# funzip only
	$(CC) $(CF) -DFUNZIP -Fo$@ crypt.c

globalsf.o:	globals.c $(UNZIP_H)				# funzip only
	$(CC) $(CF) -DFUNZIP -Fo$@ globals.c

inflatef.o:	inflate.c inflate.h $(UNZIP_H) crypt.h		# funzip only
	$(CC) $(CF) -DFUNZIP -Fo$@ inflate.c

ttyiof.o:	ttyio.c $(UNZIP_H) zip.h crypt.h ttyio.h	# funzip only
	$(CC) $(CF) -DFUNZIP -Fo$@ ttyio.c

theos.o:	theos/theos.c $(UNZIP_H) unzvers.h		# THEOS only
	$(CC) $(CF) theos/theos.c

theos_.o:	theos/theos.c $(UNZIP_H)			# THEOS unzipsfx
	$(CC) $(CF) -DSFX -Fo$@ theos/theos.c

_sprintf.o:	theos/_sprintf.c $(UNZIP_H)			# THEOS only
	$(CC) $(CF) theos/_sprintf.c

_fprintf.o:	theos/_fprintf.c $(UNZIP_H)			# THEOS only
	$(CC) $(CF) theos/_fprintf.c

_isatty.o:	theos/_isatty.c $(UNZIP_H)			# THEOS only
	$(CC) $(CF) theos/_isatty.c

_stat.o:	theos/_stat.c $(UNZIP_H)			# THEOS only
	$(CC) $(CF) theos/_stat.c

_setargv.o:	theos/_setargv.c $(UNZIP_H)			# THEOS only
	$(CC) $(CF) theos/_setargv.c

unzip.command:	$(OBJS)
	$(LD) $(LF) $@ $(LOBJS) $(LF2)

unzipsfx.command:	$(OBJX)
	$(LD) $(LF) $@ $(LOBJX) $(LF2)

funzip.command:	$(OBJF)
	$(LD) $(LF) $@ $(OBJF) $(LF2)

zipinfo.command:	unzip.command
	$(LN) $? $@ ( rep not

install:
	copy *.command /system.cmd32.=(rep noq

clean:
	erase *.o(noq not
	erase *.command(noq not
