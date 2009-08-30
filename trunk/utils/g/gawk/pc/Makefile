# Makefile for gawk (GNU awk)                                   31 Oct 1998
#
# - for GNU C (djgpp)         [32bit executable for DOS]
# - for GNU C (emx)           [32bit executable for OS/2 or DOS or Win32]
# - for GNU C (mingw32)	      [Win32 executable for Windows 9x/NT]
# - for MS-Visual C/C++ 4.x   [Win32 executable for Windows 9x/NT]
# - for Microsoft C 7         [16bit ececutable for DOS]
# - for Microsoft C 6.00A     [16bit executable for OS/2 or DOS]

# Tested with GNU make and dmake-3.8 under OS/2 and DOS, and ndmake and
# Microsoft nmake under DOS.  Compiling with dmake under DOS may require the 
# DOS-only version of dmake (so that swapping works).  nmake requires a
# few edits in the configuration section below.

default:
	@echo "Enter $(MAK) target "
	@echo " where 'target' is chosen from                          " 
	@echo "  djgpp ... DOS 32-bit exe [GNU C, Delorie, v2]         "
	@echo "  djgppv1 . DOS 32-bit exe [GNU C, Delorie, v1]         "
	@echo "  emx ..... OS/2 32-bit exe [emx/gcc; uses emxlibc.dll] "
	@echo "  emxnt ... NT exe [emx/gcc with RSXNT]                 "
	@echo "  emxbnd .. OS/2 and DOS 32-bit exe [emx/gcc]           "
	@echo "  mingw32 . Win32 exe [Mingw32 GNU C]                   "
	@echo "  msc ..... DOS exe [Microsoft C 7 & 8 (AKA 1.52)]      "
	@echo "  msc6 .... DOS exe [Microsoft C 6.00a]                 "
	@echo "  msc6os2 . OS/2 exe [Microsoft C 6.00a]                "
	@echo "  msc6bnd . OS/2 and DOS exe [Microsoft C 6.00a]        "
	@echo "  vcWin32 . Win32 exe [Microsoft Visual C]              "
	@echo "  ----------------------------------------------------- "
	@echo "  test .... Perform tests (see README_d/README.pc)      "
	@echo "  install . Install gawk under $(prefix)/               "

# Support dropped in 3.0
# - for Microsoft C 5.1       [16bit executable for OS/2 or DOS]
#	@echo "  msc51     DOS exe [Microsoft C 5.1]                   "
#	@echo "  msc51bnd  OS/2 and DOS exe [Microsoft C 5.1]          "

#======================= Configuration ==================================
RSPFILE = gawk.rsp
#
# Choose method for passing arguments to the linker.
#
# If compiling under OS/2 or if make can pass long lines
#LDRSP = $(GAWKOBJS)
#LNKRSP = $(LDRSP)
#
# else if make == dmake
# Response files for linker: dmake allows the macro expansion
#   $(macro_name:modifier_list:modifier_list:...)
# The macro mktmp creates a temporary file for the linker. 
# The 't' modifier is for tokenization. 
#LDRSP =   @$(mktmp $(<:t"\n"))
#LNKRSP = @$(mktmp $(<:t"+\n") ) # Space before final paren req
#
# else use brain-dead approach (emxbnd will need 'tr').
RSP = $(RSPFILE)
LDRSP = @$(RSP)
LNKRSP = $(LDRSP)
#------------------------------------------------------------------------
# Some makes do not define MAKE (and ndmake does not allow a define).
# Define MAK to be your make command.
#MAKE = dmake
MAK = $(MAKE) $(MAKEFILE)
#MAK = $(MAKE)
#MAKEFILE = -f Makefile
#MAK = make45 $(MAKEFILE)
#------------------------------------------------------------------------
# Define the base directory for the install. "make install" will install
# in bin, lib/awk, man, and info under $(prefix)/. Most likely, you should
# edit config.h so that $(prefix)/lib/awk appears as part of DEFPATH.
#prefix =
prefix = c:/gnu
#
# Define the install method. Method 1 is Unix-like (and requires cat,
# cp, mkdir, sed, and sh); method 2 uses gawk and batch files.
install = 1
#------------------------------------------------------------------------
# To work around command-line length problems, this makefile assumes
# that $($X) can be expanded.  If using a make (such as nmake) which 
# cannot handle such macros, define DO_LNK and DO_BIND for your target 
# as $(L<target>) and $(B<target>), resp.; e.g.,
#DO_LNK = $(LvcWin32)
#DO_BIND= $(BvcWin32)
# and then comment the following:
DO_LNK = $($(LNK))
DO_BIND= $($(BIND))
#========================================================================
# End of general configuration. Some platform-specific configuration
# notes appear below.

