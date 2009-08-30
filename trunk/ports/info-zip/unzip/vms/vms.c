/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*---------------------------------------------------------------------------

  vms.c                                        Igor Mandrichenko and others

  This file contains routines to extract VMS file attributes from a zipfile
  extra field and create a file with these attributes.  The code was almost
  entirely written by Igor, with a couple of routines by GRR and lots of
  modifications and fixes by Christian Spieler.

  Contains:  check_format()
             open_outfile()
             find_vms_attrs()
             flush()
             close_outfile()
             dos_to_unix_time()         (TIMESTAMP only)
             stamp_file()               (TIMESTAMP only)
             do_wild()
             mapattr()
             mapname()
             checkdir()
             check_for_newer()
             return_VMS
             screensize()
             screenlinewrap()
             version()

  ---------------------------------------------------------------------------*/

#ifdef VMS                      /* VMS only! */

#define UNZIP_INTERNAL

#include "unzip.h"
#include "vms.h"
#include "vmsdefs.h"

#ifdef MORE
#  include <ttdef.h>
#endif
#include <unixlib.h>

#include <lib$routines.h>
#include <stsdef.h>

/* On VAX, define Goofy VAX Type-Cast to obviate /standard = vaxc.
   Otherwise, lame system headers on VAX cause compiler warnings.
   (GNU C may define vax but not __VAX.)
*/
#ifdef vax
#  define __VAX 1
#endif

#ifdef __VAX
#  define GVTC (unsigned int)
#else
#  define GVTC
#endif

/* With GNU C, some FAB bits may be declared only as masks, not as
 * structure bits.
 */
#ifdef __GNUC__
#  define OLD_FABDEF 1
#endif

#define ASYNCH_QIO              /* Use asynchronous PK-style QIO writes */

/* buffer size for a single block write (using RMS or QIO WRITEVBLK),
   must be less than 64k and a multiple of 512 ! */
#define BUFS512 (((OUTBUFSIZ>0xFFFF) ? 0xFFFF : OUTBUFSIZ) & (~511))
/* buffer size for record output (RMS limit for max. record size) */
#define BUFSMAXREC 32767
/* allocation size for RMS and QIO output buffers */
#define BUFSALLOC (BUFS512 * 2 > BUFSMAXREC ? BUFS512 * 2 : BUFSMAXREC)
        /* locbuf size */

#define OK(s)   ((s)&1)         /* VMS success or warning status */
#define STRICMP(s1,s2)  STRNICMP(s1,s2,2147483647)

/*
 *   Local static storage
 */
static struct FAB       fileblk;
static struct XABDAT    dattim;
static struct XABRDT    rdt;
static struct RAB       rab;
static struct NAM       nam;

static struct FAB *outfab = NULL;
static struct RAB *outrab = NULL;
static struct XABFHC *xabfhc = NULL;
static struct XABDAT *xabdat = NULL;
static struct XABRDT *xabrdt = NULL;
static struct XABPRO *xabpro = NULL;
static struct XABKEY *xabkey = NULL;
static struct XABALL *xaball = NULL;
static struct XAB *first_xab = NULL, *last_xab = NULL;

static char query = '\0';

static uch rfm;

static uch locbuf[BUFSALLOC];           /* Space for 2 buffers of BUFS512 */
static unsigned loccnt = 0;
static uch *locptr;
static char got_eol = 0;

struct bufdsc
{
    struct bufdsc *next;
    uch *buf;
    unsigned bufcnt;
};

static struct bufdsc b1, b2, *curbuf;   /* buffer ring for asynchronous I/O */

static int  _flush_blocks(__GPRO__ uch *rawbuf, unsigned size, int final_flag),
            _flush_stream(__GPRO__ uch *rawbuf, unsigned size, int final_flag),
            _flush_varlen(__GPRO__ uch *rawbuf, unsigned size, int final_flag),
            _flush_qio(__GPRO__ uch *rawbuf, unsigned size, int final_flag),
            _close_rms(__GPRO),
            _close_qio(__GPRO),
#ifdef ASYNCH_QIO
            WriteQIO(__GPRO__ uch *buf, unsigned len),
#endif
            WriteBuffer(__GPRO__ uch *buf, unsigned len),
            WriteRecord(__GPRO__ uch *rec, unsigned len);

static int  (*_flush_routine)(__GPRO__ uch *rawbuf, unsigned size,
                              int final_flag),
            (*_close_routine)(__GPRO);

static void init_buf_ring(void);
static void set_default_datetime_XABs(__GPRO);
static int  create_default_output(__GPRO),
            create_rms_output(__GPRO),
            create_qio_output(__GPRO);
static int  replace(__GPRO);
static int  find_vms_attrs(__GPRO);
static void free_up(void);
#ifdef CHECK_VERSIONS
static int  get_vms_version(char *verbuf, int len);
#endif /* CHECK_VERSIONS */
static unsigned find_eol(uch *p, unsigned n, unsigned *l);
#ifdef TIMESTAMP
static time_t mkgmtime(struct tm *tm);
static void uxtime2vmstime(time_t utimeval, long int binval[2]);
#endif /* TIMESTAMP */
static void vms_msg(__GPRO__ char *string, int status);

/* 2004-11-23 SMS.
 *
 *       get_rms_defaults().
 *
 *    Get user-specified values from (DCL) SET RMS_DEFAULT.  FAB/RAB
 *    items of particular interest are:
 *
 *       fab$w_deq         default extension quantity (blocks) (write).
 *       rab$b_mbc         multi-block count.
 *       rab$b_mbf         multi-buffer count (used with rah and wbh).
 */

#define DIAG_FLAG (uO.vflag >= 2)

/* Default RMS parameter values.
 * The default extend quantity (deq) should not matter much here, as the
 * initial allocation should always be set according to the known file
 * size, and no extension should be needed.
 */

#define RMS_DEQ_DEFAULT 16384   /* About 1/4 the max (65535 blocks). */
#define RMS_MBC_DEFAULT 127     /* The max, */
#define RMS_MBF_DEFAULT 2       /* Enough to enable rah and wbh. */

/* GETJPI item descriptor structure. */
typedef struct
{
    short buf_len;
    short itm_cod;
    void *buf;
    int *ret_len;
} jpi_item_t;

/* Durable storage */
static int rms_defaults_known = 0;

/* JPI item buffers. */
static unsigned short rms_ext;
static char rms_mbc;
static unsigned char rms_mbf;

/* Active RMS item values. */
unsigned short rms_ext_active;
char rms_mbc_active;
unsigned char rms_mbf_active;

/* GETJPI item lengths. */
static int rms_ext_len;         /* Should come back 2. */
static int rms_mbc_len;         /* Should come back 1. */
static int rms_mbf_len;         /* Should come back 1. */

/* Desperation attempts to define unknown macros.  Probably doomed.
 * If these get used, expect sys$getjpiw() to return %x00000014 =
 * %SYSTEM-F-BADPARAM, bad parameter value.
 * They keep compilers with old header files quiet, though.
 */
#ifndef JPI$_RMS_EXTEND_SIZE
#  define JPI$_RMS_EXTEND_SIZE 542
#endif /* ndef JPI$_RMS_EXTEND_SIZE */

#ifndef JPI$_RMS_DFMBC
#  define JPI$_RMS_DFMBC 535
#endif /* ndef JPI$_RMS_DFMBC */

#ifndef JPI$_RMS_DFMBFSDK
#  define JPI$_RMS_DFMBFSDK 536
#endif /* ndef JPI$_RMS_DFMBFSDK */

/* GETJPI item descriptor set. */

struct
{
    jpi_item_t rms_ext_itm;
    jpi_item_t rms_mbc_itm;
    jpi_item_t rms_mbf_itm;
    int term;
} jpi_itm_lst =
     { { 2, JPI$_RMS_EXTEND_SIZE, &rms_ext, &rms_ext_len },
       { 1, JPI$_RMS_DFMBC, &rms_mbc, &rms_mbc_len },
       { 1, JPI$_RMS_DFMBFSDK, &rms_mbf, &rms_mbf_len },
       0
     };

static int get_rms_defaults()
{
    int sts;

    /* Get process RMS_DEFAULT values. */

    sts = sys$getjpiw(0, 0, 0, &jpi_itm_lst, 0, 0, 0);
    if ((sts& STS$M_SEVERITY) != STS$M_SUCCESS)
    {
        /* Failed.  Don't try again. */
        rms_defaults_known = -1;
    }
    else
    {
        /* Fine, but don't come back. */
        rms_defaults_known = 1;
    }

    /* Limit the active values according to the RMS_DEFAULT values. */

    if (rms_defaults_known > 0)
    {
        /* Set the default values. */
        rms_ext_active = RMS_DEQ_DEFAULT;
        rms_mbc_active = RMS_MBC_DEFAULT;
        rms_mbf_active = RMS_MBF_DEFAULT;

        /* Default extend quantity.  Use the user value, if set. */
        if (rms_ext > 0)
        {
            rms_ext_active = rms_ext;
        }

        /* Default multi-block count.  Use the user value, if set. */
        if (rms_mbc > 0)
        {
            rms_mbc_active = rms_mbc;
        }

        /* Default multi-buffer count.  Use the user value, if set. */
        if (rms_mbf > 0)
        {
            rms_mbf_active = rms_mbf;
        }
    }

    if (DIAG_FLAG)
    {
        fprintf(stderr, "Get RMS defaults.  getjpi sts = %%x%08x.\n", sts);

        if (rms_defaults_known > 0)
        {
            fprintf(stderr,
              "               Default: deq = %6d, mbc = %3d, mbf = %3d.\n",
              rms_ext, rms_mbc, rms_mbf);
        }
    }
    return sts;
}


int check_format(__G)
    __GDEF
{
    int rtype;
    struct FAB fab;

    fab = cc$rms_fab;
    fab.fab$l_fna = G.zipfn;
    fab.fab$b_fns = strlen(G.zipfn);

    if ((sys$open(&fab) & 1) == 0)
    {
        Info(slide, 1, ((char *)slide, "\n\
     error:  cannot open zipfile [ %s ] (access denied?).\n\n",
          FnFilter1(G.zipfn)));
        return PK_ERR;
    }
    rtype = fab.fab$b_rfm;
    sys$close(&fab);

    if (rtype == FAB$C_VAR || rtype == FAB$C_VFC)
    {
        Info(slide, 1, ((char *)slide, "\n\
     Error:  zipfile is in variable-length record format.  Please\n\
     run \"bilf l %s\" to convert the zipfile to stream-LF\n\
     record format.  (BILF is available at various VMS archives.)\n\n",
          FnFilter1(G.zipfn)));
        return PK_ERR;
    }

    return PK_COOL;
}



#define PRINTABLE_FORMAT(x)      ( (x) == FAB$C_VAR     \
                                || (x) == FAB$C_STMLF   \
                                || (x) == FAB$C_STMCR   \
                                || (x) == FAB$C_STM     )

/* VMS extra field types */
#define VAT_NONE    0
#define VAT_IZ      1   /* old Info-ZIP format */
#define VAT_PK      2   /* PKWARE format */

/*
 *  open_outfile() assignments:
 *
 *  VMS attributes ?        create_xxx      _flush_xxx
 *  ----------------        ----------      ----------
 *  not found               'default'       text mode ?
 *                                          yes -> 'stream'
 *                                          no  -> 'block'
 *
 *  yes, in IZ format       'rms'           uO.cflag ?
 *                                          yes -> switch (fab.rfm)
 *                                              VAR  -> 'varlen'
 *                                              STM* -> 'stream'
 *                                              default -> 'block'
 *                                          no -> 'block'
 *
 *  yes, in PK format       'qio'           uO.cflag ?
 *                                          yes -> switch (pka_rattr)
 *                                              VAR  -> 'varlen'
 *                                              STM* -> 'stream'
 *                                              default -> 'block'
 *                                          no -> 'qio'
 *
 *  "text mode" == G.pInfo->textmode || (uO.cflag && !uO.bflag)
 *  (simplified, for complete expression see create_default_output() code)
 */

int open_outfile(__G)           /* return 1 (PK_WARN) if fail */
    __GDEF
{
    /* Get process RMS_DEFAULT values, if not already done. */
    if (rms_defaults_known == 0)
    {
        get_rms_defaults();
    }

    switch (find_vms_attrs(__G))
    {
        case VAT_NONE:
        default:
            return  create_default_output(__G);
        case VAT_IZ:
            return  create_rms_output(__G);
        case VAT_PK:
            return  create_qio_output(__G);
    }
}

static void init_buf_ring()
{
    locptr = &locbuf[0];
    loccnt = 0;

    b1.buf = &locbuf[0];
    b1.bufcnt = 0;
    b1.next = &b2;
    b2.buf = &locbuf[BUFS512];
    b2.bufcnt = 0;
    b2.next = &b1;
    curbuf = &b1;
}


/* Static data storage for time conversion: */

