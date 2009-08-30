/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




/*
---------------------------------------------------------------------------

   find.c      ; main source module for the QNX4 find utility


   Please send comments pertaining to this utility to Eric Johnson 
   (eric@qnx.com)
 --------------------------------------------------------------------------
*/


/* NOTE: the USAGE MESSAGE is found at the END of this source file */


/*--------------------------------------------- FIND CONFIGURATION #DEFINES 

 o  Define 'RFIND' if a restricted version is to be made.
    RFIND will eliminate many primaries which are of
    questionable use to the QUICS user and/or might
    pose a security risk.
    
    The primaries which are eliminated if RFIND is defined are:
    
      -spawn       -perm       -exec       -ok       -fls
      -mountdev    -mountpoint -fprint     -fprintf  -fsmanager
      -fprint0     -xdev       -nouser     -nogroup  -links
      -extents

    Note -logical is made redundant as rfind is _always_ in
    -logical/-follow mode.

 o Define 'STRICT_PATH' if a version that does not assume
   a directory '.' if none is specified is to be made.

 o Define FNMATCH_REGEX if -name, -iname etc are to be
   converted to regular expressions internally for processing.
   (this is now typically set, below)
 -------------------------------------------------------------------------
*/

/* To make restricted find:  #define RFIND */
/* To make specification of a path mandatory (POSIX): #define STRICT_PATH */
/* To translate fnmatch type comparisons to regex: #define FNMATCH_REGEX */
#define FNMATCH_REGEX

#ifndef __QNXNTO__
#define EXTENTS_SUPPORTED
#define CREATION_TIME_SUPPORTED
#define MOUNT_INFO_SUPPORTED
#define VERSION_SUPPORTED
#define STATUS_SUPPORTED
#endif


/* --------------------------------------------------- HEADER INCLUDES ---- */
    
#include <util/util_limits.h>
#include "find.h"


/* ----------------------------------------- TEXT STRINGS FOR USER MESSAGES */

const char T_Bad_Perm_Spec[]      = "find: Illegal -perm spec '%s'\n";
#ifdef EXTENTS_SUPPORTED
const char T_Fsys_Stat_Failed[]   = "find: Can't get fsys_stat.";
#endif
const char T_Invalid_Group[]      = "find: Unknown group (%s)\n";
const char T_Invalid_User[]       = "find: Unknown user (%s)\n";
const char T_Malloc_Failed[]      = "find: Ran out of memory.";
const char T_No_Paths_Specified[] = "find: No paths specified.\n";
const char T_Stat_Failed[]        = "find: Can't get stat.";


/* ----------------------------------- TABLE OF PRIMARY OPERATORS (COMMANDS) */
static cmd cmds[]={
  { "a"       , T_AND               },  { "and"       , T_AND               },
  { "not"     , T_NOT               },  { "o"         , T_OR                },
  { "or"      , T_OR                },  { "XOR"       , T_XOR               },

  #ifdef CREATION_TIME_SUPPORTED
    { "fFnewer" , FFNEWER             },
    { "fmin"    , CREATED_ON_MIN      },
    { "ftime"     , CREATED_ON        },
  #endif

  #ifndef RFIND                       
    #ifdef EXTENTS_SUPPORTED
      { "extents" , EXTENTS_EQUAL_TO    },
    #endif
	#ifdef MOUNT_INFO_SUPPORTED
      { "mountdev", MOUNTDEV            },
      { "mountpoint", MOUNTPOINT        },
    #endif
    { "chmod"   , CHMOD               },  { "chown"     , CHOWN               },
    { "chgrp"   , CHGRP               },  { "exec"      , EXEC                },
    { "fls"       , FLS               },
    { "fprint"  , FPRINT              },  { "fprint0"   , FPRINT0             },
    { "fprintf" , FPRINTF             },  { "fsmanager" , FSMANAGER           },
    { "inode"   , INODE_EQUAL_TO      },  { "inum"      , INODE_EQUAL_TO      },
    { "links"   , LINKS_EQUAL_TO      },  { "mount"     , XDEV                },
    { "nogroup" , NOGROUP             },  { "nouser"    , NOUSER              },
    { "ok"      , DASHOK              },  { "perm"      , PERMS_EQUAL_TO      },
    { "remove!" , UNLINK              },  { "rename"    , RENAME              },
    { "spawn"   , SPAWN               },  { "xdev"      , XDEV                },
  #endif

  { "abort"   , ABORT               },  { "anewer"    , ANEWER              },
  { "atime"   , LAST_ACCESSED_ON    },  { "amin"      , LAST_ACCESSED_ON_MIN},
  { "cmin"    , LAST_STATCHG_ON_MIN },  { "cnewer"    , CNEWER              },
  { "ctime"   , LAST_STATCHG_ON     },  { "daystart"  , DAYSTART            },
  { "depth"   , DEPTH               },  { "echo"      , ECHOSPAM            },
  { "empty"   , EMPTY               },  { "error"     , XERR                },
  { "errmsg"  , ERRMSG              },  { "exists"    , EXISTS              },
  { "false"   , ALWAYS_FALSE        },  { "fanewer"   , FANEWER             },
  { "fcnewer" , FCNEWER             },  { "fmnewer"   , FMNEWER             },
  { "Fnewer"  , FNEWER              },  { "fnewer"    , FMNEWER             },
  { "gid"     , GROUP_EQUAL_TO      },  { "group"     , GROUP_EQUAL_TO      },
  { "ilname"  , ILNAME              },  { "iname"     , IFNMATCH            },
  { "ipath"   , IPFNMATCH           },  { "iregex"    , IREGEX              },
  { "level"   , LEVEL_EQUAL_TO      },  { "lname"     , LNAME               },
  { "logical" , LOGICAL             },  { "ls"        , LS                  },
  { "maxdepth", MAXDEPTH            },  { "mindepth"  , MINDEPTH            },
  { "mmin"    , LAST_MODIFIED_ON_MIN},  { "mnewer"    , MNEWER              },
  { "mtime"   , LAST_MODIFIED_ON    },  { "name"      , FNMATCH             },
  { "newer"   , MNEWER              },  { "NOP"       , ALWAYS_TRUE_NOP     },
  { "path"    , PFNMATCH            },  { "pname"     , PFNMATCH            },
  { "prune"   , PRUNE               },  { "print"     , PRINT               },
  { "print0"  , PRINT0              },  { "printf"    , PRINTF              },
  { "regex"   , REGEX               },  { "size"      , SIZE_EQUAL_TO       },
  { "true"    , ALWAYS_TRUE         },  { "type"      , TYPE_EQUAL_TO       },
  { "uid"     , USER_EQUAL_TO       },  { "used"      , USED_EQUAL_TO       },
  { "user"    , USER_EQUAL_TO       },  { "follow"    , LOGICAL             }, 
  { "status"    , STATUS              },

  { NULL      , 0                   }
};


/*-------------------------------------------------- LOCAL TYPEDEFS */

typedef struct devino_s {
    ino_t ino;
    dev_t dev;
} devino_t;

#define OWNERSHIP_USER 1
#define OWNERSHIP_GROUP 2
typedef struct ownership_s {
    int   flags;    /* OWNERSHIP_USER &| OWNERSHIP_GROUP */
    uid_t userid;
    gid_t groupid;
} ownership_t;


/*-------------------------------------------------- GLOBAL VARS */

static exprn    *Main_Expr; 

static int      Glob_Argc;
static char     **Glob_Argv;

                                       /* Description of Default Setting */
static bool     DaystartFlag  = FALSE, /* -Xtime based on 24hr period */
                DepthFlag     = FALSE, /* evaluate dirs before recursing into them */
#ifdef EXTENTS_SUPPORTED
                Fsys_Stat_Reqd= FALSE, /* is an fsys_stat() required to evaluate files? */
#endif
                GotAtLeast1   = FALSE, /* for exit status - did we find a match? */
                Print_On_Match= TRUE,  /* automatic -print if expr evaluates TRUE */
                PruneFlag     = FALSE, /* recurse into subdirectories */
                Statbuf_Valid = FALSE, /* stat buffer currently has no data */
                Trap_SIGINT   = FALSE, /* no need to trap SIGINT */
                Verbose       = FALSE, /* not verbose */
                XdevFlag      = FALSE; /* freely descend past device boundaries */

#ifdef RFIND
static bool     LogicalFlag=TRUE;      /* treat symlinks as the file they point to
                                          recurse into them if they point to
                                          directories */
#else
static bool     LogicalFlag=FALSE;     /* treat symlinks just as symlinks */
#endif

static struct stat  *Statbufl;  /* pointer to buffer for lstat() info */
static struct stat  *Statbufm;  /* pointer to stat buffer used in _match */
static struct stat  *Statbufp;  /* pointer to stat buffer used in traversing paths */

#ifdef EXTENTS_SUPPORTED
static struct _fsys_stat    *Fsys_Statbufm;
static struct _fsys_stat    *Fsys_Statbufp;
#endif

#ifdef MOUNT_INFO_SUPPORTED
#ifndef RFIND
    static char IsMountBuf1[UTIL_PATH_MAX]; /* for -mountpoint, -mountdev */
    static char IsMountBuf2[UTIL_PATH_MAX]; /* for -mountpoint */
    static char IsMountBuf3[UTIL_PATH_MAX]; /* for -mountpoint */
#endif
#endif

static char     *Working_On;    /* points to the file on the command line
                                   which is at the head of the file tree
                                   currently being worked on */

static bool     Directory_Removed; /* TRUE when -remove! has removed the
                                      directory file being examined. Do not
                                      recurse into it */

static char     Command[UTIL_PATH_MAX];          /* Used for -exec, -ok etc, also parsing */
static char     Signal_Message[UTIL_PATH_MAX];   /* for telling user what cmd was
                                           interrupted by their ctrl-c! */
static char     Current_Path[UTIL_PATH_MAX];      /* Path currently being evaluated */
static char     *Filename;              /* matcher: filename being evaluated */

static int      Glob_Index;             /* parser: index into argv */
static int      Error = 0;              /* error flag for exit status */

static int      RC; /* return code from system/spawn calls */


/*
  for -type: parser will put pointers to these into the expression structure
  which the matcher will use to determine filetype
*/
static mode_t   Mfifo=_S_IFIFO, Mchr=_S_IFCHR,  Mdir=_S_IFDIR,
                Mblk=_S_IFBLK,  Mreg=_S_IFREG,  Mlnk=_S_IFLNK,
                Mnam=_S_IFNAM,  Msock=_S_IFSOCK;


static int      Parentheses_Level;  /* parser */

static int      Levels = 0,         /* how many levels down are we in dir tree? */
                Maxdepth=-1,        /* maximum levels to recurse down */
                Mindepth=0;         /* minimum level file must be at before
                                       expression will be applied */

dev_t           Device_Id = -1;     /* for -xdev stuff */

time_t          Start_Time;     /* time at which find began processing files.
                                   used for -ctime, -atime etc */

time_t          Daystart_Time;  /* time at which find began processing files.
                                   used for -ctime, -atime etc when -daystart
                                   has been specified (DaystartFlag set) */


/* return s[0] only if s[1] is NUL */
#define BYNUL(s) ((s[1]==0)?s[0]:0)

 
/* ------------------------------------------------- prototypes --------------*/

extern char *fnmatch2regex(const char *);

void                          sigint_handler         (int);
bool                          isallnumeric           (char *);
mode_t                        get_who                (char **);
mode_t                        parse_mode             (char *, mode_t);
char *                        dirname                (char *);
char *                        find_basename          (char *);
#ifdef VERSIONS_SUPPORTED
struct _sysmsg_version_reply* get_filesystem_version (dev_t, char *);
#endif
void                          find_fprintf           (FILE*, char*, struct stat *, char *, char *);
#ifdef EXTENTS_SUPPORTED
bool                          match                  (char *,struct stat *, struct stat *, struct _fsys_stat *,exprn *);
#else
bool                          match                  (char *,struct stat *, struct stat *,exprn *);
#endif
bool                          _match                 (exprn *);
int                           special_system         (char *cmd);
bool                          eval_builtin           (exprn *);
FILE *                        FFopen                 (char*);
void                          check_argavail         (int16_t);
exprn *                       parse_expression       (int, char **);
exprn *                       _parse_expression      (int);
int16_t                         cmd_match              (char *);
void                          recurse_dir            (char *);
int                           check_devinos          (int, dev_t, ino_t);
void                          process_file           (char *);
#ifdef DIAG
void                          diag                   (exprn *,int );
#endif



/*-------------------------------------------------- sigint_handler() --------*/
/* The sigint_handler is for handling cases where find is interrupted and
   -exec, -ok, -spawn etc are being used. */

void sigint_handler (int signal)
{
    switch(signal) {
        case SIGINT:
            /* what do we really want to do here?? */
            if (*Signal_Message) {
                fprintf(stderr,"find: %s\n",Signal_Message);
                *Signal_Message = 0;
            } else {                
                fprintf(stderr,"find: interrupted by SIGINT.\n");
            }

            fprintf(stderr,"find: Continue? (N/y) ");

            fflush(stdout); fflush(stderr);
            tcflush(STDIN_FILENO,TCIFLUSH);
            {
                char c=0;
                while (c!=EOF && c!='Y' && c!='N' && c!='\n') {
                    if (read(STDIN_FILENO,&c,1)==-1) break;
                    c=toupper(c);
                }

                if (c!='Y') exit(RC);
            }
            break;
        default:
            break;
    }
}

/*-------------------------------------------------- isallnumeric(char*)------*/

bool isallnumeric (char *s1)
{
    while ((*s1<='9') && (*s1>='0')) s1++;
    return(*s1=='\000');
}

/*----------------------------------------- permission parsing ----------*/

#define USER 0100
#define GROUP 010
#define OTHER 01

#define MINUS 1
#define PLUS 2
#define EQUAL 3

mode_t get_who (char **stringp)
{
    mode_t who=0;

    while (**stringp) {
        switch (**stringp) {
            case 'u': who |= USER; break;
            case 'g': who |= GROUP; break;
            case 'o': who |= OTHER; break;
            case 'a': who = USER|GROUP|OTHER; break;
            case '-':
            case '+':
            case '=': return(who);
            default: fprintf(stderr,"parse fail (who) at '%c'\n",**stringp);
                     Error++;
                     return(-1);
        }
        (*stringp)++;
    }   
    return(0xFFFF); 
}

int get_op (char **stringp)
{
    int op;

    switch (**stringp) {
        case '-': op = MINUS; break;
        case '+': op = PLUS; break;
        case '=': op = EQUAL; break;
        default : op = 0;
                  fprintf(stderr,"get_op - should never fail, but failed!\n");
                  Error++;
    }
    (*stringp)++;
    return(op);
}
    

mode_t get_perm (char **stringp)
{
    mode_t perm=0;

    while (**stringp && (**stringp!=',')) {
        switch (**stringp) {
            case 'r': perm |= S_IROTH; break;
            case 'w': perm |= S_IWOTH; break;
            case 'x': perm |= S_IXOTH; break;
            case 's': perm |= S_ISGID; 
                      perm |= S_ISUID; break;
            default: fprintf(stderr,"parse fail (perm) at '%c'\n",**stringp);
                     Error++;
                     return(0xffff);
        }
        (*stringp)++;
    }   
    return(perm);
}

    
/*-------------------------------------------------- parse_mode(char*) -------*/
mode_t parse_mode (char *string, mode_t template)
{
    static mode_t who, perm, mask, whomask;
    static int op;
    
    if (isoctal(*string)) {
        perm = 0;

        while (isoctal(*string)) {
            perm *= 8;
            perm += *string - '0';
            string++;
        }

        if (*string) return(Error++,0xffff);
        else return(perm);
    }

    if ((who = get_who(&string))==0xffff) return(0xffff);
    if (!(op = get_op(&string))) return(-1);
    if ((perm = get_perm(&string))==0xffff) return(0xffff);

    if (!who) {
        perm = (perm&(S_ISGID|S_ISUID)) | ( (perm&S_IPERMS) * (USER+GROUP+OTHER));
        if (op==PLUS || op==EQUAL || op==MINUS) {
            mask = umask(0);
            umask(mask);
            perm &= ~mask;
        }
    } else {
        perm = (perm&(S_ISGID|S_ISUID)) | ( (perm&S_IPERMS) * who );
        if (!(who&USER)) perm &= ~S_ISUID;
        if (!(who&GROUP)) perm &= ~S_ISGID;
    }


    switch(op) {
        case PLUS: template |= perm; break;
        case EQUAL:
            whomask=0;
            if (who&USER)  whomask|=S_IRWXU+S_ISUID;
            if (who&GROUP) whomask|=S_IRWXG+S_ISGID;
            if (who&OTHER) whomask|=S_IRWXO;
            template&=~whomask;  /* mask off all bits for the who specified */
            template |= perm&whomask; /* or back in the perms set for that who */
            break;
        case MINUS: template &= ~perm; break;
    }
    if (*string==',') template = parse_mode(string+1,template);

    return(template);
}