#========================================================================
#========================== DJGPP =======================================
#========================================================================

LDJG = $(CC) $(LF) -o gawk.exe $(LDRSP) $(LF2)
BDJG = stubify -g awk.exe | stubedit awk.exe runfile=gawk

djgpp:
	$(MAK) all \
	CC=gcc O=.o CF=-O2 \
	LNK=LDJG LF=-s LF2=-lm \
	BIND=BDJG

djgpp-debug:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -g' \
	LNK=LDJG LF2=-lm \
	BIND=BDJG

LDJGv1 = $(CC) $(LF) -o gawk $(LDRSP) $(LF2)
#BDJGv1 = coff2exe -s /djgpp/bin/go32.exe gawk
BDJGv1 = coff2exe gawk

djgppv1:
	$(MAK) all \
	CC=gcc O=.o CF=-O \
	LNK=LDJGv1 LF=-s LF2=-lm \
	BIND=BDJGv1

#========================================================================
#========================== EMX =========================================
#========================================================================

# Link command for OS/2 versions.
LEMX = $(CC) $(LF) -o $@ $(GAWKOBJS) gawk.def -lbsd $(LF2)

# Link and bind for DOS and OS/2 versions.
# emx-09 needs '-p' emx option here or in EMXOPT environ var.
# The following works with 0.9a or newer
LEMXBND = $(CC) $(LF) -o gawk $(LDRSP) gawk.def -lbsd $(LF2)
BEMX = emxbind -bs gawk  -p
# The following works with 0.9c or newer
#LEMXBND = $(CC) $(LF) -o a.out $(LDRSP) gawk.def -lbsd $(LF2)
#BEMX = emxbind -bs -o $@ a.out -p
#BEMX = emxbind -bs /emx/bin/emx.exe a.out $@ -p
BEMXD = emxbind -b -o $@ a.out -p

emx:
	$(MAK) all \
	"CC=gcc -Zomf" O=.obj "CF=-O -DOS2" \
	LNK=LEMX "LF=-s -Zcrtdll -Zstack 512" RSP=

emxnt:
	$(MAK) all \
	"CC=gcc -Zwin32 -Zcrtdll=rsxntcs" O=.o "CF=-O -DOS2" \
	LNK=LEMX "LF=-s -Zstack 512" RSP=

emxbnd:
	$(MAK) all \
	CC=gcc O=.o "CF=-O -DOS2 -DMSDOS" OBJ=popen.o \
	LNK=LEMXBND \
	BIND=BEMX "P=|tr \" \" \"\n\""

emxbnd-debug:
	$(MAK) all \
	CC=gcc O=.o "CF=-g -DOS2 -DMSDOS" OBJ=popen.o \
	LNK=LEMXBND \
	BIND=BEMXD "P=|tr \" \" \"\n\""

#========================================================================
#========================== MINGW32 =====================================
#========================================================================

LMINGW32 = $(CC) $(LF) -o $@ $(GAWKOBJS) $(LF2)
# The following might work around command-line length limitations:
#LMINGW32 = $(CC) $(LF) -o $@ *.o $(LF2)

mingw32:
	$(MAK) all \
	CC=gcc O=.o CF=-O OBJ=popen.o \
	LNK=LMINGW32 LF=-s RSP=

#========================================================================
#========================== MSC =========================================
#========================================================================

# stdargv, glob, and director are from Stewartson's sh. These provide
# globbing and enhanced argument-passing. MSC setargv.obj is a 
# more limited alternative (and it will permit a bound version).
#STDARGV = stdargv.obj glob.obj director.obj 
STDARGV = setargv.obj

