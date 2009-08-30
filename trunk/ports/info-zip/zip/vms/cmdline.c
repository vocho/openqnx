/*
  Copyright (c) 1990-2006 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2005-Feb-10 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
#define module_name VMS_ZIP_CMDLINE
#define module_ident "02-006"
/*
**
**  Facility:   ZIP
**
**  Module:     VMS_ZIP_CMDLINE
**
**  Author:     Hunter Goatley <goathunter@MadGoat.com>
**
**  Date:       July 30, 1993
**
**  Abstract:   Routines to handle a VMS CLI interface for Zip.  The CLI
**              command line is parsed and a new argc/argv are built and
**              returned to Zip.
**
**  Modified by:
**
**      02-006          Onno van der Linden,
**                      Christian Spieler       07-JUL-1998 23:03
**              Support GNU CC 2.8 on Alpha AXP (vers-num unchanged).
**      02-006          Johnny Lee              25-JUN-1998 07:40
**              Fixed typo (superfluous ';') (vers-num unchanged).
**      02-006          Christian Spieler       12-SEP-1997 23:17
**              Fixed bugs in /BEFORE and /SINCE handlers (vers-num unchanged).
**      02-006          Christian Spieler       12-JUL-1997 02:05
**              Complete revision of the argv strings construction.
**              Added handling of "-P pwd", "-R", "-i@file", "-x@file" options.
**      02-005          Patrick Ellis           09-MAY-1996 22:25
**              Show UNIX style help screen when UNIX style options are used.
**      02-004          Onno van der Linden,
**                      Christian Spieler       13-APR-1996 20:05
**              Removed /ENCRYPT=VERIFY ("-ee" option).
**      02-003          Christian Spieler       11-FEB-1996 23:05
**              Added handling of /EXTRAFIELDS qualifier ("-X" option).
**      02-002          Christian Spieler       09-JAN-1996 22:25
**              Added "#include crypt.h", corrected typo.
**      02-001          Christian Spieler       04-DEC-1995 16:00
**              Fixed compilation in DEC CC's ANSI mode.
**      02-000          Christian Spieler       10-OCT-1995 17:54
**              Modified for Zip v2.1, added several new options.
**      01-000          Hunter Goatley          30-JUL-1993 07:54
**              Original version (for Zip v1.9p1).
**
*/


#if defined(__DECC) || defined(__GNUC__)
#pragma module module_name module_ident
#else
#module module_name module_ident
#endif

#include "zip.h"
#ifndef TEST
#include "crypt.h"      /* for VMSCLI_help() */
#include "revision.h"   /* for VMSCLI_help() */
#endif /* !TEST */

#include <ssdef.h>
#include <descrip.h>
#include <climsgdef.h>
#include <clidef.h>
#include <lib$routines.h>
#include <ots$routines.h>
#include <str$routines.h>

#ifndef CLI$_COMMA
globalvalue CLI$_COMMA;
#endif

/*
**  "Macro" to initialize a dynamic string descriptor.
*/
#define init_dyndesc(dsc) {\
        dsc.dsc$w_length = 0;\
        dsc.dsc$b_dtype = DSC$K_DTYPE_T;\
        dsc.dsc$b_class = DSC$K_CLASS_D;\
        dsc.dsc$a_pointer = NULL;}

/*
**  Memory allocation step for argv string buffer.
*/
#define ARGBSIZE_UNIT 256

/*
**  Memory reallocation macro for argv string buffer.
*/
#define CHECK_BUFFER_ALLOCATION(buf, reserved, requested) { \
    if ((requested) > (reserved)) { \
        char *save_buf = (buf); \
        (reserved) += ARGBSIZE_UNIT; \
        if (((buf) = (char *) realloc((buf), (reserved))) == NULL) { \
            if (save_buf != NULL) free(save_buf); \
            return (SS$_INSFMEM); \
        } \
    } \
}

