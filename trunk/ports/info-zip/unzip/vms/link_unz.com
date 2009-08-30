$ ! LINK_UNZ.COM
$ !
$ !     Command procedure to (re)link the VMS versions of
$ !     UnZip/ZipInfo and UnZipSFX
$ !
$ !     last updated:  13 February 2001
$ !
$ !     Command args:
$ !     - select compiler environment: "VAXC", "DECC", "GNUC"
$ !     - select installation of CLI interface version of unzip:
$ !       "VMSCLI" or "CLI"
$ !     - force installation of UNIX interface version of unzip
$ !       (override LOCAL_UNZIP environment): "NOVMSCLI" or "NOCLI"
$ !
$ !
$ on error then goto error
$ on control_y then goto error
$ OLD_VERIFY = f$verify(0)
$!
$ say := write sys$output
$!##################### Read settings from environment ########################
$!
$ if f$type(LOCAL_UNZIP).eqs.""
$ then
$       local_unzip = ""
$ else  ! Trim blanks and append comma if missing
$       local_unzip = f$edit(local_unzip, "TRIM")
$       if f$extract(f$length(local_unzip)-1, 1, local_unzip).nes."," then -
                local_unzip = local_unzip + ","
$ endif
$! Check for the presence of "VMSCLI" in local_unzip. If yes, we will define
$! the foreign command for "unzip" to use the executable containing the
$! CLI interface.
$ pos_cli = f$locate("VMSCLI",local_unzip)
$ len_local_unzip = f$length(local_unzip)
$ if pos_cli.ne.len_local_unzip
$ then
$   CLI_IS_DEFAULT = 1
$   ! Remove "VMSCLI" macro from local_unzip. The UnZip executable including
$   ! the CLI interface is now created unconditionally.
$   local_unzip = f$extract(0, pos_cli, local_unzip) + -
$               f$extract(pos_cli+7, len_local_unzip-(pos_cli+7), local_unzip)
$ else
$   CLI_IS_DEFAULT = 0
$ endif
$ delete/symbol/local pos_cli
$ delete/symbol/local len_local_unzip
$!##################### Customizing section #############################
$!
$ unzx_unx = "unzip"
$ unzx_cli = "unzip_cli"
$ unzsfx_unx = "unzipsfx"
$ unzsfx_cli = "unzipsfx_cli"
$!
$ MAY_USE_DECC = 1
$ MAY_USE_GNUC = 0
$!
$! Process command line parameters requesting optional features:
$ arg_cnt = 1
$ argloop:
$  current_arg_name = "P''arg_cnt'"
$  curr_arg = f$edit('current_arg_name',"UPCASE")
$  IF curr_arg .eqs. "" THEN GOTO argloop_out
$  IF curr_arg .eqs. "VAXC"
$  THEN MAY_USE_DECC = 0
$    MAY_USE_GNUC = 0
$  ENDIF
$  IF curr_arg .eqs. "DECC"
$  THEN MAY_USE_DECC = 1
$    MAY_USE_GNUC = 0
$  ENDIF
$  IF curr_arg .eqs. "GNUC"
$  THEN MAY_USE_DECC = 0
$    MAY_USE_GNUC = 1
$  ENDIF
$  IF (curr_arg .eqs. "VMSCLI") .or. (curr_arg .eqs. "CLI")
$  THEN
$    CLI_IS_DEFAULT = 1
$  ENDIF
$  IF (curr_arg .eqs. "NOVMSCLI") .or. (curr_arg .eqs. "NOCLI")
$  THEN
$    CLI_IS_DEFAULT = 0
$  ENDIF
$  arg_cnt = arg_cnt + 1
$ GOTO argloop
$ argloop_out:
$!
$ if CLI_IS_DEFAULT
$ then
$       UNZEXEC = unzx_cli
$ else
$       UNZEXEC = unzx_unx
$ endif
$!
$!#######################################################################
$!
$ ! Find out current disk, directory, compiler and options
$ !
$ my_name = f$env("procedure")
$ workdir = f$env("default")
$ here = f$parse(workdir,,,"device") + f$parse(workdir,,,"directory")
$ axp = f$getsyi("HW_MODEL").ge.1024
$ if axp
$ then
$       ! Alpha AXP
$       ARCH_NAME == "Alpha"
$       ARCH_PREF = "AXP_"
$       HAVE_DECC_VAX = 0
$       USE_DECC_VAX = 0
$       IF (f$search("SYS$DISK:[]UNZIP.''ARCH_PREF'OLB").eqs."")
$       THEN
$         say "Cannot find any AXP object library for UnZip."
$         say "  You must keep all binary files of the object distribution"
$         say "  in the current directory !"
$         goto error
$       ENDIF
$       if MAY_USE_GNUC
$       then say "GNU C has not yet been ported to OpenVMS AXP."
$            say "You must use DEC C to build UnZip."
$            goto error
$       endif
$       ARCH_CC_P = ARCH_PREF
$       opts = ""
$       say "Linking on AXP using DEC C"
$ else
$       ! VAX
$       ARCH_NAME == "VAX"
$       ARCH_PREF = "VAX_"
$       ! check which object libraries are present:
$       HAVE_DECC_VAX = -
                (f$search("SYS$DISK:[]UNZIP.''ARCH_PREF'DECC_OLB").nes."")
