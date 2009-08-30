#------------------------------------------------------------------------------
# Makefile for UnZip 5.5 and later                       Mark Wright and others
# Version:  Watcom C                                                  10 Feb 01
#------------------------------------------------------------------------------


# WARNING:  this is a hacked-up version of an ancient (1993) makefile.  It will
#   not work without modifications to the UnZip 5.3 sources.  This makefile is
#   (for now) included only for completeness and as a starting point for a real
#   Novell Netware NLM port.  (This makefile was intended for Netware 3.11.)


# Commands to execute before making any target
# Set environment variables for compiler
.BEFORE
    @set inc386=\watcom\novh
    @set wcg386=\watcom\binp\wcl386.exe

# Macro definitions
NLMNAME = unzip
DESCRIPTION = unzip utility
VERSION = 5.5.0
COPYRIGHT = Copyright 1990-2001 Info-ZIP (Zip-Bugs@lists.wku.edu).
SCREENNAME = Info-ZIP's UnZip Utility
CLIBIMP = \watcom\novi\clib.imp
OBJFILE = $NLMNAME.obj
PRELUDE = \watcom\novi\prelude.obj

# Compile switches
# d2    include full symbolic debugging information
# 5s    generate 586 instructions, use stack-based argument-passing conventions
# zdp   allows DS register to "peg" it to DGROUP
# zq    "quiet" mode
# NLM   produce Novell Loadable Module
# DEBUG include debug info

CC = wcc386
# COMPILE = wcc386 -zq -d2 -3s -zdp -w4 -DNLM
# COMPILE = wcc386 -zq -d2 -5s -zdp -w4 -DNLM $(LOCAL_UNZIP)
COMPILE = $(CC) -zq -olax -5s -zp1 -ei -ez -ri -w4 -DNLM -DN_PLAT_NLM -U_WIN32 $(LOCAL_UNZIP)
LINK = wlink
DESTDIR = target