/*   string constants for month names */
static ZCONST char *month[] =
            {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
             "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

/*   buffer for time string */
static char timbuf[24];         /* length = first entry in "date_str" + 1 */

/*   fixed-length string descriptor for timbuf: */
static ZCONST struct dsc$descriptor date_str =
            {sizeof(timbuf)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, timbuf};


static void set_default_datetime_XABs(__GPRO)
{
    unsigned yr, mo, dy, hh, mm, ss;
#ifdef USE_EF_UT_TIME
    iztimes z_utime;
    struct tm *t;

    if (G.extra_field &&
#ifdef IZ_CHECK_TZ
        G.tz_is_valid &&
#endif
        (ef_scan_for_izux(G.extra_field, G.lrec.extra_field_length, 0,
                          G.lrec.last_mod_dos_datetime, &z_utime, NULL)
         & EB_UT_FL_MTIME))
        t = localtime(&(z_utime.mtime));
    else
        t = (struct tm *)NULL;
    if (t != (struct tm *)NULL)
    {
        yr = t->tm_year + 1900;
        mo = t->tm_mon;
        dy = t->tm_mday;
        hh = t->tm_hour;
        mm = t->tm_min;
        ss = t->tm_sec;
    }
    else
    {
        yr = ((G.lrec.last_mod_dos_datetime >> 25) & 0x7f) + 1980;
        mo = ((G.lrec.last_mod_dos_datetime >> 21) & 0x0f) - 1;
        dy = (G.lrec.last_mod_dos_datetime >> 16) & 0x1f;
        hh = (G.lrec.last_mod_dos_datetime >> 11) & 0x1f;
        mm = (G.lrec.last_mod_dos_datetime >> 5) & 0x3f;
        ss = (G.lrec.last_mod_dos_datetime << 1) & 0x3e;
    }
#else /* !USE_EF_UT_TIME */

    yr = ((G.lrec.last_mod_dos_datetime >> 25) & 0x7f) + 1980;
    mo = ((G.lrec.last_mod_dos_datetime >> 21) & 0x0f) - 1;
    dy = (G.lrec.last_mod_dos_datetime >> 16) & 0x1f;
    hh = (G.lrec.last_mod_dos_datetime >> 11) & 0x1f;
    mm = (G.lrec.last_mod_dos_datetime >> 5) & 0x3f;
    ss = (G.lrec.last_mod_dos_datetime << 1) & 0x1f;
#endif /* ?USE_EF_UT_TIME */

    dattim = cc$rms_xabdat;     /* fill XABs with default values */
    rdt = cc$rms_xabrdt;
    sprintf(timbuf, "%02u-%3s-%04u %02u:%02u:%02u.00", dy, month[mo],
            yr, hh, mm, ss);
    sys$bintim(&date_str, &dattim.xab$q_cdt);
    memcpy(&rdt.xab$q_rdt, &dattim.xab$q_cdt, sizeof(rdt.xab$q_rdt));
}


static int create_default_output(__GPRO)      /* return 1 (PK_WARN) if fail */
{
    int ierr;
    int text_output, bin_fixed;

    /* extract the file in text (variable-length) format, when
     * a) explicitely requested by the user (through the -a option)
     *  or
     * b) piping to SYS$OUTPUT, unless "binary" piping was requested
     *    by the user (through the -b option)
     */
    text_output = G.pInfo->textmode ||
                  (uO.cflag &&
                   (!uO.bflag || (!(uO.bflag - 1) && G.pInfo->textfile)));
    /* use fixed length 512 byte record format for disk file when
     * a) explicitly requested by the user (-b option)
     *  and
     * b) entry is not extracted in text mode
     */
    bin_fixed = !text_output &&
                (uO.bflag != 0) && ((uO.bflag != 1) || !G.pInfo->textfile);

    rfm = FAB$C_STMLF;  /* Default, stream-LF format from VMS or UNIX */

    if (!uO.cflag)              /* Redirect output */
    {
        rab = cc$rms_rab;       /* fill RAB with default values */
        fileblk = cc$rms_fab;   /* fill FAB with default values */

        outfab = &fileblk;
        outfab->fab$l_xab = NULL;

        outrab = &rab;
        rab.rab$l_fab = outfab;

        if (text_output)
        {   /* Default format for output `real' text file */

            outfab->fab$b_rfm = FAB$C_VAR;      /* variable length records */
            outfab->fab$b_rat = FAB$M_CR;       /* implied (CR) carriage ctrl */
        }
        else if (bin_fixed)
        {   /* Default format for output `real' binary file */

            outfab->fab$b_rfm = FAB$C_FIX;      /* fixed length record format */
            outfab->fab$w_mrs = 512;            /* record size 512 bytes */
            outfab->fab$b_rat = 0;              /* no carriage ctrl */
        }
        else
        {   /* Default format for output misc (bin or text) file */

            outfab->fab$b_rfm = FAB$C_STMLF;    /* stream-LF record format */
            outfab->fab$b_rat = FAB$M_CR;       /* implied (CR) carriage ctrl */
        }

        outfab->fab$l_fna = G.filename;
        outfab->fab$b_fns = strlen(outfab->fab$l_fna);

        set_default_datetime_XABs(__G);
        dattim.xab$l_nxt = outfab->fab$l_xab;
        outfab->fab$l_xab = (void *) &dattim;

        outfab->fab$w_ifi = 0;  /* Clear IFI. It may be nonzero after ZIP */
        outfab->fab$b_fac = FAB$M_BRO | FAB$M_PUT;  /* {block|record} output */

        /* 2004-11-23 SMS.
         * If RMS_DEFAULT values have been determined, and have not been
         * set by the user, then set some FAB/RAB parameters for faster
         * output.  User-specified RMS_DEFAULT values override the
         * built-in default values, so if the RMS_DEFAULT values could
         * not be determined, then these (possibly unwise) values could
         * not be overridden, and hence will not be set.  Honestly,
         * this seems to be excessively cautious, but only old VMS
         * versions will be affected.
         */

        /* If RMS_DEFAULT (and adjusted active) values are available,
         * then set the FAB/RAB parameters.  If RMS_DEFAULT values are
         * not available, then suffer with the default behavior.
         */
        if (rms_defaults_known > 0)
        {
            /* Set the FAB/RAB parameters accordingly. */
            outfab-> fab$w_deq = rms_ext_active;
            outrab-> rab$b_mbc = rms_mbc_active;
            outrab-> rab$b_mbf = rms_mbf_active;

#ifdef OLD_FABDEF

            /* Truncate at EOF on close, as we may over-extend. */
            outfab-> fab$l_fop |= FAB$M_TEF ;

            /* If using multiple buffers, enable write-behind. */
            if (rms_mbf_active > 1)
            {
                outrab-> rab$l_rop |= RAB$M_WBH;
            }
        }

        /* Set the initial file allocation according to the file
         * size.  Also set the "sequential access only" flag, as
         * otherwise, on a file system with highwater marking
         * enabled, allocating space for a large file may lock the
         * disk for a long time (minutes).
         */
        outfab-> fab$l_alq = (unsigned) (G.lrec.ucsize+ 511)/ 512;
        outfab-> fab$l_fop |= FAB$M_SQO;

#else /* !OLD_FABDEF */

            /* Truncate at EOF on close, as we may over-extend. */
            outfab-> fab$v_tef = 1;

            /* If using multiple buffers, enable write-behind. */
            if (rms_mbf_active > 1)
            {
                outrab-> rab$v_wbh = 1;
            }
        }

        /* Set the initial file allocation according to the file
         * size.  Also set the "sequential access only" flag, as
         * otherwise, on a file system with highwater marking
         * enabled, allocating space for a large file may lock the
         * disk for a long time (minutes).
         */
        outfab-> fab$l_alq = (unsigned) (G.lrec.ucsize+ 511)/ 512;
        outfab-> fab$v_sqo = 1;

#endif /* ?OLD_FABDEF */

        ierr = sys$create(outfab);
        if (ierr == RMS$_FEX)
            ierr = replace(__G);

        if (ierr == 0)          /* Canceled */
            return (free_up(), PK_WARN);

        if (ERR(ierr))
        {
            char buf[256];

            sprintf(buf, "[ Cannot create output file %s ]\n", G.filename);
            vms_msg(__G__ buf, ierr);
            vms_msg(__G__ "", outfab->fab$l_stv);
            free_up();
            return PK_WARN;
        }

        if (!text_output)
        {
            rab.rab$l_rop |= (RAB$M_BIO | RAB$M_ASY);
        }
        rab.rab$b_rac = RAB$C_SEQ;

        if ((ierr = sys$connect(outrab)) != RMS$_NORMAL)
        {
#ifdef DEBUG
            vms_msg(__G__ "create_default_output: sys$connect failed.\n", ierr);
            vms_msg(__G__ "", outfab->fab$l_stv);
#endif
            Info(slide, 1, ((char *)slide,
                 "Can't create output file:  %s\n", FnFilter1(G.filename)));
            free_up();
            return PK_WARN;
        }
    }                   /* end if (!uO.cflag) */

    init_buf_ring();

    _flush_routine = text_output ? got_eol=0,_flush_stream : _flush_blocks;
    _close_routine = _close_rms;
    return PK_COOL;
}



static int create_rms_output(__GPRO)          /* return 1 (PK_WARN) if fail */
{
    int ierr;
    int text_output;

    /* extract the file in text (variable-length) format, when
     * piping to SYS$OUTPUT, unless "binary" piping was requested
     * by the user (through the -b option); the "-a" option is
     * ignored when extracting zip entries with VMS attributes saved
     */
    text_output = uO.cflag &&
                  (!uO.bflag || (!(uO.bflag - 1) && G.pInfo->textfile));

    rfm = outfab->fab$b_rfm;    /* Use record format from VMS extra field */

    if (uO.cflag)
    {
        if (text_output && !PRINTABLE_FORMAT(rfm))
        {
            Info(slide, 1, ((char *)slide,
               "[ File %s has illegal record format to put to screen ]\n",
               FnFilter1(G.filename)));
            free_up();
            return PK_WARN;
        }
    }
    else                        /* Redirect output */
    {
        rab = cc$rms_rab;       /* fill RAB with default values */

        /* The output FAB has already been initialized with the values
         * found in the Zip file's "VMS attributes" extra field */

        outfab->fab$l_fna = G.filename;
        outfab->fab$b_fns = strlen(outfab->fab$l_fna);

        /* If no XAB date/time, use attributes from non-VMS fields. */
        if (!(xabdat && xabrdt))
        {
            set_default_datetime_XABs(__G);

            if (xabdat == NULL)
            {
                dattim.xab$l_nxt = outfab->fab$l_xab;
                outfab->fab$l_xab = (void *) &dattim;
            }
        }

        outfab->fab$w_ifi = 0;  /* Clear IFI. It may be nonzero after ZIP */
        outfab->fab$b_fac = FAB$M_BIO | FAB$M_PUT;      /* block-mode output */

        /* 2004-11-23 SMS.
         * Set the "sequential access only" flag, as otherwise, on a
         * file system with highwater marking enabled, allocating space
         * for a large file may lock the disk for a long time (minutes).
         */
#ifdef OLD_FABDEF
        outfab-> fab$l_fop |= FAB$M_SQO;
#else /* !OLD_FABDEF */
        outfab-> fab$v_sqo = 1;
#endif /* ?OLD_FABDEF */

        ierr = sys$create(outfab);
        if (ierr == RMS$_FEX)
            ierr = replace(__G);

        if (ierr == 0)          /* Canceled */
            return (free_up(), PK_WARN);

        if (ERR(ierr))
        {
            char buf[256];

            sprintf(buf, "[ Cannot create output file %s ]\n", G.filename);
            vms_msg(__G__ buf, ierr);
            vms_msg(__G__ "", outfab->fab$l_stv);
            free_up();
            return PK_WARN;
        }

        if (outfab->fab$b_org & (FAB$C_REL | FAB$C_IDX)) {
            /* relative and indexed files require explicit allocation */
            ierr = sys$extend(outfab);
            if (ERR(ierr))
            {
                char buf[256];

                sprintf(buf, "[ Cannot allocate space for %s ]\n", G.filename);
                vms_msg(__G__ buf, ierr);
                vms_msg(__G__ "", outfab->fab$l_stv);
                free_up();
                return PK_WARN;
            }
        }

        outrab = &rab;
        rab.rab$l_fab = outfab;
        {
            rab.rab$l_rop |= (RAB$M_BIO | RAB$M_ASY);
        }
        rab.rab$b_rac = RAB$C_SEQ;

        if ((ierr = sys$connect(outrab)) != RMS$_NORMAL)
        {
#ifdef DEBUG
            vms_msg(__G__ "create_rms_output: sys$connect failed.\n", ierr);
            vms_msg(__G__ "", outfab->fab$l_stv);
#endif
            Info(slide, 1, ((char *)slide,
                 "Can't create output file:  %s\n", FnFilter1(G.filename)));
            free_up();
            return PK_WARN;
        }
    }                   /* end if (!uO.cflag) */

    init_buf_ring();

    if ( text_output )
        switch (rfm)
        {
            case FAB$C_VAR:
                _flush_routine = _flush_varlen;
                break;
            case FAB$C_STM:
            case FAB$C_STMCR:
            case FAB$C_STMLF:
                _flush_routine = _flush_stream;
                got_eol = 0;
                break;
            default:
                _flush_routine = _flush_blocks;
                break;
        }
    else
        _flush_routine = _flush_blocks;
    _close_routine = _close_rms;
    return PK_COOL;
}



static  int pka_devchn;
static  int pka_io_pending;
static  unsigned pka_vbn;

#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __save
#pragma __nomember_alignment
#endif /* __DECC || __DECCXX */
static struct
{
    short   status;
    long    count;
    short   dummy;
} pka_io_sb;
#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __restore
#endif /* __DECC || __DECCXX */

static struct
{
    short   status;
    short   dummy;
    void    *addr;
} pka_acp_sb;

static struct fibdef    pka_fib;
static struct atrdef    pka_atr[VMS_MAX_ATRCNT];
static int              pka_idx;
static ulg              pka_uchar;
static struct fatdef    pka_rattr;

static struct dsc$descriptor    pka_fibdsc =
{   sizeof(pka_fib), DSC$K_DTYPE_Z, DSC$K_CLASS_S, (void *) &pka_fib  };

static struct dsc$descriptor_s  pka_devdsc =
{   0, DSC$K_DTYPE_T, DSC$K_CLASS_S, &nam.nam$t_dvi[1]  };

static struct dsc$descriptor_s  pka_fnam =
{   0, DSC$K_DTYPE_T, DSC$K_CLASS_S, NULL  };

static char exp_nam[NAM$C_MAXRSS];
static char res_nam[NAM$C_MAXRSS];

#define PK_PRINTABLE_RECTYP(x)   ( (x) == FAT$C_VARIABLE \
                                || (x) == FAT$C_STREAMLF \
                                || (x) == FAT$C_STREAMCR \
                                || (x) == FAT$C_STREAM   )


