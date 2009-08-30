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





#include <devctl.h>
#include <errno.h>
#include <fcntl.h>
#include <ioctl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dcmd_chr.h>
#include <sys/iomgr.h>
#include <unistd.h>

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST __based(__segname("CONST2"))
#else
#define FIXCONST
#endif

#define PTYMASTER			'p'
#define PTYSLAVE			't'
static const char FIXCONST	_pty_template[] = "/dev/%cty%c%c";
static const char FIXCONST	_pty_letters[] = "pqrstuvwxyz", _pty_digits[] = "0123456789abcdef";

int posix_openpt(int oflag)
{
const char	*cp1, *cp2;
int			fd;
char		name[_POSIX_PATH_MAX];

	if (oflag & ~(O_ACCMODE | O_NOCTTY)) {
		errno = EINVAL;
	}
	else {
		for (cp1 = _pty_letters; *cp1 != '\0'; ++cp1) {
			for (cp2 = _pty_digits; *cp2 != '\0'; ++cp2) {
				sprintf(name, _pty_template, PTYMASTER, *cp1, *cp2);
				if ((fd = open(name, oflag)) != -1) {
					return(fd);
				}
			}
		}
		errno = EAGAIN;
	}
	return(-1);
}

int grantpt(int filedes)
{
#if defined(DCMD_CHR_GRANTPT)
int		status;

	if ((status = _devctl(filedes, DCMD_CHR_GRANTPT, NULL, 0, _DEVCTL_FLAG_NOTTY | _DEVCTL_FLAG_NORETVAL)) == -1)
		if (errno == ENOTTY)
			errno = EINVAL;
	return(status);
#else
char	name[_POSIX_PATH_MAX];

	if (fcntl(filedes, F_GETFL) == -1) {
		errno = EBADF;
	}
	else if (ptsname_r(filedes, name, sizeof(name)) == NULL) {
		errno = EINVAL;
	}
	else if (chown(name, getuid(), getgid()) || chmod(name, S_IRUSR | S_IWUSR | S_IWGRP)) {
		errno = EACCES;
	}
	else {
		return(0);
	}
	return(-1);
#endif
}

int unlockpt(int filedes)
{
#if defined(DCMD_CHR_UNLOCKPT)
int		status;

	if ((status = _devctl(filedes, DCMD_CHR_UNLOCKPT, NULL, 0, _DEVCTL_FLAG_NOTTY | _DEVCTL_FLAG_NORETVAL)) == -1)
		if (errno == ENOTTY)
			errno = EINVAL;
	return(status);
#else
int		oflag;
char	name[_POSIX_PATH_MAX];

	if ((oflag = fcntl(filedes, F_GETFL)) == -1 || (oflag & O_ACCMODE) == O_RDONLY) {
		errno = EBADF;
	}
	else if (ptsname_r(filedes, name, sizeof(name)) == NULL) {
		errno = EINVAL;
	}
	else {
		return(0);
	}
	return(-1);
#endif
}

char *ptsname_r(int filedes, char *buffer, size_t buflen)
{
struct _fdinfo	info;
int				len;
char			master, letter, digit;

	if ((len = iofdinfo(filedes, _FDINFO_FLAG_LOCALPATH, &info, buffer, buflen)) == -1 || len > buflen)
		return(NULL);
	if (!S_ISCHR(info.mode) || isatty(filedes))
		return(NULL);
	if (sscanf(buffer, _pty_template, &master, &letter, &digit) != 3 || master != PTYMASTER)
		return(NULL);
	sprintf(buffer, _pty_template, PTYSLAVE, letter, digit);
	return(buffer);
}

char *ptsname(int filedes)
{
static char		name[_POSIX_PATH_MAX];

	return(ptsname_r(filedes, name, sizeof(name)));
}

__SRCVERSION("pty.c $Rev: 153052 $");
