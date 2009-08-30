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






#pragma disable_message(107,202)

#include <unix.h>
#include <sys/select.h>
#include <sys/dir.h>
#include <inttypes.h>

#ifndef LOCK_EX
#define LOCK_EX F_WRLCK 
#endif

#ifndef LOCK_NB
#define LOCK_NB 4
#endif

#ifndef LOCK_SH
#define LOCK_SH F_RDLCK
#endif

//int _validuser(FILE *hostf, char *rhost, char *luser, char *ruser, int baselen);
extern int getport(signed char *);
extern long getline(FILE *);
extern int getq(struct queue ***);
extern char *checkremote(void);
extern void fatal(signed char *, ... );
extern void displayq(int );
extern void warn(void);
extern void header(void);
extern void inform(signed char *);
extern int inlist(signed char *,signed char *);
extern void show(signed char *,signed char *,int );
extern void blankfill(int );
extern void dump(signed char *,signed char *,int );
extern void ldump(signed char *,signed char *,int );
extern void prank(int );
//extern void main(int ,signed char **);
extern void doit(void);
extern void startup(void);
extern void chkhost(struct sockaddr_in *);
extern int getprent(char *);
extern void endprent(void);
extern int pgetent(char *,char *);
extern int pnchktc(void);
extern int pnamatch(char *);
extern int pgetnum(char *);
extern int pgetflag(char *);
extern char *pgetstr(char *, char **);
extern void printjob(void);
extern int printit(signed char *);
extern int print(int ,signed char *);
extern int sendit(signed char *);
extern int sendfile(int ,signed char *);
extern int response(void);
extern void banner(signed char *,signed char *);
extern char *scnline(int ,signed char *,int );
extern void scan_out(int ,signed char *,int );
extern int dropit(int );
extern void sendmail(signed char *,int );
extern int dofork(int );
extern void init(void);
extern void openpr(void);
extern void setty(void);
//extern void status(char *, char * );
extern void recvjob(void);
extern int readjob(void);
extern int readfile(signed char *, long );
extern int noresponse(void);
extern int chksize( long );
extern int read_number(signed char *);
extern void rcleanup(void);
extern void frecverr(signed char *, ...);
extern void rmjob(void);
extern int lockchk(signed char *);
extern void process(signed char *);
extern int chk(signed char *);
extern int isowner(signed char *,signed char *);
extern void rmremote(void);
extern int iscf(struct dirent *);
extern int startdaemon(signed char *);

#ifndef __QNXNTO__
extern int rresvport(void *);
#endif

#ifdef __QNXNTO__
int flock(int , int ) ;
int getnid(void);
#endif

extern int daemon(int, int);
//extern int _validuser(void *hostf, char *rhost, char *luser, char *ruser, int baselen);
extern int __ivaliduser(FILE *hostf, uint32_t raddr, const char *luser, const char *ruser);
extern int disk_space(int, void *, void *);