static int create_qio_output(__GPRO)          /* return 1 (PK_WARN) if fail */
{
    int status;
    int i;
    int text_output;

    /* extract the file in text (variable-length) format, when
     * piping to SYS$OUTPUT, unless "binary" piping was requested
     * by the user (through the -b option); the "-a" option is
     * ignored when extracting zip entries with VMS attributes saved
     */
    text_output = uO.cflag &&
                  (!uO.bflag || (!(uO.bflag - 1) && G.pInfo->textfile));

    if ( uO.cflag )
    {
        int rtype;

        if (text_output)
        {
            rtype = pka_rattr.fat$v_rtype;
            if (!PK_PRINTABLE_RECTYP(rtype))
            {
                Info(slide, 1, ((char *)slide,
                   "[ File %s has illegal record format to put to screen ]\n",
                   FnFilter1(G.filename)));
                return PK_WARN;
            }
        }
        else
            /* force "block I/O" for binary piping mode */
            rtype = FAT$C_UNDEFINED;

        init_buf_ring();

        switch (rtype)
        {
            case FAT$C_VARIABLE:
                _flush_routine = _flush_varlen;
                break;
            case FAT$C_STREAM:
            case FAT$C_STREAMCR:
            case FAT$C_STREAMLF:
                _flush_routine = _flush_stream;
                got_eol = 0;
                break;
            default:
                _flush_routine = _flush_blocks;
                break;
        }
        _close_routine = _close_rms;
    }
    else                        /* !(uO.cflag) : redirect output */
    {

        fileblk = cc$rms_fab;
        fileblk.fab$l_fna = G.filename;
        fileblk.fab$b_fns = strlen(G.filename);

        nam = cc$rms_nam;
        fileblk.fab$l_nam = &nam;
        nam.nam$l_esa = exp_nam;
        nam.nam$b_ess = sizeof(exp_nam);
        nam.nam$l_rsa = res_nam;
        nam.nam$b_rss = sizeof(res_nam);

        if ( ERR(status = sys$parse(&fileblk)) )
        {
            vms_msg(__G__ "create_qio_output: sys$parse failed.\n", status);
            return PK_WARN;
        }

        pka_devdsc.dsc$w_length = (unsigned short)nam.nam$t_dvi[0];

        if ( ERR(status = sys$assign(&pka_devdsc,&pka_devchn,0,0)) )
        {
            vms_msg(__G__ "create_qio_output: sys$assign failed.\n", status);
            return PK_WARN;
        }

        pka_fnam.dsc$a_pointer = nam.nam$l_name;
        pka_fnam.dsc$w_length  = nam.nam$b_name + nam.nam$b_type;
        if ( uO.V_flag /* keep versions */ )
            pka_fnam.dsc$w_length += nam.nam$b_ver;

        for (i=0;i<3;i++)
        {
            pka_fib.FIB$W_DID[i]=nam.nam$w_did[i];
            pka_fib.FIB$W_FID[i]=0;
        }

        /* 2004-11-23 SMS.
         * Set the "sequential access only" flag, as otherwise, on a
         * file system with highwater marking enabled, allocating space
         * for a large file may lock the disk for a long time (minutes).
         * (The "no other readers" flag is also required, if you want
         * the "sequential access only" flag to have any effect.)
         */
        pka_fib.FIB$L_ACCTL = FIB$M_WRITE | FIB$M_SEQONLY | FIB$M_NOREAD;

        /* Allocate space for the file */
        pka_fib.FIB$W_EXCTL = FIB$M_EXTEND;
        if ( pka_uchar & FCH$M_CONTIG )
            pka_fib.FIB$W_EXCTL |= FIB$M_ALCON | FIB$M_FILCON;
        if ( pka_uchar & FCH$M_CONTIGB )
            pka_fib.FIB$W_EXCTL |= FIB$M_ALCONB;

#define SWAPW(x)        ( (((x)>>16)&0xFFFF) + ((x)<<16) )

        pka_fib.fib$l_exsz = SWAPW(pka_rattr.fat$l_hiblk);

        status = sys$qiow(0, pka_devchn, IO$_CREATE|IO$M_CREATE|IO$M_ACCESS,
                          &pka_acp_sb, 0, 0,
                          &pka_fibdsc, &pka_fnam, 0, 0, &pka_atr, 0);

        if ( !ERR(status) )
            status = pka_acp_sb.status;

        if ( ERR(status) )
        {
            vms_msg(__G__ "[ Create file QIO failed. ]\n", status);
            sys$dassgn(pka_devchn);
            return PK_WARN;
        }

#ifdef ASYNCH_QIO
        init_buf_ring();
        pka_io_pending = FALSE;
#else
        locptr = locbuf;
        loccnt = 0;
#endif
        pka_vbn = 1;
        _flush_routine = _flush_qio;
        _close_routine = _close_qio;
    }                   /* end if (!uO.cflag) */
    return PK_COOL;
}



static int replace(__GPRO)
{                       /*
                         *      File exists. Inquire user about further action.
                         */
    char answ[10];
    struct NAM nam;
    int ierr;

    if (query == '\0')
    {
        do
        {
            Info(slide, 0x81, ((char *)slide,
                 "%s exists:  [o]verwrite, new [v]ersion or [n]o extract?\n\
  (uppercase response [O,V,N] = do same for all files): ",
                 FnFilter1(G.filename)));
            fflush(stderr);
        } while (fgets(answ, 9, stderr) == NULL && !isalpha(answ[0])
                 && tolower(answ[0]) != 'o'
                 && tolower(answ[0]) != 'v'
                 && tolower(answ[0]) != 'n');

        if (isupper(answ[0]))
            query = answ[0] = tolower(answ[0]);
    }
    else
        answ[0] = query;

    switch (answ[0])
    {
        case 'n':
            ierr = 0;
            break;
        case 'v':
            nam = cc$rms_nam;
            nam.nam$l_rsa = G.filename;
            nam.nam$b_rss = FILNAMSIZ - 1;

            outfab->fab$l_fop |= FAB$M_MXV;
            outfab->fab$l_nam = &nam;

            ierr = sys$create(outfab);
            if (!ERR(ierr))
            {
                outfab->fab$l_nam = NULL;
                G.filename[outfab->fab$b_fns = nam.nam$b_rsl] = '\0';
            }
            break;
        case 'o':
            outfab->fab$l_fop |= FAB$M_SUP;
            ierr = sys$create(outfab);
            break;
    }
    return ierr;
}



#define W(p)    (*(unsigned short*)(p))
#define L(p)    (*(unsigned long*)(p))
#define EQL_L(a,b)      ( L(a) == L(b) )
#define EQL_W(a,b)      ( W(a) == W(b) )

/****************************************************************
 * Function find_vms_attrs scans ZIP entry extra field if any   *
 * and looks for VMS attribute records. Returns 0 if either no  *
 * attributes found or no fab given.                            *
 ****************************************************************/
static int find_vms_attrs(__G)
    __GDEF
{
    uch *scan = G.extra_field;
    struct  EB_header *hdr;
    int len;
    int type=VAT_NONE;

    outfab = NULL;
    xabfhc = NULL;
    xabdat = NULL;
    xabrdt = NULL;
    xabpro = NULL;
    first_xab = last_xab = NULL;

    if (scan == NULL)
        return VAT_NONE;
    len = G.lrec.extra_field_length;

#define LINK(p) {/* Link xaballs and xabkeys into chain */      \
                if ( first_xab == NULL )                \
                        first_xab = (void *) p;         \
                if ( last_xab != NULL )                 \
                        last_xab->xab$l_nxt = (void *) p;       \
                last_xab = (void *) p;                  \
                p->xab$l_nxt = NULL;                    \
        }
    /* End of macro LINK */

    while (len > 0)
    {
        hdr = (struct EB_header *)scan;
        if (EQL_W(&hdr->tag, IZ_SIGNATURE))
        {
            /*
             *  Info-ZIP style extra block decoding
             */
            uch *blk;
            unsigned siz;
            uch *block_id;

            type = VAT_IZ;

            siz = hdr->size;
            blk = (uch *)(&hdr->data[0]);
            block_id = (uch *)(&((struct IZ_block *)hdr)->bid);

            if (EQL_L(block_id, FABSIG)) {
                outfab = (struct FAB *)extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_fab, FABL);
            } else if (EQL_L(block_id, XALLSIG)) {
                xaball = (struct XABALL *)extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xaball, XALLL);
                LINK(xaball);
            } else if (EQL_L(block_id, XKEYSIG)) {
                xabkey = (struct XABKEY *)extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xabkey, XKEYL);
                LINK(xabkey);
            } else if (EQL_L(block_id, XFHCSIG)) {
                xabfhc = (struct XABFHC *) extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xabfhc, XFHCL);
            } else if (EQL_L(block_id, XDATSIG)) {
                xabdat = (struct XABDAT *) extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xabdat, XDATL);
            } else if (EQL_L(block_id, XRDTSIG)) {
                xabrdt = (struct XABRDT *) extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xabrdt, XRDTL);
            } else if (EQL_L(block_id, XPROSIG)) {
                xabpro = (struct XABPRO *) extract_izvms_block(__G__ blk, siz,
                                        NULL, (uch *)&cc$rms_xabpro, XPROL);
            } else if (EQL_L(block_id, VERSIG)) {
#ifdef CHECK_VERSIONS
                char verbuf[80];
                unsigned verlen = 0;
                uch *vers;
                char *m;

                get_vms_version(verbuf, sizeof(verbuf));
                vers = extract_izvms_block(__G__ blk, siz,
                                           &verlen, NULL, 0);
                if ((m = strrchr((char *) vers, '-')) != NULL)
                    *m = '\0';  /* Cut out release number */
                if (strcmp(verbuf, (char *) vers) && uO.qflag < 2)
                {
                    Info(slide, 0, ((char *)slide,
                         "[ Warning: VMS version mismatch."));

                    Info(slide, 0, ((char *)slide,
                         "   This version %s --", verbuf));
                    strncpy(verbuf, (char *) vers, verlen);
                    verbuf[verlen] = '\0';
                    Info(slide, 0, ((char *)slide,
                         " version made by %s ]\n", verbuf));
                }
                free(vers);
#endif /* CHECK_VERSIONS */
            } else {
                Info(slide, 1, ((char *)slide,
                     "[ Warning: Unknown block signature %s ]\n",
                     block_id));
            }
        }
        else if (hdr->tag == PK_SIGNATURE)
        {
            /*
             *  PKWARE-style extra block decoding
             */
            struct  PK_header   *blk;
            register byte   *scn;
            register int    len;

            type = VAT_PK;

            blk = (struct PK_header *)hdr;
            len = blk->size - (PK_HEADER_SIZE - EB_HEADSIZE);
            scn = (byte *)(&blk->data);
            pka_idx = 0;

            if (blk->crc32 != crc32(CRCVAL_INITIAL, scn, (extent)len))
            {
                Info(slide, 1, ((char *)slide,
                  "[Warning: CRC error, discarding PKWARE extra field]\n"));
                len = 0;
                type = VAT_NONE;
            }

            while (len > PK_FLDHDR_SIZE)
            {
                register struct  PK_field  *fld;
                int skip=0;

                fld = (struct PK_field *)scn;
                switch(fld->tag)
                {
                    case ATR$C_UCHAR:
                        pka_uchar = L(&fld->value);
                        break;
                    case ATR$C_RECATTR:
                        pka_rattr = *(struct fatdef *)(&fld->value);
                        break;
                    case ATR$C_UIC:
                    case ATR$C_ADDACLENT:
                        skip = !uO.X_flag;
                        break;
                }

                if ( !skip )
                {
                    pka_atr[pka_idx].atr$w_size = fld->size;
                    pka_atr[pka_idx].atr$w_type = fld->tag;
                    pka_atr[pka_idx].atr$l_addr = GVTC &fld->value;
                    ++pka_idx;
                }
                len -= fld->size + PK_FLDHDR_SIZE;
                scn += fld->size + PK_FLDHDR_SIZE;
            }
            pka_atr[pka_idx].atr$w_size = 0;    /* End of list */
            pka_atr[pka_idx].atr$w_type = 0;
            pka_atr[pka_idx].atr$l_addr = 0; /* NULL when DECC VAX gets fixed */
        }
        len -= hdr->size + EB_HEADSIZE;
        scan += hdr->size + EB_HEADSIZE;
    }


    if ( type == VAT_IZ )
    {
        if (outfab != NULL)
        {   /* Do not link XABPRO,XABRDT now. Leave them for sys$close() */

            outfab->fab$l_xab = NULL;
            if (xabfhc != NULL)
            {
                xabfhc->xab$l_nxt = outfab->fab$l_xab;
                outfab->fab$l_xab = (void *) xabfhc;
            }
            if (xabdat != NULL)
            {
                xabdat->xab$l_nxt = outfab->fab$l_xab;
                outfab->fab$l_xab = (void *) xabdat;
            }
            if (first_xab != NULL)      /* Link xaball,xabkey subchain */
            {
                last_xab->xab$l_nxt = outfab->fab$l_xab;
                outfab->fab$l_xab = (void *) first_xab;
            }
        }
        else
            type = VAT_NONE;
    }
    return type;
}



static void free_up()
{
    /*
     * Free up all allocated xabs
     */
    if (xabdat != NULL) free(xabdat);
    if (xabpro != NULL) free(xabpro);
    if (xabrdt != NULL) free(xabrdt);
    if (xabfhc != NULL) free(xabfhc);
    while (first_xab != NULL)
    {
        struct XAB *x;

        x = (struct XAB *) first_xab->xab$l_nxt;
        free(first_xab);
        first_xab = x;
    }
    if (outfab != NULL && outfab != &fileblk)
        free(outfab);
}



#ifdef CHECK_VERSIONS

static int get_vms_version(verbuf, len)
    char *verbuf;
    int len;
{
    int i = SYI$_VERSION;
    int verlen = 0;
    struct dsc$descriptor version;
    char *m;

    version.dsc$a_pointer = verbuf;
    version.dsc$w_length  = len - 1;
    version.dsc$b_dtype   = DSC$K_DTYPE_B;
    version.dsc$b_class   = DSC$K_CLASS_S;

    if (ERR(lib$getsyi(&i, 0, &version, &verlen, 0, 0)) || verlen == 0)
        return 0;

    /* Cut out trailing spaces "V5.4-3   " -> "V5.4-3" */
    for (m = verbuf + verlen, i = verlen - 1; i > 0 && verbuf[i] == ' '; --i)
        --m;
    *m = '\0';

    /* Cut out release number "V5.4-3" -> "V5.4" */
    if ((m = strrchr(verbuf, '-')) != NULL)
        *m = '\0';
    return strlen(verbuf) + 1;  /* Transmit ending '\0' too */
}

#endif /* CHECK_VERSIONS */



/* flush contents of output buffer */
int flush(__G__ rawbuf, size, unshrink)    /* return PK-type error code */
    __GDEF
    uch *rawbuf;
    ulg size;
    int unshrink;
{
    G.crc32val = crc32(G.crc32val, rawbuf, (extent)size);
    if (uO.tflag)
        return PK_COOL; /* Do not output. Update CRC only */
    else
        return (*_flush_routine)(__G__ rawbuf, size, 0);
}



static int _flush_blocks(__G__ rawbuf, size, final_flag)
                                                /* Asynchronous version */
    __GDEF
    uch *rawbuf;
    unsigned size;
    int final_flag;   /* 1 if this is the final flushout */
{
    int status;
    unsigned off = 0;

    while (size > 0)
    {
        if (curbuf->bufcnt < BUFS512)
        {
            unsigned ncpy;

            ncpy = size > (BUFS512 - curbuf->bufcnt) ?
                   (BUFS512 - curbuf->bufcnt) : size;
            memcpy(curbuf->buf + curbuf->bufcnt, rawbuf + off, ncpy);
            size -= ncpy;
            curbuf->bufcnt += ncpy;
            off += ncpy;
        }
        if (curbuf->bufcnt == BUFS512)
        {
            status = WriteBuffer(__G__ curbuf->buf, curbuf->bufcnt);
            if (status)
                return status;
            curbuf = curbuf->next;
            curbuf->bufcnt = 0;
        }
    }

    return (final_flag && (curbuf->bufcnt > 0)) ?
        WriteBuffer(__G__ curbuf->buf, curbuf->bufcnt) :
        PK_COOL;
}



#ifdef ASYNCH_QIO
static int WriteQIO(__G__ buf, len)
    __GDEF
    uch *buf;
    unsigned len;
{
    int status;

    if (pka_io_pending) {
        status = sys$synch(0, &pka_io_sb);
        if (!ERR(status))
            status = pka_io_sb.status;
        if (ERR(status))
        {
            vms_msg(__G__ "[ WriteQIO: sys$synch found I/O failure ]\n",
                    status);
            return PK_DISK;
        }
        pka_io_pending = FALSE;
    }
    /*
     *   Put content of buffer as a single VB
     */
    status = sys$qio(0, pka_devchn, IO$_WRITEVBLK,
                     &pka_io_sb, 0, 0,
                     buf, len, pka_vbn,
                     0, 0, 0);
    if (ERR(status))
    {
        vms_msg(__G__ "[ WriteQIO: sys$qio failed ]\n", status);
        return PK_DISK;
    }
    pka_io_pending = TRUE;
    pka_vbn += (len>>9);

    return PK_COOL;
}