/*
**  Define descriptors for all of the CLI parameters and qualifiers.
*/
$DESCRIPTOR(cli_delete,         "DELETE");              /* -d */
$DESCRIPTOR(cli_freshen,        "FRESHEN");             /* -f */
$DESCRIPTOR(cli_move,           "MOVE");                /* -m */
$DESCRIPTOR(cli_update,         "UPDATE");              /* -u */
$DESCRIPTOR(cli_exclude,        "EXCLUDE");             /* -x */
$DESCRIPTOR(cli_include,        "INCLUDE");             /* -i */
$DESCRIPTOR(cli_exlist,         "EXLIST");              /* -x@ */
$DESCRIPTOR(cli_inlist,         "INLIST");              /* -i@ */
$DESCRIPTOR(cli_adjust,         "ADJUST_OFFSETS");      /* -A */
$DESCRIPTOR(cli_append,         "APPEND");              /* -g */
$DESCRIPTOR(cli_batch,          "BATCH");               /* -@ */
$DESCRIPTOR(cli_before,         "BEFORE");              /* -tt */
$DESCRIPTOR(cli_comments,       "COMMENTS");            /* -c,-z */
$DESCRIPTOR(cli_comment_zipfile,"COMMENTS.ZIP_FILE");   /* -z */
$DESCRIPTOR(cli_comment_files,  "COMMENTS.FILES");      /* -c */
$DESCRIPTOR(cli_dirnames,       "DIRNAMES");            /* -D */
$DESCRIPTOR(cli_encrypt,        "ENCRYPT");             /* -e,-P */
$DESCRIPTOR(cli_extra_fields,   "EXTRA_FIELDS");        /* -X */
$DESCRIPTOR(cli_fix_archive,    "FIX_ARCHIVE");         /* -F[F] */
$DESCRIPTOR(cli_fix_normal,     "FIX_ARCHIVE.NORMAL");  /* -F */
$DESCRIPTOR(cli_fix_full,       "FIX_ARCHIVE.FULL");    /* -FF */
$DESCRIPTOR(cli_full_path,      "FULL_PATH");           /* -p */
$DESCRIPTOR(cli_help,           "HELP");                /* -h */
$DESCRIPTOR(cli_junk,           "JUNK");                /* -j */
$DESCRIPTOR(cli_keep_version,   "KEEP_VERSION");        /* -w */
$DESCRIPTOR(cli_latest,         "LATEST");              /* -o */
$DESCRIPTOR(cli_level,          "LEVEL");               /* -[0-9] */
$DESCRIPTOR(cli_license,        "LICENSE");             /* -L */
$DESCRIPTOR(cli_must_match,     "MUST_MATCH");          /* -MM */
$DESCRIPTOR(cli_pkzip,          "PKZIP");               /* -k */
$DESCRIPTOR(cli_quiet,          "QUIET");               /* -q */
$DESCRIPTOR(cli_recurse,        "RECURSE");             /* -r,-R */
$DESCRIPTOR(cli_recurse_path,   "RECURSE.PATH");        /* -r */
$DESCRIPTOR(cli_recurse_fnames, "RECURSE.FILENAMES");   /* -R */
$DESCRIPTOR(cli_since,          "SINCE");               /* -t */
$DESCRIPTOR(cli_store_types,    "STORE_TYPES");         /* -n */
$DESCRIPTOR(cli_temp_path,      "TEMP_PATH");           /* -b */
$DESCRIPTOR(cli_test,           "TEST");                /* -T */
$DESCRIPTOR(cli_translate_eol,  "TRANSLATE_EOL");       /* -l[l] */
$DESCRIPTOR(cli_transl_eol_lf,  "TRANSLATE_EOL.LF");    /* -l */
$DESCRIPTOR(cli_transl_eol_crlf,"TRANSLATE_EOL.CRLF");  /* -ll */
$DESCRIPTOR(cli_unsfx,          "UNSFX");               /* -J */
$DESCRIPTOR(cli_verbose,        "VERBOSE");             /* -v (?) */
$DESCRIPTOR(cli_verbose_normal, "VERBOSE.NORMAL");      /* -v */
$DESCRIPTOR(cli_verbose_more,   "VERBOSE.MORE");        /* -vv */
$DESCRIPTOR(cli_verbose_debug,  "VERBOSE.DEBUG");       /* -vvv */
$DESCRIPTOR(cli_verbose_command,"VERBOSE.COMMAND");     /* (none) */
$DESCRIPTOR(cli_vms,            "VMS");                 /* -V */
$DESCRIPTOR(cli_vms_all,        "VMS.ALL");             /* -VV */

$DESCRIPTOR(cli_yyz,            "YYZ_ZIP");

$DESCRIPTOR(cli_zipfile,        "ZIPFILE");
$DESCRIPTOR(cli_infile,         "INFILE");
$DESCRIPTOR(zip_command,        "zip ");

static int show_VMSCLI_help;

