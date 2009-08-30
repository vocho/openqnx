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



 
#include "externs.h"

static int openlog(char *fname, int fsize,  int flags);

//
// This is a separate thread which opens /dev/slog (just like any application)
// and read events and saves them in a file. By using a thread and building it
// into slogger we save having to write another utility to log all calls.
//
void *logger(void *dummy) {
	int			rfd, wfd, events[2048/sizeof(int)];
	int			cnt, n, size, tty = 0;
	int			*evp;

	rfd = open("/dev/slog", O_RDONLY);
	if(rfd == -1) {
		fprintf(stderr, "%s: Unable to open /dev/slog for logging : %s\n", __progname,  strerror(errno));
		return(0);
		}

	for(wfd = -1, size = 0;;) {

		n = read(rfd, events, sizeof(events));
		if(n > 0) {
			if(size <= 0) {
				close(wfd);

				wfd = openlog(LogFname, LogFsize, LogFflags);
				if(wfd != -1) {
					size = LogFsize ? LogFsize : INT_MAX;
					tty = isatty(wfd);
				}
			}

			for(evp = events ; evp < &events[n/sizeof(int)] ; evp += cnt) {
				cnt = _SLOG_GETCOUNT(*evp) + _SLOG_HDRINTS;

				// Filter based upon major and severity.
				if(_SLOG_GETSEVERITY(*evp) > FilterLog)
					continue;

				if(tty) {
					if(_SLOG_GETTEXT(*evp))
						write(wfd, evp+_SLOG_HDRINTS, (cnt-_SLOG_HDRINTS) * sizeof(int));
				}
				else {
					write(wfd, evp, cnt * sizeof(int));
					size -= cnt * sizeof(int);
				}
			}
		}
		else
			sleep(1);	// So we don't spin on a hard read error
	}
}


static int openlog(char *fname, int fsize, int flags) {
	int			fd;
	struct stat	sbuf1, sbuf2;
	char		fname1[256], fname2[256];
	int         osync;

	if(fsize) {
		memset(&sbuf1, 0, sizeof(sbuf1));
		sprintf(fname1, "%s1", fname);
		memset(&sbuf2, 0, sizeof(sbuf2));
		sprintf(fname2, "%s2", fname);

		stat(fname1, &sbuf1);
		stat(fname2, &sbuf2);

		// Pick the newer file (later date) if it has room for more events.
		// Otherwise truncate the older file (earlier date) and use it.

		if(sbuf1.st_mtime >= sbuf2.st_mtime) {
			fname = fname1;
			if(sbuf1.st_size >= fsize) {
				printf("Unlink %s\n", fname2);
				unlink(fname = fname2);
			}
		} else {
			fname = fname2;
			if(sbuf2.st_size >= fsize) {
				printf("Unlink %s\n", fname1);
				unlink(fname = fname1);
			}
		}
	}

	osync = (flags & LOGF_FLAG_OSYNC) ? O_SYNC : 0;
	fd = open(fname, O_WRONLY|O_APPEND|O_CREAT|osync, 0666);

	return(fd);
}

__SRCVERSION("logger.c $Rev: 157840 $");
