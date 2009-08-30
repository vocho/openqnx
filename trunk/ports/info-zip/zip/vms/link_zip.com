$ ! LINK_ZIP.COM
$ !
$ !     Command procedure to (re)link the VMS versions of
$ !     Zip, ZipCloak, ZipNote, and ZipSplit
$ !
$ !     Command args:
$ !     - select compiler environment: "VAXC", "DECC", "GNUC"
$ !     - select installation of CLI interface version of zip:
$ !       "VMSCLI" or "CLI"
$ !     - force installation of UNIX interface version of zip
$ !       (override LOCAL_ZIP environment): "NOVMSCLI" or "NOCLI"
$ !
$ on error then goto error
$ on control_y then goto error
$ OLD_VERIFY = f$verify(0)
$!
$ say := write sys$output
$!##################### Read settings from environment ########################
$!
$ if f$type(LOCAL_ZIP).eqs.""
$ then
$       local_zip = ""
$ else  ! Trim blanks and append comma if missing
$       local_zip = f$edit(local_zip, "TRIM")
$       if f$extract(f$length(local_zip)-1, 1, local_zip).nes."," then -
                local_zip = local_zip + ","
$ endif
$! Check for the presence of "VMSCLI" in local_zip. If yes, we will define
$! the foreign command for "zip" to use the executable containing the
$! CLI interface.
$ pos_cli = f$locate("VMSCLI",local_zip)
$ len_local_zip = f$length(local_zip)
$ if pos_cli.ne.len_local_zip
$ then
$   CLI_IS_DEFAULT = 1
$   ! Remove "VMSCLI" macro from local_zip. The Zip executable including
$   ! the CLI interface is now created unconditionally.
$   local_zip = f$extract(0, pos_cli, local_zip) + -
$               f$extract(pos_cli+7, len_local_zip-(pos_cli+7), local_zip)
$ else
$   CLI_IS_DEFAULT = 0
$ endif
$ delete/symbol/local pos_cli
$ delete/symbol/local len_local_zip
$!##################### Customizing section #############################
$!
$ zipx_unx = "zip"
$ zipx_cli = "zip_cli"
$!
$ MAY_USE_DECC = 1      ! Use DEC C when its presence is detected
$ MAY_USE_GNUC = 0      ! Do not prefer GNUC over DEC or VAX C
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
$       ZIPEXEC = zipx_cli
$ else
$       ZIPEXEC = zipx_unx
$ endif
$!
$!#######################################################################
$!
$ ! Find out current disk, directory, compiler and options
$ !
$ my_name = f$env("procedure")
$ workdir = f$env("default")
$ here = f$parse(workdir,,,"device") + f$parse(workdir,,,"directory")
$!
$ if (f$getsyi("HW_MODEL") .lt. 1024)
$ then
$       ! VAX
$       ARCH_NAME == "VAX"
$       ARCH_PREF = "VAX_"
$       ! check which object libraries are present:
$       HAVE_DECC =(f$search("SYS$DISK:[]ZIP.''ARCH_PREF'DECC_OLB").nes."")
$       HAVE_VAXC =(f$search("SYS$DISK:[]ZIP.''ARCH_PREF'VAXC_OLB").nes."")
$       HAVE_GNUC =(f$search("SYS$DISK:[]ZIP.''ARCH_PREF'GNUC_OLB").nes."")
$       IF .not.HAVE_DECC .and. .not.HAVE_VAXC .and. .not.HAVE_GNUC
$       THEN
$         say "Cannot find any VAX object library for Zip."
$         say "  You must keep all binary files of the object distribution"
$         say "  in the current directory !"
$         goto error
$       ENDIF
$       IF HAVE_DECC .AND. MAY_USE_DECC
$       THEN
$         ! We use DECC:
$         ARCH_CC_P = "''ARCH_PREF'DECC_"
$         opts = ""
$         say "Linking on VAX using DEC C"
$       ELSE
$         ! We use VAXC (or GNU C):
$         opts = ",SYS$DISK:[.VMS]VAXCSHR.OPT/OPTIONS"
$         if HAVE_GNUC .and. (.not.HAVE_VAXC .or. MAY_USE_GNUC)
$         then
$               ARCH_CC_P = "''ARCH_PREF'GNUC_"
$               opts = ",GNU_CC:[000000]GCCLIB.OLB/LIB ''opts'"
$               say "Linking on VAX using GNU C"
$         else
$               ARCH_CC_P = "''ARCH_PREF'VAXC_"
$               say "Linking on VAX using VAX C"
$         endif
$       ENDIF
$ else
$       if (f$getsyi( "ARCH_TYPE") .eq. 2)
$       then
$           ! Alpha AXP
$           ARCH_NAME == "Alpha"
$           ARCH_PREF = "AXP_"
$           IF (f$search("SYS$DISK:[]ZIP.''ARCH_PREF'OLB").eqs."")
$           THEN
$             say "Cannot find any AXP object library for Zip."
$             say "  You must keep all binary files of the object distribution"
$             say "  in the current directory !"
$             goto error
$           ENDIF
$           if MAY_USE_GNUC
$           then
$             say "GNC C is not supported on OpenVMS Alpha."
$             goto error
$           endif
$           ARCH_CC_P = ARCH_PREF
$           opts = ""
$           say "Linking on AXP using DEC C"
$       else
$           ! IA64
$           ARCH_NAME == "IA64"
$           ARCH_PREF = "I64_"
$           IF (f$search("SYS$DISK:[]ZIP.''ARCH_PREF'OLB").eqs."")
$           THEN
$             say "Cannot find any I64 object library for Zip."
$             say "  You must keep all binary files of the object distribution"
$             say "  in the current directory !"
$             goto error
$           ENDIF
$           if MAY_USE_GNUC
$           then
$             say "GNC C is not supported on OpenVMS IA64."
$             goto error
$           endif
$           ARCH_CC_P = ARCH_PREF
$           opts = ""
$           say "Linking on IA64 using DEC C"
$       endif
$ endif
$ LFLAGS = "/notrace"
$ if (opts .nes. "") .and. -
     (f$locate("VAXCSHR",f$edit(opts,"UPCASE")) .lt. f$length(opts)) .and. -
     (f$search("[.vms]vaxcshr.opt") .eqs. "")