/*
   2004-10-01 SMS.  Changed to clear the extra byte written out by qio()
   and sys$write() when an odd byte count is incremented to the next
   even value, either explicitly (qio), or implicitly (sys$write), on
   the theory that a reliable NUL beats left-over garbage.  Alpha and
   VAX object files seem frequently to have even more than one byte of
   extra junk past EOF, so this may not help them.
*/

static int _flush_qio(__G__ rawbuf, size, final_flag)
                                                /* Asynchronous version */
    __GDEF
    uch *rawbuf;
    unsigned size;
    int final_flag;   /* 1 if this is the final flushout */
{
    int status;
    unsigned off = 0;

    while (size > 0)
    {
        if (curbuf->bufcnt < BUFS512)
        {
            unsigned ncpy;

            ncpy = size > (BUFS512 - curbuf->bufcnt) ?
                   (BUFS512 - curbuf->bufcnt) : size;
            memcpy(curbuf->buf + curbuf->bufcnt, rawbuf + off, ncpy);
            size -= ncpy;
            curbuf->bufcnt += ncpy;
            off += ncpy;
        }
        if (curbuf->bufcnt == BUFS512)
        {
            status = WriteQIO(__G__ curbuf->buf, curbuf->bufcnt);
            if (status)
                return status;
            curbuf = curbuf->next;
            curbuf->bufcnt = 0;
        }
    }

    if (final_flag && (curbuf->bufcnt > 0))
    {
        unsigned bufcnt_even;

        /* Round up to an even byte count. */
        bufcnt_even = (curbuf->bufcnt+1) & (~1);
        /* If there is one, clear the extra byte. */
        if (bufcnt_even > curbuf->bufcnt)
            curbuf->buf[curbuf->bufcnt] = '\0';

        return WriteQIO(curbuf->buf, bufcnt_even);
    }
    else
    {
        return PK_COOL;
    }
}

#else /* !ASYNCH_QIO */

static int _flush_qio(__G__ rawbuf, size, final_flag)
    __GDEF
    uch *rawbuf;
    unsigned size;
    int final_flag;   /* 1 if this is the final flushout */
{
    int status;
    uch *out_ptr=rawbuf;

    if ( final_flag )
    {
        if ( loccnt > 0 )
        {
            unsigned loccnt_even;

            /* Round up to an even byte count. */
            loccnt_even = (loccnt+1) & (~1);
            /* If there is one, clear the extra byte. */
            if (loccnt_even > loccnt)
                locbuf[ loccnt] = '\0';

            status = sys$qiow(0, pka_devchn, IO$_WRITEVBLK,
                              &pka_io_sb, 0, 0,
                              locbuf,
                              loccnt_even,
                              pka_vbn,
                              0, 0, 0);
            if (!ERR(status))
                status = pka_io_sb.status;
            if (ERR(status))
            {
                vms_msg(__G__ "[ Write QIO failed ]\n", status);
                return PK_DISK;
            }
        }
        return PK_COOL;
    }

    if ( loccnt > 0 )
    {
        /*
         *   Fill local buffer upto 512 bytes then put it out
         */
        unsigned ncpy;

        ncpy = 512-loccnt;
        if ( ncpy > size )
            ncpy = size;

        memcpy(locptr, out_ptr, ncpy);
        locptr += ncpy;
        loccnt += ncpy;
        size -= ncpy;
        out_ptr += ncpy;
        if ( loccnt == 512 )
        {
            status = sys$qiow(0, pka_devchn, IO$_WRITEVBLK,
                              &pka_io_sb, 0, 0,
                              locbuf, loccnt, pka_vbn,
                              0, 0, 0);
            if (!ERR(status))
                status = pka_io_sb.status;
            if (ERR(status))
            {
                vms_msg(__G__ "[ Write QIO failed ]\n", status);
                return PK_DISK;
            }

            pka_vbn++;
            loccnt = 0;
            locptr = locbuf;
        }
    }

    if ( size >= 512 )
    {
        unsigned nblk, put_cnt;

        /*
         *   Put rest of buffer as a single VB
         */
        put_cnt = (nblk = size>>9)<<9;
        status = sys$qiow(0, pka_devchn, IO$_WRITEVBLK,
                          &pka_io_sb, 0, 0,
                          out_ptr, put_cnt, pka_vbn,
                          0, 0, 0);
        if (!ERR(status))
            status = pka_io_sb.status;
        if (ERR(status))
        {
            vms_msg(__G__ "[ Write QIO failed ]\n", status);
            return PK_DISK;
        }

        pka_vbn += nblk;
        out_ptr += put_cnt;
        size -= put_cnt;
    }

    if ( size > 0 )
    {
        memcpy(locptr, out_ptr, size);
        loccnt += size;
        locptr += size;
    }

    return PK_COOL;
}
#endif /* ?ASYNCH_QIO */



/*
 * The routine _flush_varlen() requires: "(size & 1) == 0"
 * (The variable-length record algorithm assumes an even byte-count!)
 */
static int _flush_varlen(__G__ rawbuf, size, final_flag)
    __GDEF
    uch *rawbuf;
    unsigned size;
    int final_flag;
{
    unsigned nneed;
    unsigned reclen;
    uch *inptr=rawbuf;

    /*
     * Flush local buffer
     */

    if ( loccnt > 0 )           /* incomplete record left from previous call */
    {
        reclen = *(ush*)locbuf;
        nneed = reclen + 2 - loccnt;
        if ( nneed > size )
        {
            if ( size+loccnt > BUFSMAXREC )
            {
                char buf[80];
                Info(buf, 1, (buf,
                     "[ Record too long (%u bytes) ]\n", reclen));
                return PK_DISK;
            }
            memcpy(locbuf+loccnt, inptr, size);
            loccnt += size;
            size = 0;
        }
        else
        {
            memcpy(locbuf+loccnt, inptr, nneed);
            loccnt += nneed;
            size -= nneed;
            inptr += nneed;
            if ( reclen & 1 )
            {
                size--;
                inptr++;
            }
            if ( WriteRecord(__G__ locbuf+2, reclen) )
                return PK_DISK;
            loccnt = 0;
        }
    }
    /*
     * Flush incoming records
     */
    while (size > 0)
    {
        reclen = *(ush*)inptr;
        if ( reclen+2 <= size )
        {
            if (WriteRecord(__G__ inptr+2, reclen))
                return PK_DISK;
            size -= 2+reclen;
            inptr += 2+reclen;
            if ( reclen & 1 )
            {
                --size;
                ++inptr;
            }
        }
        else
        {
            memcpy(locbuf, inptr, size);
            loccnt = size;
            size = 0;
        }

    }
    /*
     * Final flush rest of local buffer
     */
    if ( final_flag && loccnt > 0 )
    {
        char buf[80];

        Info(buf, 1, (buf,
             "[ Warning, incomplete record of length %u ]\n",
             (unsigned)*(ush*)locbuf));
        if ( WriteRecord(__G__ locbuf+2, loccnt-2) )
            return PK_DISK;
    }
    return PK_COOL;
}



/*
 *   Routine _flush_stream breaks decompressed stream into records
 *   depending on format of the stream (fab->rfm, G.pInfo->textmode, etc.)
 *   and puts out these records. It also handles CR LF sequences.
 *   Should be used when extracting *text* files.
 */

#define VT      0x0B
#define FF      0x0C

/* The file is from MSDOS/OS2/NT -> handle CRLF as record end, throw out ^Z */

/* GRR NOTES:  cannot depend on hostnum!  May have "flip'd" file or re-zipped
 * a Unix file, etc. */

#ifdef USE_ORIG_DOS
# define ORG_DOS \
          (G.pInfo->hostnum==FS_FAT_    \
        || G.pInfo->hostnum==FS_HPFS_   \
        || G.pInfo->hostnum==FS_NTFS_)
#else
# define ORG_DOS    1
#endif

/* Record delimiters */
#ifdef undef
#define RECORD_END(c,f)                                                 \
(    ( ORG_DOS || G.pInfo->textmode ) && c==CTRLZ                       \
  || ( f == FAB$C_STMLF && c==LF )                                      \
  || ( f == FAB$C_STMCR || ORG_DOS || G.pInfo->textmode ) && c==CR      \
  || ( f == FAB$C_STM && (c==CR || c==LF || c==FF || c==VT) )           \
)
#else
#   define  RECORD_END(c,f)   ((c) == LF || (c) == (CR))
#endif

static unsigned find_eol(p,n,l)
/*
 *  Find first CR, LF, CR/LF or LF/CR in string 'p' of length 'n'.
 *  Return offset of the sequence found or 'n' if not found.
 *  If found, return in '*l' length of the sequence (1 or 2) or
 *  zero if sequence end not seen, i.e. CR or LF is last char
 *  in the buffer.
 */
    uch *p;
    unsigned n;
    unsigned *l;
{
    unsigned off = n;
    uch *q;

    *l = 0;

    for (q=p ; n > 0 ; --n,++q)
        if ( RECORD_END(*q,rfm) )
        {
            off = q-p;
            break;
        }

    if ( n > 1 )
    {
        *l = 1;
        if ( ( q[0] == CR && q[1] == LF ) || ( q[0] == LF && q[1] == CR ) )
            *l = 2;
    }

    return off;
}

/* Record delimiters that must be put out */
#define PRINT_SPEC(c)   ( (c)==FF || (c)==VT )



static int _flush_stream(__G__ rawbuf, size, final_flag)
    __GDEF
    uch *rawbuf;
    unsigned size;
    int final_flag; /* 1 if this is the final flushout */
{
    int rest;
    unsigned end = 0, start = 0;

    if (size == 0 && loccnt == 0)
        return PK_COOL;         /* Nothing to do ... */

    if ( final_flag )
    {
        unsigned recsize;

        /*
         * This is flush only call. size must be zero now.
         * Just eject everything we have in locbuf.
         */
        recsize = loccnt - (got_eol ? 1 : 0);
        /*
         *  If the last char of file was ^Z ( end-of-file in MSDOS ),
         *  we will see it now.
         */
        if ( recsize==1 && locbuf[0] == CTRLZ )
            return PK_COOL;

        return WriteRecord(__G__ locbuf, recsize);
    }


    if ( loccnt > 0 )
    {
        /* Find end of record partially saved in locbuf */

        unsigned recsize;
        int complete=0;

        if ( got_eol )
        {
            recsize = loccnt - 1;
            complete = 1;

            if ( (got_eol == CR && rawbuf[0] == LF) ||
                 (got_eol == LF && rawbuf[0] == CR) )
                end = 1;

            got_eol = 0;
        }
        else
        {
            unsigned eol_len;
            unsigned eol_off;

            eol_off = find_eol(rawbuf, size, &eol_len);

            if ( loccnt+eol_off > BUFSMAXREC )
            {
                /*
                 *  No room in locbuf. Dump it and clear
                 */
                char buf[80];           /* CANNOT use slide for Info() */

                recsize = loccnt;
                start = 0;
                Info(buf, 1, (buf,
                     "[ Warning: Record too long (%u) ]\n", loccnt+eol_off));
                complete = 1;
                end = 0;
            }
            else
            {
                if ( eol_off >= size )
                {
                    end = size;
                    complete = 0;
                }
                else if ( eol_len == 0 )
                {
                    got_eol = rawbuf[eol_off];
                    end = size;
                    complete = 0;
                }
                else
                {
                    memcpy(locptr, rawbuf, eol_off);
                    recsize = loccnt + eol_off;
                    locptr += eol_off;
                    loccnt += eol_off;
                    end = eol_off + eol_len;
                    complete = 1;
                }
            }
        }

        if ( complete )
        {
            if (WriteRecord(__G__ locbuf, recsize))
                return PK_DISK;
            loccnt = 0;
            locptr = locbuf;
        }
    }                           /* end if ( loccnt ) */

    for (start = end; start < size && end < size; )
    {
        unsigned eol_off, eol_len;

        got_eol = 0;

#ifdef undef
        if (uO.cflag)
            /* skip CR's at the beginning of record */
            while (start < size && rawbuf[start] == CR)
                ++start;
#endif

        if ( start >= size )
            continue;

        /* Find record end */
        end = start+(eol_off = find_eol(rawbuf+start, size-start, &eol_len));

        if ( end >= size )
            continue;

        if ( eol_len > 0 )
        {
            if ( WriteRecord(__G__ rawbuf+start, end-start) )
                return PK_DISK;
            start = end + eol_len;
        }
        else
        {
            got_eol = rawbuf[end];
            end = size;
            continue;
        }
    }

    rest = size - start;

    if (rest > 0)
    {
        if ( rest > BUFSMAXREC )
        {
            unsigned recsize;
            char buf[80];               /* CANNOT use slide for Info() */

            recsize = rest - (got_eol ? 1 : 0 );
            Info(buf, 1, (buf,
                 "[ Warning: Record too long (%u) ]\n", recsize));
            got_eol = 0;
            return WriteRecord(__G__ rawbuf+start, recsize);
        }
        else
        {
            memcpy(locptr, rawbuf + start, rest);
            locptr += rest;
            loccnt += rest;
        }
    }
    return PK_COOL;
}



static int WriteBuffer(__G__ buf, len)
    __GDEF
    uch *buf;
    unsigned len;
{
    int status;

    if (uO.cflag)
    {
        (void)(*G.message)((zvoid *)&G, buf, len, 0);
    }
    else
    {
        status = sys$wait(outrab);
        if (ERR(status))
        {
            vms_msg(__G__ "[ WriteBuffer: sys$wait failed ]\n", status);
            vms_msg(__G__ "", outrab->rab$l_stv);
        }

        /* If odd byte count, then this must be the final record.
           Clear the extra byte past EOF to help keep the file clean.
        */
        if (len & 1)
            buf[len] = '\0';

        outrab->rab$w_rsz = len;
        outrab->rab$l_rbf = (char *) buf;

        if (ERR(status = sys$write(outrab)))
        {
            vms_msg(__G__ "[ WriteBuffer: sys$write failed ]\n", status);
            vms_msg(__G__ "", outrab->rab$l_stv);
            return PK_DISK;
        }
    }
    return PK_COOL;
}



static int WriteRecord(__G__ rec, len)
    __GDEF
    uch *rec;
    unsigned len;
{
    int status;

    if (uO.cflag)
    {
        (void)(*G.message)((zvoid *)&G, rec, len, 0);
        (void)(*G.message)((zvoid *)&G, (uch *) ("\n"), 1, 0);
    }
    else
    {
        if (ERR(status = sys$wait(outrab)))
        {
            vms_msg(__G__ "[ WriteRecord: sys$wait failed ]\n", status);
            vms_msg(__G__ "", outrab->rab$l_stv);
        }
        outrab->rab$w_rsz = len;
        outrab->rab$l_rbf = (char *) rec;

        if (ERR(status = sys$put(outrab)))
        {
            vms_msg(__G__ "[ WriteRecord: sys$put failed ]\n", status);
            vms_msg(__G__ "", outrab->rab$l_stv);
            return PK_DISK;
        }
    }
    return PK_COOL;
}



