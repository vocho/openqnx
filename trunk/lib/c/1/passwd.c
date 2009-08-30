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
 * The prototypes in question were switched to follow
 * POSIX at the same time libc was bumped to version 3.
 * To keep ABI compatibility we continue to return out
 * a value.  The following define can be removed if this
 * is ever deemed to be no longer necessary.
 */
#define LIBC2_COMPAT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef LIBC2_COMPAT
#define setpwent	setpwent_hide
#define endpwent	endpwent_hide
#define setgrent	setgrent_hide
#define endgrent	endgrent_hide
#include <pwd.h>
#include <grp.h>
#undef setpwent
#undef endpwent
#undef setgrent
#undef endgrent
int setpwent( void );
int endpwent( void );
int setgrent( void );
int endgrent( void );
#else
#include <pwd.h>
#include <grp.h>
#endif
#include <paths.h>
#include <shadow.h>

#ifdef _POSIX_THREADS
#include <pthread.h>
#else
#ifndef getc_unlocked
#define getc_unlocked getc
#endif
#endif

union record {
	struct passwd	pwd;
	struct group	grp;
	struct spwd		spwd;
};

union id {
	uid_t			uid;
	gid_t			gid;
	const char		*name;
};

struct request {
	union record	*record;
	char			*buffer;
	size_t			bufsize;
	struct dbase	*dbase;
};

struct dbase {
	const char		*fname;
	int				(*parse)(struct request *request);
	FILE			*fp;
	int				sticky;
#ifdef __QNXNTO__
	pthread_t		thread;
	off_t			offset;
#endif
};

static struct request *dbase_request_init(struct dbase *dbase, int recsize, int bufsize) {
	struct request		*req;

	if((req = malloc(sizeof *req + recsize + bufsize))) {
		req->record = (union record *)((char *)req + sizeof *req);
		req->buffer = (char *)(req->record) + recsize;
		req->bufsize = bufsize;
		req->dbase = dbase;
	}
	return req;
}
		
static int dbase_chkfile(struct dbase *dbase) {
	if(!dbase->fp && !(dbase->fp = fopen(dbase->fname, "r"))) {
		return -1;
	}
	return 0;
}

static int dbase_set(struct dbase *dbase) {
	if(dbase_chkfile(dbase) == -1) {
		return -1;
	}
	dbase->sticky = 1;
	return 0;
}

static int dbase_donefile(struct dbase *dbase) {
	if(!dbase->sticky && dbase->fp) {
		fclose(dbase->fp);
		dbase->fp = 0;
	}
	return 0;
}

static int dbase_end(struct dbase *dbase) {
	dbase->sticky = 0;
	return dbase_donefile(dbase);
}

static int dbase_lookup(struct request *request, union id id, int (*compare)(union record *record, union id id), union record **record) {
	register struct dbase			*dbase = request->dbase;
	int								status, istatus;

	*record = 0;
	istatus = status = EOK;

	if(dbase_chkfile(dbase) == -1) {
		return errno;
	}

	if(compare) {
		rewind(dbase->fp);
	}

	while(fgets(request->buffer, request->bufsize, dbase->fp)) {
		int rc;

		if((rc = dbase->parse(request)) == EOK) {
			if(!compare || compare(request->record, id)) {
				*record = request->record;
				if(!compare) {
					return EOK;
				}
				break;
			}
		} else if (istatus == EOK) {
			/* save 1st internal dbase error code */
			istatus = rc;
		}
	}

	/* if we could not find the record, try to produce an error code */
	if (*record == NULL) {
		if (istatus) {
			status = istatus;
		}

		if (ferror(dbase->fp)) {
			status = errno;
		}
	}

	if(!compare) {
		rewind(dbase->fp);
	}
		
	(void)dbase_donefile(dbase);

	return status;
}

/*-------------------------------------------------------------*/

#ifndef PASSWD
#define PASSWD		"/etc/passwd"
#endif