$ then  create [.vms]vaxcshr.opt
$       open/append tmp [.vms]vaxcshr.opt
$       write tmp "SYS$SHARE:VAXCRTL.EXE/SHARE"
$       close tmp
$ endif
$ set verify    ! like "echo on", eh?
$ !
$ !------------------------------- Zip section --------------------------------
$ !
$ link'LFLAGS'/exe='zipx_unx'.'ARCH_CC_P'exe -
        zip.'ARCH_CC_P'olb;/incl=(zip,globals)/lib 'opts'
$ !
$ !------------------------ Zip (CLI interface) section -----------------------
$ !
$ link'LFLAGS'/exe='zipx_cli'.'ARCH_CC_P'exe -
        zipcli.'ARCH_CC_P'olb;/incl=(zip)/lib, -
        zip.'ARCH_CC_P'olb;/incl=(globals)/lib 'opts'
$ !
$ !-------------------------- Zip utilities section ---------------------------
$ !
$ link'LFLAGS'/exe=zipcloak.'ARCH_CC_P'exe zipcloak.'ARCH_CC_P'obj, -
        ziputils.'ARCH_CC_P'olb;/incl=(globals)/lib 'opts'
$ link'LFLAGS'/exe=zipnote.'ARCH_CC_P'exe zipnote.'ARCH_CC_P'obj, -
        ziputils.'ARCH_CC_P'olb;/incl=(globals)/lib 'opts'
$ link'LFLAGS'/exe=zipsplit.'ARCH_CC_P'exe zipsplit.'ARCH_CC_P'obj, -
        ziputils.'ARCH_CC_P'olb;/incl=(globals)/lib 'opts'
$ !
$ !----------------------------- Symbols section ------------------------------
$ !
$ ! Set up symbols for the various executables.  Edit the example below,
$ ! changing "disk:[directory]" as appropriate.
$ !
$ zip           == "$''here'''ZIPEXEC'.''ARCH_CC_P'exe"
$ zipcloak      == "$''here'zipcloak.''ARCH_CC_P'exe"
$ zipnote       == "$''here'zipnote.''ARCH_CC_P'exe"
$ zipsplit      == "$''here'zipsplit.''ARCH_CC_P'exe"
$ !
$error:
$ if here .nes. "" then set default 'here'
$ dummy = f$verify(OLD_VERIFY)
$ exit