#if !defined(zip_clitable)
#  define zip_clitable ZIP_CLITABLE
#endif
#if defined(__DECC) || defined(__GNUC__)
extern void *zip_clitable;
#else
globalref void *zip_clitable;
#endif

/* extern unsigned long LIB$GET_INPUT(void), LIB$SIG_TO_RET(void); */

#ifndef __STARLET_LOADED
#ifndef sys$bintim
#  define sys$bintim SYS$BINTIM
#endif
#ifndef sys$numtim
#  define sys$numtim SYS$NUMTIM
#endif
extern int sys$bintim ();
extern int sys$numtim ();
#endif /* !__STARLET_LOADED */
#ifndef cli$dcl_parse
#  define cli$dcl_parse CLI$DCL_PARSE
#endif
#ifndef cli$present
#  define cli$present CLI$PRESENT
#endif
#ifndef cli$get_value
#  define cli$get_value CLI$GET_VALUE
#endif
extern unsigned long cli$dcl_parse ();
extern unsigned long cli$present ();
extern unsigned long cli$get_value ();

unsigned long vms_zip_cmdline (int *, char ***);
static unsigned long get_list (struct dsc$descriptor_s *,
                               struct dsc$descriptor_d *, int,
                               char **, unsigned long *, unsigned long *);
static unsigned long get_time (struct dsc$descriptor_s *qual, char *timearg);
static unsigned long check_cli (struct dsc$descriptor_s *);
static int verbose_command = 0;


#ifdef TEST
int
main(int argc, char **argv)
{
    return (vms_zip_cmdline(&argc, &argv));
}
#endif /* TEST */


