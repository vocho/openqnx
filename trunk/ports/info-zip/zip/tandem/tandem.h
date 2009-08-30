/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
#ifndef __tandem_h   /* prevent multiple inclusions */
#define __tandem_h

#define TANDEM       /* better than __TANDEM */

/* LICENSED define now supplied by compile time option (MAKE) */

#define NO_UNISTD_H
#define NO_RMDIR
#define NO_MKTEMP
#ifdef ZIP
#  define USE_CASE_MAP
#  define USE_EF_UT_TIME
#endif /* ZIP */

/* Include file for TANDEM */

#ifndef NULL
#  define NULL 0
#endif

#include <time.h>               /* the usual non-BSD time functions */
#include <stdio.h>
#include <sysstat.h>
#include <unistd.h>
#include <errno.h>

#define PASSWD_FROM_STDIN
                  /* Kludge until we know how to open a non-echo tty channel */

#define NSK_UNSTRUCTURED   0
#define NSK_RELATIVE       1
#define NSK_ENTRYSEQUENCED 2
#define NSK_KEYSEQUENCED   3
#define NSK_OBJECTFILECODE 100
#define NSK_EDITFILECODE   101
#define NSK_ZIPFILECODE    1001
#define TANDEM_BLOCKSIZE   4096
#define MAX_NORMAL_READ    4096
#define MAX_EDIT_READ      255
#define MAX_LARGE_READ        57344
#define MAX_LARGE_READ_EXPAND 30720

#define MAXFILEPARTLEN     8
#define MAXPATHLEN         128
#define EXTENSION_MAX      3
/* FILENAME_MAX is defined in stdio.h */

#define EXIT zexit     /*  To stop creation of Abend files */
#define RETURN zexit   /*  To stop creation of Abend files */
#define fopen zipopen  /*  To allow us to set extent sizes */
#define putc zputc     /*  To allow us to auto flush  */


/* Prototype function declarations */

void zexit (int);

FILE *zipopen(
const char *,
const char *
);

int zputc(
int,
FILE *
);

int zgetch (void);

short parsename(
  const char *,
  char *,
  char *
);

int islicensed (void);

time_t gmt_to_time_t (long long *);

/* End of prototype function declarations */


#define FOPR "rb"
#define FOPM "r+"
#define FOPW "wb"
#define FOPWT "w"

#define CBSZ 0x10000  /* Was used for both fcopy and file_read.        */
                      /* Created separate define (SBSZ) for file_read  */
                      /* fcopy param is type size_t (unsigned long)    */
                      /* For Guardian we choose a multiple of 4K       */

#define ZBSZ 0x10000  /* This is used in call to setvbuf, 64K seems to work  */
                      /* in all memory models. Again it is an unsigned long  */
                      /* For Guardian we choose a multiple of 4K             */

#ifndef __INT32
#define SBSZ 0x0e000  /* Maximum of size unsigned (int). Only used in STORE  */
                      /* method.  We can use up to 56K bytes thanks to large */
                      /* transfer mode.  Note WSIZE is limited to 32K, which */
                      /* limits the DEFLATE read size to same value.         */
#else
#define SBSZ 0x10000  /* WIDE model so we can use 64K                        */
#endif /* __INT32 */

#define NAMELEN FILENAME_MAX+1+EXTENSION_MAX   /* allow for space extension */

struct dirent {
   struct dirent *d_next;
   char   d_name[NAMELEN+1];
};

typedef struct _DIR {
   struct  dirent *D_list;
   struct  dirent *D_curpos;
   char            D_path[NAMELEN+1];
} DIR;

DIR *          opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
void           rewinddir(DIR *dirp);
int            closedir(DIR *dirp);
char *         readd(DIR *dirp);

#define DISK_DEVICE         3

/* SETMODE Literals */
#define SET_FILE_SECURITY   1
#define SET_FILE_OWNER      2
#define SET_FILE_BUFFERED   90
#define SET_FILE_BUFFERSIZE 93
#define SET_LARGE_TRANSFERS 141

/* FILE_OPEN_ Literals */
#define NSK_RDWR             0
#define NSK_RDONLY           1
#define NSK_WRONLY           2
#define NSK_APPEND           3
#define NSK_SHARED           0
#define NSK_EXCLUSIVE        1
#define NSK_PROCESSEXCLUSIVE 2
#define NSK_PROTECTED        3
#define NSK_UNSTRUCTUREDACCESS 0x8000
#define NSK_NOUPDATEOPENTIME   0x2000

#define NSK_NO_DELIMITER        0x0001
#define NSK_USE_FF_DELIMITER    0x0002
#define NSK_SPACE_FILL          0x0004
#define NSK_TRIM_TRAILING_SPACE 0x0008
#define NSK_LARGE_READ_EXPAND   0x0100     /* use smaller value for Expand */

#define DOS_EXTENSION      '.'
#define TANDEM_EXTENSION   ' '
#define TANDEM_DELIMITER   '.'
#define TANDEM_NODE        '\\'
#define INTERNAL_DELIMITER '/'
#define INTERNAL_NODE      '//'
#define TANDEM_WILD_1      '*'
#define TANDEM_WILD_2      '?'

#define DOS_EXTENSION_STR      "."
#define TANDEM_EXTENSION_STR   " "
#define TANDEM_DELIMITER_STR   "."
#define TANDEM_NODE_STR        "\\"
#define INTERNAL_DELIMITER_STR "/"
#define INTERNAL_NODE_STR      "//"

/* Use 'spare' area at end of stat structure to hold additional Tandem/NSK
   file details. Initially used to hold Creation time, now also holds most
   Enscribe details */

struct nsk_stat_reserved
{
  int64_t spare[3];
};

typedef struct
{
  unsigned group   : 8;
  unsigned user    : 8;
} nsk_owner;

typedef struct
{
  unsigned buffered    : 1;
  unsigned audited     : 1;
  unsigned acompress   : 1;
  unsigned icompress   : 1;
  unsigned dcompress   : 1;
  unsigned oddunstr    : 1;
  unsigned verified    : 1;
  unsigned serial      : 1;
  unsigned refresheof  : 1;
  unsigned broken      : 1;
  unsigned corrupt     : 1;
  unsigned primpart    : 1;
  unsigned secpart     : 1;
  unsigned crashopen   : 1;
  unsigned rollforward : 1;
  unsigned clearonpurge: 1;
} nsk_file_flags;

typedef struct
{
  unsigned short filecode;  /* 16 */
  unsigned short block;     /* 16 */  /* Allow of block > 4096 one day ! */
  nsk_file_flags flags;     /* 16 */
  nsk_owner owner;          /* 16 */
  unsigned short primext;   /* 16 */
  unsigned short secext;    /* 16 */
  unsigned maxext    : 10;
  unsigned read      : 3;
  unsigned write     : 3;
  unsigned execute   : 3;
  unsigned delete    : 3;
  unsigned licensed  : 1;
  unsigned progid    : 1;
  unsigned keylen    : 8;
  unsigned           : 5;
  unsigned keyoff    : 11;
  unsigned           : 1;
  unsigned filetype  : 2;
  unsigned fileopen  : 1;
  unsigned reclen    : 12;
} nsk_file_attrs;

struct nsk_stat_overlay
{
  time_t creation_time;  /* 32 */
  char   nsk_ef_start;   /* Start of ef region */
};

typedef union
{
  struct nsk_stat_reserved reserved;
  struct nsk_stat_overlay  ov;
} nsk_stat_ov;

#endif /* !__tandem_h */