/* snarfed from dirname utility */
char *dirname (char *pathname) 
{
    static char buffer[UTIL_PATH_MAX];
    char *src_str, *dst_str=NULL;
    int i;

    src_str=buffer;
    strncpy(buffer,pathname,UTIL_PATH_MAX);

    /* step 1 - if string is //, skip steps (2) through (5)  */

    /* I don't really need this goto since in this implementation steps 7
     * and 8 are skipped for a file of //. However, in the interests of
     * having things not break if this is changed, the goto is here anyway.
     */

    if (!strcmp(src_str,"//")) goto step6;
    else {
        /* step 2 - if string consists entirely of slash characters, string
         *          shall be set to a single slash character. In this case,
         *          skip steps 3 through 8.
         */
        for (i=0,dst_str=src_str;*dst_str;dst_str++)
            if (*dst_str!='/') i++; /* i non-zero if any chars other than / encountered */

        if (!i) src_str=--dst_str;  /* all /s, back off 1 so dst_str = "/" */
        else {
            /* step 3 - if there are any trailing slash characters in string,
             *          they shall be removed.
             */
            for (;*--dst_str=='/';*dst_str=0);

            /* step 4 - if there are no slash characters remaining in string,
             *          string shall be set to a single period character. In
             *          this case, skip steps 5 through 8. 
             */
            dst_str = strrchr(src_str,'/');
            if (dst_str==NULL) src_str = ".";
            else {
                /* step 5 - if there are any trailing nonslash characters in
                 *          string, they shall be removed.
                 */
                if (*++dst_str!='/') *dst_str=0;
                
                /* step 6 - if the remaining string is //, it is implementation
                 *          defined whether steps 7 and 8 are skipped or
                 *          processed.
                 *
                 * We will skip 7 and 8 in this case.
                 */
step6:          /* entry point for special case of //. In this implementation
                 * these next two steps are skipped anyway (if conditional below)
                 */
                if (strcmp(src_str,"//")) {
                    /* step 7 - if there are any trailing slash characters in
                     *          string, they shall be removed. 
                     */
                    for (;dst_str!=src_str && *--dst_str=='/';*dst_str=0);

                    /* step 8 - if the remaining string is empty, string shall
                     * be set to a single slash character                   
                     */    
                    if (!*src_str) src_str = "/";
                }
            }
        }
    }

    return src_str;
}

#ifdef VERSIONS_SUPPORTED
struct _sysmsg_version_reply *get_filesystem_version (dev_t device,char *cur_path)
{
    static dev_t last_device=-1;
    static struct _sysmsg_version_reply last_reply;

    int     fd;
    union {
        struct _io_open         s;
        struct _io_open_reply   r;
    } *iomsg;

    union {
        struct {
            struct _sysmsg_hdr              hdr;
            struct _sysmsg_version          data;

        } s;
        struct {
            struct _sysmsg_hdr_reply        hdr;
            struct _sysmsg_version_reply    data;
        } r;
    } vmsg;

    if (device==last_device) return &last_reply;

    last_device = device;

    /* GET IO HANDLE FOR THE FILE */
    if((iomsg = alloca(sizeof(struct _io_open) + UTIL_PATH_MAX)) == NULL)
    {
        fprintf(stderr,"find: out of memory (stack)\n");
        exit(EXIT_FAILURE);
    }

    iomsg->s.oflag = _IO_HNDL_INFO;
    fd = __resolve_net(_IO_HANDLE, 1, iomsg, cur_path, sizeof(iomsg->r), 0);

    if(fd == -1) {
        strcpy(last_reply.name,"Unknown");
        return &last_reply;
    }

    vmsg.s.hdr.type = _SYSMSG;
    vmsg.s.hdr.subtype = _SYSMSG_SUBTYPE_VERSION;
    vmsg.s.data.unit = 0; /* unit 0 - some mgrs store versions for drivers etc */

    Sendfd(fd, &vmsg.s, &vmsg.r, sizeof(vmsg.s), sizeof(vmsg.r));
    if(vmsg.r.hdr.status == EOK) {
        memcpy(&last_reply,&vmsg.r.data,sizeof(vmsg.r.data));
        /* msg.r.data.version/100, version%100, letter, date */
    }

    close(fd);

    return &last_reply;
}
#endif

void find_fprintf (FILE *fp,
                   char *format,
                   struct stat *statbuf,
                   char *curpath,
                   char *workon)
{
    char *readp;
    char c=0;     /* temp storage spot for 1 character */
    static char printf_string[UTIL_PATH_MAX],
                strftime_string[4],
                strftime_buffer[50],
                string_format[16],
                decimal_format[16],
                long_decimal_format[16],
                long_long_decimal_format[16],
                long_long_unsigned_format[16],
                octal_format[16];
    time_t  *timeptr=NULL;
    int     minsize=-1, precision=-1, left_justify=0, i;

    for (readp=format;*readp;readp++) {
        switch (*readp) {
          case '\\':    /* backslash escapes */
            readp++;
            switch (*readp) {
                case 'a': c=7; break;   /* BEL */
                case 'b': c=8; break;   /* BS */
                case 'c': /* stop printing from this format and flush output */
                    fflush(fp);
                    break;
                case 'f': c=0x0c; break; /* FF */
                case 'n': c='\n'; break; /* LF */
                case 'r': c='\r'; break; /* CR */
                case 't': c=0x09; break; /* HT */
                case 'v': c=0x0b; break; /* VT */
                case '\\': c='\\'; break; /* \ */
                default: c=*readp; fprintf(fp,"\\"); break;
            }
            if (c) fprintf(fp,"%c",c);
            break;

          case '%':     /* %format codes */
            readp++;
            if (*readp=='-') {
                left_justify=1;
                readp++;
            } else left_justify=0;

            if (*readp<='9' && *readp>='0') {
                for (minsize=0;*readp<='9' && *readp>='0';readp++) {
                    /* a minimum field size is specified */
                    minsize*=10;
                    minsize+=*readp-'0';
                }
            } else minsize=-1;

            if (*readp=='.') {
                /* a maximum field size is specified */
                for (readp++,precision=0;*readp<='9' && *readp>='0';readp++) {
                    precision*=10;
                    precision+=*readp-'0';
                }
            } else precision=-1;

            if (precision!=-1 && precision>UTIL_PATH_MAX) {
                fprintf(stderr,"find: -*printf illegal format '%s' (.precision>%d)\n",format,UTIL_PATH_MAX);
                exit(EXIT_FAILURE);
            }

            if (minsize!=-1 && minsize>UTIL_PATH_MAX) {
                fprintf(stderr,"find: -*printf illegal format '%s' (minsize>%d)\n",format,UTIL_PATH_MAX);
                exit(EXIT_FAILURE);
            }

#if _FILE_OFFSET_BITS-0 == 64
#define LONG_LONG "l"
#else
#define LONG_LONG
#endif

            if (minsize==-1 && precision==-1) {
                /* neither minsize nor precision specified */
                sprintf(string_format,"%%s");
                sprintf(decimal_format,"%%d");
                sprintf(long_decimal_format,"%%ld");
                sprintf(octal_format,"%%o");
                sprintf(long_long_decimal_format,"%%" LONG_LONG "ld");
                sprintf(long_long_unsigned_format,"%%" LONG_LONG "lu");
            } else if (minsize!=-1 && precision==-1) {
                /* minsize is specified but no precision */
                sprintf(string_format,"%%%s%ds",left_justify?"-":"",minsize);
                sprintf(decimal_format,"%%%s%dd",left_justify?"-":"",minsize);
                sprintf(long_decimal_format,"%%%s%dld",left_justify?"-":"",minsize);
                sprintf(octal_format,"%%%s%do",left_justify?"-":"",minsize);
                sprintf(long_long_decimal_format,"%%%s%d" LONG_LONG "ld",left_justify?"-":"",minsize);
                sprintf(long_long_unsigned_format,"%%%s%d" LONG_LONG "lu",left_justify?"-":"",minsize);
            } else if (minsize==-1 && precision!=-1) {                              
                /* precision specified but no minsize */                
                sprintf(string_format,"%%%s.%ds",       left_justify?"-":"",precision);
                sprintf(decimal_format,"%%%s.%dd",      left_justify?"-":"",precision);
                sprintf(long_decimal_format,"%%%s.%dld",left_justify?"-":"",precision);
                sprintf(octal_format,"%%%s.%do",        left_justify?"-":"",precision);
                sprintf(long_long_decimal_format,"%%%s.%d" LONG_LONG "ld",left_justify?"-":"",precision);
                sprintf(long_long_unsigned_format,"%%%s.%d" LONG_LONG "lu",left_justify?"-":"",precision);
            } else {                        
                /* both minsize and precision specified */
                sprintf(string_format,"%%%s%d.%ds",       left_justify?"-":"",minsize,precision);
                sprintf(decimal_format,"%%%s%d.%dd",      left_justify?"-":"",minsize,precision);
                sprintf(long_decimal_format,"%%%s%d.%dld",left_justify?"-":"",minsize,precision);
                sprintf(octal_format,"%%%s%d.%do",        left_justify?"-":"",minsize,precision);
                sprintf(long_long_decimal_format,"%%%s%d.%d" LONG_LONG "ld",left_justify?"-":"",minsize,precision);
                sprintf(long_long_unsigned_format,"%%%s%d.%d" LONG_LONG "lu",left_justify?"-":"",minsize,precision);
            }

            switch(*readp) {
                case '%': sprintf(printf_string,"%%"); break;
                case 'p': sprintf(printf_string,string_format,curpath); break;
                case 'f': sprintf(printf_string,string_format,find_basename(curpath)); break;
                case 'h': sprintf(printf_string,string_format,dirname(curpath)); break;
                case 'P':
                   i=strlen(workon);
                   while (curpath[i]=='/') i++;
                   sprintf(printf_string,"%s",curpath+i);
                break;
                case 'H': sprintf(printf_string,string_format,workon); break;
                case 'g': sprintf(printf_string,string_format,gid(statbuf->st_gid)); break;
                case 'G': sprintf(printf_string,decimal_format,statbuf->st_gid); break;
                case 'u': sprintf(printf_string,string_format,uid(statbuf->st_uid)); break;
                case 'U': sprintf(printf_string,decimal_format,statbuf->st_uid); break;
                case 'm': sprintf(printf_string,octal_format,statbuf->st_mode); break;

                /* size requires calculation - block special sizes are not in bytes */
                case 'k': if (S_ISBLK(Statbufm->st_mode)) {
							sprintf(printf_string,long_long_decimal_format,(Statbufm->st_size+1)/2);
						  } else {
							sprintf(printf_string,long_long_decimal_format,(Statbufm->st_size+1023)/1024);
						  }
                          break;
                case 'b': if (S_ISBLK(Statbufm->st_mode)) {
							sprintf(printf_string,long_long_decimal_format,Statbufm->st_size); 
						  } else {
							sprintf(printf_string,long_long_decimal_format,(Statbufm->st_size+511)/512); 
                          }
                          break;
                case 's': if (S_ISBLK(Statbufm->st_mode)) {
 							double d;
							char scratch[32];
							d=Statbufm->st_size*512.0;
							sprintf(scratch,"%.0f",d);
						
							sprintf(printf_string,string_format,scratch);
						  } else {
							sprintf(printf_string,long_long_decimal_format,Statbufm->st_size); 
                          }
                          break;

                case 'd': sprintf(printf_string,decimal_format,Levels); break;
#ifdef VERSIONS_SUPPORTED
                case 'F': /* call get_filesystem_version() */
                          sprintf(printf_string,string_format,(get_filesystem_version(Statbufm->st_dev,curpath))->name);

                          break;
#endif
                case 'l': i=readlink(curpath,printf_string,sizeof(printf_string));
                          if (i>=0) printf_string[i]=0; /* null-terminate */
                          else printf_string[0]=0;
                          break;
                case 'i': sprintf(printf_string,long_long_decimal_format,Statbufm->st_ino); break;
                case 'n': sprintf(printf_string,decimal_format,Statbufm->st_nlink); break;

                case 'A':
                case 'C':
                case 'T':
                          switch(*readp) {
                            case 'A': timeptr=&Statbufm->st_atime; break;
                            case 'C': timeptr=&Statbufm->st_ctime; break;
                            case 'T': timeptr=&Statbufm->st_mtime; break;
                          }

                          if (*(readp+1)) {
                            readp++;
                            strftime_string[0]='%';
                            strftime_string[1]=*readp;
                            strftime_string[2]=0;
                            strftime(strftime_buffer,sizeof(strftime_buffer),strftime_string,localtime(timeptr));
                            sprintf(printf_string,string_format,strftime_buffer);
                          } else {
                            sprintf(printf_string,string_format,"A");
                          }
                          break;
                case 'a':
                case 'c':
                case 't':
                          switch(*readp) {
                            case 'a': timeptr=&Statbufm->st_atime; break;
                            case 'c': timeptr=&Statbufm->st_ctime; break;
                            case 't': timeptr=&Statbufm->st_mtime; break;
                          }
						  strftime(strftime_buffer,sizeof(strftime_buffer),"%a %b %d %T %Y",localtime(timeptr));
                          sprintf(printf_string,string_format,strftime_buffer); break;
                          break;
#ifdef MOUNT_INFO_SUPPORTED
#ifndef RFIND
				case 'D': /* raw device name */
						  if (-1==fsys_get_mount_dev(Current_Path,IsMountBuf1))
								strcpy(IsMountBuf1,"<unknown>");

                	      sprintf(printf_string,string_format,IsMountBuf1); break;
						  break;


				case 'M': /* mount point of filesystem */
						  if (-1==fsys_get_mount_dev(Current_Path,IsMountBuf1))
								strcpy(IsMountBuf2,"<unknown>");
						  else {
							  if (-1==fsys_get_mount_pt(IsMountBuf1,IsMountBuf2))
									strcpy(IsMountBuf2,"<unknown>");
						  }
                	      sprintf(printf_string,string_format,IsMountBuf2); break;
						  break;
                         break;
#endif
#endif

                default: /* error */
                         fprintf(stderr,"find: bogus -printf spec (%%%c) in '%s'\n",*readp,format);
                         exit(EXIT_FAILURE);
            }

            fprintf(fp,"%s",printf_string);
            break;

          default: fprintf(fp,"%c",*readp); break;
        } /* switch */
        
    }
}



/*-------------------------------------------------- match() -----------------*/

#ifdef EXTENTS_SUPPORTED
bool match (char *fnm,struct stat *statbuf, struct stat *lstatbuf, struct _fsys_stat *fsys_statbuf,exprn *expr)
#else
bool match (char *fnm,struct stat *statbuf, struct stat *lstatbuf,exprn *expr)
#endif
{
    int rc;

    Filename = fnm;
    Statbufm = statbuf;
    Statbufl = lstatbuf; 
#ifdef EXTENTS_SUPPORTED
    Fsys_Statbufm = fsys_statbuf;
#endif

#ifdef DIAG
	fprintf(stdout,"match checking Filename='%s'\n",Filename);
#endif

    strcpy(Current_Path,fnm);
    rc = _match(expr);
    if (rc) GotAtLeast1=TRUE;
    return(rc);
}