unsigned long
vms_zip_cmdline (int *argc_p, char ***argv_p)
{
/*
**  Routine:    vms_zip_cmdline
**
**  Function:
**
**      Parse the DCL command line and create a fake argv array to be
**      handed off to Zip.
**
**      NOTE: the argv[] is built as we go, so all the parameters are
**      checked in the appropriate order!!
**
**  Formal parameters:
**
**      argc_p          - Address of int to receive the new argc
**      argv_p          - Address of char ** to receive the argv address
**
**  Calling sequence:
**
**      status = vms_zip_cmdline (&argc, &argv);
**
**  Returns:
**
**      SS$_NORMAL      - Success.
**      SS$_INSFMEM     - A malloc() or realloc() failed
**      SS$_ABORT       - Bad time value
**
*/
    register unsigned long status;
    char options[48];
    char *the_cmd_line;                 /* buffer for argv strings */
    unsigned long cmdl_size;            /* allocated size of buffer */
    unsigned long cmdl_len;             /* used size of buffer */
    char *ptr;
    int  x, len;

    int new_argc;
    char **new_argv;

    struct dsc$descriptor_d work_str;
    struct dsc$descriptor_d foreign_cmdline;

    init_dyndesc(work_str);
    init_dyndesc(foreign_cmdline);

    /*
    **  See if the program was invoked by the CLI (SET COMMAND) or by
    **  a foreign command definition.  Check for /YYZ_ZIP, which is a
    **  valid default qualifier solely for this test.
    */
    show_VMSCLI_help = TRUE;
    status = check_cli(&cli_yyz);
    if (!(status & 1)) {
        lib$get_foreign(&foreign_cmdline);
        /*
        **  If nothing was returned or the first character is a "-", then
        **  assume it's a UNIX-style command and return.
        */
        if (foreign_cmdline.dsc$w_length == 0)
            return (SS$_NORMAL);
        if ((*(foreign_cmdline.dsc$a_pointer) == '-') ||
            ((foreign_cmdline.dsc$w_length > 1) &&
             (*(foreign_cmdline.dsc$a_pointer) == '"') &&
             (*(foreign_cmdline.dsc$a_pointer + 1) == '-'))) {
            show_VMSCLI_help = FALSE;
            return (SS$_NORMAL);
        }

        str$concat(&work_str, &zip_command, &foreign_cmdline);
        status = cli$dcl_parse(&work_str, &zip_clitable, lib$get_input,
                        lib$get_input, 0);
        if (!(status & 1)) return (status);
    }

    /*
    **  There's always going to be a new_argv[] because of the image name.
    */
    if ((the_cmd_line = (char *) malloc(cmdl_size = ARGBSIZE_UNIT)) == NULL)
        return (SS$_INSFMEM);

    strcpy(the_cmd_line, "zip");
    cmdl_len = sizeof("zip");

    /*
    **  First, check to see if any of the regular options were specified.
    */

    options[0] = '-';
    ptr = &options[1];          /* Point to temporary buffer */

    /*
    **  Delete the specified files from the zip file?
    */
    status = cli$present(&cli_delete);
    if (status & 1)
        *ptr++ = 'd';

    /*
    **  Freshen (only changed files).
    */
    status = cli$present(&cli_freshen);
    if (status & 1)
        *ptr++ = 'f';

    /*
    **  Delete the files once they've been added to the zip file.
    */
    status = cli$present(&cli_move);
    if (status & 1)
        *ptr++ = 'm';

    /*
    **  Add changed and new files.
    */
    status = cli$present(&cli_update);
    if (status & 1)
        *ptr++ = 'u';

    /*
    **  Check for the compression level (-0 through -9).
    */
    status = cli$present(&cli_level);
    if (status & 1) {

        unsigned long binval;

        status = cli$get_value(&cli_level, &work_str);
        status = ots$cvt_tu_l(&work_str, &binval);
        if (!(status & 1) || (binval > 9)) {
           return (SS$_ABORT);
        }
        *ptr++ = binval + '0';
    }

    /*
    **  Adjust offsets of zip archive entries.
    */
    status = cli$present(&cli_adjust);
    if (status & 1)
        *ptr++ = 'A';

    /*
    **  Add comments?
    */
    status = cli$present(&cli_comments);
    if (status & 1) {
/*        while ((status = cli$get_value(&cli_comments, &work_str)) & 1) {
            if (strncmp(work_str.dsc$a_pointer,"ZIP",3) == 0)
                *ptr++ = 'z';
            if (strncmp(work_str.dsc$a_pointer,"FIL",3) == 0)
                *ptr++ = 'c';
        } */
        if ((status = cli$present(&cli_comment_zipfile)) & 1)
            *ptr++ = 'z';
        if ((status = cli$present(&cli_comment_files)) & 1)
            *ptr++ = 'c';
    }

    /*
    **  Do not add/modify directory entries.
    */
    status = cli$present(&cli_dirnames);
    if (!(status & 1))
        *ptr++ = 'D';

    /*
    **  Encrypt?
    */
    status = cli$present(&cli_encrypt);
    if (status & 1)
        if ((status = cli$get_value(&cli_encrypt, &work_str)) & 1) {
            x = cmdl_len;
            cmdl_len += work_str.dsc$w_length + 4;
            CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
            strcpy(&the_cmd_line[x], "-P");
            strncpy(&the_cmd_line[x+3], work_str.dsc$a_pointer,
                    work_str.dsc$w_length);
            the_cmd_line[cmdl_len-1] = '\0';
        } else {
            *ptr++ = 'e';
        }

    /*
    **  Fix the zip archive structure.
    */
    status = cli$present(&cli_fix_archive);
    if (status & 1) {
        *ptr++ = 'F';
        if ((status = cli$present(&cli_fix_full)) & 1) {
            *ptr++ = 'F';
        }
    }

    /*
    **  Append (allow growing of existing zip file).
    */
    status = cli$present(&cli_append);
    if (status & 1)
        *ptr++ = 'g';

    /*
    **  Show the help.
    */
    status = cli$present(&cli_help);
    if (status & 1)
        *ptr++ = 'h';

    /*
    **  Junk path names (directory specs).
    */
    status = cli$present(&cli_junk);
    if (status & 1)
        *ptr++ = 'j';

    /*
    **  Simulate zip file made by PKZIP.
    */
    status = cli$present(&cli_pkzip);
    if (status & 1)
        *ptr++ = 'k';

    /*
    **  Translate end-of-line.
    */
    status = cli$present(&cli_translate_eol);
    if (status & 1) {
        *ptr++ = 'l';
        if ((status = cli$present(&cli_transl_eol_crlf)) & 1) {
            *ptr++ = 'l';
        }
    }

    /*
    **  Show the software license.
    */
    status = cli$present(&cli_license);
    if (status & 1)
        *ptr++ = 'L';

    /*
    **  Set zip file time to time of latest file in it.
    */
    status = cli$present(&cli_latest);
    if (status & 1)
        *ptr++ = 'o';

    /*
    **  Store full path (default).
    */
    status = cli$present(&cli_full_path);
    if (status == CLI$_PRESENT)
        *ptr++ = 'p';
    else if (status == CLI$_NEGATED)
        *ptr++ = 'j';

    /*
    **  Junk Zipfile prefix (SFX stub etc.).
    */
    status = cli$present(&cli_unsfx);
    if (status & 1)
        *ptr++ = 'J';

    /*
    **  Recurse through subdirectories.
    */
    status = cli$present(&cli_recurse);
    if (status & 1) {
        if ((status = cli$present(&cli_recurse_fnames)) & 1)
            *ptr++ = 'R';
        else
            *ptr++ = 'r';
    }

    /*
    **  Be verbose.
    */
    status = cli$present(&cli_verbose);
    if (status & 1) {
        int i;
        int verbo = 0;

        /* /VERBOSE */
        if ((status = cli$present(&cli_verbose_command)) & 1)
        {
            /* /VERBOSE = COMMAND */
            verbose_command = 1;
        }

        /* Note that any or all of the following options may be
           specified, and the maximum one is used.
        */
        if ((status = cli$present(&cli_verbose_normal)) & 1)
            /* /VERBOSE [ = NORMAL ] */
            verbo = 1;
        if ((status = cli$present(&cli_verbose_more)) & 1)
            /* /VERBOSE = MORE */
            verbo = 2;
        if ((status = cli$present(&cli_verbose_debug)) & 1) {
            /* /VERBOSE = DEBUG */
            verbo = 3;
        }
        for (i = 0; i < verbo; i++)
            *ptr++ = 'v';
    }

    /*
    **  Quiet mode.
    **  (Quiet mode is processed after verbose, because a "-v" modifier
    **  resets "noisy" to 1.)
    */
    status = cli$present(&cli_quiet);
    if (status & 1)
        *ptr++ = 'q';

    /*
    **  Suppress creation of any extra field.
    */
    status = cli$present(&cli_extra_fields);
    if (!(status & 1))
        *ptr++ = 'X';

    /*
    **  Save the VMS file attributes (and all allocated blocks?).
    */
    status = cli$present(&cli_vms);
    if (status & 1) {
        /* /VMS */
        *ptr++ = 'V';
        if ((status = cli$present(&cli_vms_all)) & 1) {
            /* /VMS = ALL */
            *ptr++ = 'V';
        }
    }

    /*
    **  Keep the VMS version number as part of the file name when stored.
    */
    status = cli$present(&cli_keep_version);
    if (status & 1)
        *ptr++ = 'w';

    /*
    **  `Batch' processing: read filenames to archive from stdin
    **  or the specified file.
    */
    status = cli$present(&cli_batch);
    if (status & 1) {
        status = cli$get_value(&cli_batch, &work_str);
        if (status & 1) {
            work_str.dsc$a_pointer[work_str.dsc$w_length] = '\0';
            if ((stdin = freopen(work_str.dsc$a_pointer, "r", stdin)) == NULL)
            {
                sprintf(errbuf, "could not open list file: %s",
                        work_str.dsc$a_pointer);
                ziperr(ZE_PARMS, errbuf);
            }
        }
        *ptr++ = '@';
    }

    /*
    **  Now copy the final options string to the_cmd_line.
    */
    len = ptr - &options[0];
    if (len > 1) {
        options[len] = '\0';
        x = cmdl_len;
        cmdl_len += len + 1;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], options);
    }

    /*
    **
    **  OK.  We've done all the regular options, so check for -b (temporary
    **  file path), -t (exclude before time), -n (special suffixes), zipfile,
    **  files to zip, and exclude list.
    **
    */
    status = cli$present(&cli_temp_path);
    if (status & 1) {
        status = cli$get_value(&cli_temp_path, &work_str);
        x = cmdl_len;
        cmdl_len += work_str.dsc$w_length + 4;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-b");
        strncpy(&the_cmd_line[x+3], work_str.dsc$a_pointer,
                work_str.dsc$w_length);
        the_cmd_line[cmdl_len-1] = '\0';
    }

    /*
    **  Demand that all input files exist (and wildcards match something).
    */