# Optimization and library options:
# Os == optimize for size, Ot == optimize for speed, G2 == 286 or better
#MSCOPT = -Os -G2
MSCOPT = -Ot # -G2
# Alternate lib, does not use math coprocessor.
#MSCLIB = llibca
#MSCCL = -FPa
# Emulator lib, uses math coprocessor if present.
MSCLIB = llibce
MSCCL = -FPi
#MSCCL = -FPc

LMSC = link $(LF) $(LNKRSP) $(STDARGV)/NOE,$@,,/NOD:llibce $(MSCLIB)$(LF2)/STACK:0x4990,nul

# CLMSC-linking works when building under OS/2
CLMSC = $(CC) -o $@ $(LF) $(GAWKOBJS) $(STDARGV) $(LF2) -link /NOE/NOI/STACK:0x6f00

BMSC = bind $@ /n DOSMAKEPIPE DOSCWAIT


# Ugly hack: config.h defines __STDC__ if not defined on command-line.
# OS/2 versions can't use -Za in getid.c. MSC7 uses stub headers in pc/ 
# due to ANSI conflicts. MSC 5.1 defines __STDC__=0 regardless of ANSI flag.

# dmake-3.8 runs out of memory under DOS. Request that dmake
# swap itself out on these targets. Note that this won't have
# any affect on the bound OS/2 and DOS version of dmake-3.8.

.SWAP: msc msc-debug msc6 msc6os2 msc6bnd msc51 check

msc: 
	$(MAK) all \
	"CC=cl -nologo $(MSCCL)" O=.obj "CF=-AL -Ze -Ipc/include $(MSCOPT)" \
	OBJ=popen.obj \
	LNK=LMSC P=+
Lmsc = $(LMSC)  # for broken makes (nmake) which cannot expand $($X)
Bmsc =

msc-debug: 
	$(MAK) all \
	"CC=cl $(MSCCL)" O=.obj "CF=-AL -Ze -Ipc/include -W2 -Zi -Od" \
	OBJ=popen.obj \
	LNK=LMSC LF2=/CO P=+

msc6: 
	$(MAK) builtin.obj \
	"CC=cl -nologo $(MSCCL)" O=.obj "CF=-AL -Za $(MSCOPT) -Od"
	$(MAK) all \
	"CC=cl -nologo $(MSCCL)" O=.obj "CF=-AL -Za $(MSCOPT)" \
	OBJ=popen.obj \
	LNK=LMSC P=+
Lmsc6 = $(LMSC)  # for broken makes (nmake) which cannot expand $($X)
Bmsc6 =

msc6os2: 
	$(MAK) builtin.obj \
	"CC=cl -nologo $(MSCCL)" O=.obj "CF=-AL -DOS2 -UMSDOS $(MSCOPT) -Od"
	$(MAK) all \
	"CC=cl $(MSCCL)" O=.obj "CF=-AL -DOS2 -UMSDOS $(MSCOPT)" \
	LNK=LMSC "LF2=p,gawk.def" P=+

msc6bnd:
	$(MAK) builtin.obj \
	"CC=cl -nologo $(MSCCL)" O=.obj "CF=-AL -DOS2 $(MSCOPT) -Od"
	$(MAK) all \
	"CC=cl $(MSCCL)" O=.obj "CF=-AL -DOS2 $(MSCOPT)" \
	OBJ=popen.obj \
	LNK=LMSC "LF2=p,gawk.def" P=+ \
	BIND=BMSC
Lmsc6bnd = $(LMSC)  # for broken makes (nmake) which cannot expand $($X)
Bmsc6bnd = $(BMSC)


# Windows '9x / NT 
LvcWin32 = link -nologo -subsystem:console -release -out:$@ $(LNKRSP)

vcWin32:
	$(MAK) all \
	"CC=cl -nologo" O=.obj "CF=-o2 -DWIN32 -D__STDC__=0" \
	OBJ=popen.obj \
	LNK=LvcWin32


# Support dropped in 3.0
#msc51: 
#	$(MAK) all \
#	"CC=cl $(MSCCL)" O=.obj "CF=-AL -Za -D_MSC_VER=510 $(MSCOPT)" \
#	OBJ=popen.obj \
#	LNK=LMSC P=+
#
#msc51bnd:
#	$(MAK) all \
#	"CC=cl -AL ($MSCCL)" O=.obj "CF=-DOS2 -D_MSC_VER=510 $(MSCOPT)" \
#	OBJ=popen.obj \
#	LNK=CLMSC "LF=-Lp -Fb" "LF2=gawk.def"