bool _match (exprn *expr)
{
    bool r, result=0;

    if (expr==NULL) return(TRUE);

    do {
        if ((expr->op==T_AND) && (!result)) {
            /* instead of return (result), skip ahead to next -or */
            while ((expr=expr->next)) {
                if (expr->op==T_OR) break;
            }
            if (!expr) return(result);
        }

        if ((expr->op==T_OR) && (result)) return(result);   /* change */

        if (expr->builtin_fn) r=eval_builtin(expr);
        else r=_match(expr->subexpr);

        if (expr->flags & FLAG_NOT) r=!r;

        switch(expr->op) {
            case 0      : result = r;                           break;
            case T_OR   : result = result || r;                 break;
            case T_AND  : result = result && r;                 break;
            case T_XOR  : result = ((result!=0)^(r!=0)!=0);     break;
        }

        /* process early out depending on current result */
        switch (expr->op) {
            case T_AND: if (!result) return(result); break;
            case T_OR:  if (result)  return(result); break;
            /* no early out on  XOR */
        }
    } while ((expr = expr->next));

    return(result);
}


/*
--------------------------------------------------------------------------
    special_system() is like system() but will MASK but not ignore SIGINT.
*/

int special_system (char *cmd)
{
    int stat, err;
    /* pid_t pid; used only for 1003.2 implementation */
    struct sigaction sa, savequit;
#ifdef STANDARD_SYSTEM
    struct sigaction saveintr;
#endif
    sigset_t saveblock;
    char *sh;

    if (cmd==NULL) return(1);   /* in a posix-conformant system, the shell
                                 * is always available. Hence, non-zero (shell
                                 * present) is returned. There is substantial
                                 * rationale to this in 1003.2 draft 11 B.3.1.5
                                 */
    sh="/bin/sh";

    /* ignore SIGQUIT, mask SIGCHLD and SIGINT */

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);   /* make sa.sa_mask contain no signals */
    sa.sa_flags=0;


#ifdef STANDARD_SYSTEM
    sigaction(SIGINT,&sa,&savintr);
#endif

    sigaction(SIGQUIT,&sa,&savequit);

    sigaddset(&sa.sa_mask, SIGCHLD);

    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);

    sigprocmask(SIG_BLOCK, &sa.sa_mask, &saveblock);

    /* this is the QNX implementation */
    stat=spawnlp(P_WAIT,sh,find_basename(sh),"-c",cmd,(char*)0);
    err=errno;

    *Signal_Message=0;

    /* the shell puts exit status in lower 7 bits if no signal occurred,
       shift up to the normal location

       NOW, since the shell returns as an exit CODE the status (thus
       farged) of the last command it ran, this will appear in the
       top (not bottom, as it would appear from a shell!) byte back
       from the spawn P_WAIT.
    */

#ifdef DIAG
    fprintf(stderr,"stat = %04x\n",stat);
#endif

    if (stat&0x8000 && !(stat&0x00ff)) {
        stat&=0x7f00;
        stat>>=8;

        sprintf(Signal_Message,"'%s' not run to completion (%s)",
                cmd,strsignal(stat&0xff));
    }

    if ((!*Signal_Message) && WTERMSIG(stat)) {
        /* the shell itself terminated abnormally */
        sprintf(Signal_Message,"%s terminated by signal '%s' (%s)",
                sh,cmd,strsignal(stat));
    }

#ifdef DIAG
    printf("%s\n",Signal_Message);
#endif

    /* this stuff common to both QNX and 1003.2 implementation */
    /* restore SIGINT, SIGQUIT, SIGCHLD */
#ifdef STANDARD_SYSTEM
    sigaction(SIGINT,&savintr, (struct sigaction *)NULL);
#endif

    sigprocmask(SIG_SETMASK,&saveblock,(sigset_t *)NULL);
    sigaction(SIGQUIT,&savequit,(struct sigaction *)NULL);


    if (stat==-1) errno=err;

    /* return the status */
    return(stat);
}



/*-------------------------------------------------- eval_builtin(exprn*) ----*/