#define OPT_MM  "-MM"           /* Input file specs must exist. */
    status = cli$present(&cli_must_match);
    if (status & 1) {
        x = cmdl_len;
        cmdl_len += strlen( OPT_MM)+ 1;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy( &the_cmd_line[ x], OPT_MM);
    }

    /*
    **  Handle "-t mmddyyyy".
    */
    status = cli$present(&cli_since);
    if (status & 1) {
        char since_time[9];

        status = get_time(&cli_since, &since_time[0]);
        if (!(status & 1)) return (status);

        /*
        **  Now let's add the option "-t mmddyyyy" to the new command line.
        */
        x = cmdl_len;
        cmdl_len += (3 + 9);
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-t");
        strcpy(&the_cmd_line[x+3], since_time);
    }

    /*
    **  Handle "-tt mmddyyyy".
    */
    status = cli$present(&cli_before);
    if (status & 1) {
        char before_time[9];

        status = get_time(&cli_before, &before_time[0]);
        if (!(status & 1)) return (status);

        /*
        **  Now let's add the option "-tt mmddyyyy" to the new command line.
        */
        x = cmdl_len;
        cmdl_len += (4 + 9);
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-tt");
        strcpy(&the_cmd_line[x+4], before_time);
    }

    /*
    **  Handle "-n suffix:suffix:...".  (File types to store only.)
    */
    status = cli$present(&cli_store_types);
    if (status & 1) {
        x = cmdl_len;
        cmdl_len += 3;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-n");

        status = get_list(&cli_store_types, &foreign_cmdline, ':',
                          &the_cmd_line, &cmdl_size, &cmdl_len);
        if (!(status & 1)) return (status);
    }

    /*
    **  Now get the specified zip file name.
    */
    status = cli$present(&cli_zipfile);
    if (status & 1) {
        status = cli$get_value(&cli_zipfile, &work_str);

        x = cmdl_len;
        cmdl_len += work_str.dsc$w_length + 1;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strncpy(&the_cmd_line[x], work_str.dsc$a_pointer,
                work_str.dsc$w_length);
        the_cmd_line[cmdl_len-1] = '\0';

    }

    /*
    **  Run through the list of input files.
    */
    status = cli$present(&cli_infile);
    if (status & 1) {
        status = get_list(&cli_infile, &foreign_cmdline, '\0',
                          &the_cmd_line, &cmdl_size, &cmdl_len);
        if (!(status & 1)) return (status);
    }

    /*
    **  List file containing exclude patterns present? ("-x@exclude.lst")
    */
    status = cli$present(&cli_exlist);
    if (status & 1) {
        status = cli$get_value(&cli_exlist, &work_str);
        x = cmdl_len;
        cmdl_len += work_str.dsc$w_length + 4;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strncpy(&the_cmd_line[x], "-x@", 3);
        strncpy(&the_cmd_line[x+3], work_str.dsc$a_pointer,
                work_str.dsc$w_length);
        the_cmd_line[cmdl_len-1] = '\0';
    }

    /*
    **  Any files to exclude? ("-x file file")
    */
    status = cli$present(&cli_exclude);
    if (status & 1) {
        x = cmdl_len;
        cmdl_len += 3;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-x");

        status = get_list(&cli_exclude, &foreign_cmdline, '\0',
                          &the_cmd_line, &cmdl_size, &cmdl_len);
        if (!(status & 1)) return (status);
    }

    /*
    **  List file containing include patterns present? ("-x@exclude.lst")
    */
    status = cli$present(&cli_inlist);
    if (status & 1) {
        status = cli$get_value(&cli_inlist, &work_str);
        x = cmdl_len;
        cmdl_len += work_str.dsc$w_length + 4;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strncpy(&the_cmd_line[x], "-i@", 3);
        strncpy(&the_cmd_line[x+3], work_str.dsc$a_pointer,
                work_str.dsc$w_length);
        the_cmd_line[cmdl_len-1] = '\0';
    }

    /*
    **  Any files to include? ("-i file file")
    */
    status = cli$present(&cli_include);
    if (status & 1) {
        x = cmdl_len;
        cmdl_len += 3;
        CHECK_BUFFER_ALLOCATION(the_cmd_line, cmdl_size, cmdl_len)
        strcpy(&the_cmd_line[x], "-i");

        status = get_list(&cli_exclude, &foreign_cmdline, '\0',
                          &the_cmd_line, &cmdl_size, &cmdl_len);
        if (!(status & 1)) return (status);
    }


    /*
    **  We have finished collecting the strings for the argv vector,
    **  release unused space.
    */
    if ((the_cmd_line = (char *) realloc(the_cmd_line, cmdl_len)) == NULL)
        return (SS$_INSFMEM);

    /*
    **  Now that we've built our new UNIX-like command line, count the
    **  number of args and build an argv array.
    */
    for (new_argc = 0, x = 0; x < cmdl_len; x++)
        if (the_cmd_line[x] == '\0')
            new_argc++;

    /*
    **  Allocate memory for the new argv[].  The last element of argv[]
    **  is supposed to be NULL, so allocate enough for new_argc+1.
    */
    if ((new_argv = (char **) calloc(new_argc+1, sizeof(char *))) == NULL)
        return (SS$_INSFMEM);

    /*
    **  For each option, store the address in new_argv[] and convert the
    **  separating blanks to nulls so each argv[] string is terminated.
    */
    for (ptr = the_cmd_line, x = 0; x < new_argc; x++) {
        new_argv[x] = ptr;
        ptr += strlen(ptr) + 1;
    }
    new_argv[new_argc] = NULL;

