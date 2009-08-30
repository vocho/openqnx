# VMS Makefile for Zip, ZipNote, ZipCloak and ZipSplit

#
#  Modified to support both AXP and VAX by Hunter Goatley, 10-SEP-1993 06:43
#  Modified (DECC VAX, Zip 2.1) by Christian Spieler, 16-SEP-1995
#  Modified (Addition of VMS CLI) by Christian Spieler, 10-OCT-1995
#  Modified (fixed VAXC, changed compiler opts) by C. Spieler, 10-DEC-1995
#  Modified (removed zipup_.obj from Zip utils) by C. Spieler, 08-JAN-1996
#  Modified (cmdline$O depends on crypt.h) by C. Spieler, 09-JAN-1996
#  Modified (split crypt -> crypt, ttyio) by C. Spieler, 16-JAN-1996
#  Modified (modified VMSCLI compilation) by C. Spieler, 25-JUL-1997
#  Modified (comment concerning online help) by C. Spieler, 14-OCT-1997
#  Modified (removed bits.c source file) by C. Spieler, 25-JUN-1998
#  Last modified (Alpha, IA64 sensing) by Steven Schweda, 30-MAY-2006
#
#  To build Zip and the Ziputils, copy this file (DESCRIP.MMS) into the
#  main directory, then use one of the following commands, depending on
#  your system.  Modern MMS and MMK versions should correctly determine
#  the system type without the "/MACRO=" qualifier, but on VAX the
#  compiler should be specified explicitly, and it should always be safe
#  to specify the proper system type.  (If you have installed both DEC C
#  and VAX C on your VAX and want to use VAX C, you should define the
#  macro "__FORCE_VAXC__" instead of "__VAXC__".)
#
#	$ MMS                                   ! Should work on non-VAX.
#
#	$ MMS/MACRO=(__ALPHA__=1)		! Alpha AXP, (DEC C).
#	$ MMS/MACRO=(__IA64__=1)		! IA64, (DEC C).
#	$ MMS/MACRO=(__DECC__=1)		! VAX, using DEC C.
#
#	$ MMS/MACRO=(__FORCE_VAXC__=1)		! VAX, using VAX C, not DEC C.
#	$ MMS/MACRO=(__VAXC__=1)		! VAX, where VAX C is default.
#	$ MMS/MACRO=(__GNUC__=1)		! VAX, using GNU C.
#
#  Other MMS macros intended for use on the MMS command line are:
#	__DEBUG__=1				! Compile for debugging.
#
#  For some discussion on the compiler options used, see documentation
#  in [.VMS]00README.VMS.
#

# Define MMK architecture macros when using MMS.

.IFDEF __MMK__                  # __MMK__
.ELSE                           # __MMK__
ALPHA_X_ALPHA = 1
IA64_X_IA64 = 1
VAX_X_VAX = 1
.IFDEF $(MMS$ARCH_NAME)_X_ALPHA     # $(MMS$ARCH_NAME)_X_ALPHA
__ALPHA__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_ALPHA
.IFDEF $(MMS$ARCH_NAME)_X_IA64      # $(MMS$ARCH_NAME)_X_IA64
__IA64__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_IA64
.IFDEF $(MMS$ARCH_NAME)_X_VAX       # $(MMS$ARCH_NAME)_X_VAX
__VAX__ = 1
.ENDIF                              # $(MMS$ARCH_NAME)_X_VAX
.ENDIF                          # __MMK__

.IFDEF __ALPHA__                # __ALPHA__
E = .AXP_EXE
O = .AXP_OBJ
A = .AXP_OLB
.ELSE                           # __ALPHA__
.IFDEF __IA64__                     # __IA64__
E = .I64_EXE
O = .I64_OBJ
A = .I64_OLB
.ELSE                               # __IA64__
.IFDEF __DECC__                         # __DECC__
E = .VAX_DECC_EXE
O = .VAX_DECC_OBJ
A = .VAX_DECC_OLB
.ENDIF                                  # __DECC__
.IFDEF __FORCE_VAXC__                   # __FORCE_VAXC__
__VAXC__ = 1
.ENDIF                                  # __FORCE_VAXC__
.IFDEF __VAXC__                         # __VAXC__
E = .VAX_VAXC_EXE
O = .VAX_VAXC_OBJ
A = .VAX_VAXC_OLB
.ENDIF                                  # __VAXC__
.IFDEF __GNUC__                         # __GNUC__
E = .VAX_GNUC_EXE
O = .VAX_GNUC_OBJ
A = .VAX_GNUC_OLB
.ENDIF                                  # __GNUC__
.ENDIF                              # __IA64__
.ENDIF                          # __ALPHA__