void close_outfile(__G)
    __GDEF
{
    int status;

    status = (*_flush_routine)(__G__ NULL, 0, 1);
    if (status)
        return /* PK_DISK */;
    if (uO.cflag)
        return /* PK_COOL */;   /* Don't close stdout */
    /* return */ (*_close_routine)(__G);
}



static int _close_rms(__GPRO)
{
    int status;
    struct XABPRO pro;

    /* Link XABRDT, XABDAT and optionally XABPRO */
    if (xabrdt != NULL)
    {
        xabrdt->xab$l_nxt = NULL;
        outfab->fab$l_xab = (void *) xabrdt;
    }
    else
    {
        rdt.xab$l_nxt = NULL;
        outfab->fab$l_xab = (void *) &rdt;
    }
    if (xabdat != NULL)
    {
        xabdat->xab$l_nxt = outfab->fab$l_xab;
        outfab->fab$l_xab = (void *)xabdat;
    }

    if (xabpro != NULL)
    {
        if ( !uO.X_flag )
            xabpro->xab$l_uic = 0;    /* Use default (user's) uic */
        xabpro->xab$l_nxt = outfab->fab$l_xab;
        outfab->fab$l_xab = (void *) xabpro;
    }
    else
    {
        pro = cc$rms_xabpro;
        pro.xab$w_pro = G.pInfo->file_attr;
        pro.xab$l_nxt = outfab->fab$l_xab;
        outfab->fab$l_xab = (void *) &pro;
    }

    status = sys$wait(outrab);
    if (ERR(status))
    {
        vms_msg(__G__ "[ _close_rms: sys$wait failed ]\n", status);
        vms_msg(__G__ "", outrab->rab$l_stv);
    }

    status = sys$close(outfab);
#ifdef DEBUG
    if (ERR(status))
    {
        vms_msg(__G__
          "\r[ Warning: cannot set owner/protection/time attributes ]\n",
          status);
        vms_msg(__G__ "", outfab->fab$l_stv);
    }
#endif
    free_up();
    return PK_COOL;
}



static int _close_qio(__GPRO)
{
    int status;

    pka_fib.FIB$L_ACCTL =
        FIB$M_WRITE | FIB$M_NOTRUNC ;
    pka_fib.FIB$W_EXCTL = 0;

    pka_fib.FIB$W_FID[0] =
    pka_fib.FIB$W_FID[1] =
    pka_fib.FIB$W_FID[2] =
    pka_fib.FIB$W_DID[0] =
    pka_fib.FIB$W_DID[1] =
    pka_fib.FIB$W_DID[2] = 0;

#ifdef ASYNCH_QIO
    if (pka_io_pending) {
        status = sys$synch(0, &pka_io_sb);
        if (!ERR(status))
            status = pka_io_sb.status;
        if (ERR(status))
        {
            vms_msg(__G__ "[ _close_qio: sys$synch found I/O failure ]\n",
                    status);
        }
        pka_io_pending = FALSE;
    }
#endif /* ASYNCH_QIO */

    status = sys$qiow(0, pka_devchn, IO$_DEACCESS, &pka_acp_sb,
                      0, 0,
                      &pka_fibdsc, 0, 0, 0,
                      &pka_atr, 0);

    sys$dassgn(pka_devchn);
    if ( !ERR(status) )
        status = pka_acp_sb.status;
    if ( ERR(status) )
    {
        vms_msg(__G__ "[ Deaccess QIO failed ]\n", status);
        return PK_DISK;
    }
    return PK_COOL;
}



#ifdef TIMESTAMP

/* Nonzero if `y' is a leap year, else zero. */
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

/* Number of leap years from 1970 to `y' (not including `y' itself). */
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