#if defined(TEST) || defined(DEBUG)
    printf("new_argc    = %d\n", new_argc);
    for (x = 0; x < new_argc; x++)
        printf("new_argv[%d] = %s\n", x, new_argv[x]);
#endif /* TEST || DEBUG */

    /* Show the complete UNIX command line, if requested. */
    if (verbose_command != 0)
    {
        printf( "   UNIX command line args (argc = %d):\n", new_argc);
        for (x = 0; x < new_argc; x++)
            printf( "%s\n", new_argv[ x]);
        printf( "\n");
    }

    /*
    **  All finished.  Return the new argc and argv[] addresses to Zip.
    */
    *argc_p = new_argc;
    *argv_p = new_argv;

    return (SS$_NORMAL);
}



static unsigned long
get_list (struct dsc$descriptor_s *qual, struct dsc$descriptor_d *rawtail,
          int delim, char **p_str, unsigned long *p_size, unsigned long *p_end)
{
/*
**  Routine:    get_list
**
**  Function:   This routine runs through a comma-separated CLI list
**              and copies the strings to the argv buffer.  The
**              specified separation character is used to separate
**              the strings in the argv buffer.
**
**              All unquoted strings are converted to lower-case.
**
**  Formal parameters:
**
**      qual    - Address of descriptor for the qualifier name
**      rawtail - Address of descriptor for the full command line tail
**      delim   - Character to use to separate the list items
**      p_str   - Address of pointer pointing to output buffer (argv strings)
**      p_size  - Address of number containing allocated size for output string
**      p_end   - Address of number containing used length in output buf
**
*/

    register unsigned long status;
    struct dsc$descriptor_d work_str;

    init_dyndesc(work_str);

    status = cli$present(qual);
    if (status & 1) {

        unsigned long len, old_len;
        long ind, sind;
        int keep_case;
        char *src, *dst; int x;

        /*
        **  Just in case the string doesn't exist yet, though it does.
        */
        if (*p_str == NULL) {
            *p_size = ARGBSIZE_UNIT;
            if ((*p_str = (char *) malloc(*p_size)) == NULL)
                return (SS$_INSFMEM);
            len = 0;
        } else {
            len = *p_end;
        }

        while ((status = cli$get_value(qual, &work_str)) & 1) {
            old_len = len;
            len += work_str.dsc$w_length + 1;
            CHECK_BUFFER_ALLOCATION(*p_str, *p_size, len)

            /*
            **  Look for the filename in the original foreign command
            **  line to see if it was originally quoted.  If so, then
            **  don't convert it to lowercase.
            */
            keep_case = FALSE;
            str$find_first_substring(rawtail, &ind, &sind, &work_str);
            if ((ind > 1 && *(rawtail->dsc$a_pointer + ind - 2) == '"') ||
                (ind == 0))
                keep_case = TRUE;

            /*
            **  Copy the string to the buffer, converting to lowercase.
            */
            src = work_str.dsc$a_pointer;
            dst = *p_str+old_len;
            for (x = 0; x < work_str.dsc$w_length; x++) {
                if (!keep_case && ((*src >= 'A') && (*src <= 'Z')))
                    *dst++ = *src++ + 32;
                else
                    *dst++ = *src++;
            }
            if (status == CLI$_COMMA)
                (*p_str)[len-1] = (char)delim;
            else
                (*p_str)[len-1] = '\0';
        }
        *p_end = len;
    }

    return (SS$_NORMAL);

}