.IFDEF O                        # O
.ELSE                           # O
!If EXE and OBJ extensions aren't defined, define them
E = .EXE
O = .OBJ
A = .OLB
.ENDIF                          # O

!The following preprocessor macros are set to enable the VMS CLI$ interface:
CLI_DEFS = VMSCLI,

!!!!!!!!!!!!!!!!!!!!!!!!!!! USER CUSTOMIZATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!
! add any other optional preprocessor flags (macros) except VMSCLI to the
! following line for a custom version (do not forget a trailing comma!!):
COMMON_DEFS =
!
! WARNING: Do not use VMSCLI here!! The creation of a Zip executable
!          utilizing the VMS CLI$ command interface is handled differently.
!!!!!!!!!!!!!!!!!!!!!!!! END OF USER CUSTOMIZATION !!!!!!!!!!!!!!!!!!!!!!!!

.IFDEF __GNUC__
CC = gcc
LIBS = ,GNU_CC:[000000]GCCLIB.OLB/LIB
.ELSE
CC = cc
LIBS =
.ENDIF

CFLAGS = /NOLIST/INCL=(SYS$DISK:[])

OPTFILE = sys$disk:[.vms]vaxcshr.opt

.IFDEF __ALPHA__                # __ALPHA__
CFLG_ARCH = /STANDARD=RELAX/PREFIX=ALL/ANSI_ALIAS
OPTFILE_LIST =
OPTIONS = $(LIBS)
.ELSE                           # __ALPHA__
.IFDEF __IA64__                     # __IA64__
CFLG_ARCH = /STANDARD=RELAX/PREFIX=ALL/ANSI_ALIAS
OPTFILE_LIST =
OPTIONS = $(LIBS)
.ELSE                               # __IA64__
.IFDEF __DECC__                         # __DECC__
CFLG_ARCH = /DECC/STANDARD=RELAX/PREFIX=ALL
OPTFILE_LIST =
OPTIONS = $(LIBS)
.ELSE				        # __DECC__
.IFDEF __FORCE_VAXC__                       # __FORCE_VAXC__
CFLG_ARCH = /VAXC
.ELSE                                       # __FORCE_VAXC__
CFLG_ARCH =
.ENDIF                                      # __FORCE_VAXC__
OPTFILE_LIST = ,$(OPTFILE)
OPTIONS = $(LIBS),$(OPTFILE)/OPT
.ENDIF                                  # __DECC__
.ENDIF                              # __IA64__
.ENDIF                          # __ALPHA__

.IFDEF __DEBUG__
CDEB = /DEBUG/NOOPTIMIZE
LDEB = /DEBUG
.ELSE
CDEB =
LDEB = /NOTRACE
.ENDIF

CFLAGS_ALL  = $(CFLG_ARCH) $(CFLAGS) $(CDEB) -
	/def=($(COMMON_DEFS) VMS)
CFLAGS_CLI  = $(CFLG_ARCH) $(CFLAGS) $(CDEB) -
	/def=($(COMMON_DEFS) $(CLI_DEFS) VMS)
CFLAGS_UTIL = $(CFLG_ARCH) $(CFLAGS) $(CDEB) -
	/def=($(COMMON_DEFS) UTIL, VMS)

LINKFLAGS = $(LDEB)


OBJM =	zip$(O), zipcli$(O)
OBJZ =	crc32$(O), crctab$(O), crypt$(O), ttyio$(O), -
	zipfile$(O), zipup$(O), fileio$(O), globals$(O), util$(O)
OBJV =	vmszip$(O), vms$(O), vmsmunch$(O)
OBJI =	deflate$(O), trees$(O)
OBJU =	ZIPFILE=zipfile_$(O), FILEIO=fileio_$(O), globals$(O), -
	UTIL=util_$(O), VMS=vms_$(O), vmsmunch$(O)
OBJR =	crctab$(O), CRYPT=crypt_$(O), ttyio$(O)
OBJC =	zipcloak$(O)
OBJN =	zipnote$(O)
OBJS =	zipsplit$(O)

ZIPX_UNX = zip
ZIPX_CLI = zip_cli
OBJSZIPLIB = $(OBJZ), $(OBJI), $(OBJV)
OBJSZIP = zip$(O), $(OBJSZIPLIB)
OBJSCLI = ZIP=zipcli$(O), -
	ZIP_CLITABLE=zip_cli$(O), VMS_ZIP_CMDLINE=cmdline$(O)