/* Accumulated number of days from 01-Jan up to start of current month. */
static ZCONST short ydays[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

/***********************/
/* Function mkgmtime() */
/***********************/

static time_t mkgmtime(tm)
    struct tm *tm;
{
    time_t m_time;
    int yr, mo, dy, hh, mm, ss;
    unsigned days;

    yr = tm->tm_year - 70;
    mo = tm->tm_mon;
    dy = tm->tm_mday - 1;
    hh = tm->tm_hour;
    mm = tm->tm_min;
    ss = tm->tm_sec;

    /* calculate days from BASE to this year and add expired days this year */
    dy = (unsigned)dy + ((unsigned)yr * 365) + (unsigned)nleap(yr+1970) +
         (unsigned)ydays[mo] + ((mo > 1) && leap(yr+1970));

    /* convert date & time to seconds relative to 00:00:00, 01/01/1970 */
    return (time_t)((unsigned long)(unsigned)dy * 86400L +
                    (unsigned long)hh * 3600L +
                    (unsigned long)(mm * 60 + ss));

} /* end function mkgmtime() */



/*******************************/
/* Function dos_to_unix_time() */  /* only used for timestamping of archives */
/*******************************/

time_t dos_to_unix_time(dosdatetime)
    ulg dosdatetime;
{
    struct tm *ltm;             /* Local time. */
    time_t loctime;             /* The time_t value of local time. */
    time_t then;                /* The time to return. */
    long tzoffset_adj;          /* timezone-adjustment `remainder' */
    int bailout_cnt;            /* counter of tries for tz correction */

    then = time(NULL);
    ltm = localtime(&then);

    /* dissect date */
    ltm->tm_year = ((int)(dosdatetime >> 25) & 0x7f) + 80;
    ltm->tm_mon  = ((int)(dosdatetime >> 21) & 0x0f) - 1;
    ltm->tm_mday = ((int)(dosdatetime >> 16) & 0x1f);

    /* dissect time */
    ltm->tm_hour = (int)(dosdatetime >> 11) & 0x1f;
    ltm->tm_min  = (int)(dosdatetime >> 5) & 0x3f;
    ltm->tm_sec  = (int)(dosdatetime << 1) & 0x3e;

    loctime = mkgmtime(ltm);

    /* Correct for the timezone and any daylight savings time.
       The correction is verified and repeated when not correct, to
       take into account the rare case that a change to or from daylight
       savings time occurs between when it is the time in `tm' locally
       and when it is that time in Greenwich. After the second correction,
       the "timezone & daylight" offset should be correct in all cases. To
       be sure, we allow a third try, but then the loop is stopped. */
    bailout_cnt = 3;
    then = loctime;
    do {
      ltm = localtime(&then);
      tzoffset_adj = (ltm != NULL) ? (loctime - mkgmtime(ltm)) : 0L;
      if (tzoffset_adj == 0L)
        break;
      then += tzoffset_adj;
    } while (--bailout_cnt > 0);

    if ( (dosdatetime >= DOSTIME_2038_01_18) &&
         (then < (time_t)0x70000000L) )
        then = U_TIME_T_MAX;    /* saturate in case of (unsigned) overflow */
    if (then < (time_t)0L)      /* a converted DOS time cannot be negative */
        then = S_TIME_T_MAX;    /*  -> saturate at max signed time_t value */
    return then;

} /* end function dos_to_unix_time() */



/*******************************/
/*  Function uxtime2vmstime()  */
/*******************************/

static void uxtime2vmstime(  /* convert time_t value into 64 bit VMS bintime */
    time_t utimeval,
    long int binval[2] )
{
    time_t m_time = utimeval;
    struct tm *t = localtime(&m_time);

    if (t == (struct tm *)NULL) {
        /* time conversion error; use current time instead, hoping
           that localtime() does not reject it as well! */
        m_time = time(NULL);
        t = localtime(&m_time);
    }
    sprintf(timbuf, "%02d-%3s-%04d %02d:%02d:%02d.00",
            t->tm_mday, month[t->tm_mon], t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
    sys$bintim(&date_str, binval);
} /* end function uxtime2vmstime() */



/***************************/
/*  Function stamp_file()  */  /* adapted from VMSmunch...it just won't die! */
/***************************/

int stamp_file(fname, modtime)
    ZCONST char *fname;
    time_t modtime;
{
    int status;
    int i;
    static long int Cdate[2], Rdate[2], Edate[2], Bdate[2];
    static short int revisions;
#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __save
#pragma __nomember_alignment
#endif /* __DECC || __DECCXX */
    static union {
      unsigned short int value;
      struct {
        unsigned system : 4;
        unsigned owner : 4;
        unsigned group : 4;
        unsigned world : 4;
      } bits;
    } prot;
#if defined(__DECC) || defined(__DECCXX)
#pragma __member_alignment __restore
#endif /* __DECC || __DECCXX */
    static unsigned long uic;
    static struct fjndef jnl;

    static struct atrdef Atr[] = {
        {sizeof(pka_rattr), ATR$C_RECATTR, GVTC &pka_rattr},
        {sizeof(pka_uchar), ATR$C_UCHAR, GVTC &pka_uchar},
        {sizeof(Cdate), ATR$C_CREDATE, GVTC &Cdate[0]},
        {sizeof(Rdate), ATR$C_REVDATE, GVTC &Rdate[0]},
        {sizeof(Edate), ATR$C_EXPDATE, GVTC &Edate[0]},
        {sizeof(Bdate), ATR$C_BAKDATE, GVTC &Bdate[0]},
        {sizeof(revisions), ATR$C_ASCDATES, GVTC &revisions},
        {sizeof(prot), ATR$C_FPRO, GVTC &prot},
        {sizeof(uic), ATR$C_UIC, GVTC &uic},
        {sizeof(jnl), ATR$C_JOURNAL, GVTC &jnl},
        {0, 0, 0}
    };

    fileblk = cc$rms_fab;
    fileblk.fab$l_fna = (char *)fname;
    fileblk.fab$b_fns = strlen(fname);

    nam = cc$rms_nam;
    fileblk.fab$l_nam = &nam;
    nam.nam$l_esa = exp_nam;
    nam.nam$b_ess = sizeof(exp_nam);
    nam.nam$l_rsa = res_nam;
    nam.nam$b_rss = sizeof(res_nam);

    if ( ERR(status = sys$parse(&fileblk)) )
    {
        vms_msg(__G__ "stamp_file: sys$parse failed.\n", status);
        return -1;
    }

    pka_devdsc.dsc$w_length = (unsigned short)nam.nam$t_dvi[0];

    if ( ERR(status = sys$assign(&pka_devdsc,&pka_devchn,0,0)) )
    {
        vms_msg(__G__ "stamp_file: sys$assign failed.\n", status);
        return -1;
    }

    pka_fnam.dsc$a_pointer = nam.nam$l_name;
    pka_fnam.dsc$w_length  = nam.nam$b_name + nam.nam$b_type + nam.nam$b_ver;

    for (i=0;i<3;i++)
    {
        pka_fib.FIB$W_DID[i]=nam.nam$w_did[i];
        pka_fib.FIB$W_FID[i]=nam.nam$w_fid[i];
    }

    /* Use the IO$_ACCESS function to return info about the file */
    /* Note, used this way, the file is not opened, and the expiration */
    /* and revision dates are not modified */
    status = sys$qiow(0, pka_devchn, IO$_ACCESS,
                      &pka_acp_sb, 0, 0,
                      &pka_fibdsc, &pka_fnam, 0, 0, &Atr, 0);

    if ( !ERR(status) )
        status = pka_acp_sb.status;

    if ( ERR(status) )
    {
        vms_msg(__G__ "[ Access file QIO failed. ]\n", status);
        sys$dassgn(pka_devchn);
        return -1;
    }

    uxtime2vmstime(modtime, Cdate);
    memcpy(Rdate, Cdate, sizeof(Cdate));

    /* note, part of the FIB was cleared by earlier QIOW, so reset it */
    pka_fib.FIB$L_ACCTL = FIB$M_NORECORD;
    for (i=0;i<3;i++)
    {
        pka_fib.FIB$W_DID[i]=nam.nam$w_did[i];
        pka_fib.FIB$W_FID[i]=nam.nam$w_fid[i];
    }

    /* Use the IO$_MODIFY function to change info about the file */
    /* Note, used this way, the file is not opened, however this would */
    /* normally cause the expiration and revision dates to be modified. */
    /* Using FIB$M_NORECORD prohibits this from happening. */
    status = sys$qiow(0, pka_devchn, IO$_MODIFY,
                      &pka_acp_sb, 0, 0,
                      &pka_fibdsc, &pka_fnam, 0, 0, &Atr, 0);

    if ( !ERR(status) )
        status = pka_acp_sb.status;

    if ( ERR(status) )
    {
        vms_msg(__G__ "[ Modify file QIO failed. ]\n", status);
        sys$dassgn(pka_devchn);
        return -1;
    }

    if ( ERR(status = sys$dassgn(pka_devchn)) )
    {
        vms_msg(__G__ "stamp_file: sys$dassgn failed.\n", status);
        return -1;
    }

    return 0;

} /* end function stamp_file() */

#endif /* TIMESTAMP */



#ifdef DEBUG
#if 0   /* currently not used anywhere ! */
void dump_rms_block(p)
    unsigned char *p;
{
    unsigned char bid, len;
    int err;
    char *type;
    char buf[132];
    int i;

    err = 0;
    bid = p[0];
    len = p[1];
    switch (bid)
    {
        case FAB$C_BID:
            type = "FAB";
            break;
        case XAB$C_ALL:
            type = "xabALL";
            break;
        case XAB$C_KEY:
            type = "xabKEY";
            break;
        case XAB$C_DAT:
            type = "xabDAT";
            break;
        case XAB$C_RDT:
            type = "xabRDT";
            break;
        case XAB$C_FHC:
            type = "xabFHC";
            break;
        case XAB$C_PRO:
            type = "xabPRO";
            break;
        default:
            type = "Unknown";
            err = 1;
            break;
    }
    printf("Block @%08X of type %s (%d).", p, type, bid);
    if (err)
    {
        printf("\n");
        return;
    }
    printf(" Size = %d\n", len);
    printf(" Offset - Hex - Dec\n");
    for (i = 0; i < len; i += 8)
    {
        int j;

        printf("%3d - ", i);
        for (j = 0; j < 8; j++)
            if (i + j < len)
                printf("%02X ", p[i + j]);
            else
                printf("   ");
        printf(" - ");
        for (j = 0; j < 8; j++)
            if (i + j < len)
                printf("%03d ", p[i + j]);
            else
                printf("    ");
        printf("\n");
    }
}

#endif                          /* never */
#endif                          /* DEBUG */



static void vms_msg(__GPRO__ char *string, int status)
{
    static char msgbuf[256];

    $DESCRIPTOR(msgd, msgbuf);
    int msglen = 0;

    if (ERR(lib$sys_getmsg(&status, &msglen, &msgd, 0, 0)))
        Info(slide, 1, ((char *)slide,
             "%s[ VMS status = %d ]\n", string, status));
    else
    {
        msgbuf[msglen] = '\0';
        Info(slide, 1, ((char *)slide, "%s[ %s ]\n", string, msgbuf));
    }
}



#ifndef SFX

/* 2004-11-23 SMS.
 * Changed to return the resulting file name even when sys$search()
 * fails.  Before, if the user specified "fred.zip;4" and there was
 * none, the error message would complain:
 *    cannot find either fred.zip;4 or fred.zip;4.zip.
 * when it wasn't really looking for "fred.zip;4.zip".
 */

char *do_wild( __G__ wld )
    __GDEF
    ZCONST char *wld;
{
    int status;

    static char filenam[256];
    static char efn[256];
    static char last_wild[256];
    static struct FAB fab;
    static struct NAM nam;
    static int first_call=1;
    static ZCONST char deflt[] = "[]*.zip";

    if ( first_call || strcmp(wld, last_wild) )
    {   /* (Re)Initialize everything */

        strcpy( last_wild, wld );
        first_call = 1;            /* New wild spec */

        fab = cc$rms_fab;
        fab.fab$l_fna = last_wild;
        fab.fab$b_fns = strlen(last_wild);
        fab.fab$l_dna = (char *) deflt;
        fab.fab$b_dns = sizeof(deflt)-1;
        fab.fab$l_nam = &nam;
        nam = cc$rms_nam;
        nam.nam$l_esa = efn;
        nam.nam$b_ess = sizeof(efn)-1;
        nam.nam$l_rsa = filenam;
        nam.nam$b_rss = sizeof(filenam)-1;

        if ( !OK(sys$parse(&fab)) )
            return (char *)NULL;     /* Initialization failed */

        first_call = 0;

        /* 2004-11-23 SMS.
         * Don't do this.  I see no good reason to lie about the file
         * being sought just because it wasn't found.  If you find one,
         * please explain it here when you change this code back.  I'll
         * admit that the full file spec from sys$parse() may be ugly,
         * but at least it's never misleading.
         */
        status = sys$search(&fab);
        if ( !OK(status) )
        {
#if 0
            strcpy( filenam, wld );
            return filenam;
#endif /* 0 */
        }

    }
    else
    {
        if ( !OK(sys$search(&fab)) )
        {
            first_call = 1;        /* Reinitialize next time */
            return (char *)NULL;
        }
    }
    filenam[nam.nam$b_rsl] = 0;         /* Add the NUL terminator. */
    return filenam;

} /* end function do_wild() */

#endif /* !SFX */



static ulg unix_to_vms[8]={ /* Map from UNIX rwx to VMS rwed */
                            /* Note that unix w bit is mapped to VMS wd bits */
                                                              /* no access */
    XAB$M_NOREAD | XAB$M_NOWRITE | XAB$M_NODEL | XAB$M_NOEXE,    /* --- */
    XAB$M_NOREAD | XAB$M_NOWRITE | XAB$M_NODEL,                  /* --x */
    XAB$M_NOREAD |                               XAB$M_NOEXE,    /* -w- */
    XAB$M_NOREAD,                                                /* -wx */
                   XAB$M_NOWRITE | XAB$M_NODEL | XAB$M_NOEXE,    /* r-- */
                   XAB$M_NOWRITE | XAB$M_NODEL,                  /* r-x */
                                                 XAB$M_NOEXE,    /* rw- */
    0                                                            /* rwx */
                                                              /* full access */
};

#define SETDFPROT   /* We are using undocumented VMS System Service     */
                    /* SYS$SETDFPROT here. If your version of VMS does  */
                    /* not have that service, undef SETDFPROT.          */
                    /* IM: Maybe it's better to put this to Makefile    */
                    /* and DESCRIP.MMS */

#ifdef SETDFPROT
# ifndef sys$setdfprot
extern int sys$setdfprot();
# endif /* !sys$setdfprot */
#endif /* SETDFPROT */


int mapattr(__G)
    __GDEF
{
    ulg tmp = G.crec.external_file_attributes;
    ulg theprot;
    static ulg  defprot = (ulg)-1L,
                sysdef,owndef,grpdef,wlddef;  /* Default protection fields */


    /* IM: The only field of XABPRO we need to set here is */
    /*     file protection, so we need not to change type */
    /*     of G.pInfo->file_attr. WORD is quite enough. */

    if ( defprot == (ulg)-1L )
    {
        /*
         * First time here -- Get user default settings
         */

#ifdef SETDFPROT    /* Undef this if linker cat't resolve SYS$SETDFPROT */
        defprot = (ulg)0L;
        if ( !ERR(sys$setdfprot(0, &defprot)) )
        {
            sysdef = defprot & ( (1L<<XAB$S_SYS)-1 ) << XAB$V_SYS;
            owndef = defprot & ( (1L<<XAB$S_OWN)-1 ) << XAB$V_OWN;
            grpdef = defprot & ( (1L<<XAB$S_GRP)-1 ) << XAB$V_GRP;
            wlddef = defprot & ( (1L<<XAB$S_WLD)-1 ) << XAB$V_WLD;
        }
        else
#endif /* SETDFPROT */
        {
            umask(defprot = umask(0));
            defprot = ~defprot;
            wlddef = unix_to_vms[defprot & 07] << XAB$V_WLD;
            grpdef = unix_to_vms[(defprot>>3) & 07] << XAB$V_GRP;
            owndef = unix_to_vms[(defprot>>6) & 07] << XAB$V_OWN;
            sysdef = owndef >> (XAB$V_OWN - XAB$V_SYS);
            defprot = sysdef | owndef | grpdef | wlddef;
        }
    }

    switch (G.pInfo->hostnum) {
        case AMIGA_:
            tmp = (unsigned)(tmp>>16 & 0x0f);   /* Amiga RWED bits */
            G.pInfo->file_attr =  (tmp << XAB$V_OWN) |
                                   grpdef | sysdef | wlddef;
            break;

        case THEOS_:
            tmp &= 0xF1FFFFFFL;
            if ((tmp & 0xF0000000L) != 0x40000000L)
                tmp &= 0x01FFFFFFL;     /* not a dir, mask all ftype bits */
            else
                tmp &= 0x41FFFFFFL;     /* leave directory bit as set */
            /* fall through! */

        case UNIX_:
        case VMS_:  /*IM: ??? Does VMS Zip store protection in UNIX format ?*/
                    /* GRR:  Yup.  Bad decision on my part... */
        case ACORN_:
        case ATARI_:
        case ATHEOS_:
        case BEOS_:
        case QDOS_:
        case TANDEM_:
            {
              unsigned uxattr = (unsigned)(tmp >> 16);  /* drwxrwxrwx */
              int r = FALSE;

              if (uxattr == 0 && G.extra_field) {
                /* Some (non-Info-ZIP) implementations of Zip for Unix and
                   VMS (and probably others ??) leave 0 in the upper 16-bit
                   part of the external_file_attributes field. Instead,
                   they store file permission attributes in an e.f. block.
                   As a work-around, we search for the presence of one of
                   these extra fields and fall back to the MSDOS compatible
                   part of external_file_attributes if one of the known
                   e.f. types has been detected.
                   Later, we might implement extraction of the permission
                   bits from the VMS extra field. But for now, the
                   work-around should be sufficient to provide "readable"
                   extracted files.
                   (For ASI Unix e.f., an experimental remap of the e.f.
                   mode value IS already provided!)
                 */
                ush ebID;
                unsigned ebLen;
                uch *ef = G.extra_field;
                unsigned ef_len = G.crec.extra_field_length;

                while (!r && ef_len >= EB_HEADSIZE) {
                    ebID = makeword(ef);
                    ebLen = (unsigned)makeword(ef+EB_LEN);
                    if (ebLen > (ef_len - EB_HEADSIZE))
                        /* discoverd some e.f. inconsistency! */
                        break;
                    switch (ebID) {
                      case EF_ASIUNIX:
                        if (ebLen >= (EB_ASI_MODE+2)) {
                            uxattr =
                              (unsigned)makeword(ef+(EB_HEADSIZE+EB_ASI_MODE));
                            /* force stop of loop: */
                            ef_len = (ebLen + EB_HEADSIZE);
                            break;
                        }
                        /* else: fall through! */
                      case EF_PKVMS:
                        /* "found nondecypherable e.f. with perm. attr" */
                        r = TRUE;
                      default:
                        break;
                    }
                    ef_len -= (ebLen + EB_HEADSIZE);
                    ef += (ebLen + EB_HEADSIZE);
                }
              }
              if (!r) {
                  theprot  = (unix_to_vms[uxattr & 07] << XAB$V_WLD)
                           | (unix_to_vms[(uxattr>>3) & 07] << XAB$V_GRP)
                           | (unix_to_vms[(uxattr>>6) & 07] << XAB$V_OWN);
                  if ( uxattr & 0x4000 )
                      /* Directory -- set D bits */
                      theprot |= (XAB$M_NODEL << XAB$V_SYS)
                              | (XAB$M_NODEL << XAB$V_OWN)
                              | (XAB$M_NODEL << XAB$V_GRP)
                              | (XAB$M_NODEL << XAB$V_WLD);
                  G.pInfo->file_attr = theprot;
                  break;
              }
            }
            /* fall through! */

        /* all remaining cases:  expand MSDOS read-only bit into write perms */
        case FS_FAT_:
        case FS_HPFS_:
        case FS_NTFS_:
        case MAC_:
        case TOPS20_:
        default:
            theprot = defprot;
            if ( tmp & 1 )   /* Test read-only bit */
            {   /* Bit is set -- set bits in all fields */
                tmp = XAB$M_NOWRITE | XAB$M_NODEL;
                theprot |= (tmp << XAB$V_SYS) | (tmp << XAB$V_OWN) |
                           (tmp << XAB$V_GRP) | (tmp << XAB$V_WLD);
            }
            G.pInfo->file_attr = theprot;
            break;
    } /* end switch (host-OS-created-by) */

    return 0;

} /* end function mapattr() */



#ifndef EEXIST
#  include <errno.h>    /* For mkdir() status codes */
#endif

#include <fscndef.h> /* for filescan */

#   define FN_MASK   7
#   define USE_DEFAULT  (FN_MASK+1)

/*
 * Checkdir function codes:
 *      ROOT        -   set root path from unzip qq d:[dir]
 *      INIT        -   get ready for "filename"
 *      APPEND_DIR  -   append pathcomp
 *      APPEND_NAME -   append filename
 *      APPEND_NAME | USE_DEFAULT   -    expand filename using collected path
 *      GETPATH     -   return resulting filespec
 *      END         -   free dynamically allocated space prior to program exit
 */

static  int created_dir;

int mapname(__G__ renamed)
        /* returns: */
        /* MPN_OK if no error, */
        /* MPN_INF_TRUNC if caution (filename trunc), */
        /* MPN_INF_SKIP if warning (skip file, dir doesn't exist), */
        /* MPN_ERR_SKIP if error (skip file), */
        /* MPN_CREATED_DIR if has created directory, */
        /* MPN_VOL_LABEL if path was volume label (skip it) */
        /* MPN_NOMEM if no memory (skip file) */
    __GDEF
    int renamed;
{
    char pathcomp[FILNAMSIZ];      /* path-component buffer */
    char *pp, *cp=(char *)NULL;    /* character pointers */
    char *lastsemi = NULL;         /* pointer to last semi-colon in pathcomp */
    char *last_dot = NULL;         /* last dot not converted to underscore */
    int dotname = FALSE;           /* flag:  path component begins with dot */
    int killed_ddot = FALSE;       /* is set when skipping "../" pathcomp */
    int error = MPN_OK;
    register unsigned workch;      /* hold the character being tested */

    if ( renamed )
    {
        if ( !(error = checkdir(__G__ pathcomp, APPEND_NAME | USE_DEFAULT)) )
            strcpy(G.filename, pathcomp);
        return error;
    }

/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    if (G.pInfo->vollabel)
        return MPN_VOL_LABEL;   /* can't set disk volume labels on VMS */

    /* can create path as long as not just freshening, or if user told us */
    G.create_dirs = !uO.fflag;

    created_dir = FALSE;        /* not yet */

/* GRR:  for VMS, convert to internal format now or later? or never? */
    if (checkdir(__G__ pathcomp, INIT) == 10)
        return MPN_NOMEM;       /* initialize path buffer, unless no memory */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (uO.jflag)               /* junking directories */
/* GRR:  watch out for VMS version... */
        cp = (char *)strrchr(G.filename, '/');
    if (cp == NULL)             /* no '/' or not junking dirs */
        cp = G.filename;        /* point to internal zipfile-member pathname */
    else
        ++cp;                   /* point to start of last component of path */

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if (((error = checkdir(__G__ pathcomp, APPEND_DIR))
                     & MPN_MASK) > MPN_INF_TRUNC)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                last_dot = NULL;  /* directory names must not contain dots */
                lastsemi = NULL;  /* leave directory semi-colons alone */
                break;

            case '.':
                if (pp == pathcomp) {     /* nothing appended yet... */
                    if (*cp == '/') {     /* don't bother appending a "./" */
                        ++cp;             /*  component to the path:  skip */
                                          /*  to next char after the '/' */
                        break;
                    } else if (*cp == '.' && cp[1] == '/') {   /* "../" */
                        if (!uO.ddotflag) { /* "../" dir traversal detected */
                            cp += 2;        /*  skip over behind the '/' */
                            killed_ddot = TRUE;
                        } else {
                            *pp++ = '.';    /* add first dot, unchanged... */
                            *pp++ = '.';    /* add second dot, unchanged... */
                            ++cp;           /* skip second dot */
                        }                   /* next char is  the '/' */
                        break;
                    }
                }
                last_dot = pp;    /* point at last dot so far... */
                *pp++ = '_';      /* convert dot to underscore for now */
                break;

            case ';':             /* start of VMS version? */
                if (lastsemi)
                    *lastsemi = '_';   /* convert previous one to underscore */
                lastsemi = pp;
                *pp++ = ';';      /* keep for now; remove VMS vers. later */
                break;

            case ':':   /* drive names illegal in zipfile, so no ':' allowed */
            case ' ':
                *pp++ = '_';
                break;

            default:
                if ( isalpha(workch) || isdigit(workch) ||
                    workch=='$' || workch=='-' )
                    *pp++ = (char)workch;
                else
                    *pp++ = '_';  /* convert everything else to underscore */
                break;
        } /* end switch */

    } /* end while loop */

    /* Show warning when stripping insecure "parent dir" path components */
    if (killed_ddot && QCOND2) {
        Info(slide, 0, ((char *)slide,
          "warning:  skipped \"../\" path component(s) in %s\n",
          FnFilter1(G.filename)));
        if (!(error & ~MPN_MASK))
            error = (error & MPN_MASK) | PK_WARN;
    }

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (G.filename[strlen(G.filename) - 1] == '/') {
        checkdir(__G__ "", APPEND_NAME);   /* create directory, if not found */
        checkdir(__G__ G.filename, GETPATH);
        if (created_dir) {
            if (QCOND2) {
                Info(slide, 0, ((char *)slide, "   creating: %s\n",
                  FnFilter1(G.filename)));
            }
            /* set dir time (note trailing '/') */
            return (error & ~MPN_MASK) | MPN_CREATED_DIR;
        }
        /* dir existed already; don't look for data to extract */
        return (error & ~MPN_MASK) | MPN_INF_SKIP;
    }

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended ";###") */
    if (lastsemi) {
        pp = lastsemi + 1;        /* expect all digits after semi-colon */
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp)                  /* not version number:  convert ';' to '_' */
            *lastsemi = '_';
        else if (!uO.V_flag)      /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
        /* else only digits and we're saving version number:  do nothing */
    }

    if (last_dot != NULL)         /* one dot is OK:  put it back in */
        *last_dot = '.';

    if (*pathcomp == '\0') {
        Info(slide, 1, ((char *)slide, "mapname:  conversion of %s failed\n",
          FnFilter1(G.filename)));
        return (error & ~MPN_MASK) | MPN_ERR_SKIP;
    }

    checkdir(__G__ pathcomp, APPEND_NAME);  /* returns 1 if truncated: care? */
    checkdir(__G__ G.filename, GETPATH);

    return error;

} /* end function mapname() */