static unsigned long
get_time (struct dsc$descriptor_s *qual, char *timearg)
{
/*
**  Routine:    get_time
**
**  Function:   This routine reads the argument string of the qualifier
**              "qual" that should be a VMS syntax date-time string.  The
**              date-time string is converted into the standard format
**              "mmddyyyy", specifying an absolute date.  The converted
**              string is written into the 9 bytes wide buffer "timearg".
**
**  Formal parameters:
**
**      qual    - Address of descriptor for the qualifier name
**      timearg - Address of a buffer carrying the 8-char time string returned
**
*/

    register unsigned long status;
    struct dsc$descriptor_d time_str;
    struct quadword {
        long high;
        long low;
    } bintimbuf = {0,0};
#ifdef __DECC
#pragma member_alignment save
#pragma nomember_alignment
#endif  /* __DECC */
    struct tim {
        short year;
        short month;
        short day;
        short hour;
        short minute;
        short second;
        short hundred;
    } numtimbuf;
#ifdef __DECC
#pragma member_alignment restore
#endif

    init_dyndesc(time_str);

    status = cli$get_value(qual, &time_str);
    /*
    **  If a date is given, convert it to 64-bit binary.
    */
    if (time_str.dsc$w_length) {
        status = sys$bintim(&time_str, &bintimbuf);
        if (!(status & 1)) return (status);
        str$free1_dx(&time_str);
    }
    /*
    **  Now call $NUMTIM to get the month, day, and year.
    */
    status = sys$numtim(&numtimbuf, (bintimbuf.low ? &bintimbuf : NULL));
    /*
    **  Write the "mmddyyyy" string to the return buffer.
    */
    if (!(status & 1)) {
        *timearg = '\0';
    } else {
        sprintf(timearg, "%02d%02d%04d", numtimbuf.month,
                numtimbuf.day, numtimbuf.year);
    }
    return (status);
}