ZIPHELP_UNX_RNH = [.vms]vms_zip.rnh
ZIPHELP_CLI_RNH = [.vms]zip_cli.rnh

OLBZIP = zip$(A)
OLBCLI = zipcli$(A)
OLBUTI = ziputils$(A)

ZIP_H =	zip.h,ziperr.h,tailor.h,[.vms]osdep.h

ZIPS =	$(ZIPX_UNX)$(E), $(ZIPX_CLI)$(E),-
	zipcloak$(E), zipnote$(E), zipsplit$(E)
ZIPHELPS = $(ZIPX_UNX).hlp, $(ZIPX_CLI).hlp

#
#  Define our new suffixes list
#
.SUFFIXES :
.SUFFIXES :	$(E) $(A) $(O) .C .MAR .CLD .HLP .RNH

$(O)$(E) :
	$(LINK) $(LINKFLAGS) /EXE=$(MMS$TARGET) $(MMS$SOURCE)

$(O)$(A) :
	If "''F$Search("$(MMS$TARGET)")'" .EQS. "" Then $(LIBR)/Create $(MMS$TARGET)
	$(LIBR)$(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

.CLD$(O) :
	SET COMMAND /OBJECT=$(MMS$TARGET) $(CLDFLAGS) $(MMS$SOURCE)

.c$(O) :
	$(CC) $(CFLAGS_ALL) /OBJ=$(MMS$TARGET) $(MMS$SOURCE)

.RNH.HLP :
	runoff /out=$@ $<


# rules for zip, zipnote, zipsplit, and VMS online help file.

default :	$(ZIPS), $(ZIPHELPS)
	@ !

vmszip$(O)   : [.vms]vmszip.c
vmsmunch$(O) : [.vms]vmsmunch.c
vms$(O)      : [.vms]vms.c [.vms]vms_im.c [.vms]vms_pk.c [.vms]vms.h
zipcli$(O)   : zip.c
	$(CC) $(CFLAGS_CLI) /OBJ=$(MMS$TARGET) $<
cmdline$(O)  : [.vms]cmdline.c $(ZIP_H) crypt.h revision.h
	$(CC) $(CFLAGS_CLI) /OBJ=$(MMS$TARGET) $<
zip_cli$(O)  : [.vms]zip_cli.cld


zipfile_$(O) :	zipfile.c,[.vms]vmsmunch.h,[.vms]vmsdefs.h
	$(CC) $(CFLAGS_UTIL) /OBJECT=$(MMS$TARGET) $<
fileio_$(O)  :	fileio.c
	$(CC) $(CFLAGS_UTIL) /OBJECT=$(MMS$TARGET) $<
util_$(O)    :	util.c
	$(CC) $(CFLAGS_UTIL) /OBJECT=$(MMS$TARGET) $<
crypt_$(O)   :	crypt.c,crypt.h,ttyio.h
	$(CC) $(CFLAGS_UTIL) /OBJECT=$(MMS$TARGET) $<
vms_$(O)     :	[.vms]vms.c,[.vms]vms_im.c,[.vms]vms_pk.c, -
		[.vms]vms.h,[.vms]vmsdefs.h
	$(CC) $(CFLAGS_UTIL) /OBJECT=$(MMS$TARGET) $<

$(OBJM),zipcloak$(O),zipnote$(O),zipsplit$(O),zipup$(O) : revision.h

$(OBJM),zipcloak$(O),zipup$(O),crypt$(O),ttyio$(O) : crypt.h

$(OBJM),zipcloak$(O),crypt$(O),ttyio$(O) : ttyio.h

zipup$(O) : [.vms]zipup.h

$(OBJM), zipfile$(O), vmszip$(O), vmsmunch$(O) : [.vms]vmsmunch.h

zipfile$(O), vms$(O), vmsmunch$(O) : [.vms]vmsdefs.h

$(OBJM) : $(ZIP_H)
$(OBJZ) : $(ZIP_H)
$(OBJV) : $(ZIP_H)
$(OBJI) : $(ZIP_H)
$(OBJU) : $(ZIP_H)
$(OBJR) : $(ZIP_H)
$(OBJC) : $(ZIP_H)
$(OBJN) : $(ZIP_H)
$(OBJS) : $(ZIP_H)


$(ZIPX_UNX)$(E) : $(OLBZIP)($(OBJSZIP))$(OPTFILE_LIST)
	$(LINK)$(LINKFLAGS) /EXE=$@ -
	$(OLBZIP)/inc=(zip,globals)/lib$(OPTIONS)

$(ZIPX_CLI)$(E) : $(OLBCLI)($(OBJSCLI)),$(OLBZIP)($(OBJSZIPLIB))$(OPTFILE_LIST)
	$(LINK)$(LINKFLAGS) /EXE=$@ -
	$(OLBCLI)/inc=(zip)/lib, $(OLBZIP)/inc=(globals)/lib$(OPTIONS)

zipcloak$(E) : $(OBJC),$(OLBUTI)($(OBJR),$(OBJU))$(OPTFILE_LIST)
	$(LINK)$(LINKFLAGS) /EXE=$@ $<, -
	$(OLBUTI)/inc=(globals)/lib$(OPTIONS)

zipnote$(E) : $(OBJN),$(OLBUTI)($(OBJU))$(OPTFILE_LIST)
	$(LINK)$(LINKFLAGS) /EXE=$@ $<, -
	$(OLBUTI)/inc=(globals)/lib$(OPTIONS)

zipsplit$(E) : $(OBJS),$(OLBUTI)($(OBJU))$(OPTFILE_LIST)
	$(LINK)$(LINKFLAGS) /EXE=$@ $<, -
	$(OLBUTI)/inc=(globals)/lib$(OPTIONS)

$(OPTFILE) :
	@ open/write tmp $(OPTFILE)
	@ write tmp "SYS$SHARE:VAXCRTL.EXE/SHARE"
	@ close tmp

$(ZIPHELP_CLI_RNH) : [.vms]zip_cli.help
	@ set default [.vms]
	edit/tpu/nosection/nodisplay/command=cvthelp.tpu zip_cli.help
	@ set default [-]

$(ZIPX_UNX).hlp : $(ZIPHELP_UNX_RNH)
	runoff /out=$@ $<

$(ZIPX_CLI).hlp : $(ZIPHELP_CLI_RNH)

clean.com :
	@ open/write tmp $(MMS$TARGET)
	@ write tmp "$!"
	@ write tmp "$!	Clean.com --	procedure to delete files. It always returns success"
	@ write tmp "$!			status despite any error or warnings. Also it extracts"
	@ write tmp "$!			filename from MMS ""module=file"" format."
	@ write tmp "$!"
	@ write tmp "$ on control_y then goto ctly"
	@ write tmp "$ if p1.eqs."""" then exit 1"
	@ write tmp "$ i = -1"
	@ write tmp "$scan_list:"
	@ write tmp "$	i = i+1"
	@ write tmp "$	item = f$elem(i,"","",p1)"
	@ write tmp "$	if item.eqs."""" then goto scan_list"
	@ write tmp "$	if item.eqs."","" then goto done		! End of list"
	@ write tmp "$	item = f$edit(item,""trim"")		! Clean of blanks"
	@ write tmp "$	wild = f$elem(1,""="",item)"
	@ write tmp "$	show sym wild"
	@ write tmp "$	if wild.eqs.""="" then wild = f$elem(0,""="",item)"
	@ write tmp "$	vers = f$parse(wild,,,""version"",""syntax_only"")"
	@ write tmp "$	if vers.eqs."";"" then wild = wild - "";"" + "";*"""
	@ write tmp "$scan:"
	@ write tmp "$		f = f$search(wild)"
	@ write tmp "$		if f.eqs."""" then goto scan_list"
	@ write tmp "$		on error then goto err"
	@ write tmp "$		on warning then goto warn"
	@ write tmp "$		delete/log 'f'"
	@ write tmp "$warn:"
	@ write tmp "$err:"
	@ write tmp "$		goto scan"
	@ write tmp "$done:"
	@ write tmp "$ctly:"
	@ write tmp "$	exit 1"
	@ close tmp

clean : clean.com
	@clean "$(OBJM)"
	@clean "$(OBJZ)"
	@clean "$(OBJI)"
	@clean "$(OBJV)"
	@clean "$(OBJU)"
	@clean "$(OBJR)"
	@clean "$(OBJN)"
	@clean "$(OBJS)"
	@clean "$(OBJC)"
	@clean "$(OBJSCLI)"
	@clean "$(OLBZIP)"
	@clean "$(OLBCLI)"
	@clean "$(OLBUTI)"
	@clean "$(OPTFILE)"
	@clean "$(ZIPS)"
	@clean "$(ZIPHELP_CLI_RNH)"
	@clean "$(ZIPHELPS)"
	- delete/noconfirm/nolog clean.com;*
