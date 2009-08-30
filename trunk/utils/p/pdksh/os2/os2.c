#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSSESMGR
#define INCL_WINPROGRAMLIST
#include <os2.h>
#include "config.h"
#include "sh.h"				/* To get inDOS(). */
#include <sys/ioctl.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>

static int isfullscreen(void)
{
  PTIB ptib;
  PPIB ppib;

  DosGetInfoBlocks(&ptib, &ppib);
  return (ppib -> pib_ultype != SSF_TYPE_WINDOWABLEVIO);
}

static int
quoted_strlen(char *s)
{
    int ret = 0;
    int seen_space = 0;
    while (*s) {
	if (seen_space == 0 && *s == ' ') {
	    ret += 2;
	    seen_space = 1;
	} else if (*s == '\"') {
	    if (seen_space == 0) {
		seen_space = 1;
		ret += 4;
	    } else ret += 2;
	} else ret++;
	s++;
    }
    return ret;
}

static char *
quoted_strcpy(char *targ, char* src)
{
    int seen_space = 0;
    char *s = src, *t = targ;
    
    while (*s) {
	if ((*s == ' ') || (*s == '\"')) {
	    seen_space = 1;
	    break;
	}
	s++;
    }
    if (seen_space) {
	*targ++ = '\"';
    }
    while (*src) {
	if (*src == '\"') {
	    *targ++ = '\\';
	} 
	*targ++ = *src++;
    }
    if (seen_space) {
	*targ++ = '\"';
    }
    *targ = '\0';
    return t;
}

static int 
newsession(int type, int mode, char *cmd, char **args, char **env)
{
  STARTDATA sd;
  STATUSDATA st;
  REQUESTDATA qr;
  ULONG sid, pid, len, cnt, rc;
  PVOID ptr;
  BYTE prio;
  static char queue[18];
  static HQUEUE qid = -1;
  char *ap, *ep, *p;
  char object[256] = {0};

  for ( cnt = 1, len = 0; args[cnt] != NULL; cnt++ )
    len += quoted_strlen(args[cnt]) + 1;
  p = ap = alloca(len + 2);
  *p = 0;
  for ( cnt = 1, len = 0; args[cnt] != NULL; cnt++ )
  {
    if ( cnt > 1 )
      *p++ = ' ';
    quoted_strcpy(p, args[cnt]);
    p += strlen(p);
  }
  for ( cnt = 0, len = 0; env[cnt] != NULL; cnt++ )
    len += strlen(env[cnt]) + 1;
  p = ep = alloca(len + 2);
  *p = 0;
  for ( cnt = 0, len = 0; env[cnt] != NULL; cnt++ )
  {
    strcpy(p, env[cnt]);
    p += strlen(p) + 1;
  }
  *p = 0;

  if ( mode == P_WAIT && qid == -1 )
  {
    sprintf(queue, "\\queues\\ksh%04d", getpid());
    if ( DosCreateQueue(&qid, QUE_FIFO, queue) )
      return -1;
  }

  sd.Length = sizeof(sd);
  sd.Related = (mode == P_WAIT) ? SSF_RELATED_CHILD : SSF_RELATED_INDEPENDENT;
  sd.FgBg = SSF_FGBG_FORE;
  sd.TraceOpt = SSF_TRACEOPT_NONE;
  sd.PgmTitle = NULL;
  sd.PgmName = cmd;
  sd.PgmInputs = (PBYTE) ap;
  sd.TermQ = (mode == P_WAIT) ? (PBYTE) queue : NULL;
  sd.Environment = NULL;
  sd.InheritOpt = SSF_INHERTOPT_PARENT;
  sd.SessionType = type;
  sd.IconFile = NULL;
  sd.PgmHandle = 0;
  sd.PgmControl = 0;
  sd.Reserved = 0;
  sd.ObjectBuffer = object;
  sd.ObjectBuffLen = sizeof(object);

  if ( DosStartSession(&sd, &sid, &pid) )
    return errno = ENOEXEC, -1;

  if ( mode == P_WAIT )
  {
    st.Length = sizeof(st);
    st.SelectInd = SET_SESSION_UNCHANGED;
    st.BondInd = SET_SESSION_BOND;
    DosSetSession(sid, &st);
    if ( DosReadQueue(qid, &qr, &len, &ptr, 0, DCWW_WAIT, &prio, 0) )
      return -1;
    rc = ((PUSHORT)ptr)[1];
    DosFreeMem(ptr);
    exit(rc);
  }
  else
    exit(0);
}