static unsigned long
check_cli (struct dsc$descriptor_s *qual)
{
/*
**  Routine:    check_cli
**
**  Function:   Check to see if a CLD was used to invoke the program.
**
**  Formal parameters:
**
**      qual    - Address of descriptor for qualifier name to check.
**
*/
    lib$establish(lib$sig_to_ret);      /* Establish condition handler */
    return (cli$present(qual));         /* Just see if something was given */
}


#ifndef TEST

void VMSCLI_help(void)  /* VMSCLI version */
/* Print help (along with license info) to stdout. */
{
  extent i;             /* counter for help array */

  /* help array */
  static char *text[] = {
"Zip %s (%s). Usage: (zip :== $ dev:[dir]zip_cli.exe)",
"zip zipfile[.zip] [list] [/EXCL=(xlist)] /options /modifiers",
"  The default action is to add or replace zipfile entries from list, except",
"  those in xlist. The include file list may contain the special name \"-\" to",
"  compress standard input.  If both zipfile and list are omitted, zip",
"  compresses stdin to stdout.",
"  Type zip -h for Unix-style flags.",
"  Major options include:",
"    /DELETE, /FRESHEN, /MOVE, /UPDATE, /TEST, /COMMENTS[={ZIP_FILE|FILES}],",
"    /LATEST, /ADJUST_OFFSETS, /FIX_ARCHIVE[={NORMAL|FULL}], /UNSFX,",
"  Modifiers include:",
"    /EXCLUDE=(file_list), /EXLIST=file, /INCLUDE=(file_list), /INLIST=file,",
"    /BATCH[=list_file], /BEFORE=\"creation_time\", /SINCE=\"creation_time\",",
"    /NORECURSE|/RECURSE[={PATH|FILENAMES}], /STORE_TYPES=(type_list),",
#if CRYPT
"\
    /QUIET, /VERBOSE[={MORE|DEBUG}], /[NO]DIRNAMES, /JUNK, /ENCRYPT[=\"pwd\"],\
",
#else /* !CRYPT */
"    /QUIET, /VERBOSE[={MORE|DEBUG}], /[NO]DIRNAMES, /JUNK,",
#endif /* ?CRYPT */
"    /LEVEL=[0-9], /[NO]EXTRA_FIELDS, /[NO]KEEP_VERSION, /MUST_MATCH,",
"    /NOVMS|/VMS[=ALL], /TEMP_PATH=directory, /TRANSLATE_EOL[={LF|CRLF}],",
"    /[NO]PKZIP"
  };

  if (!show_VMSCLI_help) {
     help();
     return;
  }

  for (i = 0; i < sizeof(copyright)/sizeof(char *); i++)
  {
    printf(copyright[i], "zip");
    putchar('\n');
  }
  for (i = 0; i < sizeof(text)/sizeof(char *); i++)
  {
    printf(text[i], VERSION, REVDATE);
    putchar('\n');
  }
} /* end function VMSCLI_help() */

#endif /* !TEST */
