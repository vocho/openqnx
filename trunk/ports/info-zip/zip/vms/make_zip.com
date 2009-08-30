$ ! MAKE_ZIP.COM
$ !
$ !     "Makefile" for VMS versions of Zip, ZipCloak, ZipNote,
$ !     and ZipSplit (stolen from Unzip)
$ !
$ !     Command args:
$ !     - select compiler environment: "VAXC", "DECC", "GNUC"
$ !     - select installation of CLI interface version of zip:
$ !       "VMSCLI" or "CLI"
$ !     - force installation of UNIX interface version of zip
$ !       (override LOCAL_ZIP environment): "NOVMSCLI" or "NOCLI"
$ !
$ !     To define additional options, define the global symbol
$ !     LOCAL_ZIP prior to executing MAKE_ZIP.COM, e.g.:
$ !
$ !             $ LOCAL_ZIP == "VMS_IM_EXTRA,"
$ !             $ @MAKE_ZIP
$ !
$ !     The trailing "," may be omitted.  Valid VMS-specific options
$ !     include VMS_PK_EXTRA and VMS_IM_EXTRA; see the INSTALL file
$ !     for other options.
$ !     NOTE: This command procedure does always generate both the
$ !           "default" Zip containing the UNIX style command interface
$ !           and the "VMSCLI" Zip containing the CLI compatible command
$ !           interface. There is no need to add "VMSCLI" to the LOCAL_ZIP
$ !           symbol. (The only effect of "VMSCLI" is now the selection of the
$ !           CLI style Zip executable in the foreign command definition.)
$ !
$ !
$ on error then goto error
$ on control_y then goto error
$ OLD_VERIFY = f$verify(0)
$!
$ edit := edit                  ! override customized edit commands
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
$ if (f$getsyi( "HW_MODEL") .lt. 1024)
$ then
$       ! VAX
$       ARCH_NAME == "VAX"
$       ARCH_PREF = "VAX_"
$       HAVE_DECC = (f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes."")
$       HAVE_VAXC = (f$search("SYS$SYSTEM:VAXC.EXE").nes."")
$       MAY_HAVE_GNUC = (f$trnlnm("GNU_CC").nes."")
$       IF HAVE_DECC .AND. MAY_USE_DECC
$       THEN
$         ! We use DECC:
$         cc = "cc/decc/standard=relax/prefix=all"
$         ARCH_CC_P = "''ARCH_PREF'DECC_"
$         defs = "''local_zip'VMS"
$         opts = ""
$         say "Compiling on VAX using DEC C"
$       ELSE
$         ! We use VAXC (or GNU C):
$         defs = "''local_zip'VMS"
$         opts = ",SYS$DISK:[.VMS]VAXCSHR.OPT/OPTIONS"
$         if (.not.HAVE_VAXC .and. MAY_HAVE_GNUC) .or. (MAY_USE_GNUC)
$         then
$               ARCH_CC_P = "''ARCH_PREF'GNUC_"
$               cc = "gcc"
$               opts = ",GNU_CC:[000000]GCCLIB.OLB/LIB ''opts'"
$               say "Compiling on VAX using GNU C"
$         else
$               ARCH_CC_P = "''ARCH_PREF'VAXC_"
$               if HAVE_DECC
$               then
$                   cc = "cc/vaxc"
$               else
$                   cc = "cc"
$               endif
$               say "Compiling on VAX using VAX C"
$         endif
$       ENDIF
$ else
$       if (f$getsyi( "ARCH_TYPE") .eq. 2)
$       then
$           ! Alpha AXP
$           ARCH_NAME == "Alpha"
$           ARCH_PREF = "AXP_"
$           HAVE_DECC = (f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes."")
$           MAY_HAVE_GNUC = (f$trnlnm("GNU_CC_VERSION").nes."")
$           IF (.not.HAVE_DECC .and. MAY_HAVE_GNUC) .or. (MAY_USE_GNUC)
$           THEN
$             say "GNC C is not supported on OpenVMS Alpha."
$             goto error
$           ENDIF
$           ! We use DECC:
$           ARCH_CC_P = ARCH_PREF
$           cc = "cc/standard=relax/prefix=all/ansi"
$           defs = "''local_zip'VMS"
$           opts = ""
$           say "Compiling on AXP using DEC C"
$       else
$           ! IA64
$           ARCH_NAME == "IA64"
$           ARCH_PREF = "I64_"
$           HAVE_DECC = (f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes."")
$           MAY_HAVE_GNUC = (f$trnlnm("GNU_CC_VERSION").nes."")
$           IF (.not.HAVE_DECC .and. MAY_HAVE_GNUC) .or. (MAY_USE_GNUC)
$           THEN
$             say "GNC C is not supported on OpenVMS IA64."
$             goto error
$           ENDIF
$           ! We use DECC:
$           ARCH_CC_P = ARCH_PREF
$           cc = "cc/standard=relax/prefix=all/ansi"
$           defs = "''local_zip'VMS"
$           opts = ""
$           say "Compiling on IA64 using DEC C"
$       endif
$ endif
$ DEF_UNX = "/def=(''DEFS')"
$ DEF_CLI = "/def=(''DEFS',VMSCLI)"
$ DEF_UTIL = "/def=(''DEFS',UTIL)"
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
$ runoff/out=zip.hlp [.vms]vms_zip.rnh
$ !
$ cc 'DEF_UNX' /obj=zip.'ARCH_CC_P'obj zip.c
$ cc 'DEF_UNX' /obj=crc32.'ARCH_CC_P'obj crc32.c
$ cc 'DEF_UNX' /obj=crctab.'ARCH_CC_P'obj crctab.c
$ cc 'DEF_UNX' /obj=crypt.'ARCH_CC_P'obj crypt.c
$ cc 'DEF_UNX' /obj=ttyio.'ARCH_CC_P'obj ttyio.c
$ cc 'DEF_UNX' /obj=zipfile.'ARCH_CC_P'obj zipfile.c
$ cc 'DEF_UNX' /obj=zipup.'ARCH_CC_P'obj zipup.c
$ cc 'DEF_UNX' /obj=fileio.'ARCH_CC_P'obj fileio.c
$ cc 'DEF_UNX' /obj=globals.'ARCH_CC_P'obj globals.c
$ cc 'DEF_UNX' /obj=util.'ARCH_CC_P'obj util.c
$ cc 'DEF_UNX' /obj=deflate.'ARCH_CC_P'obj deflate.c
$ cc 'DEF_UNX' /obj=trees.'ARCH_CC_P'obj trees.c
$ cc 'DEF_UNX' /obj=vmszip.'ARCH_CC_P'obj/inc=SYS$DISK:[] [.vms]vmszip.c
$ cc 'DEF_UNX' /obj=vms.'ARCH_CC_P'obj/inc=SYS$DISK:[] [.vms]vms.c
$ cc 'DEF_UNX' /obj=vmsmunch.'ARCH_CC_P'obj/inc=SYS$DISK:[] [.vms]vmsmunch.c
$ !
$ if f$search("zip.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create zip.'ARCH_CC_P'olb
$ lib/obj/replace zip.'ARCH_CC_P'olb -
        zip.'ARCH_CC_P'obj;, -
        crc32.'ARCH_CC_P'obj;, crctab.'ARCH_CC_P'obj;, -
        crypt.'ARCH_CC_P'obj;, ttyio.'ARCH_CC_P'obj;, -
        zipfile.'ARCH_CC_P'obj;, zipup.'ARCH_CC_P'obj;, -
        fileio.'ARCH_CC_P'obj;, util.'ARCH_CC_P'obj;, globals.'ARCH_CC_P'obj;,-
        deflate.'ARCH_CC_P'obj;, trees.'ARCH_CC_P'obj;, -
        vmszip.'ARCH_CC_P'obj;, vms.'ARCH_CC_P'obj;, -
        vmsmunch.'ARCH_CC_P'obj;
$ !
$ link'LFLAGS'/exe='zipx_unx'.'ARCH_CC_P'exe -
        zip.'ARCH_CC_P'olb;/incl=(zip,globals)/lib 'opts'
$ !
$ !------------------------ Zip (CLI interface) section -----------------------
$ !
$ set default [.vms]
$ edit/tpu/nosection/nodisplay/command=cvthelp.tpu zip_cli.help
$ set default [-]
$ runoff/out=zip_cli.hlp [.vms]zip_cli.rnh
$ !
$ cc 'DEF_CLI' /obj=zipcli.'ARCH_CC_P'obj zip.c
$ cc 'DEF_CLI'/INCLUDE=SYS$DISK:[] /OBJ=cmdline.'ARCH_CC_P'obj -
        [.vms]cmdline.c
$ set command/obj=zip_cli.'ARCH_CC_P'obj [.vms]zip_cli.cld
$ !
$ if f$search("zipcli.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create zipcli.'ARCH_CC_P'olb
$ lib/obj/replace zipcli.'ARCH_CC_P'olb -
        zipcli.'ARCH_CC_P'obj;, -
        cmdline.'ARCH_CC_P'obj;, zip_cli.'ARCH_CC_P'obj;
$ !
$ link'LFLAGS'/exe='zipx_cli'.'ARCH_CC_P'exe -
        zipcli.'ARCH_CC_P'olb;/incl=(zip)/lib, -
        zip.'ARCH_CC_P'olb;/incl=(globals)/lib 'opts'
$ !
$ !-------------------------- Zip utilities section ---------------------------
$ !
$ cc 'DEF_UTIL' /obj=zipfile_.'ARCH_CC_P'obj zipfile.c
$ cc 'DEF_UTIL' /obj=fileio_.'ARCH_CC_P'obj fileio.c
$ cc 'DEF_UTIL' /obj=util_.'ARCH_CC_P'obj util.c
$ cc 'DEF_UTIL' /obj=crypt_.'ARCH_CC_P'obj crypt.c
$ cc 'DEF_UTIL'/incl=SYS$DISK:[] /obj=vms_.'ARCH_CC_P'obj [.vms]vms.c
$ if f$search("ziputils.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create ziputils.'ARCH_CC_P'olb
$ lib/obj/replace ziputils.'ARCH_CC_P'olb -
        zipfile_.'ARCH_CC_P'obj;, fileio_.'ARCH_CC_P'obj;, -
        util_.'ARCH_CC_P'obj;, globals.'ARCH_CC_P'obj;, -
        crctab.'ARCH_CC_P'obj;, crypt_.'ARCH_CC_P'obj;, ttyio.'ARCH_CC_P'obj;,-
        vms_.'ARCH_CC_P'obj;, vmsmunch.'ARCH_CC_P'obj;
$ cc 'DEF_UNX' /obj=zipcloak.'ARCH_CC_P'obj zipcloak.c
$ cc 'DEF_UNX' /obj=zipnote.'ARCH_CC_P'obj zipnote.c
$ cc 'DEF_UNX' /obj=zipsplit.'ARCH_CC_P'obj zipsplit.c
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