static int parse_pwd(struct request *request) {
	char					*fields[7];
	register char			*cp;
	int						i;
	struct passwd			*pwd;

	if ((cp = strchr(request->buffer, '\n')) == NULL) {
		int tc;

		/* eat the entire line */
		while ((tc = getc_unlocked(request->dbase->fp)) != '\n' && tc != EOF) {
			/* nothing to do */
		}

		return ERANGE;
	}
	*cp = '\0';

	for(cp = request->buffer, i = 0; *cp && i < sizeof fields / sizeof *fields; i++) {
		fields[i] = cp;
		while(*cp && *cp != ':') {
			cp++;
		}

		if(*cp) {
			*cp++ = '\0';
		}
	}

	if(i == sizeof fields / sizeof *fields - 1) {
		fields[i++] = "";
	}

	if(*cp || i != sizeof fields / sizeof *fields) {
		return EINVAL;
	}

	pwd = &request->record->pwd;
	pwd->pw_name = fields[0];
	pwd->pw_passwd = fields[1];
	pwd->pw_uid = strtol(fields[2], 0, 10);
	pwd->pw_gid = strtol(fields[3], 0, 10);
	pwd->pw_age = "";
	pwd->pw_comment = pwd->pw_gecos = fields[4];
	pwd->pw_dir = fields[5];
	pwd->pw_shell = fields[6];

	return EOK;
}

static int compare_pwuid(union record *record, union id id) {
	return record->pwd.pw_uid == id.uid;
}

static int compare_pwnam(union record *record, union id id) {
	return !strcmp(record->pwd.pw_name, id.name);
}

static struct dbase			dbase_passwd = { PASSWD, parse_pwd };
static struct request		*passwd_request;

#ifdef LIBC2_COMPAT
int setpwent(void) {
	return dbase_set(&dbase_passwd);
}

int endpwent(void) {
	return dbase_end(&dbase_passwd);
}
#else
void setpwent(void) {
	(void)dbase_set(&dbase_passwd);
}

void endpwent(void) {
	(void)dbase_end(&dbase_passwd);
}
#endif

struct passwd *getpwent(void) {
	struct passwd			*pwd;
	union id				id;