# All .obj files implicitly depend on .c files
.c.obj :
   @echo Compiling $[*.c
   @$COMPILE $[*.c


UNZIP_H = unzip.h unzpriv.h globals.h novell/nlmcfg.h

crc32.obj:      crc32.c $(UNZIP_H) zip.h
crctab.obj:     crctab.c $(UNZIP_H) zip.h
crypt.obj:      crypt.c $(UNZIP_H) crypt.h ttyio.h zip.h
envargs.obj:    envargs.c $(UNZIP_H)
explode.obj:    explode.c $(UNZIP_H)
extract.obj:    extract.c $(UNZIP_H) crypt.h
fileio.obj:     fileio.c $(UNZIP_H) crypt.h ttyio.h ebcdic.h
globals.obj:    globals.c $(UNZIP_H)
inflate.obj:    inflate.c inflate.h $(UNZIP_H)
list.obj:       list.c $(UNZIP_H)
match.obj:      match.c $(UNZIP_H)
process.obj:    process.c $(UNZIP_H)
ttyio.obj:      ttyio.c $(UNZIP_H) crypt.h ttyio.h zip.h
unreduce.obj:   unreduce.c $(UNZIP_H)
unshrink.obj:   unshrink.c $(UNZIP_H)
unzip.obj:      unzip.c $(UNZIP_H) crypt.h unzvers.h consts.h
zipinfo.obj:    zipinfo.c $(UNZIP_H)

# individual dependencies and action rules:
#crc_i86.obj:    msdos\crc_i86.asm
#	$(AS) $(ASFLAGS) -D$(ASUNMODEL) msdos\crc_i86.asm, $@;

novell.obj:     novell/novell.c $(UNZIP_H)
	$(CC) -c -A$(UNMODEL) $(CFLAGS) novell/novell.c


OBJ01 = unzip.obj
OBJ02 = crc32.obj
OBJ03 = crctab.obj
OBJ04 = crypt.obj
OBJ05 = envargs.obj
OBJ06 = explode.obj
OBJ07 = extract.obj
OBJ08 = fileio.obj
OBJ09 = globals.obj
OBJ10 = inflate.obj
OBJ11 = list.obj
OBJ12 = match.obj
OBJ13 = process.obj
OBJ14 = ttyio.obj
OBJ15 = unreduce.obj
OBJ16 = unshrink.obj
OBJ17 = zipinfo.obj
OBJ18 = novell.obj
#OBJ19 = $(ASMOBJS)
OBJS = $OBJFILE $OBJ01 $OBJ02 $OBJ03 $OBJ04 $OBJ05 $OBJ06 $OBJ07 $OBJ08 \
	$OBJ09 $OBJ10 $OBJ11 $OBJ12 $OBJ13 $OBJ14 $OBJ15 $OBJ16 $OBJ17 \
	$OBJ18


# if .obj or .lnk files are modified, link new .nlm and maybe copy to DESTDIR
$NLMNAME.nlm : $OBJS
   @echo Linking...
   @$LINK @$NLMNAME
#   @echo Copying $[*.nlm to $DESTDIR
#   @copy $NLMNAME.nlm $DESTDIR


# if makefile is modified, create new linker option file
$NLMNAME.lnk : $NLMNAME.mak
   @echo FORMAT   NOVELL NLM	'$DESCRIPTION'	 >$NLMNAME.lnk
   @echo OPTION   THREADNAME    '$NLMNAME'	>>$NLMNAME.lnk
   @echo OPTION   SCREENNAME '$SCREENNAME'	>>$NLMNAME.lnk
   @echo NAME $NLMNAME				>>$NLMNAME.lnk
   @echo OPTION   VERSION=$VERSION		>>$NLMNAME.lnk
   @echo OPTION   COPYRIGHT '$COPYRIGHT'	>>$NLMNAME.lnk
   @echo DEBUG    NOVELL                        >>$NLMNAME.lnk
   @echo DEBUG    ALL                           >>$NLMNAME.lnk
   @echo OPTION   NODEFAULTLIBS			>>$NLMNAME.lnk
   @echo OPTION   DOSSEG			>>$NLMNAME.lnk
   @echo OPTION   STACK=40000			>>$NLMNAME.lnk
   @echo OPTION   CASEEXACT			>>$NLMNAME.lnk
   @echo OPTION   PSEUDOPREEMPTION		>>$NLMNAME.lnk
   @echo OPTION   MAP				>>$NLMNAME.lnk
   @echo FILE $PRELUDE				>>$NLMNAME.lnk
   @echo FILE $OBJFILE				>>$NLMNAME.lnk
   @echo FILE $OBJ01				>>$NLMNAME.lnk
   @echo FILE $OBJ02				>>$NLMNAME.lnk
   @echo FILE $OBJ03				>>$NLMNAME.lnk
   @echo FILE $OBJ04				>>$NLMNAME.lnk
   @echo FILE $OBJ05				>>$NLMNAME.lnk
   @echo FILE $OBJ06				>>$NLMNAME.lnk
   @echo FILE $OBJ07				>>$NLMNAME.lnk
   @echo FILE $OBJ08				>>$NLMNAME.lnk
   @echo FILE $OBJ09				>>$NLMNAME.lnk
   @echo FILE $OBJ10				>>$NLMNAME.lnk
   @echo FILE $OBJ11				>>$NLMNAME.lnk
   @echo FILE $OBJ12				>>$NLMNAME.lnk
   @echo FILE $OBJ13				>>$NLMNAME.lnk
   @echo FILE $OBJ14				>>$NLMNAME.lnk
   @echo FILE $OBJ15				>>$NLMNAME.lnk
   @echo FILE $OBJ16				>>$NLMNAME.lnk
   @echo FILE $OBJ17				>>$NLMNAME.lnk
   @echo FILE $OBJ18				>>$NLMNAME.lnk
   @echo MODULE   clib				>>$NLMNAME.lnk
   @echo IMPORT   @$CLIBIMP			>>$NLMNAME.lnk
