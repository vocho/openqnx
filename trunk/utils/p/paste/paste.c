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



#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define FI_BUFSIZE 4096
struct finfo {
	int	fi_fd;
	int	fi_nbytes;
	int	fi_eof;
	char	*fi_file;
	char	fi_buf[FI_BUFSIZE];
};

int merge_parallel(struct finfo *, char **, int, char *);
int merge_serial(struct finfo *, char **, int, char *);
void doerr(char *, char *, int);
int getline(struct finfo *, char **);
void delim(char *delims, char **delim_curp, int todo);



int
main(int argc, char **argv)
{
	int		sflag, nfiles, opt, ret;
	char		*delims, *delim_cur;
	struct finfo	*finfo;

	sflag = 0;
	delims = "\t";

	while ((opt = getopt(argc, argv, "sd:")) != -1) {
		switch (opt) {
		case 's':
		    sflag = 1;
		    break;

		case 'd':
		    delims = optarg;
		    break;

		default:
		    break;
		}
	}

	delim_cur = delims;
	while (*delim_cur != '\0') {
		if (*delim_cur == '\\') {
			if (*(delim_cur + 1) == '\0') {
				doerr("invalid delimiter detected", NULL, EINVAL);
				return 1;
			}
			delim_cur += 2;
		}
		else {
			delim_cur++;
		}
	}

	if (delim_cur == delims) {
		doerr("no delimiters specified", NULL, EINVAL);
		return 1;
	}
	
	if ((nfiles = argc - optind) == 0)
		return 0;

	if ((finfo = calloc(nfiles, sizeof(*finfo))) == NULL) {
		doerr("calloc", NULL, errno);
		return 1;
	}

	if (sflag == 0)
		ret = merge_parallel(finfo, argv + optind, nfiles, delims);
	else
		ret = merge_serial(finfo, argv + optind, nfiles, delims);

	free(finfo);

	return ret;
}


int
merge_parallel(struct finfo *finfo, char **files, int nfiles, char *delims)
{
	char		*delim_cur, *newlinep;
	int		ret, processed, to_delim, i, nread, to_write;
	char		newline;
	struct finfo	*fip, *fip_lim, *fip_stdin;


	ret = 0;
	newline = '\n';
	fip_stdin = NULL;

	/*
	 * POSIX says if any file can't be opened and no -s,
	 * throw an error with no output so we open them all
	 * up front.
	 */
	fip_lim = finfo;
	for (i = 0; i < nfiles; i++) {
		fip = finfo + i;
		fip->fi_file = *(files + i);
		if (strcmp(fip->fi_file, "-") == 0) {
			fip->fi_fd = STDIN_FILENO;
			/*
			 * The last one found is the one
			 * actually used for stdin.
			 */
			fip_stdin = fip;
		}
		else if ((fip->fi_fd = open(fip->fi_file, O_RDONLY)) == -1) {
			doerr(fip->fi_file, NULL, errno);
			ret = 1;
			goto out;
		}
		fip_lim++; /* Successfully opened one */
	}

	for (;;) {
		to_delim = 0;
		delim_cur = delims;
		processed = 0;

		for (i = 0; i < nfiles; i++) {
			fip = finfo + i;
			if (fip->fi_fd == STDIN_FILENO)
				fip = fip_stdin;

			if (getline(fip, &newlinep) == -1) {
				ret = 1;
				goto out;
			}

			if (newlinep == NULL) {
				to_delim++;
			}
			else {
				nread = newlinep - fip->fi_buf + 1;
				to_write = nread;
				delim(delims, &delim_cur, to_delim);
				to_delim = 0;
				if (i != nfiles - 1) {
					to_write--; /* strip newline */
					to_delim++;
				}
				write(STDOUT_FILENO, fip->fi_buf, to_write);
				processed = 1;
				if (nread == FI_BUFSIZE) {
					fip->fi_nbytes = 0;
				}
				else {
					memmove(fip->fi_buf, newlinep + 1, FI_BUFSIZE - nread);
					fip->fi_nbytes -= nread;
				}

			}
		}
		if (processed == 0)
			break;

		if (to_delim == 0)
			continue;

		if (to_delim > 1)
			delim(delims, &delim_cur, to_delim - 1);

		write(STDOUT_FILENO, &newline, 1);
	}

out:
	for (fip = finfo; fip < fip_lim; fip++) {
		if (fip->fi_fd != STDIN_FILENO)
			close(fip->fi_fd);
	}

	return ret;
}