bool eval_builtin (exprn *expr)
{
    static struct stat statbufs;               /* scratch stat buffer */
    static char exist_path[UTIL_PATH_MAX+1]; /* buffer for storing path formed
                                                  by a path expansion using {}s */

#ifdef DIAG
	fprintf(stdout,"Filename='%s'\n",Filename);
#endif

    switch(expr->builtin_fn) {  
        case EXACTNAME:
			#ifdef DIAG
				fprintf(stdout,"strcmp(%s,find_basename(%s)=%s)\n",(char*)expr->subexpr, Filename,find_basename(Filename));
			#endif
            return !strcmp((char *)expr->subexpr, find_basename(Filename));

#ifdef FNMATCH_REGEX
        case FNMATCH:
        case IFNMATCH:
#ifdef DIAG
            fprintf(stdout,"regexec(expr, %s, 0, NULL, 0)\n",find_basename(Filename));
#endif
            RC = regexec((regex_t *)expr->subexpr, find_basename(Filename),0,NULL,0);
            return !RC;

        case PFNMATCH:
        case IPFNMATCH:
#endif
        case REGEX:
        case IREGEX:
            RC = regexec((regex_t *)expr->subexpr, Filename,0,NULL,0);
            return !RC;

        case LNAME:
        case ILNAME:
            if (LogicalFlag || S_ISLNK(Statbufm->st_mode)) {
				int term_index;

                if (0>(term_index=readlink(Filename,exist_path,sizeof(exist_path)))) RC=1;
                else {
					exist_path[term_index]=0;
#ifdef FNMATCH_REGEX
					RC = regexec((regex_t *)expr->subexpr, exist_path,0,NULL,0);
#else
                    RC = fnmatch((char*)expr->subexpr,exist_path,0);
#endif
				}
            } else RC=1;    /* inverted when returned */

            return !RC;


#ifndef FNMATCH_REGEX
        case FNMATCH:               return !fnmatch((char*)expr->subexpr,find_basename(Filename),0);
        case IFNMATCH:              return !ifnmatch((char*)expr->subexpr,find_basename(Filename),0);
        case PFNMATCH:              return !fnmatch((char*)expr->subexpr,Filename,0);
        case IPFNMATCH:             return !fnmatch((char*)expr->subexpr,Filename,0);
#endif

#ifdef VERSIONS_SUPPORTED
        case FSMANAGER:             return !strcmp((char*)expr->subexpr,(get_filesystem_version(Statbufm->st_dev,Current_Path))->name);
#endif
#ifdef STATUS_SUPPORTED
		case STATUS:                return (0!=(Statbufm->st_status&*(unsigned char *)expr->subexpr));
#endif
        case TYPE_EQUAL_TO:         return ((Statbufm->st_mode&S_IFMT)==(S_IFMT&*((mode_t*)expr->subexpr))); 
        case HAS_PERMS:             return ((Statbufm->st_mode&*(mode_t*)expr->subexpr)==*(mode_t*)expr->subexpr);
#ifndef RFIND
        case PERMS_EQUAL_TO:        return ((Statbufm->st_mode&S_IPERMS)==(*(mode_t*)expr->subexpr&S_IPERMS) );
#endif

        case MNEWER:
        case ANEWER:
        case CNEWER:
        case FNEWER:
            {
                time_t hightime;

                hightime = *(time_t*)expr->subexpr;
                switch(expr->builtin_fn) {
                    case MNEWER: return (Statbufm->st_mtime  > hightime);
                    case ANEWER: return (Statbufm->st_atime  > hightime);
                    case CNEWER: return (Statbufm->st_ctime  > hightime);
#ifdef CREATION_TIME_SUPPORTED
                    case FNEWER: return (Statbufm->st_ftime  > hightime);
#endif
                };
            };
            break;

        case LAST_ACCESSED_ON:
        case LAST_ACCESSED_BEFORE:
        case LAST_ACCESSED_AFTER:
        case LAST_MODIFIED_ON:
        case LAST_MODIFIED_BEFORE:
        case LAST_MODIFIED_AFTER:
        case LAST_STATCHG_ON:
        case LAST_STATCHG_BEFORE:
        case LAST_STATCHG_AFTER:
#ifdef CREATION_TIME_SUPPORTED
        case CREATED_ON: 
        case CREATED_BEFORE:
        case CREATED_AFTER:
#endif
            {
                time_t lowtime, hightime;

                /* the wording of the spec is confusing. this imlements
                   true if the file is (n) to (n-1) days old e.g. 1 means
                   within the last 24 to 0 hours */
                if (!DaystartFlag) {
                    lowtime  = Start_Time - ((*(time_t*)expr->subexpr)*24L*3600L);
                    hightime = lowtime + 24L*3600L; /* one day after */
                } else {
                    lowtime  = Daystart_Time - ((*(time_t*)expr->subexpr)*24L*3600L);
                    hightime = lowtime + 24L*3600L; /* one day after */
                }

                switch(expr->builtin_fn) {
                    case LAST_ACCESSED_ON:      return (Statbufm->st_atime>lowtime && Statbufm->st_atime<=hightime);
                    case LAST_ACCESSED_BEFORE:  return (Statbufm->st_atime  <= lowtime);
                    case LAST_ACCESSED_AFTER:   return (Statbufm->st_atime  > lowtime);
                    case LAST_MODIFIED_ON:      return (Statbufm->st_mtime>lowtime && Statbufm->st_mtime<=hightime);
                    case LAST_MODIFIED_BEFORE:  return (Statbufm->st_mtime  <= lowtime);
                    case LAST_MODIFIED_AFTER:   return (Statbufm->st_mtime  > lowtime);
                    case LAST_STATCHG_ON:       return (Statbufm->st_ctime>lowtime && Statbufm->st_ctime<=hightime);
                    case LAST_STATCHG_BEFORE:   return (Statbufm->st_ctime  <= lowtime);
                    case LAST_STATCHG_AFTER:    return (Statbufm->st_ctime  > lowtime);
#ifdef CREATION_TIME_SUPPORTED
                    case CREATED_ON:            return (Statbufm->st_ftime>lowtime && Statbufm->st_ftime<=hightime);
                    case CREATED_BEFORE:        return (Statbufm->st_ftime  <= lowtime);
                    case CREATED_AFTER:         return (Statbufm->st_ftime  > lowtime);
#endif
                }
            };
            break;

        case LAST_ACCESSED_ON_MIN:
        case LAST_ACCESSED_BEFORE_MIN:
        case LAST_ACCESSED_AFTER_MIN:
        case LAST_MODIFIED_ON_MIN:
        case LAST_MODIFIED_BEFORE_MIN:
        case LAST_MODIFIED_AFTER_MIN:
        case LAST_STATCHG_ON_MIN:
        case LAST_STATCHG_BEFORE_MIN:
        case LAST_STATCHG_AFTER_MIN:
#ifdef CREATION_TIME_SUPPORTED
        case CREATED_ON_MIN: 
        case CREATED_BEFORE_MIN:
        case CREATED_AFTER_MIN:
#endif
            {
                time_t lowtime, hightime;

                /* the wording of the spec is confusing. this imlements
                   true if the file is (n) to (n-1) minutes old e.g. 1 means
                   within the last 1 to 0 minutes */
                lowtime  = Start_Time - ((*(time_t*)expr->subexpr)*60L);
                hightime = lowtime + 60L;   /* one minute after */

                switch(expr->builtin_fn) {
                    case LAST_ACCESSED_ON_MIN:      return (Statbufm->st_atime>lowtime && Statbufm->st_atime<=hightime);
                    case LAST_ACCESSED_BEFORE_MIN:  return (Statbufm->st_atime  <= lowtime);
                    case LAST_ACCESSED_AFTER_MIN:   return (Statbufm->st_atime  > lowtime);
                    case LAST_MODIFIED_ON_MIN:      return (Statbufm->st_mtime>lowtime && Statbufm->st_mtime<=hightime);
                    case LAST_MODIFIED_BEFORE_MIN:  return (Statbufm->st_mtime  <= lowtime);
                    case LAST_MODIFIED_AFTER_MIN:   return (Statbufm->st_mtime  > lowtime);
                    case LAST_STATCHG_ON_MIN:       return (Statbufm->st_ctime>lowtime && Statbufm->st_ctime<=hightime);
                    case LAST_STATCHG_BEFORE_MIN:   return (Statbufm->st_ctime  <= lowtime);
                    case LAST_STATCHG_AFTER_MIN:    return (Statbufm->st_ctime  > lowtime);
#ifdef CREATION_TIME_SUPPORTED
                    case CREATED_ON_MIN:            return (Statbufm->st_ftime>lowtime && Statbufm->st_ftime<=hightime);
                    case CREATED_BEFORE_MIN:        return (Statbufm->st_ftime  <= lowtime);
                    case CREATED_AFTER_MIN:         return (Statbufm->st_ftime  > lowtime);
#endif
                }
            };
            break;

        case USED_EQUAL_TO: 
        case USED_LESS_THAN:
        case USED_GREATER_THAN:
            {
                time_t lowtime, hightime;

                /* the wording of the spec is confusing. this imlements
                   true if the file is (n) to (n-1) days old e.g. 1 means
                   within the last 24 to 0 hours */
                lowtime  = Statbufm->st_ctime + ((*(time_t*)expr->subexpr)*24L*3600L) - 24L*3600L;
                hightime = lowtime + 24L*3600L; /* one day after */

                switch(expr->builtin_fn) {
                    case USED_EQUAL_TO:     return (Statbufm->st_atime>lowtime && Statbufm->st_atime<=hightime);
                    case USED_LESS_THAN:    return (Statbufm->st_atime  <= lowtime);
                    case USED_GREATER_THAN: return (Statbufm->st_atime  > lowtime);
                }
            };
            break;

        case SIZE_EQUAL_TO:         return ((Statbufm->st_size+511)/512  == *(long*)expr->subexpr);
        case SIZE_LESS_THAN:        return ((Statbufm->st_size+511)/512   < *(long*)expr->subexpr); 
        case SIZE_GREATER_THAN:     return ((Statbufm->st_size+511)/512   > *(long*)expr->subexpr); 
        case BYTES_EQUAL_TO:        return (Statbufm->st_size  == *(long*)expr->subexpr);
        case BYTES_LESS_THAN:       return (Statbufm->st_size   < *(long*)expr->subexpr);
        case BYTES_GREATER_THAN:    return (Statbufm->st_size   > *(long*)expr->subexpr);
#ifndef RFIND
        case LINKS_EQUAL_TO:        return (Statbufm->st_nlink == *(nlink_t*)expr->subexpr);
        case LINKS_LESS_THAN:       return (Statbufm->st_nlink  < *(nlink_t*)expr->subexpr);
        case LINKS_GREATER_THAN:    return (Statbufm->st_nlink  > *(nlink_t*)expr->subexpr);
#endif
        case LEVEL_EQUAL_TO:        return (Levels == *(long*)expr->subexpr);
        case LEVEL_LESS_THAN:       return (Levels < *(long*)expr->subexpr);
        case LEVEL_GREATER_THAN:    return (Levels > *(long*)expr->subexpr);
        case GROUP_EQUAL_TO:        return (Statbufm->st_gid   == *(gid_t*)expr->subexpr);  
        case GROUP_LESS_THAN:       return (Statbufm->st_gid    < *(gid_t*)expr->subexpr);  
        case GROUP_GREATER_THAN:    return (Statbufm->st_gid    > *(gid_t*)expr->subexpr);  
        case USER_EQUAL_TO:         return (Statbufm->st_uid   == *(uid_t*)expr->subexpr);  
        case USER_LESS_THAN:        return (Statbufm->st_uid    < *(uid_t*)expr->subexpr);  
        case USER_GREATER_THAN:     return (Statbufm->st_uid    > *(uid_t*)expr->subexpr);  
#ifndef RFIND
        case INODE_EQUAL_TO:        return (
                                             (Statbufm->st_ino == ((devino_t*)expr->subexpr)->ino) &&
                                             (
                                               (((devino_t*)expr->subexpr)->dev==-1L) ||
                                               (Statbufm->st_dev == ((devino_t*)expr->subexpr)->dev)
                                             )
                                           );
        case INODE_LESS_THAN:       return ((Statbufm->st_ino   < ((devino_t*)expr->subexpr)->ino));
        case INODE_GREATER_THAN:    return ((Statbufm->st_ino   > ((devino_t*)expr->subexpr)->ino));
#ifdef EXTENTS_SUPPORTED
        case EXTENTS_EQUAL_TO:      return (Fsys_Statbufm->st_num_xtnts == *(_nxtnt_t*)expr->subexpr);
        case EXTENTS_LESS_THAN:     return (Fsys_Statbufm->st_num_xtnts < *(_nxtnt_t*)expr->subexpr);
        case EXTENTS_GREATER_THAN:  return (Fsys_Statbufm->st_num_xtnts > *(_nxtnt_t*)expr->subexpr);
#endif
        case NOUSER:                return (getpwuid(Statbufm->st_uid)==NULL); 
        case NOGROUP:               return (getgrgid(Statbufm->st_gid)==NULL);
#endif
        case PRUNE:                 return (PruneFlag = TRUE);
        case ABORT:                 exit(EXIT_FAILURE);
        case XERR:                  return (Error++,FALSE);
#ifdef MOUNT_INFO_SUPPORTED
#ifndef RFIND
        case MOUNTPOINT:        
            /* is the file a mount point? */
            if (qnx_fullpath(IsMountBuf1,Current_Path)==NULL) return FALSE;
            if (fsys_get_mount_dev(IsMountBuf1,IsMountBuf2)==-1) return FALSE;
            if (fsys_get_mount_pt(IsMountBuf2, IsMountBuf3)==-1) return FALSE;
            if (qnx_fullpath(IsMountBuf2,IsMountBuf3)==NULL) return FALSE;
            if (strcmp(IsMountBuf1, IsMountBuf2)) return FALSE;
            else return TRUE;

        case MOUNTDEV:
            /* is the file a mounted device? */
            if (S_ISBLK(Statbufm->st_mode))
                if (fsys_get_mount_pt(Current_Path,IsMountBuf1)!=-1) return TRUE;
            return FALSE;
#endif
#endif

        case EMPTY:
            /* if not a regular file or directory, return FALSE */
            if (Statbufm->st_mode&_S_IFREG) {
                if (Statbufm->st_size==0L) return TRUE;
                else return FALSE;
            } else if (Statbufm->st_mode&_S_IFDIR) {
                DIR *dirptr;
                struct dirent *direntp;

                /* easy-out - a directory that contains other
                   directories will have >2 links */
                if (Statbufm->st_nlink>2) return FALSE;
                /* ugh. We gotta read the directory to determine if
                   it is empty or not. SLOW. Ugh ugh ugh. */

                if (NULL==(dirptr=opendir(Current_Path))) {
                    fprintf(stderr,"find: -empty opendir(%s) failed (%s)\n",Current_Path, strerror(errno));
                    return FALSE;
                }
#ifdef __QNXNTO__
				{
                int dirflags;
                /* attempt to set performance flags on dirptr */
                if( (dirflags = dircntl(dirptr, D_GETFLAG)) != -1){
                	dirflags |= D_FLAG_STAT;
                	dircntl(dirptr, D_SETFLAG, dirflags);
                	/* ignore failure since it won't hurt anything */
                }
				}
#endif
                for (;;) {
                    if (NULL==(direntp=readdir(dirptr))) break;
                    if (strcmp(direntp->d_name,".") &&
                        strcmp(direntp->d_name,"..")) break;
                }               
                closedir(dirptr);               
            
                if (direntp==NULL) return TRUE;
            }
            return FALSE;
            break;

        case ALWAYS_FALSE:          return FALSE;   /* internal */
        case ALWAYS_TRUE_NOP:
        case ALWAYS_TRUE:           return TRUE;

#ifndef RFIND
        case FLS:
#endif
        case LS:
            {
                FILE *fp;

                if (expr->builtin_fn==FLS) {
                    fp=(FILE *)expr->subexpr;
                } else fp=stdout;

#if _FILE_OFFSET_BITS-0 == 64
#define FIND_SIZE_FMT "%9llu"
#else
#define FIND_SIZE_FMT "%9lu"
#endif

                fprintf(fp,"%s %2u %-9s %-9s "FIND_SIZE_FMT" %s %s\n",
                    str_mode(Statbufm->st_mode), 
                    Statbufm->st_nlink,
                    uid(Statbufm->st_uid),
                    gid(Statbufm->st_gid),
                    Statbufm->st_size,
                    age(Statbufm->st_mtime,Statbufm->st_mode),
                    Current_Path);
            }
            return(TRUE);

#ifndef RFIND
        case FPRINTF:
#endif
        case PRINTF:
            {
                FILE *fp;
                char *format;

                if (expr->builtin_fn==FPRINTF) {
                    /* FPRINTF */
                    fp=(FILE *)((char **)expr->subexpr)[0];
                    format=((char **)expr->subexpr)[1];
                } else {
                    /* PRINTF */
                    fp=stdout;
                    format=(char *)expr->subexpr;
                }

                find_fprintf(fp,format,Statbufm,Current_Path,Working_On);
            }
            return(TRUE);

#ifndef RFIND
        case FPRINT:
#endif
        case PRINT:
            {
                FILE *fp;

                if (expr->builtin_fn==FPRINT) {
                    fp=(FILE *)expr->subexpr;
                } else fp=stdout;
            
                fprintf(fp,"%s\n",Current_Path);
            }       
            return (TRUE);

#ifndef RFIND
        case FPRINT0:
#endif
        case PRINT0:
            {
                FILE *fp;

                if (expr->builtin_fn==FPRINT0) {
                    fp=(FILE *)expr->subexpr;
                } else fp=stdout;
            
                fprintf(fp,"%s%c",Current_Path,'\000');
            }       
            return (TRUE);

#ifndef RFIND
        case DASHOK:    
            fflush(stdout); fflush(stderr);
            expandbrace(Command,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT|QUOTE);
            fprintf(stderr,"find: Execute '%s' ? ",Command);

            fflush(stdout); fflush(stderr);
            {
                int c=0;
                while (c!=EOF && c!='Y' && c!='N') c=toupper(getchar());
                if (c!='Y') return(FALSE);
            }
            RC = special_system(Command);
            return !RC;


        case EXEC:
            fflush(stdout); fflush(stderr);
            expandbrace(Command,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT|QUOTE);
            RC = special_system(Command);
            /* the shell puts exit status in lower 7 bits if no signal occurred,
               shift up to the normal location

               NOW, since the shell returns as an exit CODE the status (thus
               farged) of the last command it ran, this will appear in the
               top (not bottom, as it would appear from a shell!) byte back
               from the spawn P_WAIT.
            */
            return !RC;
#endif

        case EXISTS:
            expandbrace(exist_path,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT);
            {
                int f;

                if (-1!=(f=open(exist_path,0))) {
                    close(f);
                    RC = 1;
                } else {
                    RC = 0;
                }
            }
                
            return RC;

        case FMNEWER:
        case FANEWER:
        case FCNEWER:
#ifdef CREATION_TIME_SUPPORTED
        case FFNEWER:
#endif
            expandbrace(exist_path,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT);
            {
                RC=0;
    
                if (-1==(LogicalFlag?stat(exist_path,&statbufs):lstat(exist_path,&statbufs))) {
                    /* if this file is older than the file find is evaluating
                       currently, return true */
                    /* if the comparison file doesn't exist, fnewer is true */
                    if (errno==ENOENT) RC=1;
                    else fprintf(stderr,"find: (-f*newer) can't %sstat '%s' : %s\n",LogicalFlag?"":"l", exist_path, strerror(errno));

                    return RC;
                }

                switch(expr->builtin_fn) {
                    case FMNEWER: if (statbufs.st_mtime<Statbufm->st_mtime) RC=1; break;
                    case FANEWER: if (statbufs.st_atime<Statbufm->st_atime) RC=1; break;
                    case FCNEWER: if (statbufs.st_ctime<Statbufm->st_ctime) RC=1; break;
#ifdef CREATION_TIME_SUPPORTED
                    case FFNEWER: if (statbufs.st_ftime<Statbufm->st_ftime) RC=1; break;
#endif
                }
            }
                
            return RC;

        case ERRMSG:
            fflush(stdout);
        case ECHOSPAM:
            expandbrace(Command,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT);
            fprintf((expr->builtin_fn==ECHOSPAM)?stdout:stderr,
                    "%s\n",Command);
            fflush((expr->builtin_fn==ECHOSPAM)?stdout:stderr);
            return TRUE;


#ifndef RFIND
        /* DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER DANGER */
        /* DO NOT FALL THRU TO THIS CODE!!!! */

        case UNLINK:
            if (Statbufm->st_mode&_S_IFDIR) {
				Directory_Removed=1;	/* set here to cover both dir and symlink->dir cases */
                RC=rmdir(Current_Path);
                if (RC==-1) {
                    if (errno!=ENOTDIR || !LogicalFlag) {
                        fprintf(stderr,"find: -remove! rmdir(\"%s\") failed (%s)\n",Current_Path,strerror(errno));
						Directory_Removed=0;	/* correct the status */
                        return !RC;
                    }
                } else {
					return !RC;
				}
                /* we MAY fall thru if rmdir() failed because it was a symlink
                   pointing to a directory -- try an unlink() */
            }

            if (-1==(RC=unlink(Current_Path))) {
                fprintf(stderr,"find: -rm unlink(\"%s\") failed (%s)\n",Current_Path,strerror(errno));
				Directory_Removed=0;  /* correct the status */
            }
            return !RC;

        case RENAME:
            expandbrace(Command,(char*) expr->subexpr, Current_Path, REPLACE_BRACES|REPLACE_AT);
            if (-1==(RC=rename(Current_Path, Command))) {
                fprintf(stderr,"find: -rename(\"%s\", '%s') failed (%s)\n", Current_Path, Command, strerror(errno));
            }
            return !RC;

        case CHGRP:
        case CHOWN:
            RC=((ownership_t*)expr->subexpr)->flags;
            if (-1==(RC=chown(
                  Current_Path,
                  (RC&OWNERSHIP_USER)  ? ((ownership_t*)expr->subexpr)->userid:Statbufm->st_uid,
                  (RC&OWNERSHIP_GROUP) ? ((ownership_t*)expr->subexpr)->groupid:Statbufm->st_gid
                 )
                )
               )
            {
                fprintf(stderr,"find: chown(\"%s\",--,--) failed (%s)\n",Current_Path,strerror(errno));
            }
            return !RC;

        case CHMOD:
            RC=chmod(Current_Path, 
                     (Statbufm->st_mode|((mode_t*)expr->subexpr)[0])
                                       &((mode_t*)expr->subexpr)[1]
                    );
            return !RC;

#endif
            

#ifndef RFIND
        case SPAWN:
            fflush(stdout); fflush(stderr);
            #ifdef EXPANDBRACE_IN_SPAWN
                /* Current_Path is current pathname */
                /* count the number of items in the argv array */
                for (argc=0;((char**)expr->subexpr[argc];argc++);
                printf("argc = %d\n");
                /* for each item, do an expansion on it into a temporary buffer */
                    expandbrace
                    /* then allocate memory of just the right size, copy the expanded vsn into it */
                    /* put this entry into the temporary argv array */
                /* terminate the temporary array with a null ptr */
                /* spawn the command */
                /* free the memory areas allocated */
            #else
                RC = spawnvp(P_WAIT,*(char**)expr->subexpr,(char**)expr->subexpr);
                return !RC;
            #endif
#endif

    }
    return(FALSE);
}

/*-------------------------------------------------- parse_expression() ------*/

struct expression *parse_expression (int argc,char **argv)
{
    Glob_Argc = argc;
    Glob_Argv = argv;
    if (Glob_Argc)  return(_parse_expression(NOT_ORED_SUBEXPR));
    else {
        Print_On_Match = TRUE;
        return NULL;
    }
}

FILE * FFopen (char *filestring) {
    if (!strcmp(filestring,"/dev/stdout")) return stdout;
    if (!strcmp(filestring,"/dev/stderr")) return stderr;
    return fopen(filestring,"w");
}

void check_argavail (int16_t token) 
{
    cmd *c;
    char *primitive;

    if (Glob_Index<Glob_Argc) return;

    for(c=cmds;c->n && c->tok!=token;c++);

    if (c->n) primitive=c->n;
    else primitive="<Internal Error!>";

    fprintf(stderr,"\nfind: **Error** '-%s' must be supplied with a parameter.\n",primitive);

    exit(EXIT_FAILURE);
}

exprn *_parse_expression (int in_ored_subexpr)
{
    int16_t tok;
    exprn *cur_expr=NULL, *root_expr_of_this_expr = NULL;
    exprn *prev_expr=NULL;
    static int timesin = 0, next_expr_negated = 0;
    
    while ( (Glob_Index<Glob_Argc) && (tok = cmd_match(Glob_Argv[Glob_Index++])) )  {
        /* make the whole thing the user gave be FALSE -or <expression> -
           this is necessary for correct processing of early outs on first
           component of the expression */
        if (timesin++<2) {
            tok = ((timesin==1)?ALWAYS_FALSE:T_OR);
            Glob_Index--; /* counteracts automatic increment in while statement */
        }

        #ifdef DIAG
            printf("Glob_Argv[Glob_Index] = '%s'\n",Glob_Argv[Glob_Index-1]);
            printf("tok = %d\n",tok);
        #endif

        /* allocate a new expression block if we need it */
        switch(tok) {
            case T_RIGHT_PARENTHESES:
                next_expr_negated = FALSE;  /* actually an error i.e. '! )' */
                if (in_ored_subexpr) {
                    /* end ored subexpr first */
                    Glob_Index--; /* counteracts automatic increment in while statement */
                    return(root_expr_of_this_expr);
                }
                break;

            /* this _was_ = TRUE but then ! ! still means not! */
            case T_NOT: next_expr_negated = !next_expr_negated; break;

            case T_AND:
            case NO_TOKEN:
            case HEY_DUDE_THAT_IS_BOGUS_HELLO: break;

            case T_XOR:
            case T_OR:
                if (in_ored_subexpr) {
                    Glob_Index--;  /* counteracts automatic increment in while statement */
                    return(root_expr_of_this_expr);
                }
                /* drop through otherwise */

            case T_LEFT_PARENTHESES:
            default:
                cur_expr = calloc(sizeof(struct expression),1);
                if (root_expr_of_this_expr == NULL) {
                    prev_expr = root_expr_of_this_expr = cur_expr;
                    cur_expr->op = NULL;    /* at root this is NULL */
                } else {
                    /* do not change next line without looking carefully! */
                    prev_expr = prev_expr->next = cur_expr;

                    cur_expr->op = ((tok==T_OR)||(tok==T_XOR))?tok:T_AND;
                }

                /* stuff some fn codes - codes are same as token values */
                cur_expr->builtin_fn = (uint8_t) tok;
                if (next_expr_negated) {
                    next_expr_negated = FALSE;
                    cur_expr->flags = FLAG_NOT;
                } else cur_expr->flags &= ~FLAG_NOT;
                break;
        }
 
        switch(tok) {
            case T_AND: break;

            case T_LEFT_PARENTHESES:
                Parentheses_Level++;
                cur_expr->builtin_fn = 0;
timesin=0; /* reset code which prepends -false -or to subexpression */
                cur_expr->subexpr = _parse_expression(NOT_ORED_SUBEXPR);
                break;
                
            case T_RIGHT_PARENTHESES:
                /* end subexpression - return? */
                Parentheses_Level--;
                return(root_expr_of_this_expr);
                break;

            case T_XOR:
            case T_OR:
                cur_expr->builtin_fn = 0;
                cur_expr->subexpr = _parse_expression(ORED_SUBEXPR);
                break;

            case T_NOT:
                /* set not flag for next 'real' operation */
                break;

            /* 'real' operations */
            case FLS:
            case FPRINT:
            /* FPRINTF is special since it takes TWO args */
            case FPRINT0:
                Print_On_Match = FALSE;
                check_argavail(tok);
                if (NULL==(cur_expr->subexpr = (struct expression*) FFopen(Glob_Argv[Glob_Index++]))) {
                    fprintf(stderr,"find: -f* : cannot open %s (%s)\n",Glob_Argv[Glob_Index-1],strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            
            case PRINTF:
                Print_On_Match = FALSE;
            case FMNEWER:
            case FANEWER:
            case FCNEWER:
#ifdef CREATION_TIME_SUPPORTED
            case FFNEWER:
#endif
            case EXISTS:
#ifndef FNMATCH_REGEX
            case FNMATCH:
            case IFNMATCH:
            case PFNMATCH:
            case IPFNMATCH:
            case LNAME:
            case ILNAME:
#endif
            case FSMANAGER:
                check_argavail(tok);
                cur_expr->subexpr = (struct expression*) Glob_Argv[Glob_Index++];
                break;

            case RENAME:
                check_argavail(tok);
                cur_expr->subexpr = (struct expression*) Glob_Argv[Glob_Index++];
                Print_On_Match=FALSE;
                break;


            case CHMOD:
                Print_On_Match=FALSE;
                check_argavail(tok);
                {
                    mode_t *temp;
                    if (!(cur_expr->subexpr = malloc(2*sizeof(mode_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }
    
                    temp = (mode_t *) cur_expr->subexpr;
                    
                    if ((temp[0] = parse_mode( Glob_Argv[Glob_Index],0))==0xffff)
                    {
                          fprintf(stderr,T_Bad_Perm_Spec,Glob_Argv[Glob_Index]);
                          exit(EXIT_FAILURE);
                    }

                    if ((temp[1] = parse_mode( Glob_Argv[Glob_Index],07777))==0xffff)
                    {
                          fprintf(stderr,T_Bad_Perm_Spec,Glob_Argv[Glob_Index]);
                          exit(EXIT_FAILURE);
                    }

#ifdef DIAG
                    fprintf(stderr,"find chmod: mode[0]=0%o\n",((mode_t*) cur_expr->subexpr)[0]);
                    fprintf(stderr,"find chmod: mode[1]=0%o (~=0%o)\n",
                            ((mode_t*) cur_expr->subexpr)[1],
                            07777&((mode_t*) cur_expr->subexpr)[1]);
#endif
                    Glob_Index++;
                }
                break;
            

#ifdef FNMATCH_REGEX
            case FNMATCH:
            case IFNMATCH:
            case PFNMATCH:
            case IPFNMATCH:
            case LNAME:
            case ILNAME:
#endif
            case REGEX:
            case IREGEX:
                {
                    regex_t *expr_ptr;
                    int i, regex_flags=0;
                    char *pattern;

                    check_argavail(tok);

                    pattern = Glob_Argv[Glob_Index++];

                    if (tok==FNMATCH) {
                        /* CHECK TO SEE IF A STRCMP CAN BE USED INSTEAD OF
                           AN FNMATCH OR REGEX.. TURN THIS INTO AN EXACTNAME
                           PRIMITIVE IF THIS IS THE CASE */
                        if (strpbrk(pattern,"[*?")==NULL) {
                            /* there is no pattern - can use an exact match */
                            cur_expr->builtin_fn = EXACTNAME;
                            cur_expr->subexpr = (struct expression*) pattern;
                            break;
                        }
                    }

                    if (!(expr_ptr=malloc(sizeof(regex_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }


                    switch(tok) {
#ifdef FNMATCH_REGEX
                        case FNMATCH: 
                        case PFNMATCH:
                        case LNAME:
                           pattern=fnmatch2regex(pattern);
                           regex_flags=REG_NOSUB;
                           break;
                        case IPFNMATCH:
                        case IFNMATCH:
                        case ILNAME:
                           pattern=fnmatch2regex(pattern);
                           regex_flags=REG_NOSUB|REG_ICASE;
                           break;
#endif
                        case REGEX:
                           regex_flags=REG_NOSUB|REG_EXTENDED;
                           break;
                        case IREGEX:
                           regex_flags=REG_NOSUB|REG_EXTENDED|REG_ICASE;
                           break;
                    }

                    if ((i=regcomp(expr_ptr, pattern,regex_flags))) {
                        char errmsg[80];
                        regerror(i,expr_ptr,errmsg,sizeof errmsg);
                        fprintf(stderr,"find: %s (reg expression '%s')\n",pattern,errmsg);
                        exit(EXIT_FAILURE);
                    }
            
                    cur_expr->subexpr=(struct expression*)expr_ptr;

                }
                break;

            case FPRINTF:   /* has 2 arguments */
                {
                    char **ptrptr;

                    Print_On_Match=FALSE;

                    check_argavail(tok);

                    if (!(cur_expr->subexpr=malloc(2*sizeof(char *)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }
                    ptrptr=(char **)cur_expr->subexpr;
                
                    if (NULL==(ptrptr[0] = (char *)FFopen(Glob_Argv[Glob_Index++]))) {
                        fprintf(stderr,"find: -fprintf: can't open %s (%s)\n",Glob_Argv[Glob_Index-1],strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    check_argavail(tok);
                    ptrptr[1] = Glob_Argv[Glob_Index++];    /* format string */
                }
                break;

#ifdef STATUS_SUPPORTED
			case STATUS:
				check_argavail(tok);
				{
                    if (!(cur_expr->subexpr = malloc(sizeof(unsigned char)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

					if (!strncmp("bu",Glob_Argv[Glob_Index],2)) {
						*(unsigned char *)cur_expr->subexpr = (unsigned char) _FILE_BUSY;
					} else if (!strncmp("gr",Glob_Argv[Glob_Index],2)) {
						*(unsigned char *)cur_expr->subexpr = (unsigned char) _FILE_GROWN;
					} else {
                        fprintf(stderr,"find: Unrecognised status type (%s)\n",Glob_Argv[Glob_Index]);
                        exit(EXIT_FAILURE);
                    }
                    Glob_Index++;
                }
                break;
#endif


            case TYPE_EQUAL_TO:
                check_argavail(tok);
                {
                    mode_t *temp;

                    if (!(cur_expr->subexpr = malloc(sizeof(mode_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (mode_t *) cur_expr->subexpr;
                
                    switch(BYNUL(Glob_Argv[Glob_Index])) {
                        case 'b':       /* block special file */
                            cur_expr->subexpr = (struct expression*) &Mblk; break;
					    case 'c':       /* character special file */
                            cur_expr->subexpr = (struct expression*) &Mchr; break;
                        case 'd':       /* directory */
                            cur_expr->subexpr = (struct expression*) &Mdir; break;
                        case 'p':       /* FIFO/pipe */
                            cur_expr->subexpr = (struct expression*) &Mfifo; break;
                        case 'f':       /* regular file */
                            cur_expr->subexpr = (struct expression*) &Mreg; break;
                        case 'l':       /* symbolic link */
                            cur_expr->subexpr = (struct expression*) &Mlnk; break;
                        case 'n':       /* named special file */
                            cur_expr->subexpr = (struct expression*) &Mnam; break;
                        case 's':       /* socket */
                            cur_expr->subexpr = (struct expression*) &Msock; break;
                        default:            /* error */
                            fprintf(stderr,"find: Unrecognised file type (%s)\n",Glob_Argv[Glob_Index]);
                            exit(EXIT_FAILURE);
                    }
                    Glob_Index++;
                }
                break;

            case PERMS_EQUAL_TO:
                check_argavail(tok);
                {
                    mode_t *temp;
                    if (!(cur_expr->subexpr = malloc(sizeof(mode_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }
    
                    temp = (mode_t *) cur_expr->subexpr;
                    
                    /* equal to */
                    if ((*temp = parse_mode( (Glob_Argv[Glob_Index]+((Glob_Argv[Glob_Index][0]=='-')?1:0)),0))==0xffff) {
                        fprintf(stderr,T_Bad_Perm_Spec,Glob_Argv[Glob_Index]);
                        exit(EXIT_FAILURE);
                    }

                    if (Glob_Argv[Glob_Index][0]=='-') cur_expr->builtin_fn = HAS_PERMS;
                    #ifdef DIAG
                    fprintf(stderr,"mode = 0%o\n",*(mode_t*) cur_expr->subexpr);
                    #endif
                    Glob_Index++;
                }
                break;

            case INODE_EQUAL_TO:
                check_argavail(tok);
                switch (Glob_Argv[Glob_Index][0]) {
                    case '+':
                    case '-':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': break;
                    default:
                    {
                        devino_t *temp;
                        struct stat sbuf;

                        /* non-numerical -- assume this is a filename */
                        if (!(cur_expr->subexpr = malloc(sizeof(devino_t)))) {
                            perror(T_Malloc_Failed);
                            exit(EXIT_FAILURE);
                        }

                        if (stat(Glob_Argv[Glob_Index],&sbuf)==-1) {
							fprintf(stderr,"%s (%s): %s\n",T_Stat_Failed,Glob_Argv[Glob_Index],sys_errlist[errno]);
                            exit(EXIT_FAILURE);
                        }

                        temp = (devino_t*) cur_expr->subexpr;
                        temp->ino = sbuf.st_ino;
                        temp->dev = sbuf.st_dev; 

                        Glob_Argv[Glob_Index][0]=0; /* used as a flag to break */
                    }
                    break;
                    
                }
                if (!Glob_Argv[Glob_Index][0]) { /* non-numeric */
                    Glob_Index++;
                    break;
                }
                /* fall through for numeric values */

            case MAXDEPTH:
            case MINDEPTH:
            case USED_EQUAL_TO:
#ifdef CREATION_TIME_SUPPORTED
            case CREATED_ON:
            case CREATED_ON_MIN:
#endif
            case LAST_ACCESSED_ON:
            case LAST_MODIFIED_ON:
            case LAST_STATCHG_ON:
            case LAST_ACCESSED_ON_MIN:
            case LAST_MODIFIED_ON_MIN:
            case LAST_STATCHG_ON_MIN:
            case SIZE_EQUAL_TO:
            case LEVEL_EQUAL_TO:
            case LINKS_EQUAL_TO:
#ifdef CREATION_TIME_SUPPORTED
            case EXTENTS_EQUAL_TO:
#endif
                check_argavail(tok);
                {
                    time_t *temp;
                    char *ctemp;

                    /* devino_t is the biggest thing that we might be required
                       to stuff here, so malloc space for all of it. In the
                       inode case the ino is at the beginning of the struct
                       hence will be filled in correctly. At the end the dev
                       field will be stuffed in the inode-specific code in
                       the switch (April 98 - made *2 to be safe) */
                    if (!(cur_expr->subexpr = malloc(2*sizeof(devino_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (time_t *) cur_expr->subexpr;

                    switch(*(ctemp = Glob_Argv[Glob_Index])) {
                        /* note that +- have no effect on MAXDEPTH and MINDEPTH */
                        case '+':   if (tok!=MAXDEPTH && tok!=MINDEPTH) cur_expr->builtin_fn+=2;
                                    ctemp++;
                                    break;
                        case '-':   if (tok!=MAXDEPTH && tok!=MINDEPTH) cur_expr->builtin_fn++;
                                    ctemp++;
                                    break;
                    }
                    Glob_Index++;

                    /* ctemp now points to ascii number, possibly followed
                       by 'c' if this is a size */
                
                    switch(tok) {
                        case MAXDEPTH: Maxdepth = atol(ctemp); cur_expr->builtin_fn=ALWAYS_TRUE; break;
                        case MINDEPTH: Mindepth = atol(ctemp); cur_expr->builtin_fn=ALWAYS_TRUE; break;

                        case LAST_ACCESSED_ON:
                        case LAST_MODIFIED_ON:
                        case LAST_STATCHG_ON:
#ifdef CREATION_TIME_SUPPORTED
                        case CREATED_ON:
                        case CREATED_ON_MIN:
#endif
                        case LAST_ACCESSED_ON_MIN:
                        case LAST_MODIFIED_ON_MIN:
                        case LAST_STATCHG_ON_MIN:
                        case USED_EQUAL_TO:
                            *temp = atol(ctemp);
                            break;

                        case SIZE_EQUAL_TO:
                            /* funky shit with size... default is multiples
                               of 512 bytes. If there is a 'c' after the
                               numbers, it is a multiple of one byte.
                               QNX extension; k=multiple of 1kb */
                            {
                                char *last_char;
                                for (last_char=ctemp;*last_char;last_char++);                               last_char--;
                                if (*last_char == 'c') {
                                    *last_char = '\000';
                                    cur_expr->builtin_fn += (BYTES_EQUAL_TO-SIZE_EQUAL_TO);
	                               *temp = atol(ctemp);
                                } else if (tolower(*last_char) == 'k') {
									*last_char = '\000';
									*temp=2L*atol(ctemp);
								} else {
	                               *temp = atol(ctemp);
								}
                            }
                            break;

                        case LINKS_EQUAL_TO:
                            {
                                nlink_t *temp2 = (nlink_t*) temp;
                                *temp2 = (nlink_t) atol(ctemp);
                            }
                            break;
                        case LEVEL_EQUAL_TO:
                            {
                                long *temp2 = (long*) temp;
                                *temp2 = (long) atol(ctemp);
                            }
                            break;
#ifdef EXTENTS_SUPPORTED
                        case EXTENTS_EQUAL_TO:
                            {
                                _nxtnt_t *temp2 = (_nxtnt_t*) temp;
                                *temp2 = (_nxtnt_t) atol(ctemp);
                                Fsys_Stat_Reqd = TRUE;
                            }
                            break;
#endif
                        case INODE_EQUAL_TO:
                            ((devino_t*)temp)->ino = (ino_t) atol(ctemp);
                            ((devino_t*)temp)->dev = -1L;
                            break;
                    }
                }               
                break;
                
            case GROUP_EQUAL_TO:
                check_argavail(tok);
                {
                    long *temp;
                    struct group *grtemp;
                    char *ctemp;
                
                    if (!(cur_expr->subexpr = malloc(sizeof(long)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (long *) cur_expr->subexpr;

                    ctemp = Glob_Argv[Glob_Index];

                    grtemp = getgrnam(ctemp);

                    if (grtemp==NULL) {
						/* didn't match an existing groupID; check for
                           [+/-] group_number */
	                    switch(*ctemp) {
	                        case '+':   cur_expr->builtin_fn+=2;
	                                    ctemp++;
	                                    break;
	                        case '-':   cur_expr->builtin_fn++;
	                                    ctemp++;
	                                    break;
	                    }
                        if (isallnumeric(ctemp)) {
                            *temp = atol(ctemp);
                        } else {
                            fprintf(stderr,T_Invalid_Group,Glob_Argv[Glob_Index]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        *temp = (long) grtemp->gr_gid;
                    }
                }
                Glob_Index++;
                break;

            case USER_EQUAL_TO:
                check_argavail(tok);
                {
                    long *temp;
                    struct passwd *pwtemp;
                    char *ctemp;
                
                    if (!(cur_expr->subexpr = malloc(sizeof(long)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (long*) cur_expr->subexpr;

					ctemp = Glob_Argv[Glob_Index];

                    pwtemp = getpwnam(ctemp);
                    if (!pwtemp) {
						/* didn't match an existing userID; check for
                           [+/-] user_number */
	                   switch(*ctemp) {
	                       case '+':   cur_expr->builtin_fn+=2;
	                                   ctemp++;
	                                   break;
	                       case '-':   cur_expr->builtin_fn++;
	                                   ctemp++;
	                                   break;
	                   }
                        if (isallnumeric(ctemp)) {
                            *temp = atol(ctemp);
                        } else {
                            fprintf(stderr,T_Invalid_User,Glob_Argv[Glob_Index]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        *temp = (long) pwtemp->pw_uid;
                    }
                }
                Glob_Index++;
                break;

            case CHOWN:
            case CHGRP:
                Print_On_Match=FALSE;
                check_argavail(tok);
                {
                    ownership_t  *temp;
                    char *usertext=NULL;
                    char *grouptext=NULL;
                
                    if (!(cur_expr->subexpr = malloc(sizeof(ownership_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (ownership_t *) cur_expr->subexpr;

                    temp->flags=0;

                    if (tok==CHOWN) {
                        /* is it a straight user or user:group? */
                        if ((grouptext=strchr(Glob_Argv[Glob_Index],':'))) {
                            /* user:group */
                            usertext=Glob_Argv[Glob_Index++];
                            *grouptext=0;
                            grouptext+=1;
                            temp->flags=OWNERSHIP_USER|OWNERSHIP_GROUP;
                        } else {
                            /* user only */
                            usertext=Glob_Argv[Glob_Index++];
                            temp->flags=OWNERSHIP_USER;
                        }
                    }
                    if (tok==CHGRP) {
                        /* can be a group only */
                        grouptext=Glob_Argv[Glob_Index++];
                        usertext=NULL;
                        temp->flags=OWNERSHIP_GROUP;
                    }

                    if (temp->flags&OWNERSHIP_GROUP) {
                        struct group *grtemp;

                        grtemp = getgrnam(grouptext);
                    
                        if (grtemp==NULL) {
                            if (isallnumeric(grouptext)) {
                                temp->groupid= atol(grouptext);
                            } else {
                                fprintf(stderr,T_Invalid_Group,grouptext);
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            temp->groupid = grtemp->gr_gid;
                        }
                    }

                    if (temp->flags&OWNERSHIP_USER) {
                        struct passwd *pwtemp;
                        pwtemp = getpwnam(usertext);
                        if (!pwtemp) {
                            if (isallnumeric(usertext)) {
                                temp->userid = atol(usertext);
                            } else {
                                fprintf(stderr,T_Invalid_User,usertext);
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            temp->userid = pwtemp->pw_uid;
                        }
                    }                   
                }
                break;

            case MNEWER:
            case ANEWER:
            case CNEWER:
            case FNEWER:
                check_argavail(tok);
                {
                    struct stat sbuf;

                    time_t *temp;

                    if (!(cur_expr->subexpr = malloc(sizeof(time_t)))) {
                        perror(T_Malloc_Failed);
                        exit(EXIT_FAILURE);
                    }

                    temp = (time_t *) cur_expr->subexpr;

                    /* cur_expr->builtin_fn = LAST_MODIFIED_AFTER; */

                    if (Glob_Index>=Glob_Argc) {
                        fprintf(stderr,"find: Newer than what file? -*newer must be followed by a filename.\n");
                        exit(EXIT_FAILURE);
                    }

                    /* now fo a stat on Glob_Argv[Glob_Index++], stuff the mtime in temp! */
                    if (stat(Glob_Argv[Glob_Index++],&sbuf)==-1) {
						fprintf(stderr,"%s (%s): %s\n","find: Can't obtain date of '%s' ",Glob_Argv[Glob_Index-1],sys_errlist[errno]);
                        exit(EXIT_FAILURE);
                    }

                    switch(tok) {
                        case MNEWER: *temp = sbuf.st_mtime; break;
                        case ANEWER: *temp = sbuf.st_atime; break;
                        case CNEWER: *temp = sbuf.st_ctime; break;
#ifdef CREATION_TIME_SUPPORTED
                        case FNEWER: *temp = sbuf.st_ftime; break;
#endif
                    }
                    #ifdef DIAG
                    fprintf(stderr,"FIND: %d -*newer after %ld\n",tok,*temp);
                    #endif
                }
                break;


            case XDEV:
                XdevFlag = TRUE;
                cur_expr->builtin_fn = ALWAYS_TRUE;
                break;

            case DAYSTART:
                DaystartFlag = TRUE;
                cur_expr->builtin_fn = ALWAYS_TRUE;
                break;

            case DEPTH:
                DepthFlag = TRUE;
                cur_expr->builtin_fn = ALWAYS_TRUE;
                break;

            case LOGICAL:
                LogicalFlag = TRUE;
                cur_expr->builtin_fn = ALWAYS_TRUE;
                break;
    
            case ALWAYS_TRUE_NOP:
            case PRINT:
            case PRINT0:
            case LS:
            case UNLINK:
                Print_On_Match = FALSE;
            case ALWAYS_TRUE:
            case ALWAYS_FALSE:
            case NOUSER:
            case NOGROUP:
            case PRUNE:
            case ABORT:
            case XERR:
#ifdef MOUNT_INFO_SUPPORTED
            case MOUNTDEV:
            case MOUNTPOINT:
#endif
            case EMPTY:
                break;  /* none of these take an argument */

            case EXEC:
            case DASHOK:
                Trap_SIGINT=TRUE;

            case ECHOSPAM:
            case ERRMSG:

                check_argavail(tok);

                Print_On_Match = FALSE;

                Command[0] = 0;

                if (Glob_Index<Glob_Argc) {

                    if (strcmp(Glob_Argv[Glob_Index],";")) strcpy(Command,Glob_Argv[Glob_Index++]);

                    while (Glob_Index<Glob_Argc && strcmp(Glob_Argv[Glob_Index],";")) {
                        strcat(Command," ");
                        strcat(Command,Glob_Argv[Glob_Index]);
                        Glob_Index++;
                    }
    
                    Glob_Index++;
                }

                if ( (cur_expr->subexpr=malloc(sizeof(Command)+1))==NULL) {
                    fprintf(stderr,"barf - no memory\n");
                    exit(EXIT_FAILURE);
                }
                strcpy((char *) cur_expr->subexpr,Command);
                break;
                
                
#ifndef RFIND
            case SPAWN:
                check_argavail(tok);
                Trap_SIGINT=TRUE;
                /*  golly, but these sure do! subexpr will point midway
                    into an array of ptrs, ending with a NULL pointer.
                    Pointers to '{}' are changed to point to Current_Path
                    (char[])
                */
                #ifdef DIAG
                    fprintf(stderr,"parsing EXEC\n");
                #endif

                Print_On_Match = FALSE;
                cur_expr->subexpr = (struct expression *) (Glob_Argv + Glob_Index);
                #ifdef DIAG
                    fprintf(stderr,"cur_expr->subexpr = %x\n",cur_expr->subexpr);
                    fprintf(stderr,"which -> '%s'\n",*(char**)cur_expr->subexpr);
                #endif
                while (Glob_Index<Glob_Argc && strcmp(Glob_Argv[Glob_Index],";")) {
                    #ifdef DIAG
                        fprintf(stderr,"%s != ';'\n",Glob_Argv[Glob_Index]);
                    #endif
                    #ifndef EXPANDBRACE
                    /* zzx remove this since we will massage the whole thing later w/ expandbrace() */
                    if (!strcmp(Glob_Argv[Glob_Index],"{}")) Glob_Argv[Glob_Index] = Current_Path;
                    #endif
                    Glob_Index++;
                }
                #ifdef DIAG
                    fprintf(stderr,"Glob_Index = %d\n",Glob_Index);
                #endif
                /* create null pointer at end of arg array */
                Glob_Argv[Glob_Index] = NULL;
                Glob_Index++;
                break;
#endif
            default:
                /* BOGUS, DUDE! */
                fprintf(stderr,"find: Unrecognised primary expression (%s)\n",Glob_Argv[Glob_Index-1]);
                exit(EXIT_FAILURE);
        }
    }
    if (Parentheses_Level != 0) {
        fprintf(stderr,"find: Unmatched parentheses in expression.\n");
        exit(EXIT_FAILURE);
    }
    return(root_expr_of_this_expr);
}


/*-------------------------------------------------- cmd_match(char *) -------*/

int16_t cmd_match (char *string)
{
    int i;

    if (*string=='-') {
        string++;
        for (i=0;cmds[i].n!=NULL;i++)
            if (!strcmp(string,cmds[i].n)) return(cmds[i].tok);

    } else {
        switch(BYNUL(string)) {
            case '(': return(T_LEFT_PARENTHESES);
            case ')': return(T_RIGHT_PARENTHESES);
            case '!': return(T_NOT);
        }                               
    }
    return(HEY_DUDE_THAT_IS_BOGUS_HELLO);
}

/*-------------------------------------------------- recurse_dir(char*) ------*/

void recurse_dir (char *path)
{
    DIR *dirp;              
    struct dirent *entry;

//#ifdef __QNXNTO__
	if(Levels != 0) {
		static char netpath[UTIL_PATH_MAX];
		static int netpathlen;
		char buf[UTIL_PATH_MAX], *cp;

		if(netpath[0] == '\0') {
//			netmgr_ndtostr(ND2S_DIR_SHOW|ND2S_NAME_HIDE|ND2S_DOMAIN_HIDE|ND2S_QOS_HIDE, ND_LOCAL_NODE, netpath, sizeof(netpath));
strcpy(netpath, "/tmp/net");
			netpathlen = strlen(netpath);
		}

		// We use strstr to trim the number of fullpaths we may need to do.
		if(strstr(path, "proc/mount") || strstr(path, netpath)) {
//			if(_fullpath(buf, path, sizeof(buf)) != -1) {
if(strcpy(buf, path)) {

				// Check for /proc/mount or the net dir
				if(strcmp(buf, "/proc/mount") == 0 || strcmp(buf, netpath) == 0)
					return;

				//Check for /proc/mount on a remote node through net dir
				if(strncmp(buf, netpath, netpathlen) == 0 && buf[netpathlen] == '/') {
					cp = strchr(buf + netpathlen + 1, '/');
					if(cp && strcmp(cp, "/proc/mount") == 0)
						return;
				}
			}
		}
	}
//#endif

    /* already know that this is a directory, so I won't stat it here */
    Levels++;

    if ((dirp=opendir(path))==NULL) {
        if ((errno!=EACCES)||Verbose) 
			fprintf(stderr,"%s (%s): %s\n","find: Can't open directory.",path,sys_errlist[errno]);
        Levels--;
        Error++;
        return;
    }

#ifdef __QNXNTO__
	{
    int dirflags;
    /* attempt to set performance flags on dirp */
    if( (dirflags = dircntl(dirp, D_GETFLAG)) != -1){
    	dirflags |= D_FLAG_STAT;
    	dircntl(dirp, D_SETFLAG, dirflags);
    	/* ignore failure since it won't hurt anything */
    }
	}
#endif

    while (errno=0,(entry=readdir(dirp)) || errno) {
        if (errno) {
			fprintf(stderr,"%s (%s): %s\n","find: Can't read directory entry.",path,sys_errlist[errno]);
            Error++;
            break;
        }

        if (entry->d_name[0] == '.') {
            if (!entry->d_name[1]) continue;
            if ((entry->d_name[1]=='.') && (!entry->d_name[2])) continue;
        }

		if (LogicalFlag) {
			if (stat_optimize(entry, Statbufp)==-1) Statbuf_Valid=FALSE;
			else Statbuf_Valid=TRUE;
		} else {
			if (lstat_optimize(entry, Statbufp)==-1) Statbuf_Valid=FALSE;
			else Statbuf_Valid=TRUE;
		}
			
        /* add filename to end of path */
        {
            char *endp;
            if (path[strlen(path)-1] != '/') {
                endp=path+strlen(path);
                strcat(path,"/");
                strcat(path,entry->d_name);
                process_file(path);
                *(endp) = (char) 0x00;
                /* *(strrchr(path,'/')) = (char) 0x00; */
            } else {
                endp=path+strlen(path);
                strcat(path,entry->d_name);
                process_file(path);
                *(endp) = (char) 0x00;
                /* *(strrchr(path,'/')+1) = (char) 0x00; */
            }
        }
        errno = 0;  /* zzx */
    }

    if (closedir(dirp)==-1) {
		fprintf(stderr,"%s (%s): %s\n","find: Error closing directory.",path,sys_errlist[errno]);
        Error++;
    }

    Levels--;

    return;
}   

int check_devinos(int Levels, dev_t dev, ino_t ino)
{
    static ino_t Inodes[UTIL_PATH_MAX/2];
    static dev_t Devs[UTIL_PATH_MAX/2];
    int i;

    for (i=0; i<Levels; i++) {
        if (Inodes[i]!=ino) continue;
        if (Devs[i]!=dev) continue;

        /* uh oh. This same directory is somewhere above us in
           the tree we are already traversing. Infinite loop! */
        return FALSE;
    }
    
    Devs[Levels+1]=dev;
    Inodes[Levels+1]=ino;

    return TRUE;
}


/*-------------------------------------------------- process_file (char*) ----*/

void process_file (char *fname) 
{
    if (!Statbuf_Valid) {
        if (LogicalFlag) {
            /* in logical mode use stat instead of lstat */
            if (stat(fname,Statbufp)==-1) {
				fprintf(stderr,"%s (%s): %s\n",T_Stat_Failed,fname,sys_errlist[errno]);
                Error++;
                return;
            }
        } else {
            if (lstat(fname,Statbufp)==-1) {
				fprintf(stderr,"%s (%s): %s\n",T_Stat_Failed,fname,sys_errlist[errno]);
                Error++;
                return;
            }
        }
    }

#ifdef DIAG
    if (XdevFlag) {
        fprintf(stderr,"find xdev: '%s' Device_Id = %x, Statbufp->st_dev = %x\n",
            fname, Device_Id, Statbufp->st_dev);
    }
#endif
    
    if (XdevFlag && (Device_Id==-1)) Device_Id = Statbufp->st_dev;

#ifdef EXTENTS_SUPPORTED
    if (Fsys_Stat_Reqd) {
        if ((!LogicalFlag) && S_ISLNK(Statbufp->st_mode)) {
            /* treat a symbolic link like a file that has NO extents */
            Fsys_Statbufp->st_num_xtnts = 0;
        } else {
            if (fsys_stat(fname,Fsys_Statbufp)==-1) {
				fprintf(stderr,"%s (%s): %s\n",T_Fsys_Stat_Failed,fname,sys_errlist[errno]);
                Error++;
                return;
            }
        }
    }
#endif
    
    /* s'ok, its on my device _or_ don't care (yet) */
    if (S_ISDIR(Statbufp->st_mode) && !PruneFlag) {
        if (!XdevFlag || (Device_Id==Statbufp->st_dev)) {

            Directory_Removed=0;

            if (DepthFlag==FALSE && Levels>=Mindepth) {
#ifdef EXTENTS_SUPPORTED
                if (match(fname,Statbufp,Statbufl,Fsys_Statbufp,Main_Expr) && Print_On_Match)
                    printf("%s\n",fname);
#else
                if (match(fname,Statbufp,Statbufl,Main_Expr) && Print_On_Match)
                    printf("%s\n",fname);
#endif
                /* PruneFlag or Directory_Removed may have been set by this! */
            }

            if (!Directory_Removed && !PruneFlag && ( Maxdepth==-1 || Levels<Maxdepth) ) {
                if (check_devinos(Levels,Statbufp->st_dev,Statbufp->st_ino)) {
                    recurse_dir(fname);
                } else {
                    fprintf(stderr,"find: Cannot recurse into '%s' - filesystem forms an infinite loop\n",fname);
                }
            }
            PruneFlag=FALSE;
    
            if (DepthFlag==TRUE && Levels>=Mindepth) {
                /* Statbuf_Valid is no longer relevant - the call to
                   recurse_dir() will have destroyed the stat buffer.
                   So, must stat this file again!! */
                if (LogicalFlag) {
                    if (stat(fname,Statbufp)==-1) {
                        if (Verbose) 
							fprintf(stderr,"%s (%s): %s\n",T_Stat_Failed,fname,sys_errlist[errno]);
                        Error++;
                        return;
                    }
                } else {
                    if (lstat(fname,Statbufp)==-1) {
                        if (Verbose) 
							fprintf(stderr,"%s (%s): %s\n",T_Stat_Failed,fname,sys_errlist[errno]);
                        Error++;
                        return;
                    }
                }
#ifdef EXTENTS_SUPPORTED
                if (Fsys_Stat_Reqd) {
                    if ((!LogicalFlag) && S_ISLNK(Statbufp->st_mode)) {
                        /* treat a symbolic link like a file that has NO extents */
                        Fsys_Statbufp->st_num_xtnts = 0;
                    } else {
                        if (fsys_stat(fname,Fsys_Statbufp)==-1) {
                            if (Verbose) 
								fprintf(stderr,"%s (%s): %s\n",T_Fsys_Stat_Failed,fname,sys_errlist[errno]);
                            Error++;
                            return;
                        }
                    }
                }

                if (match(fname,Statbufp,Statbufl,Fsys_Statbufp,Main_Expr) && Print_On_Match)
                    printf("%s\n",fname);
#else
                if (match(fname,Statbufp,Statbufl,Main_Expr) && Print_On_Match)
                    printf("%s\n",fname);
#endif
            }
        } /* else we have just skipped a dir because of xdev */
        /* else fprintf(stderr,"find: rejected directory '%s'\n",fname); */
    } else {
      if (Levels>=Mindepth) {
#ifdef EXTENTS_SUPPORTED
        if (match(fname,Statbufp,Statbufl,Fsys_Statbufp,Main_Expr) && (Print_On_Match))
            printf("%s\n",fname);
#else
        if (match(fname,Statbufp,Statbufl,Main_Expr) && (Print_On_Match))
            printf("%s\n",fname);
#endif
      }
    }

    return;
}

/*-------------------------------------------------- diag(exprn*, int) -------*/
#ifdef DIAG

    #define INDENT {for(i=0;i<indent;i++) putchar(' ');}

    void diag (exprn *exp, int indent)
    {
        int i;
        while (exp)
        {
            INDENT printf("EXPR @ 0x%x\n",exp);
            INDENT printf("{\n");
            INDENT printf("  flags      = 0x%x\n",exp->flags);      
            INDENT printf("  op         = %d\n",exp->op);
            INDENT printf("  builtin_fn = %d\n",exp->builtin_fn);
            if (exp->builtin_fn)
            {
                INDENT printf("  subexpr    = '%s'\n",exp->subexpr);
            } else {
                INDENT printf("  subexpr    = 0x%x\n",exp->subexpr);
            }
            if ((exp->subexpr)&&(!exp->builtin_fn)) diag(exp->subexpr,indent+6);
            INDENT printf("  next       = 0x%x\n",exp->next);
            INDENT printf("}\n");
            exp = exp->next;
        }
    }

    #undef INDENT
#endif

/*-------------------------------------------------- main(int, char** --------*/

int main (int argc,char **argv)
{
    int i,num_paths;

    errno = 0;

#ifdef STRICT_PATH
    while (getopt(argc,argv,"")!=-1) Error++;

    if (Error && isatty(fileno(stdin))) {
        fprintf(stderr, "find: Did you specify a path?\n");
        exit(EXIT_FAILURE);
    }
#endif

    for (i=0;(i<argc)&&(*argv[i]!='-')&&(*argv[i]!='(')&&(*argv[i]!='!');i++);
    num_paths = i-1;

    if (num_paths<=0) {
        #ifdef STRICT_PATH
            fprintf(stderr,T_No_Paths_Specified);
            exit(EXIT_FAILURE);
        #endif
    }

    Main_Expr = parse_expression(argc-i,&argv[i]);

    #ifdef DIAG
        diag(Main_Expr,0);
    #endif

#ifndef __QNXNTO__
    /* encourage command cacheing */
    qnx_spawn_options.flags |= _SPAWN_XCACHE;
#endif

    Statbufp  = malloc(sizeof(struct stat ));
    Statbufl  = malloc(sizeof(struct stat ));
#ifdef EXTENTS_SUPPORTED
    Fsys_Statbufp = malloc(sizeof(struct _fsys_stat));
#endif

    if (Statbufp==NULL ||
#ifdef EXTENTS_SUPPORTED
        Fsys_Statbufp==NULL ||
#endif
        Statbufl==NULL)
    {
        perror(T_Malloc_Failed);
        exit(EXIT_FAILURE);
    }

    /* trap sigint if we are supposed to - parse_expression sets it
       if there are any -exec, -ok, or -spawn primaries in the expression */
    if (Trap_SIGINT) signal(SIGINT,sigint_handler);

    Start_Time = time(NULL);

    /* calculate daystart time. Time as of the start of the current day. */
    {
        struct tm *timeptr;

        timeptr=localtime(&Start_Time);

        /* zero out everything so we are left with the start of the day */
        timeptr->tm_sec=0;
        timeptr->tm_min=0;      
        timeptr->tm_hour=0;     
        timeptr->tm_isdst=-1;

        Daystart_Time=mktime(timeptr);
    }

#ifndef STRICT_PATH
    if (num_paths==0) {
        if (Trap_SIGINT) {
            /* if Trap_SIGINT is set, -exec or -ok or -spawn was specified. Dangerous! */
            /* also -spawn reuses the contents of argv so it would _not_ be safe to
               overwrite argv[1] with a pathname of '.' */
            fprintf(stderr,T_No_Paths_Specified);
            exit(EXIT_FAILURE);
        } else {
            /* relatively safe, assume the user _meant_ 'find .' */
            num_paths=1;
            argv[1]=".";
        }
    }
#endif

    for (i=1;i<=num_paths;i++) {
        char workspace[50*1024];

        Working_On = argv[i];

        strcpy(workspace,argv[i]);

        Device_Id = -1; /* -1 means, if XdevFlag is set, to record the device id */
        PruneFlag = FALSE;
        Statbuf_Valid = FALSE;
        process_file(workspace);
    }

    return (Error?EXIT_FAILURE:EXIT_SUCCESS);
}

/*--------------------------------- USAGE MESSAGE */

/*
#ifdef __USAGE
%C  - find files (POSIX)

%C  <path>... [operand_expression]

Operand expressions:

                      -primary_expression
                    (  operand_expression  )
                    !  operand_expression                  (not)
           operand_expression [-a] operand_expression      (and)
           operand_expression  -o  operand_expression      (or)

Primary expressions:

 -abort                   Causes termination of find with non-zero exit status.
 -amin  [+|-]<n>          True when file was last accessed n minutes ago.
 -anewer <file>           True if the file being evaluated was accessed more
                          recently than <file> was.
 -atime [+|-]<n>          True when file access time subtracted from the
                          initialization time is n-1 to n multiples of 24 hours
 -chgrp <groupname>|<groupid>
                          Changes group ownership of the file currently being
                          evaluated to the group specified. Will evaluate to
                          true if the operation succeeds, false if it fails.
 -chmod <mode>            Changes the file permissions to those indicated 
                          by <mode>. The mode may be either an octal number
                          or a symbolic mode (see the chmod utility).
 -chown <username>|<userid>[:<groupname>|<groupid>]
                          Changes ownership of the file currently being
                          evaluated to the user (and optionally, group) 
                          specified. Will evaluate to true if the operation
                          succeeds, false if it fails.
 -cmin  [+|-]<n>          True when file status was last changed n minutes ago.
 -cnewer <file>           True if the file being evaluated had its status
                          changed more recently than that of <file>.
 -ctime [+|-]<n>          True when time of last change of file status infor-
                          mation subtracted from initialization time is n-1 to
                          n multiples of 24 hours.
 -daystart                When set, this flag will cause -atime, -ctime, -ftime
                          and -mtime primitives to calculate times based not
                          on the current time, but on the start of the current
                          day.
 -depth                   Causes files which are directories to be evaluated
                          AFTER they are recursed into, rather than before.
                          Always true. (See Note)
 -echo [text]... ;        Will write to standard output the text supplied.
                          {}s will be interpreted as per the -exec primitive to
                          represent the filename being evaluated.
 -empty                   True if the file being evaluated is a regular file
                          of size 0, or a directory which contains no files.
 -errmsg [text]... ;      Similar to -echo except the output is written to
                          standard error.
 -error                   Sets a flag which will cause find to exit with non-
                          zero status when the find is finished. Always 
                          evaluates to false.
 -exec <cmd> [args]... ;  True when exit status of <cmd> is 0.
 -exists <filestring>     True when the file that <filestring> evaluates to
                          exists. <filestring> may be a simple filename or
                          may contain {}s representing the name of the file
                          currently being evaluated.
 -extents [+|-]<n>        True if the file has n extents.
 -false                   Always false.
 -fanewer <filestring>    True if the file being evaluated was accessed more
                          recently than the file represented by <filestring>.
 -fcnewer <filestring>    True if the file being evaluated had its status 
                          changed more recently than the file represented by
                          <filestring>.
 -fFnewer <filestring>    True if the file being evaluated was created more
                          recently than the file represented by <filestring>.
 -fls <file>              Similar to -ls, but output is written to the file
                          represented by <file>. <file> will be 
                          truncated on startup. Always true.
 -fmin  [+|-]<n>          True when file was created n minutes ago.
 -fmnewer <filestring>    True if the file being evaluated was modified more
                          recently than the file represented by <filestring>.
 -fnewer <filestring>     Synonym for -fmnewer.
 -Fnewer <file>           True if the file being evaluated was created more
                          recently than <file>.
 -follow                  Treat symbolic links as being the type of the file
                          they point to. If the link points to a directory,
                          recurse into that directory. (By default symbolic
                          links are treated as special files of type symbolic
                          link. What they point _to_ is irrelevant to find's
                          default behaviour.) Always true. (See Note)
 -fprint  <file>          Like -print, but output is written to <file>.
 -fprint0 <file>          Like -print0, but output is written to <file>.
                          File is truncated on startup. Always true.
 -fprintf <file> <format> Write data pertaining to the file currently being
                          evaluated to <file>, according to the <format>
                          specified. See Notes (below) for description of
                          valid <format>s.
 -fsmanager <name>        True when the file being evaluated is being
                          managed by a process whose name, reported in
                          response to a version query, matches <name>.
                          e.g. for a standard QNX4 filesystem, this would
                          be 'Fsys'.
 -ftime [+|-]<n>          True when file creation time is between n*24 and
                          (n-1)*24 hours ago.
 -gid   <grpid>|[+|-]<n>  Synonym for -group.
 -group <grpid>|[+|-]<n>  True when the file's group name is <grpid>, or if
                          the string isn't a valid group name and is numeric,
                          true when the file's group ID is <n>.
 -ilname <fpattern>       Like -lname, except the pattern match is case-
                          insensitive.
 -iname <fpattern>        Like -name, except the pattern match is case-
                          insensitive.
 -inum [+|-]<n>|<file>    Synonym for -inode.
 -inode <file>|[+|-]<n>   True if the file serial # matches that of <file>.
                          (Used to find links to <file>)
 -ipath <fpattern>        Like -path, except the pattern match is
                          case-insensitive.
 -iregex <rpattern>       Like -regex, except the pattern match is
                          case-insensitive.
 -level [+|-]<n>          True if the current level of directory nesting of
                          the file is <n>. Files/dirs supplied on the cmd line
                          to find are level 0.
 -links [+|-]<n>          True when the file has <n> links.
 -lname <fpattern>        True if -follow is not specified, and the file being
                          evaluated is a symbolic link whose target is a
                          pathname which matches the pattern <fpattern>.
 -logical                 Synonym for -follow.
 -ls                      Always true. Filename output to stdout in ls -l form.
 -maxdepth <n>            Descend at most <n> levels in the directory heirarchy.
                          Level 0 = files that are named on the command line.
                          The '+' and '-' modifiers have no meaning when used
                          in <n> in this context.
 -mindepth <n>            Do not apply the expression to files unless the files
                          are at least <n> levels down in the directory
                          heirarchy. Level 0 = files that are named on the
                          command line. The default is 0. The '+' and '-'
                          modifiers have no meaning when used in <n> in this
                          context.
 -mmin  [+|-]<n>          True when file data was last modified n minutes ago.
 -mnewer <file>           True if file being evaluated was modified more 
                          recently than <file> was.
 -mount                   Synonym for -xdev.
 -mountdev                True if the file is a block special file for which
                          there is a filesystem mount point. (i.e. device 
                          contains a mounted QNX filesystem)
 -mountpoint              True if the file is a directory which is the mount
                          point for a QNX filesystem.
 -mtime [+|-]<n>          True when file modification time subtracted from the
                          initialization time is n-1 to n multiples of 24 hours.
 -name <fpattern>         True for any file whose basename matches <fpattern>
 -newer <file>            POSIX notation. Synonym for -mnewer.
 -nogroup                 True when the file's group does not exist
 -NOP                     Always true, does nothing. This primitive has the 
                          side effect of disabling the implicit -print that 
                          occurs when the expression as a whole evaluates to
                          true. This primitive can be used to benchmark the 
                          time it takes to do a walk of the filesystem.
                          For example: find / -NOP
 -nouser                  True when the file's owner is not in the passwd file
 -ok <cmd> [args]... ;    True when exit status of <cmd> is 0. False when 
                          user says no to the prompt.
 -path <fpattern>        True for any file whose path (as would be printed
                          by -print), matches <fpattern>.
 -perm [-]<mode>          True when the file perms exactly match those defined
                          by <mode>. When a dash (-) precedes the mode, -perm
                          will evaluate to true when the file perms contain
                          at least those specified by <mode>. The mode is
                          expected to be in standard chmod format.
 -pname <fpattern>        Synonym for -path.
 -print                   Always true. Writes path currently being evaluated
                          followed by a newline to the standard output.
 -print0                  Always true. Writes path currently being evaluated
                          followed by an ascii NUL to the standard output.
 -printf <format>         Write data pertaining to the file currently being
                          evaluated to standard output, according to the format
                          specified. See Where: section (below) for description
                          of valid <format>s.
 -prune                   Always true. Do not descend into the current file
                          if it is a directory. If -depth is used, -prune has
                          no effect. Note that since find processes dirs before
                          descending into them (unless -depth is used), -prune
                          as the first thing in the expression will prevent
                          any directories supplied on the cmd line from being
                          descended into.
 -regex <rpattern>        true when the pathname of the file currently being
                          evaluated matches the extended regular expression
                          specified by <rpattern>. See the 'grep' utility
                          documentation for details on regular expressions.
 -rename <filestring>     Renames the file currently being evaluated to 
                          <filestring>. Will evaluate to true if the
                          rename() succeeds; false if rename() fails.
 -remove!                 Removes the file currently being evaluated. Will
                          evaluate to true if the removal succeeds; false
                          if the removal fails. Removal of a directory
                          will succeed only if it is empty.
 -size [+|-]<n>[c|k]      True when the file is <n> blocks in size (or chars
                          when c is appended, kbytes when k is appended).
 -spawn <cmd> [args]... ; Similar to -exec but command is not run through a
                          shell.
 -status [busy | grown]   True if the file has a busy or pregrown status.
 -true                    Always true.
 -type {b|c|d|p|f|l|n|s}  True when file is Block special, Char special, Dir,
                          Pipe, regular File, symbolic Link, special Named
                          file, or Socket.
 -uid  <userid>|[+|-]<n>  Synonym for -user.
 -used [+|-]<n>           File was last accesed <n> days after its status
                          was last changed.
 -user <userid>|[+|-]<n>  True when the file is owned by <userid>, or if
                          the string passed is not a valid username and is
                          numeric, true if the file is owned by the user ID
                          <n>.
 -xdev                    Causes find to skip directories which are on 
                          different devices. Always true. (See Note)

Where:

<fpattern> is a filename matching pattern to be supplied to the fnmatch()
           C function. Patterns consist of text which is to be matched
           literally plus (optionally) pattern matching special characters:

           ?                 - A '?' will match any single character element
           *                 - A '*' will match any string of zero or more
                               characters
           [<bracket_expr>]  - will match a single collating element as per
                               regular expression bracket expressions, except
                               that the '!' characters must be used instead of
                               '^^' to indicate a non-matching list, and that
                               a backslash ('\') is used as an escape
                               character within the bracket expression. Note
                               that '?', '*' and '[' are not special when used
                               inside a bracket expression.

<format>   is a string supplied as an argument to the -printf or -fprintf
           primaries. The contents of the <format> string determine, in a
           printf()-like fashion, what information will be written. The
           string is comprised of regular characters (which are written
           verbatim) as well as a set of \ escapes and % format specifiers,
           as follows:

              \\          Literal backslash (\) character.
              \a          Alarm bell.
              \b          Backspace character.
              \c          Stop printing from format and flush output.
              \f          Form feed character.
              \n          Newline character.
              \r          Carriage return character.
              \t          Horizontal tab character.
              \v          Vertical tab character.

              %%          Literal percent (%) character.
              %p          the name of the file
              %f          the basename of the file   
              %h          the dirname of the file
              %P          the name of the file with the name of the
                          file specified on the command line under which
                          this file was found removed from the beginning
              %H          the name of the file specified on the command line
                          under which the current file was found
              %g          the file's group name, or numeric ID if no name found
              %G          the file's numeric group ID
              %u          the file's user name, or numeric ID if no name found
              %U          the file's numeric user ID
              %m          the file's permissions in Octal
              %k          the file's size in 1k blocks (rounded up)
              %b          the file's size in 512-byte blocks (rounded up)
              %s          the file's size in bytes
              %d          How deep the file is in the directory tree. The
                          files specified on the command line are level 0.
              %F          name of filesystem manager responsible for the file.
              %l          object of symbolic link (NUL if not a symlink)
              %i          inode number of the file
              %n          link count of the file
              %a          file's last access date and time - equivalent to %Ac
              %A<fchar>   Portion of file's last access time where <fchar>
                          is a single character specifying a strftime()
                          format:
                              a    abbreviated weekday name
                              A    full weekday name
                              b    abbreviated month name
                              B    full month name
                              c    locale's appropriate date and time
                                   representation
                              d    day of the month as a decimal number (01-31)
                              D    date in the format mm/dd/yy 
                              h    abbreviated month name
                              H    hour (24 hr) as a decimal number (00-23)
                              I    hour (12 hr) as a decimal number (01-12)
                              j    day of year as a decimal number (001-366)
                              m    month as a decimal number (01-12)
                              M    minute as a decimal number (00-59)
                              n    newline character
                              p    AM or PM
                              r    12-hr clock time (01-12) using the AM/PM
                                   notation i.e. hh:mm:ss (AM|PM)
                              S    second as a decimal number (00-59)
                              t    tab character
                              T    23-hr clock time (00-23) i.e. hh:mm:ss
                              U    week number of the year as a decimal number
                                   (00-52) where Sunday is the first day of the
                                   week
                              w    weekday as a decimal number (0-6) where 0 is
                                   Sunday
                              W    week number of the year as a decimal number
                                   (00-52) where Monday is the first day of the
                                   week
                              x    locale's appropriate date representation
                              X    locale's appropriate time representation
                              y    year without century as a decimal number
                              Y    year with century as a decimal number
                              Z    timezone name, null if no timezone exists
              %c          file's last status change date and time - equivalent
                          to %Cc
              %C<fchar>   Portion of file's last status change time where
                          <fchar> is a single character specifying a strftime()
                          format (see %A description, above).
              %t          file's last modified date and time - equivalent
                          to %Tc
              %T<fchar>   Portion of file's last modified time where
                          <fchar> is a single character specifying a strftime()
                          format (see %A description, above).


Note:

A {} in any <filestring> or '<cmd> [args]...'  argument will be replaced by the
pathname being evaluated. There are extensions to this syntax for stripping
leading and trailing characters. Such arguments are used by -echo, -errmsg,
-exec, -exists, -fanewer, -fcnewer, -fFnewer, -fmnewer, -fnewer, -ok, -rename
and -spawn. In addition to straight filename insertion, you may also opt
to insert the filename stripped of a number of characters at the end (strip) or
the filename less a number of characters at the beginning (skip). The syntax
for this is 

    {[<strip>][,<skip>]}

So, to rename all files ending in .c to end in .C, one would use:

    find . -type f -name '*.c' -rename '{1}C'


The -daystart, -depth, -follow/-logical, -maxdepth, -mindepth and -mount/-xdev
primitives always evaluate to TRUE for the purposes of evaluating the
expression. These primitives affect find's behaviour globally no matter where
they occur on the command line. Find acts on these when it parses its command
line, not when it evaluates them as part of the expression.

The -chgrp, -chmod, -chown, -rename and -remove! primitives will act on the
file immediately; however, for the purposes of evaluating the remainder of
the expression the ownership, name, permissions and existence of the file
will be taken to be what they were _before_ the expression was evaluated.

#endif
*/


/*
#ifdef __USAGENTO
%C  - find files (POSIX)

%C  <path>... [operand_expression]

Operand expressions:

                      -primary_expression
                    (  operand_expression  )
                    !  operand_expression                  (not)
           operand_expression [-a] operand_expression      (and)
           operand_expression  -o  operand_expression      (or)

Primary expressions:

 -abort                   Causes termination of find with non-zero exit status.
 -amin  [+|-]<n>          True when file was last accessed n minutes ago.
 -anewer <file>           True if the file being evaluated was accessed more
                          recently than <file> was.
 -atime [+|-]<n>          True when file access time subtracted from the
                          initialization time is n-1 to n multiples of 24 hours
 -chgrp <groupname>|<groupid>
                          Changes group ownership of the file currently being
                          evaluated to the group specified. Will evaluate to
                          true if the operation succeeds, false if it fails.
 -chmod <mode>            Changes the file permissions to those indicated 
                          by <mode>. The mode may be either an octal number
                          or a symbolic mode (see the chmod utility).
 -chown <username>|<userid>[:<groupname>|<groupid>]
                          Changes ownership of the file currently being
                          evaluated to the user (and optionally, group) 
                          specified. Will evaluate to true if the operation
                          succeeds, false if it fails.
 -cmin  [+|-]<n>          True when file status was last changed n minutes ago.
 -cnewer <file>           True if the file being evaluated had its status
                          changed more recently than that of <file>.
 -ctime [+|-]<n>          True when time of last change of file status infor-
                          mation subtracted from initialization time is n-1 to
                          n multiples of 24 hours.
 -daystart                When set, this flag will cause -atime, -ctime, -ftime
                          and -mtime primitives to calculate times based not
                          on the current time, but on the start of the current
                          day.
 -depth                   Causes files which are directories to be evaluated
                          AFTER they are recursed into, rather than before.
                          Always true. (See Note)
 -echo [text]... ;        Will write to standard output the text supplied.
                          {}s will be interpreted as per the -exec primitive to
                          represent the filename being evaluated.
 -empty                   True if the file being evaluated is a regular file
                          of size 0, or a directory which contains no files.
 -errmsg [text]... ;      Similar to -echo except the output is written to
                          standard error.
 -error                   Sets a flag which will cause find to exit with non-
                          zero status when the find is finished. Always 
                          evaluates to false.
 -exec <cmd> [args]... ;  True when exit status of <cmd> is 0.
 -exists <filestring>     True when the file that <filestring> evaluates to
                          exists. <filestring> may be a simple filename or
                          may contain {}s representing the name of the file
                          currently being evaluated.
 -false                   Always false.
 -fanewer <filestring>    True if the file being evaluated was accessed more
                          recently than the file represented by <filestring>.
 -fcnewer <filestring>    True if the file being evaluated had its status 
                          changed more recently than the file represented by
                          <filestring>.
 -fls <file>              Similar to -ls, but output is written to the file
                          represented by <file>. <file> will be 
                          truncated on startup. Always true.
 -fmnewer <filestring>    True if the file being evaluated was modified more
                          recently than the file represented by <filestring>.
 -fnewer <filestring>     Synonym for -fmnewer.
 -Fnewer <filestring>     True if the file being evaluated was created more 
                          recently than the file represented by <filestring>.
 -follow                  Treat symbolic links as being the type of the file
                          they point to. If the link points to a directory,
                          recurse into that directory. (By default symbolic
                          links are treated as special files of type symbolic
                          link. What they point _to_ is irrelevant to find's
                          default behaviour.) Always true. (See Note)
 -fprint  <file>          Like -print, but output is written to <file>.
 -fprint0 <file>          Like -print0, but output is written to <file>.
                          File is truncated on startup. Always true.
 -fprintf <file> <format> Write data pertaining to the file currently being
                          evaluated to <file>, according to the <format>
                          specified. See Notes (below) for description of
                          valid <format>s.
 -fsmanager <name>        True when the file being evaluated is being
                          managed by a process whose name, reported in
                          response to a version query, matches <name>.
 -gid   <grpid>|[+|-]<n>  Synonym for -group.
 -group <grpid>|[+|-]<n>  True when the file's group name is <grpid>, or if
                          the string isn't a valid group name and is numeric,
                          true when the file's group ID is <n>.
 -ilname <fpattern>       Like -lname, except the pattern match is case-
                          insensitive.
 -iname <fpattern>        Like -name, except the pattern match is case-
                          insensitive.
 -inum [+|-]<n>|<file>    Synonym for -inode.
 -inode <file>|[+|-]<n>   True if the file serial # matches that of <file>.
                          (Used to find links to <file>)
 -ipath <fpattern>        Like -path, except the pattern match is
                          case-insensitive.
 -iregex <rpattern>       Like -regex, except the pattern match is
                          case-insensitive.
 -level [+|-]<n>          True if the current level of directory nesting of
                          the file is <n>. Files/dirs supplied on the cmd line
                          to find are level 0.
 -links [+|-]<n>          True when the file has <n> links.
 -lname <fpattern>        True if -follow is not specified, and the file being
                          evaluated is a symbolic link whose target is a
                          pathname which matches the pattern <fpattern>.
 -logical                 Synonym for -follow.
 -ls                      Always true. Filename output to stdout in ls -l form.
 -maxdepth <n>            Descend at most <n> levels in the directory heirarchy.
                          Level 0 = files that are named on the command line.
                          The '+' and '-' modifiers have no meaning when used
                          in <n> in this context.
 -mindepth <n>            Do not apply the expression to files unless the files
                          are at least <n> levels down in the directory
                          heirarchy. Level 0 = files that are named on the
                          command line. The default is 0. The '+' and '-'
                          modifiers have no meaning when used in <n> in this
                          context.
 -mmin  [+|-]<n>          True when file data was last modified n minutes ago.
 -mnewer <file>           True if file being evaluated was modified more 
                          recently than <file> was.
 -mount                   Synonym for -xdev.
 -mtime [+|-]<n>          True when file modification time subtracted from the
                          initialization time is n-1 to n multiples of 24 hours.
 -name <fpattern>         True for any file whose basename matches <fpattern>
 -newer <file>            POSIX notation. Synonym for -mnewer.
 -nogroup                 True when the file's group does not exist
 -NOP                     Always true, does nothing. This primitive has the
                          side effect of disabling the implicit -print that
                          occurs when the expression as a whole evaluates to
                          true. This primitive can be used to benchmark the
                          time it takes to do a walk of the filesystem.
                          For example: find / -NOP
 -nouser                  True when the file's owner is not in the passwd file
 -ok <cmd> [args]... ;    True when exit status of <cmd> is 0. False when 
                          user says no to the prompt.
 -path <fpattern>        True for any file whose path (as would be printed
                          by -print), matches <fpattern>.
 -perm [-]<mode>          True when the file perms exactly match those defined
                          by <mode>. When a dash (-) precedes the mode, -perm
                          will evaluate to true when the file perms contain
                          at least those specified by <mode>. The mode is
                          expected to be in standard chmod format.
 -pname <fpattern>        Synonym for -path.
 -print                   Always true. Writes path currently being evaluated
                          followed by a newline to the standard output.
 -print0                  Always true. Writes path currently being evaluated
                          followed by an ascii NUL to the standard output.
 -printf <format>         Write data pertaining to the file currently being
                          evaluated to standard output, according to the format
                          specified. See Where: section (below) for description
                          of valid <format>s.
 -prune                   Always true. Do not descend into the current file
                          if it is a directory. If -depth is used, -prune has
                          no effect. Note that since find processes dirs before
                          descending into them (unless -depth is used), -prune
                          as the first thing in the expression will prevent
                          any directories supplied on the cmd line from being
                          descended into.
 -regex <rpattern>        true when the pathname of the file currently being
                          evaluated matches the extended regular expression
                          specified by <rpattern>. See the 'grep' utility
                          documentation for details on regular expressions.
 -rename <filestring>     Renames the file currently being evaluated to 
                          <filestring>. Will evaluate to true if the
                          rename() succeeds; false if rename() fails.
 -remove!                 Removes the file currently being evaluated. Will
                          evaluate to true if the removal succeeds; false
                          if the removal fails. Removal of a directory
                          will succeed only if it is empty.
 -size [+|-]<n>[c|k]      True when the file is <n> blocks in size (or chars
                          when c is appended, kbytes when k is appended).
 -spawn <cmd> [args]... ; Similar to -exec but command is not run through a
                          shell.
 -true                    Always true.
 -type {b|c|d|p|f|l|n|s}  True when file is Block special, Char special, Dir,
                          Pipe, regular File, symbolic Link, special Named
                          file, or Socket.
 -uid  <userid>|[+|-]<n>  Synonym for -user.
 -used [+|-]<n>           File was last accesed <n> days after its status
                          was last changed.
 -user <userid>|[+|-]<n>  True when the file is owned by <userid>, or if
                          the string passed is not a valid username and is
                          numeric, true if the file is owned by the user ID
                          <n>.
 -xdev                    Causes find to skip directories which are on 
                          different devices. Always true. (See Note)

Where:

<fpattern> is a filename matching pattern to be supplied to the fnmatch()
           C function. Patterns consist of text which is to be matched
           literally plus (optionally) pattern matching special characters:

           ?                 - A '?' will match any single character element
           *                 - A '*' will match any string of zero or more
                               characters
           [<bracket_expr>]  - will match a single collating element as per
                               regular expression bracket expressions, except
                               that the '!' characters must be used instead of
                               '^^' to indicate a non-matching list, and that
                               a backslash ('\') is used as an escape
                               character within the bracket expression. Note
                               that '?', '*' and '[' are not special when used
                               inside a bracket expression.

<format>   is a string supplied as an argument to the -printf or -fprintf
           primaries. The contents of the <format> string determine, in a
           printf()-like fashion, what information will be written. The
           string is comprised of regular characters (which are written
           verbatim) as well as a set of \ escapes and % format specifiers,
           as follows:

              \\          Literal backslash (\) character.
              \a          Alarm bell.
              \b          Backspace character.
              \c          Stop printing from format and flush output.
              \f          Form feed character.
              \n          Newline character.
              \r          Carriage return character.
              \t          Horizontal tab character.
              \v          Vertical tab character.

              %%          Literal percent (%) character.
              %p          the name of the file
              %f          the basename of the file   
              %h          the dirname of the file
              %P          the name of the file with the name of the
                          file specified on the command line under which
                          this file was found removed from the beginning
              %H          the name of the file specified on the command line
                          under which the current file was found
              %g          the file's group name, or numeric ID if no name found
              %G          the file's numeric group ID
              %u          the file's user name, or numeric ID if no name found
              %U          the file's numeric user ID
              %m          the file's permissions in Octal
              %k          the file's size in 1k blocks (rounded up)
              %b          the file's size in 512-byte blocks (rounded up)
              %s          the file's size in bytes
              %d          How deep the file is in the directory tree. The
                          files specified on the command line are level 0.
              %l          object of symbolic link (NUL if not a symlink)
              %i          inode number of the file
              %n          link count of the file
              %a          file's last access date and time - equivalent to %Ac
              %A<fchar>   Portion of file's last access time where <fchar>
                          is a single character specifying a strftime()
                          format:
                              a    abbreviated weekday name
                              A    full weekday name
                              b    abbreviated month name
                              B    full month name
                              c    locale's appropriate date and time
                                   representation
                              d    day of the month as a decimal number (01-31)
                              D    date in the format mm/dd/yy 
                              h    abbreviated month name
                              H    hour (24 hr) as a decimal number (00-23)
                              I    hour (12 hr) as a decimal number (01-12)
                              j    day of year as a decimal number (001-366)
                              m    month as a decimal number (01-12)
                              M    minute as a decimal number (00-59)
                              n    newline character
                              p    AM or PM
                              r    12-hr clock time (01-12) using the AM/PM
                                   notation i.e. hh:mm:ss (AM|PM)
                              S    second as a decimal number (00-59)
                              t    tab character
                              T    23-hr clock time (00-23) i.e. hh:mm:ss
                              U    week number of the year as a decimal number
                                   (00-52) where Sunday is the first day of the
                                   week
                              w    weekday as a decimal number (0-6) where 0 is
                                   Sunday
                              W    week number of the year as a decimal number
                                   (00-52) where Monday is the first day of the
                                   week
                              x    locale's appropriate date representation
                              X    locale's appropriate time representation
                              y    year without century as a decimal number
                              Y    year with century as a decimal number
                              Z    timezone name, null if no timezone exists
              %c          file's last status change date and time - equivalent
                          to %Cc
              %C<fchar>   Portion of file's last status change time where
                          <fchar> is a single character specifying a strftime()
                          format (see %A description, above).
              %t          file's last modified date and time - equivalent
                          to %Tc
              %T<fchar>   Portion of file's last modified time where
                          <fchar> is a single character specifying a strftime()
                          format (see %A description, above).


Note:

A {} in any <filestring> or '<cmd> [args]...'  argument will be replaced by the
pathname being evaluated. There are extensions to this syntax for stripping
leading and trailing characters. Such arguments are used by -echo, -errmsg,
-exec, -exists, -fanewer, -fcnewer, -fFnewer, -fmnewer, -fnewer, -ok, -rename
and -spawn. In addition to straight filename insertion, you may also opt
to insert the filename stripped of a number of characters at the end (strip) or
the filename less a number of characters at the beginning (skip). The syntax
for this is 

    {[<strip>][,<skip>]}

So, to rename all files ending in .c to end in .C, one would use:

    find . -type f -name '*.c' -rename '{1}C'


The -daystart, -depth, -follow/-logical, -maxdepth, -mindepth and -mount/-xdev
primitives always evaluate to TRUE for the purposes of evaluating the
expression. These primitives affect find's behaviour globally no matter where
they occur on the command line. Find acts on these when it parses its command
line, not when it evaluates them as part of the expression.

The -chgrp, -chmod, -chown, -rename and -remove! primitives will act on the
file immediately; however, for the purposes of evaluating the remainder of
the expression the ownership, name, permissions and existence of the file
will be taken to be what they were _before_ the expression was evaluated.
#endif
*/

