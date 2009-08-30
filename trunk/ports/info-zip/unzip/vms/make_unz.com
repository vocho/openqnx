$ ! MAKE_UNZ.COM
$ !
$ !     "Makefile" for VMS versions of UnZip/ZipInfo and UnZipSFX
$ !
$ !     last revised:  13 February 2001
$ !
$ !     Command args:
$ !     - select compiler environment: "VAXC", "DECC", "GNUC"
$ !     - select installation of CLI interface version of unzip:
$ !       "VMSCLI" or "CLI"
$ !     - force installation of UNIX interface version of unzip
$ !       (override LOCAL_UNZIP environment): "NOVMSCLI" or "NOCLI"
$ !
$ !     To define additional options, define the global symbol
$ !     LOCAL_UNZIP prior to executing MAKE_UNZ.COM, e.g.:
$ !
$ !             $ LOCAL_UNZIP == "RETURN_CODES,"
$ !             $ @MAKE_UNZ
$ !
$ !     The trailing "," may be omitted.  Valid VMS-specific options
$ !     include VMSWILD and RETURN_CODES; see the INSTALL file
$ !     for other options (e.g., CHECK_VERSIONS).
$ !     NOTE: This command procedure does always generate both the
$ !           "default" UnZip containing the UNIX style command interface
$ !           and the "VMSCLI" UnZip containing the CLI compatible command
$ !           interface. There is no need to add "VMSCLI" to the LOCAL_UNZIP
$ !           symbol. (The only effect of "VMSCLI" is now the selection of the
$ !           CLI style UnZip executable in the foreign command definition.)
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
$       if MAY_USE_GNUC
$       then say "GNU C has not yet been ported to OpenVMS AXP."
$            say "You must use DEC C to build UnZip."
$            goto error
$       endif
$       ARCH_CC_P = ARCH_PREF
$       cc = "cc/standard=relax/prefix=all/ansi"
$       defs = "''local_unzip'MODERN"
$       opts = ""
$       say "Compiling on AXP using DEC C"
$ else
$       ! VAX
$       ARCH_NAME == "VAX"
$       ARCH_PREF = "VAX_"
$       HAVE_DECC_VAX = (f$search("SYS$SYSTEM:DECC$COMPILER.EXE").nes."")
$       HAVE_VAXC_VAX = (f$search("SYS$SYSTEM:VAXC.EXE").nes."")
$       MAY_HAVE_GNUC = (f$trnlnm("GNU_CC").nes."")
$       IF HAVE_DECC_VAX .AND. MAY_USE_DECC
$       THEN
$         ! We use DECC:
$         USE_DECC_VAX = 1
$         ARCH_CC_P = "''ARCH_PREF'DECC_"
$         cc = "cc/decc/standard=vaxc/prefix=all"
$         defs = "''local_unzip'MODERN"
$         opts = ""
$         say "Compiling on VAX using DEC C"
$       ELSE
$         ! We use VAXC (or GNU C):
$         USE_DECC_VAX = 0
$         defs = "''local_unzip'VMS"
$         opts = ",SYS$DISK:[.VMS]VAXCSHR.OPT/OPTIONS"
$         if (.not.HAVE_VAXC_VAX .and. MAY_HAVE_GNUC) .or. (MAY_USE_GNUC)
$         then
$               ARCH_CC_P = "''ARCH_PREF'GNUC_"
$               cc = "gcc"
$               opts = ",GNU_CC:[000000]GCCLIB.OLB/LIB ''opts'"
$               say "Compiling on VAX using GNU C"
$         else
$               ARCH_CC_P = "''ARCH_PREF'VAXC_"
$               if HAVE_DECC_VAX
$               then
$                   cc = "cc/vaxc"
$               else
$                   cc = "cc"
$               endif
$               say "Compiling on VAX using VAX C"
$         endif
$       ENDIF
$ endif
$ DEF_UNX = "/define=(''defs')"
$ DEF_CLI = "/define=(''defs',VMSCLI)"
$ DEF_SXUNX = "/define=(''defs',SFX)"
$ DEF_SXCLI = "/define=(''defs',VMSCLI,SFX)"
$ LFLAGS = "/notrace"
$ if (opts .nes. "") .and. (f$search("[.vms]vaxcshr.opt") .eqs. "")
$ then  create [.vms]vaxcshr.opt
$       open/append tmp [.vms]vaxcshr.opt
$       write tmp "SYS$SHARE:VAXCRTL.EXE/SHARE"
$       close tmp
$ endif
$ !
$ ! Currently, the following section is not needed, as vms.c does no longer
$ ! include any of the headers from SYS$LIB_C.TLB.
$ ! The commented section is solely maintained for reference.
$ ! In case system headers from SYS$LIB_C.TLB are needed again,
$ ! just append "'x'" to the respective source file specification.
$! x = f$search("SYS$LIBRARY:SYS$LIB_C.TLB")
$! if x .nes. "" then x = "+" + x
$ !
$ tmp = f$verify(1)     ! Turn echo on to see what's happening
$ !
$ !------------------------------- UnZip section ------------------------------
$ !
$ runoff/out=unzip.hlp [.vms]unzip_def.rnh
$ !
$ cc/NOLIST'DEF_UNX' /OBJ=unzip.'ARCH_CC_P'obj unzip.c
$ cc/NOLIST'DEF_UNX' /OBJ=crc32.'ARCH_CC_P'obj crc32.c
$ cc/NOLIST'DEF_UNX' /OBJ=crctab.'ARCH_CC_P'obj crctab.c
$ cc/NOLIST'DEF_UNX' /OBJ=crypt.'ARCH_CC_P'obj crypt.c
$ cc/NOLIST'DEF_UNX' /OBJ=envargs.'ARCH_CC_P'obj envargs.c
$ cc/NOLIST'DEF_UNX' /OBJ=explode.'ARCH_CC_P'obj explode.c
$ cc/NOLIST'DEF_UNX' /OBJ=extract.'ARCH_CC_P'obj extract.c
$ cc/NOLIST'DEF_UNX' /OBJ=fileio.'ARCH_CC_P'obj fileio.c
$ cc/NOLIST'DEF_UNX' /OBJ=globals.'ARCH_CC_P'obj globals.c
$ cc/NOLIST'DEF_UNX' /OBJ=inflate.'ARCH_CC_P'obj inflate.c
$ cc/NOLIST'DEF_UNX' /OBJ=list.'ARCH_CC_P'obj list.c
$ cc/NOLIST'DEF_UNX' /OBJ=match.'ARCH_CC_P'obj match.c
$ cc/NOLIST'DEF_UNX' /OBJ=process.'ARCH_CC_P'obj process.c
$ cc/NOLIST'DEF_UNX' /OBJ=ttyio.'ARCH_CC_P'obj ttyio.c
$ cc/NOLIST'DEF_UNX' /OBJ=unreduce.'ARCH_CC_P'obj unreduce.c
$ cc/NOLIST'DEF_UNX' /OBJ=unshrink.'ARCH_CC_P'obj unshrink.c
$ cc/NOLIST'DEF_UNX' /OBJ=zipinfo.'ARCH_CC_P'obj zipinfo.c
$ cc/INCLUDE=SYS$DISK:[]'DEF_UNX' /OBJ=vms.'ARCH_CC_P'obj; [.vms]vms.c
$ !
$ if f$search("unzip.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create unzip.'ARCH_CC_P'olb
$ lib/obj/replace unzip.'ARCH_CC_P'olb -
        unzip.'ARCH_CC_P'obj;, crc32.'ARCH_CC_P'obj;, -
        crctab.'ARCH_CC_P'obj;, crypt.'ARCH_CC_P'obj;, -
        envargs.'ARCH_CC_P'obj;, explode.'ARCH_CC_P'obj;, -
        extract.'ARCH_CC_P'obj;, fileio.'ARCH_CC_P'obj;, -
        globals.'ARCH_CC_P'obj;, inflate.'ARCH_CC_P'obj;, -
        list.'ARCH_CC_P'obj;, match.'ARCH_CC_P'obj;, -
        process.'ARCH_CC_P'obj;, ttyio.'ARCH_CC_P'obj;, -
        unreduce.'ARCH_CC_P'obj;, unshrink.'ARCH_CC_P'obj;, -
        zipinfo.'ARCH_CC_P'obj;, vms.'ARCH_CC_P'obj;
$ !
$ link'LFLAGS'/exe='unzx_unx'.'ARCH_CC_P'exe -
        unzip.'ARCH_CC_P'olb;/incl=(unzip)/lib -
        'opts', [.VMS]unzip.opt/opt
$ !
$ !----------------------- UnZip (CLI interface) section ----------------------
$ !
$ set default [.vms]
$ edit/tpu/nosection/nodisplay/command=cvthelp.tpu unzip_cli.help
$ set default [-]
$ runoff/out=unzip_cli.hlp [.vms]unzip_cli.rnh
$ !
$ cc/NOLIST'DEF_CLI' /OBJ=unzipcli.'ARCH_CC_P'obj unzip.c
$ cc/INCLUDE=SYS$DISK:[]'DEF_CLI' /OBJ=cmdline.'ARCH_CC_P'obj; -
                [.vms]cmdline.c
$ set command/obj=unz_cli.'ARCH_CC_P'obj [.vms]unz_cli.cld
$ !
$ if f$search("unzipcli.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create unzipcli.'ARCH_CC_P'olb
$ lib/obj/replace unzipcli.'ARCH_CC_P'olb -
        unzipcli.'ARCH_CC_P'obj;, -
        cmdline.'ARCH_CC_P'obj;, unz_cli.'ARCH_CC_P'obj;
$ !
$ link'LFLAGS'/exe='unzx_cli'.'ARCH_CC_P'exe -
        unzipcli.'ARCH_CC_P'olb;/incl=(unzip)/lib, -
        unzip.'ARCH_CC_P'olb;/lib -
        'opts', [.VMS]unzip.opt/opt
$ !
$ !-------------------------- UnZipSFX section --------------------------------
$ !
$ cc/NOLIST'DEF_SXUNX' /OBJ=unzipsfx.'ARCH_CC_P'obj unzip.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=crc32_.'ARCH_CC_P'obj crc32.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=crctab_.'ARCH_CC_P'obj crctab.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=crypt_.'ARCH_CC_P'obj crypt.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=extract_.'ARCH_CC_P'obj extract.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=fileio_.'ARCH_CC_P'obj fileio.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=globals_.'ARCH_CC_P'obj globals.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=inflate_.'ARCH_CC_P'obj inflate.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=match_.'ARCH_CC_P'obj match.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=process_.'ARCH_CC_P'obj process.c
$ cc/NOLIST'DEF_SXUNX' /OBJ=ttyio_.'ARCH_CC_P'obj ttyio.c
$ cc/NOLIST'DEF_SXUNX'/INCLUDE=SYS$DISK:[] /OBJ=vms_.'ARCH_CC_P'obj; -
        [.vms]vms.c
$ if f$search("unzipsfx.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create unzipsfx.'ARCH_CC_P'olb
$ lib/obj/replace unzipsfx.'ARCH_CC_P'olb -
        unzipsfx.'ARCH_CC_P'obj, crc32_.'ARCH_CC_P'obj, -
        crctab_.'ARCH_CC_P'obj, crypt_.'ARCH_CC_P'obj, -
        extract_.'ARCH_CC_P'obj, fileio_.'ARCH_CC_P'obj, -
        globals_.'ARCH_CC_P'obj, inflate_.'ARCH_CC_P'obj, -
        match_.'ARCH_CC_P'obj, process_.'ARCH_CC_P'obj, -
        ttyio_.'ARCH_CC_P'obj, vms_.'ARCH_CC_P'obj
$ !
$ link'LFLAGS'/exe='unzsfx_unx'.'ARCH_CC_P'exe -
        unzipsfx.'ARCH_CC_P'olb;/lib/incl=unzip -
        'opts', [.VMS]unzipsfx.opt/opt
$ !
$ !--------------------- UnZipSFX (CLI interface) section ---------------------
$ !
$ cc/NOLIST'DEF_SXCLI' /OBJ=unzsxcli.'ARCH_CC_P'obj unzip.c
$ cc/NOLIST/INCLUDE=SYS$DISK:[]'DEF_SXCLI' /OBJ=cmdline_.'ARCH_CC_P'obj; -
                [.vms]cmdline.c
$ if f$search("unzsxcli.''ARCH_CC_P'olb") .eqs. "" then -
        lib/obj/create unzsxcli.'ARCH_CC_P'olb
$ lib/obj/replace unzsxcli.'ARCH_CC_P'olb -
        unzsxcli.'ARCH_CC_P'obj;, -
        cmdline_.'ARCH_CC_P'obj;, unz_cli.'ARCH_CC_P'obj;
$ !
$ link'LFLAGS'/exe='unzsfx_cli'.'ARCH_CC_P'exe -
        unzsxcli.'ARCH_CC_P'olb;/lib/incl=unzip, -
        unzipsfx.'ARCH_CC_P'olb;/lib -
        'opts', [.VMS]unzipsfx.opt/opt
$ !
$ !----------------------------- Symbols section ------------------------------
$ !
$ ! Next line:  put similar lines (full pathname for unzip.'ARCH_CC_P'exe) in
$ ! login.com.  Remember to include the leading "$" before disk name.
$ !
$ unzip   == "$''here'''UNZEXEC'.''ARCH_CC_P'exe"
$ zipinfo == "$''here'''UNZEXEC'.''ARCH_CC_P'exe ""-Z"""
$ !
$error:
$ if here .nes. "" then set default 'here'
$ tmp = f$verify(OLD_VERIFY)
$ exit