	if(!passwd_request) {
		if(!(passwd_request = dbase_request_init(&dbase_passwd, sizeof *pwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.uid = -1;
	dbase_lookup(passwd_request, id, 0, (union record **)&pwd);
	return pwd;
}

struct passwd *getpwuid(uid_t uid) {
	struct passwd			*pwd;
	union id				id;

	if(!passwd_request) {
		if(!(passwd_request = dbase_request_init(&dbase_passwd, sizeof *pwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.uid = uid;
	dbase_lookup(passwd_request, id, compare_pwuid, (union record **)&pwd);
	return pwd;
}

struct passwd *getpwnam(const char *name) {
	struct passwd			*pwd;
	union id				id;

	if(!passwd_request) {
		if(!(passwd_request = dbase_request_init(&dbase_passwd, sizeof *pwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.name = name;
	dbase_lookup(passwd_request, id, compare_pwnam, (union record **)&pwd);
	return pwd;
}


int getpwent_r(struct passwd *pwd, char *buffer, int bufsize, struct passwd **result) {
	struct request			request;
	struct dbase			ldbase_passwd;
	union id				id;
	int						saved, rc;
	
	if (bufsize <= 1) {
		return NULL;
	}
	saved = errno;

	memset(&ldbase_passwd, 0, sizeof ldbase_passwd);
	ldbase_passwd.fname = PASSWD;
	ldbase_passwd.parse = parse_pwd;
	/* Need to make it so that is picks up the current fp */
	if (!dbase_passwd.fp) {
		setpwent();
		dbase_chkfile(&dbase_passwd);
	}
	ldbase_passwd.fp = dbase_passwd.fp;

	request.dbase = &ldbase_passwd;
	request.buffer = buffer;
	request.bufsize = bufsize;
	request.record = (union record *)pwd;

	id.uid = -1;
	rc = dbase_lookup(&request, id, 0, (union record **)result);
	errno = saved;
	return rc;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result) {
	struct request			request;
	struct dbase			ldbase_passwd;
	union id				id;
	int						rc, saved;

	if (bufsize <= 1) {
		return ERANGE;
	}

	id.uid = uid;
	saved = errno;

	memset(&ldbase_passwd, 0, sizeof ldbase_passwd);
	ldbase_passwd.fname = PASSWD;
	ldbase_passwd.parse = parse_pwd;

	request.dbase = &ldbase_passwd;
	request.buffer = buffer;
	request.bufsize = bufsize;
	request.record = (union record *)pwd;

	rc = dbase_lookup(&request, id, compare_pwuid, (union record **)result);
	errno = saved;
	return rc;
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result) {
	struct request			request;
	struct dbase			ldbase_passwd;
	union id				id;
	int						rc, saved;
	
	if (bufsize <= 1) {
		return ERANGE;
	}

	id.name = name;
	saved = errno;

	memset(&ldbase_passwd, 0, sizeof ldbase_passwd);
	ldbase_passwd.fname = PASSWD;
	ldbase_passwd.parse = parse_pwd;

	request.dbase = &ldbase_passwd;
	request.buffer = buffer;
	request.bufsize = bufsize;
	request.record = (union record *)pwd;

	rc = dbase_lookup(&request, id, compare_pwnam, (union record **)result);
	errno = saved;
	return rc;
}

#if 0
int putpwent(const struct passwd *pw, FILE *fp) {
    fprintf(fp, "%s:%s:%d:%d:%s:%s:%s\n",
              pw->pw_name, pw->pw_passwd,
              pw->pw_uid, pw->pw_gid,
              pw->pw_comment, pw->pw_dir, pw->pw_shell);
}
#endif

/*-------------------------------------------------------------*/

#ifndef GROUP_FILE
#define	GROUP_FILE	"/etc/group"
#endif

static int parse_grp(struct request *request) {
	register char			*l;
	char					*p;
	int						i;
	struct group			*grp;
	int						max;
	char					**mem;

	if ((l = strchr(request->buffer, '\n')) == NULL) {
		int tc;

		/* eat the entire line */
		while ((tc = getc_unlocked(request->dbase->fp)) != '\n' && tc != EOF) {
			/* nothing to do */
		}

		return ERANGE;
	}
	*l = '\0';
	i = l - request->buffer;
	

	if(!(p = strchr(l = request->buffer, ':'))) {
		return EINVAL;
	}	

	i += sizeof *mem - (i % (int)(sizeof *mem));
	max = (request->bufsize - i) / sizeof *mem - 1;

	if(max <= 0) {
		return ERANGE;
	}

	grp = &request->record->grp;
	grp->gr_mem = (char **)&request->buffer[i];

	*p++ = '\0';
	grp->gr_name = l;
	l = p;

	if(!(p = strchr(l, ':'))) {
		return EINVAL;
	}

	*p++ = '\0';
	grp->gr_passwd = l;
	l = p;
	grp->gr_gid = strtol(l, &p, 10);

	if(*p++ != ':') {
		*grp->gr_mem = 0;
		return EOK;
	}

	for(i = 0, l = p, mem = grp->gr_mem; ++i < max;) {
		if(*l) {
			*mem++ = l;
		}

		if(!(p = strchr(l, ','))) {
			break;
		}

		*p++ = '\0';

		if(*(l = p) == '\0') {
			break;
		}
	}

	*mem = 0;

	return EOK;
}

static int compare_grgid(union record *record, union id id) {
	return record->grp.gr_gid == id.gid;
}

static int compare_grnam(union record *record, union id id) {
	return !strcmp(record->grp.gr_name, id.name);
}

static struct dbase			dbase_group = { GROUP_FILE, parse_grp };
static struct request		*group_request;

#ifdef LIBC2_COMPAT
int setgrent(void) {
	return dbase_set(&dbase_group);
}

int endgrent(void) {
	return dbase_end(&dbase_group);
}
#else
void setgrent(void) {
	(void)dbase_set(&dbase_group);
}

void endgrent(void) {
	(void)dbase_end(&dbase_group);
}
#endif

struct group *getgrent(void) {
	struct group			*grp;
	union id				id;

	if(!group_request) {
		if(!(group_request = dbase_request_init(&dbase_group, sizeof *grp, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.uid = -1;
	dbase_lookup(group_request, id, 0, (union record **)&grp);
	return grp;
}

struct group *getgrgid(gid_t gid) {
	struct group			*grp;
	union id				id;

	if(!group_request) {
		if(!(group_request = dbase_request_init(&dbase_group, sizeof *grp, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.gid = gid;
	dbase_lookup(group_request, id, compare_grgid, (union record **)&grp);
	return grp;
}

struct group *getgrnam(const char *name) {
	struct group			*grp;
	union id				id;

	if(!group_request) {
		if(!(group_request = dbase_request_init(&dbase_group, sizeof *grp, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.name = name;
	dbase_lookup(group_request, id, compare_grnam, (union record **)&grp);
	return grp;
}

int getgrgid_r(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result) {
	struct request			request;
	struct dbase			ldbase_group;
	union id				id;
	int						rc, saved;

	if (bufsize <= 1) {
		return ERANGE;
	}

	id.gid = gid;
	saved = errno;

	memset(&ldbase_group, 0, sizeof ldbase_group);
	ldbase_group.fname = GROUP_FILE;
	ldbase_group.parse = parse_grp;

	request.dbase = &ldbase_group;
	request.buffer = buffer;
	request.bufsize = bufsize;
	request.record = (union record *)(void *)grp; /* keep Metaware happy */

	rc = dbase_lookup(&request, id, compare_grgid, (union record **)result);
	errno = saved;
	return rc;
}

int getgrnam_r(const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result) {
	struct request			request;
	struct dbase			ldbase_group;
	union id				id;
	int						rc, saved;

	if (bufsize <= 1) {
		return ERANGE;
	}

	id.name = name;
	saved = errno;

	memset(&ldbase_group, 0, sizeof ldbase_group);
	ldbase_group.fname = GROUP_FILE;
	ldbase_group.parse = parse_grp;

	request.dbase = &ldbase_group;
	request.buffer = buffer;
	request.bufsize = bufsize;
	request.record = (union record *)(void *)grp; /* keep Metaware happy */

	rc = dbase_lookup(&request, id, compare_grnam, (union record **)result);
	errno = saved;
	return rc;
}

#if !defined(__QNXNTO__) && defined(DEBUG)
int setgroups(int gidsetsize, const gid_t *grouplist) {
	printf("setgroups(%d,", gidsetsize);
	while(gidsetsize--) printf(" %d", *grouplist++);
	printf(")\n");
	return 0;
}
#undef NGROUPS_MAX
#define NGROUPS_MAX 8
#endif

/*----------------------------------------------------------*/
#ifndef SHADOW
#define SHADOW		"/etc/passwd"
#endif
#define DPRINTF(x)	

static int parse_shadow(struct request *request) {
	char					*fields[SPFIELDS];
	register char			*cp;
	int						i;
	struct spwd				*spwd;

	//Null everything out first
	memset(fields, 0, sizeof(fields));

	if ((cp = strchr(request->buffer, '\n')) == NULL) {
		int tc;

		/* eat the entire line */
		while ((tc = getc_unlocked(request->dbase->fp)) != '\n' && tc != EOF) {
			/* nothing to do */
		}

		return ERANGE;
	}
	*cp = '\0';

	for(cp = request->buffer, i = 0; *cp && i < sizeof fields / sizeof *fields; i++) {
		fields[i] = cp;
		while(*cp && *cp != ':') {
			cp++;
		}

		if(*cp) {
			*cp++ = '\0';
		}
	}
	
	/*
	 Not all shadow passwords are this big, so fill extra space w/ zeros
	 Just make sure that we have at least a user and a password field
	 Ideally:
		if (i != sizeof fields / sizeof *fields) 
	*/
	if (i < 2) {
		return(EINVAL);
	}

	spwd = &request->record->spwd;
	spwd->sp_namp = fields[0];
	spwd->sp_pwdp = fields[1] ? fields[1] : "xx";
	spwd->sp_lstchg = fields[2] ? strtol(fields[2], 0, 10) : 0;
	spwd->sp_max = fields[3] ? strtol(fields[3], 0, 10) : 0;
	spwd->sp_min = fields[4] ? strtol(fields[4], 0, 10) : 0;
	spwd->sp_warn = fields[5] ? strtol(fields[5], 0, 10) : 0;
	spwd->sp_inact = fields[6] ? strtol(fields[6], 0, 10) : 0;
	spwd->sp_expire = fields[7] ? strtol(fields[7], 0, 10) : 0;
	spwd->sp_flag = fields[8] ? strtol(fields[8], 0, 10) : 0;

 DPRINTF(printf("Returning entry\n\tName: %s\n\tPasswd: %s\n\t",
                 spwd->sp_namp, spwd->sp_pwdp));
 DPRINTF(printf("Last Chng: %ld\n\tMax: %ld\n\tMin: %ld\n\tWarn: %ld\n\t",
                 spwd->sp_lstchg, spwd->sp_max, spwd->sp_min, spwd->sp_warn));
 DPRINTF(printf("Inact: %ld\n\tExpire: %ld\n\tFlag: %ld\n",
                 spwd->sp_inact, spwd->sp_expire, spwd->sp_flag));
	return EOK;
}

static int compare_spnam(union record *record, union id id) {
	return !strcmp(record->spwd.sp_namp, id.name);
}

static struct dbase			dbase_shadow = { SHADOW, parse_shadow };
static struct request		*shadow_request;

void setspent(void) {
	(void)dbase_set(&dbase_shadow);
}

void endspent(void) {
	(void)dbase_end(&dbase_shadow);
}

struct spwd *getspent(void) {
	struct spwd			*spwd;
	union id			id;

	if(!shadow_request) {
		if(!(shadow_request = dbase_request_init(&dbase_shadow, sizeof *spwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.uid = -1;
	dbase_lookup(shadow_request, id, 0, (union record **)&spwd);
	return spwd;
}

/*
 result is a pointer to a spwd structure to store result in
 buffer is the area where we are to dump our value
 buflen is the length of buffer
*/
struct spwd *getspent_r(struct spwd *result, char *buffer, int buflen) {
	struct spwd				*lspwd = NULL;
	struct request			request;
	struct dbase			ldbase_shadow;
	union id				id;
	int						saved;
	
	if (buflen <= 1) {
		return NULL;
	}

	saved = errno;

	memset(&ldbase_shadow, 0, sizeof ldbase_shadow);
	ldbase_shadow.fname = SHADOW;
	ldbase_shadow.parse = parse_shadow;
	/* Need to make it so that is picks up the current fp */
	if (!dbase_shadow.fp) {
		setspent();
		(void)dbase_chkfile(&dbase_shadow);
	}
	ldbase_shadow.fp = dbase_shadow.fp;

	request.dbase = &ldbase_shadow;
	request.buffer = buffer;
	request.bufsize = buflen;
	request.record = (union record *)result;

	id.uid = -1;
	dbase_lookup(&request, id, 0, (union record **)&lspwd);
	errno = saved;
	return((lspwd && lspwd->sp_namp) ? result : NULL);
}

struct spwd *getspnam(char *name) {
	struct spwd			*spwd;
	union id			id;

	if(!shadow_request) {
		if(!(shadow_request = dbase_request_init(&dbase_shadow, sizeof *spwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	id.name = name;
	dbase_lookup(shadow_request, id, compare_spnam, (union record **)&spwd);
	return spwd;
}

struct spwd *getspnam_r(const char *name, struct spwd *result, char *buffer, int buflen) {
	struct spwd				*lspwd = NULL;
	struct request			request;
	struct dbase			ldbase_shadow;
	union id				id;
	int						saved;
	
	if (buflen <= 1) {
		return NULL;
	}

	id.name = name;
	saved = errno;

	memset(&ldbase_shadow, 0, sizeof ldbase_shadow);
	ldbase_shadow.fname = SHADOW;
	ldbase_shadow.parse = parse_shadow;

	request.dbase = &ldbase_shadow;
	request.buffer = buffer;
	request.bufsize = buflen;
	request.record = (union record *)result;

	dbase_lookup(&request, id, compare_spnam, (union record **)&lspwd);
	errno = saved;
	return((lspwd && lspwd->sp_namp) ? result : NULL);
}

struct spwd *fgetspent(FILE *f) {
	struct spwd				*lspwd = NULL;
	struct request			request;
	struct dbase			ldbase_shadow;
	union id				id;
	
	if(!shadow_request) {
		if(!(shadow_request = dbase_request_init(&dbase_shadow, sizeof *lspwd, BUFSIZ + 1))) {
			errno = ENOMEM;
			return 0;
		}
	}

	//We use the static structure for everything but the fp
	memset(&ldbase_shadow, 0, sizeof ldbase_shadow);
	ldbase_shadow.fname = dbase_shadow.fname;
	ldbase_shadow.parse = parse_shadow;
	ldbase_shadow.fp = f;

	request.dbase = &ldbase_shadow;
	request.buffer = shadow_request->buffer;
	request.bufsize = shadow_request->bufsize;
	request.record = shadow_request->record;

	id.uid = -1;
	dbase_lookup(&request, id, 0, (union record **)&lspwd);
	return((lspwd && lspwd->sp_namp) ? lspwd : NULL);
}

int putspent(const struct spwd *sh, FILE *fp) {
#if 0
    fprintf(fp,"%s:%s:%ld:%ld:%ld:%ld:%ld:%ld:%ld\n",
        sh->sp_namp,
        sh->sp_pwdp,
        sh->sp_lstchg,
        sh->sp_max,
        sh->sp_min,
        sh->sp_warn,
        sh->sp_inact,
        sh->sp_expire,
        sh->sp_flag);
#else
    fprintf(fp, "%s:%s:%ld:%ld:%ld\n",
        sh->sp_namp,
        sh->sp_pwdp,
        sh->sp_lstchg,
        sh->sp_max,
        sh->sp_min  /* rest are ignored ;-( */
#if 0
        ,
        sh->sp_warn,
        sh->sp_inact,
        sh->sp_expire,
        sh->sp_flag
#endif
#endif
        );
	return(EOK);
}

/*----------------------------------------------------------*/



#ifdef DEBUG

#include <assert.h>

void prt_grent(struct group *p, FILE *f) {
	int	i;

	fprintf(f,"name='%s'\ngid='%u'\n",p->gr_name,p->gr_gid);
	for (i=0; p->gr_mem[i] != NULL; i++) {
		fprintf(f,"\t'%s'\n",p->gr_mem[i]);
	}
}

void prt_pwent(struct passwd *p, FILE *f) {
	fprintf(f,"name='%s'\nuid='%u'\ngid='%u'\n",p->pw_name,p->pw_uid,p->pw_gid);
}

void main(int argc, char *argv[]) {
	int	i;
	struct	group	*g;
	struct passwd *p;

	switch(argc > 1 ? argv[1][0] : -1) {
	case	'0':	/*	walk through file, spitting group entries */
		while ((g=getgrent()) != NULL) {
			prt_grent(g,stdout);
		}
		break;
	case	'1':	/*	walk arg list, finding groups */
		for (i=2; i < argc; i++) {
			if ((g=getgrnam(argv[i])) != NULL) {
				prt_grent(g,stdout);
			} else {
				printf("no group '%s'\n",argv[i]);
			}
		}
		break;
	case	'2':	/* walk arg list, finding gid's */
		for (i=2; i < argc; i++) {
		gid_t	x;
			x = strtol(argv[i],NULL,10);
			if ((g=getgrgid(x)) != NULL) {
				prt_grent(g,stdout);
			} else {
				printf("no group '%u'\n",x);
			}
		}
		break;
	case	'3':	/* setgroups */
		if(argc > 2) initgroups(argv[2], argc > 3 ? strtol(argv[3], 0, 10) : -1);
		break;
	case	'4':	/*	walk through file, spitting group entries */
		while ((p=getpwent()) != NULL) {
			prt_pwent(p,stdout);
		}
		break;
	case	'5':	/*	walk arg list, finding groups */
		for (i=2; i < argc; i++) {
			if ((p=getpwnam(argv[i])) != NULL) {
				prt_pwent(p,stdout);
			} else {
				printf("no user '%s'\n",argv[i]);
			}
		}
		break;
	case	'6':	/* walk arg list, finding gid's */
		for (i=2; i < argc; i++) {
		uid_t	x;
			x = strtol(argv[i],NULL,10);
			if ((p=getpwuid(x)) != NULL) {
				prt_pwent(p,stdout);
			} else {
				printf("no user '%u'\n",x);
			}
		}
		break;
	case	'7':	/* test getgrnam_r */
		if (argc >= 3) {
			struct group grp, *pgrp;
			char buffer[1024 +1];
			size_t bufsize = sizeof buffer;
			int rc;

			if (argc >= 4) {
				bufsize = strtol(argv[3],NULL,10);
			}

			rc = getgrnam_r(argv[2], &grp, buffer, bufsize, &pgrp);
			if (rc == EOK) {
				if (pgrp) {
					prt_grent(&grp, stdout);
				} else {
					printf("no group '%s'\n",argv[2]);
				}
			} else {
				printf("error finding group '%s' %s\n",argv[2], strerror(rc));
				assert(pgrp == NULL);
			}
		}
		break;
	case	'8':	/* test getgrgid_r */
		if (argc >= 3) {
			struct group grp, *pgrp;
			char buffer[1024 +1];
			size_t bufsize = sizeof buffer;
			int rc;
			gid_t x;

			if (argc >= 4) {
				bufsize = strtol(argv[3],NULL,10);
			}

			x = strtol(argv[2],NULL,10);
			rc = getgrgid_r(x, &grp, buffer, bufsize, &pgrp);
			if (rc == EOK) {
				if (pgrp) {
					prt_grent(&grp, stdout);
				} else {
					printf("no group '%u'\n",x);
				}
			} else {
				printf("error finding group '%u' %s\n",x, strerror(rc));
				assert(pgrp == NULL);
			}
		}
		break;
	case	'9':	/*	walk arg list, finding groups */
		if (argc >= 3) {
			struct passwd pwd, *ppwd;
			char buffer[1024 +1];
			size_t bufsize = sizeof buffer;
			int rc;

			if (argc >= 4) {
				bufsize = strtol(argv[3],NULL,10);
			}

			rc = getpwnam_r(argv[2], &pwd, buffer, bufsize, &ppwd);
			if (rc == EOK) {
				if (ppwd) {
					prt_pwent(&pwd, stdout);
				} else {
					printf("no user '%s'\n",argv[2]);
				}
			} else {
				printf("error finding user '%s' %s\n",argv[2], strerror(rc));
				assert(ppwd == NULL);
			}
		}
		break;
	case	'a':	/* walk arg list, finding gid's */
		if (argc >= 3) {
			struct passwd pwd, *ppwd;
			char buffer[1024 +1];
			size_t bufsize = sizeof buffer;
			int rc;
			uid_t x;

			if (argc >= 4) {
				bufsize = strtol(argv[3],NULL,10);
			}

			x = strtol(argv[2],NULL,10);
			rc = getpwuid_r(x, &pwd, buffer, bufsize, &ppwd);
			if (rc == EOK) {
				if (ppwd) {
					prt_pwent(&pwd, stdout);
				} else {
					printf("no user '%u'\n",x);
				}
			} else {
				printf("error finding user '%u' %s\n",x, strerror(rc));
				assert(ppwd == NULL);
			}
		}
		break;
	default:
		fprintf(stderr,"use: %s 0 | 1 group | 2 gid | 3 user [gid] | 4 | 5 user : 6 uid | 7 group | 8 guid | 9 user | a uid \n", argv[0]);
	}
}

#endif

__SRCVERSION("passwd.c $Rev: 212726 $");