int
merge_serial(struct finfo *finfo, char **files, int nfiles, char *delims)
{
	char		*delim_cur, *newlinep;
	int		ret, to_delim, i, nread;
	char		newline;
	struct finfo	*fip, *fip_lim, *fip_stdin;


	ret = 0;
	newline = '\n';
	fip_stdin = NULL;


	fip_lim = finfo;
	for (i = 0; i < nfiles; i++) {
		fip = finfo + i;
		fip->fi_file = *(files + i);
		delim_cur = delims;
		to_delim = 0;
		/*
		 * POSIX says when -s, do them in order
		 * until an error is found.
		 */
		if (strcmp(fip->fi_file, "-") == 0) {
			fip->fi_fd = STDIN_FILENO;
			/*
			 * The first one found is the one
			 * actually used for stdin.
			 */
			if (fip_stdin == NULL)
				fip_stdin = fip;
			else
				fip = fip_stdin;
		}
		else if ((fip->fi_fd = open(fip->fi_file, O_RDONLY)) == -1) {
			doerr(fip->fi_file, NULL, errno);
			ret = 1;
			break;
		}
		fip_lim++; /* Successfully opened one */

		for (;;) {
			if (fip->fi_eof == 1) {
				/*
				 * Can happen if stdin specified
				 * more than once with -s.
				 */
				break;
			}
			if (getline(fip, &newlinep) == -1) {
				ret = 1;
				goto out;
			}

			if (newlinep != NULL) {
				/* to_delim == 0 first time through */
				delim(delims, &delim_cur, to_delim);
				/* strip newline */
				nread = newlinep - fip->fi_buf;
				write(STDOUT_FILENO, fip->fi_buf, nread);
				/* count newline */
				nread++;
				if (nread >= FI_BUFSIZE) {
					fip->fi_nbytes = 0;
				}
				else {
					memmove(fip->fi_buf, newlinep + 1, FI_BUFSIZE - nread);
					fip->fi_nbytes -= nread;
				}
				to_delim = 1;
			}
			else {
				write(STDOUT_FILENO, &newline, 1);
				break; /* fip->feof must be set */
			}
		}
	}

out:
	for (fip = finfo; fip < fip_lim; fip++) {
		if (fip->fi_fd != STDIN_FILENO)
			close(fip->fi_fd);
	}

	return ret;
}

int
getline(struct finfo *fip, char **newlinep)
{
	int nread;
	int ret, i;

	ret = 0;

	*newlinep = NULL;

	/* Check for newline left over from last read */
	for (i = 0; i < fip->fi_nbytes; i++) {
		if (fip->fi_buf[i] == '\n') {
			*newlinep = &fip->fi_buf[i];
			return 0;
		}
	}

	if (fip->fi_eof == 1)
		return 0;

	for (;;) {
		nread = read(fip->fi_fd, fip->fi_buf + fip->fi_nbytes,
		    FI_BUFSIZE - fip->fi_nbytes);
		if (nread == -1) {
			doerr(fip->fi_file, NULL, errno);
			ret = -1;
			break;
		}
		if (nread == 0) {
			if (fip->fi_nbytes) {
				doerr(fip->fi_file, "missing newline", 0);
				ret = -1;
				break;
			}
			fip->fi_eof = 1;
			break;
		}
		for (i = 0; i < nread; i++) {
			if (fip->fi_buf[fip->fi_nbytes + i] == '\n') {
				*newlinep = &fip->fi_buf[fip->fi_nbytes + i];
				break;
			}
		}
		fip->fi_nbytes += nread;
		if (*newlinep != NULL)
			break;
		if (fip->fi_nbytes >= FI_BUFSIZE) {
			doerr(fip->fi_file, "input line too long", 0);
			ret = -1;
			break;
		}
	}

	return ret;
}


void
delim(char *delims, char **delim_curp, int todo)
{
	char	*delim_cur;
	char	c, *cp;

	delim_cur = *delim_curp;

	for(; todo > 0; todo--) {
		if (*delim_cur == '\\') {
			cp = &c;
			delim_cur++;
			switch (*delim_cur) {
			case 'n':
				c = '\n';
				break;

			case '0':
				/* No delimiter */
				cp = NULL;
				break;

			case 't':
				c = '\t';
				break;

			case '\\':
				c = '\\';
				break;

			default:
				c = *delim_cur;
				break;
			}

		}
		else {
			cp = delim_cur;
		}
		if (cp != NULL)
			write(STDOUT_FILENO, cp, 1);
		delim_cur++;
		if (*delim_cur == '\0')
			delim_cur = delims;
	}

	*delim_curp = delim_cur;
}

void
doerr(char *msg1, char *msg2, int err)
{
	extern char	*__progname;

	fprintf(stderr, "%s: %s: %s\n", __progname, msg1, msg2 != NULL ?
	    msg2 : strerror(err));
}