int ksh_execve(char *cmd, char **args, char **env, int flags)
{
  ULONG apptype;
  char path[256], *p;
  int rc;

  strcpy(path, cmd);
  for ( p = path; *p; p++ )
    if ( *p == '/' )
      *p = '\\';

  if (_emx_env & 0x1000) {		/* RSX, do best we can do. */
      int len = strlen(cmd);

      if (len > 4 && stricmp(cmd + len - 4, ".bat") == 0) {
	  /* execve would fail anyway, but most probably segfault. */
	  errno = ENOEXEC;
	  return -1;
      }
      goto do_execve;
  }

  if ( inDOS() ) {
    fprintf(stderr, "ksh_execve requires OS/2 or RSX!\n");
    exit(255);
  }

  if ( DosQueryAppType(path, &apptype) == 0 )
  {
    if (apptype & FAPPTYP_DOS)
      return newsession(isfullscreen() ? SSF_TYPE_VDM :
                                         SSF_TYPE_WINDOWEDVDM, 
			P_WAIT, path, args, env);

    if ((apptype & FAPPTYP_WINDOWSREAL) ||
        (apptype & FAPPTYP_WINDOWSPROT) ||
        (apptype & FAPPTYP_WINDOWSPROT31))
      return newsession(isfullscreen() ? PROG_WINDOW_AUTO :
                                         PROG_SEAMLESSCOMMON,
			P_WAIT, path, args, env);

    if ( (apptype & FAPPTYP_EXETYPE) == FAPPTYP_WINDOWAPI ) {
      printf(""); /* kludge to prevent PM apps from core dumping */
      /* Start new session if interactive and not a part of a pipe. */
      return newsession(SSF_TYPE_PM, 
			( (flags & XINTACT) && (flags & XPIPE)
				 /* _isterm(0) && _isterm(1) && _isterm(2) */
			  ? P_NOWAIT 
			  : P_WAIT),
			path, args, env);
    }

    if ( (apptype & FAPPTYP_EXETYPE) == FAPPTYP_NOTWINDOWCOMPAT ||
         (apptype & FAPPTYP_EXETYPE) == FAPPTYP_NOTSPEC )
      if ( !isfullscreen() )
        return newsession(SSF_TYPE_FULLSCREEN, 
			( (flags & XINTACT) && (flags & XPIPE)
				 /* _isterm(0) && _isterm(1) && _isterm(2) */
			  ? P_NOWAIT 
			  : P_WAIT),
			path, args, env);
  }
  do_execve:
  {
      /* P_QUOTE is too agressive, it quotes `@args_from_file' too,
	 which breaks emxomfld calling LINK386 when EMXSHELL=ksh.
	 Thus we check whether we need to quote, and delegate the hard
	 work to P_QUOTE if needed.  */
      char **pp = args;
      int do_quote = 0;
      for (; !do_quote && *pp; pp++) {
	  for (p = *pp; *p; p++) {
	      if (*p == '*' || *p == '?') {
		  do_quote = 1;
		  break;
	      }
	  }
      }
      
      if ( (rc = spawnve(P_OVERLAY | (do_quote ? P_QUOTE : 0),
			 path, args, env)) != -1 )
	  exit(rc);
  }
  return -1;
}

void UnixName(char *path)
{
  for ( ; *path; path++ )
    if ( *path == '\\' )
      *path = '/';
}

char *ksh_strchr_dirsep(const char *path)
{
  char *p1 = strchr(path, '\\');
  char *p2 = strchr(path, '/');
  if ( !p1 ) return p2;
  if ( !p2 ) return p1;
  return (p1 > p2) ? p2 : p1;
}


char *ksh_strrchr_dirsep(const char *path)
{
  char *p1 = strrchr(path, '\\');
  char *p2 = strrchr(path, '/');
  if ( !p1 ) return p2;
  if ( !p2 ) return p1;
  return (p1 > p2) ? p1 : p2;
}