int checkdir(__G__ pathcomp, fcn)
/*
 * returns:
 *  MPN_OK          - no problem detected
 *  MPN_INF_TRUNC   - (on APPEND_NAME) truncated filename
 *  MPN_INF_SKIP    - path doesn't exist, not allowed to create
 *  MPN_ERR_SKIP    - path doesn't exist, tried to create and failed; or path
 *                    exists and is not a directory, but is supposed to be
 *  MPN_ERR_TOOLONG - path is too long
 *  MPN_NOMEM       - can't allocate memory for filename buffers
 */
    __GDEF
    char *pathcomp;
    int fcn;
{
    int function=fcn & FN_MASK;
    static char pathbuf[FILNAMSIZ];
    static char lastdir[FILNAMSIZ]="\t"; /* directory created last time */
                                         /* initially - impossible dir. spec. */
    static char *pathptr=pathbuf;        /* For debugger */
    static char *devptr, *dirptr, *namptr;
    static int  devlen, dirlen, namlen;
    static int  root_dirlen;
    static char *end;
    static int  first_comp,root_has_dir;
    static int  rootlen=0;
    static char *rootend;
    static int  mkdir_failed=0;
    int status;

/************
 *** ROOT ***
 ************/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (function==ROOT)
    {        /*  Assume VMS root spec */
        char  *p = pathcomp;
        char  *q;

        struct
        {
            short  len;
            short  code;
            char   *addr;
        } itl [4] =
        {
            {  0,  FSCN$_DEVICE,    NULL  },
            {  0,  FSCN$_ROOT,      NULL  },
            {  0,  FSCN$_DIRECTORY, NULL  },
            {  0,  0,               NULL  }   /* End of itemlist */
        };
        int fields = 0;
        struct dsc$descriptor  pthcmp;

        if (rootlen > 0)        /* rootpath was already set, nothing to do */
            return MPN_OK;

        /*
         *  Initialize everything
         */
        end = devptr = dirptr = rootend = pathbuf;
        devlen = dirlen = rootlen = 0;

        pthcmp.dsc$a_pointer = pathcomp;
        if ( (pthcmp.dsc$w_length = strlen(pathcomp)) > 255 )
            return MPN_ERR_TOOLONG;

        status = sys$filescan(&pthcmp, itl, &fields);
        if ( !OK(status) )
            return MPN_ERR_SKIP;

        if ( fields & FSCN$M_DEVICE )
        {
            strncpy(devptr = end, itl[0].addr, itl[0].len);
            dirptr = (end += (devlen = itl[0].len));
        }

        root_has_dir = 0;

        if ( fields & FSCN$M_ROOT )
        {
            int   len;

            strncpy(dirptr = end, itl[1].addr,
                len = itl[1].len - 1);        /* Cut out trailing ']' */
            end += len;
            root_has_dir = 1;
        }

        if ( fields & FSCN$M_DIRECTORY )
        {
            char  *ptr;
            int   len;

            len = itl[2].len-1;
            ptr = itl[2].addr;

            if ( root_has_dir /* i.e. root specified */ )
            {
                --len;                            /* Cut out leading dot */
                ++ptr;                            /* ??? [a.b.c.][.d.e] */
            }

            strncpy(dirptr=end, ptr, len);  /* Replace trailing ']' */
            *(end+=len) = '.';                    /* ... with dot */
            ++end;
            root_has_dir = 1;
        }

        /* When user specified "[a.b.c.]" or "[qq...]", we have too many
        *  trailing dots. Let's cut them out. Now we surely have at least
        *  one trailing dot and "end" points just behind it. */

        dirlen = end - dirptr;
        while ( dirlen > 1 && end[-2] == '.' )
            --dirlen,--end;

        first_comp = !root_has_dir;
        root_dirlen = end - dirptr;
        *(rootend = end) = '\0';
        rootlen = rootend - devptr;
        return MPN_OK;
    }
#endif /* !SFX || SFX_EXDIR */


/************
 *** INIT ***
 ************/

    if ( function == INIT )
    {
        if ( strlen(G.filename) + rootlen + 13 > 255 )
            return MPN_ERR_TOOLONG;

        if ( rootlen == 0 )     /* No root given, reset everything. */
        {
            devptr = dirptr = rootend = pathbuf;
            devlen = dirlen = 0;
        }
        end = rootend;
        first_comp = !root_has_dir;
        if ( dirlen = root_dirlen )
            end[-1] = '.';
        *end = '\0';
        return MPN_OK;
    }


/******************
 *** APPEND_DIR ***
 ******************/
    if ( function == APPEND_DIR )
    {
        int cmplen;

        cmplen = strlen(pathcomp);

        if ( first_comp )
        {
            *end++ = '[';
            if ( cmplen )
                *end++ = '.';   /*       "dir/..." --> "[.dir...]"    */
            /*                     else  "/dir..." --> "[dir...]"     */
            first_comp = 0;
        }

        if ( cmplen == 1 && *pathcomp == '.' )
            ; /* "..././..." -- ignore */

        else if ( cmplen == 2 && pathcomp[0] == '.' && pathcomp[1] == '.' )
        {   /* ".../../..." -- convert to "...-..." */
            *end++ = '-';
            *end++ = '.';
        }

        else if ( cmplen + (end-pathptr) > 255 )
            return MPN_ERR_TOOLONG;

        else
        {
            strcpy(end, pathcomp);
            *(end+=cmplen) = '.';
            ++end;
        }
        dirlen = end - dirptr;
        *end = '\0';
        return MPN_OK;
    }


/*******************
 *** APPEND_NAME ***
 *******************/
    if ( function == APPEND_NAME )
    {
        if ( fcn & USE_DEFAULT )
        {   /* Expand renamed filename using collected path, return
             *  at pathcomp */
            struct        FAB fab;
            struct        NAM nam;

            fab = cc$rms_fab;
            fab.fab$l_fna = G.filename;
            fab.fab$b_fns = strlen(G.filename);
            fab.fab$l_dna = pathptr;
            fab.fab$b_dns = end-pathptr;

            fab.fab$l_nam = &nam;
            nam = cc$rms_nam;
            nam.nam$l_esa = pathcomp;
            nam.nam$b_ess = 255;            /* Assume large enaugh */

            if (!OK(status = sys$parse(&fab)) && status == RMS$_DNF )
                                         /* Directory not found: */
            {                            /* ... try to create it */
                char    save;
                char    *dirend;
                int     mkdir_failed;

                dirend = (char*)nam.nam$l_dir + nam.nam$b_dir;
                save = *dirend;
                *dirend = '\0';
                if ( (mkdir_failed = mkdir(nam.nam$l_dev, 0)) &&
                     errno == EEXIST )
                    mkdir_failed = 0;
                *dirend = save;
                if ( mkdir_failed )
                    return 3;
                created_dir = TRUE;
            }                                /* if (sys$parse... */
            pathcomp[nam.nam$b_esl] = '\0';
            return MPN_OK;
        }                                /* if (USE_DEFAULT) */
        else
        {
            *end = '\0';
            if ( dirlen )
            {
                dirptr[dirlen-1] = ']'; /* Close directory */

                /*
                 *  Try to create the target directory.
                 *  Don't waste time creating directory that was created
                 *  last time.
                 */
                if ( STRICMP(lastdir,pathbuf) )
                {
                    mkdir_failed = 0;
                    if ( mkdir(pathbuf,0) )
                    {
                        if ( errno != EEXIST )
                            mkdir_failed = 1;   /* Mine for GETPATH */
                    }
                    else
                        created_dir = TRUE;
                    strcpy(lastdir,pathbuf);
                }
            }
            else
            {   /*
                 * Target directory unspecified.
                 * Try to create "sys$disk:[]"
                 */
                if ( strcmp(lastdir,"sys$disk:[]") )
                {
                    strcpy(lastdir,"sys$disk:[]");
                    mkdir_failed = 0;
                    if ( mkdir(lastdir,0) && errno != EEXIST )
                        mkdir_failed = 1;   /* Mine for GETPATH */
                }
            }
            if ( strlen(pathcomp) + (end-pathbuf) > 255 )
                return MPN_INF_TRUNC;
            strcpy(end, pathcomp);
            end += strlen(pathcomp);
            return MPN_OK;
        }
    }


/***************
 *** GETPATH ***
 ***************/
    if ( function == GETPATH )
    {
        if ( mkdir_failed )
            return MPN_ERR_SKIP;
        *end = '\0';                    /* To be safe */
        strcpy( pathcomp, pathbuf );
        return MPN_OK;
    }


/***********
 *** END ***
 ***********/
    if ( function == END )
    {
        Trace((stderr, "checkdir(): nothing to free...\n"));
        rootlen = 0;
        return MPN_OK;
    }

    return MPN_INVALID; /* should never reach */

}



int check_for_newer(__G__ filenam)   /* return 1 if existing file newer or */
    __GDEF                           /*  equal; 0 if older; -1 if doesn't */
    char *filenam;                   /*  exist yet */
{
#ifdef USE_EF_UT_TIME
    iztimes z_utime;
    struct tm *t;
#endif
    unsigned short timbuf[7];
    unsigned dy, mo, yr, hh, mm, ss, dy2, mo2, yr2, hh2, mm2, ss2;
    struct FAB fab;
    struct XABDAT xdat;


    if (stat(filenam, &G.statbuf))
        return DOES_NOT_EXIST;

    fab  = cc$rms_fab;
    xdat = cc$rms_xabdat;

    fab.fab$l_xab = (char *) &xdat;
    fab.fab$l_fna = filenam;
    fab.fab$b_fns = strlen(filenam);
    fab.fab$l_fop = FAB$M_GET | FAB$M_UFO;

    if ((sys$open(&fab) & 1) == 0)       /* open failure:  report exists and */
        return EXISTS_AND_OLDER;         /*  older so new copy will be made  */
    sys$numtim(&timbuf,&xdat.xab$q_cdt);
    fab.fab$l_xab = NULL;

    sys$dassgn(fab.fab$l_stv);
    sys$close(&fab);   /* be sure file is closed and RMS knows about it */

#ifdef USE_EF_UT_TIME
    if (G.extra_field &&
#ifdef IZ_CHECK_TZ
        G.tz_is_valid &&
#endif
        (ef_scan_for_izux(G.extra_field, G.lrec.extra_field_length, 0,
                          G.lrec.last_mod_dos_datetime, &z_utime, NULL)
         & EB_UT_FL_MTIME))
        t = localtime(&(z_utime.mtime));
    else
        t = (struct tm *)NULL;

    if (t != (struct tm *)NULL)
    {
        yr2 = (unsigned)(t->tm_year) + 1900;
        mo2 = (unsigned)(t->tm_mon) + 1;
        dy2 = (unsigned)(t->tm_mday);
        hh2 = (unsigned)(t->tm_hour);
        mm2 = (unsigned)(t->tm_min);
        ss2 = (unsigned)(t->tm_sec);

        /* round to nearest sec--may become 60,
           but doesn't matter for compare */
        ss = (unsigned)((float)timbuf[5] + (float)timbuf[6]*.01 + 0.5);
        TTrace((stderr, "check_for_newer:  using Unix extra field mtime\n"));
    }
    else
#endif /* USE_EF_UT_TIME */
    {
        yr2 = ((G.lrec.last_mod_dos_datetime >> 25) & 0x7f) + 1980;
        mo2 = (G.lrec.last_mod_dos_datetime >> 21) & 0x0f;
        dy2 = (G.lrec.last_mod_dos_datetime >> 16) & 0x1f;
        hh2 = (G.lrec.last_mod_dos_datetime >> 11) & 0x1f;
        mm2 = (G.lrec.last_mod_dos_datetime >> 5) & 0x3f;
        ss2 = (G.lrec.last_mod_dos_datetime << 1) & 0x1f;

        /* round to nearest 2 secs--may become 60,
           but doesn't matter for compare */
        ss = (unsigned)((float)timbuf[5] + (float)timbuf[6]*.01 + 1.) & (~1);
    }
    yr = timbuf[0];
    mo = timbuf[1];
    dy = timbuf[2];
    hh = timbuf[3];
    mm = timbuf[4];

    if (yr > yr2)
        return EXISTS_AND_NEWER;
    else if (yr < yr2)
        return EXISTS_AND_OLDER;

    if (mo > mo2)
        return EXISTS_AND_NEWER;
    else if (mo < mo2)
        return EXISTS_AND_OLDER;

    if (dy > dy2)
        return EXISTS_AND_NEWER;
    else if (dy < dy2)
        return EXISTS_AND_OLDER;

    if (hh > hh2)
        return EXISTS_AND_NEWER;
    else if (hh < hh2)
        return EXISTS_AND_OLDER;

    if (mm > mm2)
        return EXISTS_AND_NEWER;
    else if (mm < mm2)
        return EXISTS_AND_OLDER;

    if (ss >= ss2)
        return EXISTS_AND_NEWER;

    return EXISTS_AND_OLDER;
}



#ifdef RETURN_CODES
void return_VMS(__G__ err)
    __GDEF
#else
void return_VMS(err)
#endif
    int err;
{
    int severity;

#ifdef RETURN_CODES
/*---------------------------------------------------------------------------
    Do our own, explicit processing of error codes and print message, since
    VMS misinterprets return codes as rather obnoxious system errors ("access
    violation," for example).
  ---------------------------------------------------------------------------*/

    switch (err) {
        case PK_COOL:
            break;   /* life is fine... */
        case PK_WARN:
            Info(slide, 1, ((char *)slide, "\n\
[return-code %d:  warning error \
(e.g., failed CRC or unknown compression method)]\n", err));
            break;
        case PK_ERR:
        case PK_BADERR:
            Info(slide, 1, ((char *)slide, "\n\
[return-code %d:  error in zipfile \
(e.g., cannot find local file header sig)]\n", err));
            break;
        case PK_MEM:
        case PK_MEM2:
        case PK_MEM3:
        case PK_MEM4:
        case PK_MEM5:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  insufficient memory]\n", err));
            break;
        case PK_NOZIP:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  zipfile not found]\n", err));
            break;
        case PK_PARAM:   /* exit(PK_PARAM); gives "access violation" */
            Info(slide, 1, ((char *)slide, "\n\
[return-code %d:  bad or illegal parameters specified on command line]\n",
              err));
            break;
        case PK_FIND:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  no files found to extract/view/etc.]\n",
              err));
            break;
        case PK_DISK:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  disk full or other I/O error]\n", err));
            break;
        case PK_EOF:
            Info(slide, 1, ((char *)slide, "\n\
[return-code %d:  unexpected EOF in zipfile (i.e., truncated)]\n", err));
            break;
        case IZ_CTRLC:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  you hit ctrl-C to terminate]\n", err));
            break;
        case IZ_UNSUP:
            Info(slide, 1, ((char *)slide, "\n\
[return-code %d:  unsupported compression or encryption for all files]\n",
              err));
            break;
        case IZ_BADPWD:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  bad decryption password for all files]\n",
              err));
            break;
        default:
            Info(slide, 1, ((char *)slide,
              "\n[return-code %d:  unknown return-code (screw-up)]\n", err));
            break;
    }