$       HAVE_VAXC_VAX = -
                (f$search("SYS$DISK:[]UNZIP.''ARCH_PREF'VAXC_OLB").nes."")
$       HAVE_GNUC_VAX = -
                (f$search("SYS$DISK:[]UNZIP.''ARCH_PREF'GNUC_OLB").nes."")
$       IF .not.HAVE_DECC_VAX .and. .not.HAVE_VAXC_VAX .and. .not.HAVE_GNUC_VAX
$       THEN
$         say "Cannot find any VAX object library for UnZip."
$         say "  You must keep all binary files of the object distribution"
$         say "  in the current directory !"
$         goto error
$       ENDIF
$       IF HAVE_DECC_VAX .AND. MAY_USE_DECC
$       THEN
$         ! We use DECC:
$         USE_DECC_VAX = 1
$         ARCH_CC_P = "''ARCH_PREF'DECC_"
$         opts = ""
$         say "Linking on VAX using DEC C"
$       ELSE
$         ! We use VAXC (or GNU C):
$         USE_DECC_VAX = 0
$         opts = ",SYS$DISK:[.VMS]VAXCSHR.OPT/OPTIONS"
$         if HAVE_GNUC_VAX .and. (.not.HAVE_VAXC_VAX .or. MAY_USE_GNUC)
$         then
$               ARCH_CC_P = "''ARCH_PREF'GNUC_"
$               opts = ",GNU_CC:[000000]GCCLIB.OLB/LIB ''opts'"
$               say "Linking on VAX using GNU C"
$         else
$               ARCH_CC_P = "''ARCH_PREF'VAXC_"
$               say "Linking on VAX using VAX C"
$         endif
$       ENDIF
$ endif
$ LFLAGS = "/notrace"
$ if (opts .nes. "") .and. (f$search("[.vms]vaxcshr.opt") .eqs. "")
$ then  create [.vms]vaxcshr.opt
$       open/append tmp [.vms]vaxcshr.opt
$       write tmp "SYS$SHARE:VAXCRTL.EXE/SHARE"
$       close tmp
$ endif
$ tmp = f$verify(1)     ! Turn echo on to see what's happening
$ !
$ link'LFLAGS'/exe='unzx_unx'.'ARCH_CC_P'exe -
        unzip.'ARCH_CC_P'olb;/incl=(unzip)/lib -
        'opts', [.VMS]unzip.opt/opt
$ !
$ link'LFLAGS'/exe='unzx_cli'.'ARCH_CC_P'exe -
        unzipcli.'ARCH_CC_P'olb;/incl=(unzip)/lib, -
        unzip.'ARCH_CC_P'olb;/lib -
        'opts', [.VMS]unzip.opt/opt
$ !
$ link'LFLAGS'/exe='unzsfx_unx'.'ARCH_CC_P'exe -
        unzipsfx.'ARCH_CC_P'olb;/lib/incl=unzip -
        'opts', [.VMS]unzipsfx.opt/opt
$ !
$ link'LFLAGS'/exe='unzsfx_cli'.'ARCH_CC_P'exe -
        unzsxcli.'ARCH_CC_P'olb;/lib/incl=unzip, -
        unzipsfx.'ARCH_CC_P'olb;/lib -
        'opts', [.VMS]unzipsfx.opt/opt
$ !
$ ! Next line:  put similar lines (full pathname for unzip.'ARCH_CC_P'exe) in
$ ! login.com.  Remember to include the leading "$" before disk name.
$ !
$ unzip   == "$''here'''UNZEXEC'.''ARCH_CC_P'exe"
$ zipinfo == "$''here'''UNZEXEC'.''ARCH_CC_P'exe ""-Z"""
$ !
$error:
$ tmp = f$verify(OLD_VERIFY)
$ exit