#========================================================================

# Define BIND for BINDless compiles, otherwise $($(BIND)) may break.
BIND = EMPTY
EMPTY=

# bitwise operations (-DBITOPS) and non-decimal input data (-DNONDECDATA) are
# undocumented in 3.0.3.  They may be enabled in config.h, or added to CFLAGS.
CFLAGS = $(CF) -DGAWK -I. -DHAVE_CONFIG_H

# object files
AWKOBJS1 = array$O builtin$O eval$O field$O gawkmisc$O io$O main$O
AWKOBJS2 = missing$O msg$O node$O re$O version$O
AWKOBJS = $(AWKOBJS1) $(AWKOBJS2)

ALLOBJS = $(AWKOBJS) awktab$O getid$O $(OBJ)

# LIBOBJS
#       GNU and other stuff that gawk uses as library routines.
LIBOBJS= getopt$O getopt1$O regex$O dfa$O random$O

GAWKOBJS = $(ALLOBJS) $(LIBOBJS)

# clear out suffixes list
# .SUFFIXES:
.SUFFIXES: .c $O

.c$O:
	$(CC) -c $(CFLAGS) $<

# rules to build gawk
all : gawk.exe

gawk.exe:: $(GAWKOBJS) $(RSP)
	$(DO_LNK)
	$(DO_BIND)

$(RSPFILE) : $(GAWKOBJS)
	echo $(AWKOBJS1)$P > $@
	echo $(AWKOBJS2)$P >> $@
	echo awktab$O getid$O $(OBJ) $(LIBOBJS)$P >> $@

$(ALLOBJS):	awk.h dfa.h regex.h config.h

gawkmisc$O:	pc/gawkmisc.pc

getopt$O:	getopt.h

getopt1$O:	getopt.h

main$O:	patchlevel.h

# A bug in ndmake requires the following rule
awktab$O: awk.h awktab.c
	$(CC) -c $(CFLAGS) awktab.c

awktab.c:	awk.y
	bison -o $@ awk.y

alloca$O:	alloca.c


install: install$(install)

install1: 
	echo extproc sh $(prefix)/bin/igawk.cmd > igawk.cmd
	echo shift >> igawk.cmd
	cat pc/awklib/igawk >> igawk.cmd
	sed "s;igawk;$(prefix)/bin/igawk;" pc/awklib/igawk.bat > igawk.bat
	sh mkinstal.sh $(prefix)/bin
	sh mkinstal.sh $(prefix)/lib/awk $(prefix)/man/man1 $(prefix)/info
	cp *awk.exe igawk.bat igawk.cmd pc/awklib/igawk $(prefix)/bin
	cp awklib/eg/lib/* pc/awklib/igawk.awk $(prefix)/lib/awk
	cp doc/*.1 $(prefix)/man/man1
	cp doc/gawk.info $(prefix)/info

# install2 is equivalent to install1, but doesn't require cp, sed, etc.
install2:
	gawk -v prefix=$(prefix) -f install.awk

clean:
	rm -rf gawk gawk.exe gawk.map *.o *.obj core a.out $(RSPFILE)
#	cd doc && $(MAKE) clean
#	cd test && $(MAKE) clean
#	cd awklib && $(MAKE) clean

awklib/eg: doc/gawk.texi
	rm -fr awklib/eg
	sh -c "cd awklib && ../gawk -f extract.awk ../doc/gawk.texi"

check:
	@echo "Running the tests requires several unix-like utilities.  The"
	@echo "recommendation is to copy pc/Makefile.tst to test/Makefile.  Under"
	@echo "DOS, it may be necessary to run make from the test directory."
# The `-k' option to make should be unnecessary if using pc/Makefile.tst.
	sh -c "cd test && $(MAK) -k AWK=../gawk.exe"
#	sh -c "cd test && $(MAK) AWK=../gawk.exe bigtest extra"

test:	check

# for those who have the necessary tools:
TAGS:
	etags awk.h *.y custom.h *.c *.h

tags:
	ctags awk.h *.y custom.h *.c *.h