#endif /* RETURN_CODES */

/*---------------------------------------------------------------------------
    Return an intelligent status/severity level:

        $STATUS          $SEVERITY = $STATUS & 7
        31 .. 16 15 .. 3   2 1 0
                           -----
        VMS                0 0 0  0    Warning
        FACILITY           0 0 1  1    Success
        Number             0 1 0  2    Error
                 MESSAGE   0 1 1  3    Information
                 Number    1 0 0  4    Severe (fatal) error

    0x7FFF0000 was chosen (by experimentation) to be outside the range of
    VMS FACILITYs that have dedicated message numbers.  Hopefully this will
    always result in silent exits--it does on VMS 5.4.  Note that the C li-
    brary translates exit arguments of zero to a $STATUS value of 1 (i.e.,
    exit is both silent and has a $SEVERITY of "success").
  ---------------------------------------------------------------------------*/

    severity = (err == PK_WARN) ? 1 :           /* warn  */
               (err == 2 ||                     /* error */
                (err >= 9 && err <= 11) ||      /*  ...  */
                (err >= 80 && err <= 82)) ? 2 : /*  ...  */
               4;                               /* fatal */

    exit(                                       /* $SEVERITY:              */
         (err == PK_COOL) ? 1 :                 /*   success               */
         (0x7FFF0000 | (err << 4) | severity)   /*   warning, error, fatal */
        );

} /* end function return_VMS() */


#ifdef MORE
static int scrnlines = -1;
static int scrncolumns = -1;
static int scrnwrap = -1;


static int getscreeninfo(int *tt_rows, int *tt_cols, int *tt_wrap)
{
    /*
     * For VMS v5.x:
     *   IO$_SENSEMODE/SETMODE info:  Programming, Vol. 7A, System Programming,
     *     I/O User's: Part I, sec. 8.4.1.1, 8.4.3, 8.4.5, 8.6
     *   sys$assign(), sys$qio() info:  Programming, Vol. 4B, System Services,
     *     System Services Reference Manual, pp. sys-23, sys-379
     *   fixed-length descriptor info:  Programming, Vol. 3, System Services,
     *     Intro to System Routines, sec. 2.9.2
     * GRR, 15 Aug 91 / SPC, 07 Aug 1995, 14 Nov 1999
     */

#ifndef OUTDEVICE_NAME
#define OUTDEVICE_NAME  "SYS$OUTPUT"
#endif

    static ZCONST struct dsc$descriptor_s OutDevDesc =
        {(sizeof(OUTDEVICE_NAME) - 1), DSC$K_DTYPE_T, DSC$K_CLASS_S,
         OUTDEVICE_NAME};
     /* {dsc$w_length, dsc$b_dtype, dsc$b_class, dsc$a_pointer}; */

    short  OutDevChan, iosb[4];
    long   status;
    struct tt_characts
    {
        uch class, type;
        ush pagewidth;
        union {
            struct {
                uch ttcharsbits[3];
                uch pagelength;
            } ttdef_bits;
            unsigned ttcharflags;
        } ttdef_area;
    }      ttmode;              /* total length = 8 bytes */


    /* assign a channel to standard output */
    status = sys$assign(&OutDevDesc, &OutDevChan, 0, 0);
    if (status & 1)
    {
        /* use sys$qiow and the IO$_SENSEMODE function to determine
         * the current tty status.
         */
        status = sys$qiow(0, OutDevChan, IO$_SENSEMODE, &iosb, 0, 0,
                          &ttmode, sizeof(ttmode), 0, 0, 0, 0);
        /* deassign the output channel by way of clean-up */
        (void) sys$dassgn(OutDevChan);
    }

    if ( (status & 1) && ((status = iosb[0]) & 1) ) {
        if (tt_rows != NULL)
            *tt_rows = ( (ttmode.ttdef_area.ttdef_bits.pagelength >= 5)
                        ? (int) (ttmode.ttdef_area.ttdef_bits.pagelength)
                                                        /* TT device value */
                        : (24) );                       /* VT 100 default  */
        if (tt_cols != NULL)
            *tt_cols = ( (ttmode.pagewidth >= 10)
                        ? (int) (ttmode.pagewidth)      /* TT device value */
                        : (80) );                       /* VT 100 default  */
        if (tt_wrap != NULL)
            *tt_wrap = ((ttmode.ttdef_area.ttcharflags & TT$M_WRAP) != 0);
    } else {
        /* VT 100 defaults */
        if (tt_rows != NULL)
            *tt_rows = 24;
        if (tt_cols != NULL)
            *tt_cols = 80;
        if (tt_wrap != NULL)
            *tt_wrap = FALSE;
    }

    return ((status & 1) != 0);
}

int screensize(int *tt_rows, int *tt_cols)
{
    if (scrnlines < 0 || scrncolumns < 0)
        getscreeninfo(&scrnlines, &scrncolumns, &scrnwrap);
    if (tt_rows != NULL) *tt_rows = scrnlines;
    if (tt_cols != NULL) *tt_cols = scrncolumns;
    return !(scrnlines > 0 && scrncolumns > 0);
}

int screenlinewrap()
{
    if (scrnwrap == -1)
        getscreeninfo(&scrnlines, &scrncolumns, &scrnwrap);
    return (scrnwrap);
}
#endif /* MORE */


#ifndef SFX

/************************/
/*  Function version()  */
/************************/

/* 2004-11-23 SMS.
 * Changed to include the "-x" part of the VMS version.
 * Added the IA64 system type name.
 * Prepared for VMS versions after 9.  (We should live so long.)
 */

void version(__G)
    __GDEF
{
    int len;
#ifdef VMS_VERSION
    char *chrp1;
    char *chrp2;
    char buf[40];
    char vms_vers[16];
    int ver_maj;
#endif
#ifdef __DECC_VER
    char buf2[40];
    int  vtyp;
#endif

#ifdef VMS_VERSION
    /* Truncate the version string at the first (trailing) space. */
    strncpy(vms_vers, VMS_VERSION, sizeof(vms_vers));
    vms_vers[sizeof(vms_vers)-1] = '\0';
    chrp1 = strchr( vms_vers, ' ');
    if (chrp1 != NULL)
        *chrp1 = '\0';

    /* Determine the major version number. */
    ver_maj = 0;
    chrp1 = strchr(&vms_vers[ 1], '.');
    for (chrp2 = &vms_vers[1];
         chrp2 < chrp1;
         ver_maj = ver_maj * 10 + *(chrp2++) - '0');
#endif /* VMS_VERSION */

/*  DEC C in ANSI mode does not like "#ifdef MACRO" inside another
    macro when MACRO is equated to a value (by "#define MACRO 1").   */

    len = sprintf((char *)slide, LoadFarString(CompiledWith),

#ifdef __GNUC__
      "gcc ", __VERSION__,
#else
#  if defined(DECC) || defined(__DECC) || defined (__DECC__)
      "DEC C",
#    ifdef __DECC_VER
      (sprintf(buf2, " %c%d.%d-%03d",
               ((vtyp = (__DECC_VER / 10000) % 10) == 6 ? 'T' :
                (vtyp == 8 ? 'S' : 'V')),
               __DECC_VER / 10000000,
               (__DECC_VER % 10000000) / 100000, __DECC_VER % 1000), buf2),
#    else
      "",
#    endif
#  else
#    ifdef VAXC
      "VAX C", "",
#    else
      "unknown compiler", "",
#    endif
#  endif
#endif

#ifdef VMS_VERSION
#  if defined(__alpha)
      "OpenVMS",
      (sprintf(buf, " (%s Alpha)", vms_vers), buf),
#  elif defined(__IA64)
      "OpenVMS",
      (sprintf(buf, " (%s IA64)", vms_vers), buf),
#  else /* VAX */
      (ver_maj >= 6) ? "OpenVMS" : "VMS",
      (sprintf(buf, " (%s VAX)", vms_vers), buf),
#  endif
#else
      "VMS",
      "",
#endif /* ?VMS_VERSION */

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    (*G.message)((zvoid *)&G, slide, (ulg)len, 0);

} /* end function version() */

#endif /* !SFX */



#ifdef __DECC

/* 2004-11-20 SMS.
 *
 *       acc_cb(), access callback function for DEC C open().
 *
 *    Set some RMS FAB/RAB items, with consideration of user-specified
 * values from (DCL) SET RMS_DEFAULT.  Items of particular interest are:
 *
 *       fab$w_deq         default extension quantity (blocks) (write).
 *       rab$b_mbc         multi-block count.
 *       rab$b_mbf         multi-buffer count (used with rah and wbh).
 *
 *    See also the OPEN* macros in VMSCFG.H.  Currently, no notice is
 * taken of the caller-ID value, but options could be set differently
 * for read versus write access.  (I assume that specifying fab$w_deq,
 * for example, for a read-only file has no ill effects.)
 */

/* Global storage. */

int openr_id = OPENR_ID;        /* Callback id storage, read. */

/* acc_cb() */

int acc_cb(int *id_arg, struct FAB *fab, struct RAB *rab)
{
    int sts;

    /* Get process RMS_DEFAULT values, if not already done. */
    if (rms_defaults_known == 0)
    {
        get_rms_defaults();
    }

    /* If RMS_DEFAULT (and adjusted active) values are available, then set
     * the FAB/RAB parameters.  If RMS_DEFAULT values are not available,
     * suffer with the default parameters.
     */
    if (rms_defaults_known > 0)
    {
        /* Set the FAB/RAB parameters accordingly. */
        fab-> fab$w_deq = rms_ext_active;
        rab-> rab$b_mbc = rms_mbc_active;
        rab-> rab$b_mbf = rms_mbf_active;

        /* Truncate at EOF on close, as we'll probably over-extend. */
        fab-> fab$v_tef = 1;

        /* If using multiple buffers, enable read-ahead and write-behind. */
        if (rms_mbf_active > 1)
        {
            rab-> rab$v_rah = 1;
            rab-> rab$v_wbh = 1;
        }

        if (DIAG_FLAG)
        {
            fprintf(stderr,
              "Open callback.  ID = %d, deq = %6d, mbc = %3d, mbf = %3d.\n",
              *id_arg, fab-> fab$w_deq, rab-> rab$b_mbc, rab-> rab$b_mbf);
        }
    }

    /* Declare success. */
    return 0;
}



/*
 * 2004-09-19 SMS.
 *
 *----------------------------------------------------------------------
 *
 *       decc_init()
 *
 *    On non-VAX systems, uses LIB$INITIALIZE to set a collection of C
 *    RTL features without using the DECC$* logical name method.
 *
 *----------------------------------------------------------------------
 */

#ifdef __CRTL_VER
#if !defined(__VAX) && (__CRTL_VER >= 70301000)

#include <unixlib.h>

/*--------------------------------------------------------------------*/

/* Global storage. */

/*    Flag to sense if decc_init() was called. */

static int decc_init_done = -1;

/*--------------------------------------------------------------------*/

/* decc_init()

      Uses LIB$INITIALIZE to set a collection of C RTL features without
      requiring the user to define the corresponding logical names.
*/

/* Structure to hold a DECC$* feature name and its desired value. */

typedef struct
{
   char *name;
   int value;
} decc_feat_t;

/* Array of DECC$* feature names and their desired values. */

decc_feat_t decc_feat_array[] = {

   /* Preserve command-line case with SET PROCESS/PARSE_STYLE=EXTENDED */
 { "DECC$ARGV_PARSE_STYLE", 1 },

#if 0  /* Possibly useful in the future. */

   /* Preserve case for file names on ODS5 disks. */
 { "DECC$EFS_CASE_PRESERVE", 1 },

   /* Enable multiple dots (and most characters) in ODS5 file names,
      while preserving VMS-ness of ";version". */
 { "DECC$EFS_CHARSET", 1 },

#endif /* 0 */

   /* List terminator. */
 { (char *)NULL, 0 } };


/* LIB$INITIALIZE initialization function. */

static void decc_init(void)
{
    int feat_index;
    int feat_value;
    int feat_value_max;
    int feat_value_min;
    int i;
    int sts;

    /* Set the global flag to indicate that LIB$INITIALIZE worked. */

    decc_init_done = 1;

    /* Loop through all items in the decc_feat_array[]. */

    for (i = 0; decc_feat_array[i].name != NULL; i++)
    {
        /* Get the feature index. */
        feat_index = decc$feature_get_index(decc_feat_array[i].name);
        if (feat_index >= 0)
        {
            /* Valid item.  Collect its properties. */
            feat_value = decc$feature_get_value(feat_index, 1);
            feat_value_min = decc$feature_get_value(feat_index, 2);
            feat_value_max = decc$feature_get_value(feat_index, 3);

            if ((decc_feat_array[i].value >= feat_value_min) &&
                (decc_feat_array[i].value <= feat_value_max))
            {
                /* Valid value.  Set it if necessary. */
                if (feat_value != decc_feat_array[i].value)
                {
                    sts = decc$feature_set_value(
                              feat_index,
                              1,
                              decc_feat_array[ i].value);
                }
            }
            else
            {
                /* Invalid DECC feature value. */
                printf(" INVALID DECC FEATURE VALUE, %d: %d <= %s <= %d.\n",
                  feat_value,
                  feat_value_min, decc_feat_array[i].name, feat_value_max);
            }
        }
        else
        {
            /* Invalid DECC feature name. */
            printf(" UNKNOWN DECC FEATURE: %s.\n", decc_feat_array[i].name);
        }
    }
}

/* Get "decc_init()" into a valid, loaded LIB$INITIALIZE PSECT. */

#pragma nostandard

/* Establish the LIB$INITIALIZE PSECT, with proper alignment and
   attributes.
*/
globaldef {"LIB$INITIALIZ"} readonly _align (LONGWORD)
   int spare[ 8] = { 0 };
globaldef {"LIB$INITIALIZE"} readonly _align (LONGWORD)
   void (*x_decc_init)() = decc_init;

/* Fake reference to ensure loading the LIB$INITIALIZE PSECT. */

#pragma extern_model save
int lib$initialize(void);
#pragma extern_model strict_refdef
int dmy_lib$initialize = (int)lib$initialize;
#pragma extern_model restore

#pragma standard

#endif /* !defined( __VAX) && (__CRTL_VER >= 70301000) */
#endif /* __CRTL_VER */
#endif /* __DECC */

#endif /* VMS */
