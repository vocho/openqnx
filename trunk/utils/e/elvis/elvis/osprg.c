/* unix/osprg.c */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_osprg[] = "$Id: osprg.c,v 2.13 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef NEED_WAIT_H
# include <sys/wait.h>
#endif


#define TMPDIR	(o_directory ? tochar8(o_directory) : "/tmp")
#define SHELL	(o_shell ? tochar8(o_shell) : "/bin/sh")

static char	*command;	/* the command to run */
static char	tempfname[100];	/* name of temp file */
static int	writefd;	/* fd used for writing to program's stdin */
static int	readfd;		/* fd used for reading program's stdout */
static int	pid;		/* process ID of program */


/* Declares which program we'll run, and what we'll be doing with it.
 * This function should return ElvTrue if successful.  If there is an error,
 * it should issue an error message via msg(), and return ElvFalse.
 *
 * For UNIX, the behavior of this function depends on willwrite.
 * If willwrite, then the command is saved and a temporary file is
 * is created to store the data that will become the program's stdin,
 * and the function succeeds if the temp file was created successfully.
 * Else the program is forked (with stdout/stderr redirected to a pipe
 * if willread) and the function succedes if pipe() and fork()
 * succeed.
 */
ELVBOOL prgopen(cmd, willwrite, willread)
	char	*cmd;		/* command string */
	ELVBOOL	willwrite;	/* if ElvTrue, redirect command's stdin */
	ELVBOOL	willread;	/* if ElvTrue, redirect command's stdout */
{
	int	r0w1[2];	/* two ends of a pipe */

	/* Mark both fd's as being unused */
	writefd = readfd = -1;

	/* Don't die if a write-pipe breaks */
	signal(SIGPIPE, SIG_IGN);

	/* Next step depends on what I/O we expect to do with this program */
	if (willwrite && willread)
	{
		/* save the command */
		command = safedup(cmd);

		/* create a temporary file for feeding the program's stdin */
		sprintf(tempfname, "%s/elvis%d.tmp", TMPDIR, (int)getpid());
		writefd = open(tempfname, O_WRONLY|O_CREAT|O_EXCL, 0600);
		if (writefd < 0)
		{
			msg(MSG_ERROR, "can't make temporary file");
			safefree(command);
			command = NULL;
			return ElvFalse;
		}
	}
	else if (willwrite || willread) /* but not both */
	{
		/* create a pipe */
		if (pipe(r0w1) < 0)
		{
			msg(MSG_ERROR, "can't create pipe");
			return ElvFalse;
		}

		/* fork */
		pid = fork();
		if (pid < 0) /* error */
		{
			msg(MSG_ERROR, "can't fork");
			close(r0w1[0]);
			close(r0w1[1]);
			return ElvFalse;
		}
		else if (pid == 0) /* child */
		{
			if (willwrite)
			{
				/* close the write end of the pipe, and make
				 * the read end become stdin.
				 */
				close(r0w1[1]);
				close(0);
				dup(r0w1[0]);
				close(r0w1[0]);
			}
			else
			{
				/* close the read end of the pipe, and make
				 * the write end become stdout & stderr
				 */
				close(r0w1[0]);
				close(1);
				close(2);
				dup(r0w1[1]);
				dup(r0w1[1]);
				close(r0w1[1]);
			}

			/* run the program */
			execl(SHELL, SHELL, "-c", cmd, NULL);

			/* if we get here, fail! */
			exit(1);
		}
		else /* parent */
		{
			if (willwrite)
			{
				/* close the read end of the pipe, and remember
				 * the fd of the write end.
				 */
				close(r0w1[0]);
				writefd = r0w1[1];
			}
			else
			{
				/* close the write end of the pipe, and
				 * remember the fd of the read end.
				 */
				close(r0w1[1]);
				readfd = r0w1[0];
			}
		}
	}
	else /* no redirection */
	{
		/* fork */
		pid = fork();
		if (pid < 0) /* error */
		{
			msg(MSG_ERROR, "can't fork");
			return ElvFalse;
		}
		else if (pid == 0) /* child */
		{
			execl(SHELL, SHELL, "-c", cmd, NULL);

			/* if we get here, fail */
			exit(1);
		}
		/* else parent, but parent doesn't need to do anything */
	}

	/* if we get here, we must have succeeded */
	return ElvTrue;
}

/* Write the contents of buf to the program's stdin, and return nbytes
 * if successful, or -1 for error.  Note that this text should
 * be subjected to the same kind of transformations as textwrite().
 * In fact, it may use textwrite() internally.
 *
 * For UNIX, this is simply a write() to the temp file or pipe.
 */
int prgwrite(buf, nbytes)
	CHAR	*buf;	/* buffer, contains text to be written */
	int	nbytes;	/* number of characters in buf */
{
	assert(writefd >= 0);
	return write(writefd, buf, (size_t)nbytes);
}

/* Marks the end of writing.  Returns ElvTrue if all is okay, or ElvFalse if
 * error.
 *
 * For UNIX, the temp file is closed, and the program is forked.
 * (Since this function is only called when willwrite, the program
 * wasn't forked when prgopen() was called.)  Returns ElvTrue if the
 * fork was successful, or ElvFalse if it failed.
 */
ELVBOOL prggo()
{
	int	r0w1[2];

	/* If we weren't writing, then there's nothing to be done here */
	if (writefd < 0)
		return ElvTrue;

	/* If we're using a temp file, then close it for writing, and then
	 * fork the program with its stdin redirected to come from file.
	 */
	if (command)
	{
		/* close the temp file for writing */
		close(writefd);
		writefd = -1;

		/* make a pipe to use for reading stdout/stderr */
		if (pipe(r0w1) < 0)
		{
			msg(MSG_ERROR, "can't create pipe");
		}

		/* fork */
		pid = fork();
		if (pid < 0) /* error */
		{
			msg(MSG_ERROR, "can't fork");
			close(r0w1[0]);
			close(r0w1[1]);
			safefree(command);
			command = NULL;
			return ElvFalse;
		}
		else if (pid == 0) /* child */
		{
			/* redirect stdin to come from file */
			close(0);
			open(tempfname, O_RDONLY);

			/* connect the write end of the pipe to stdout/stderr;
			 * close the read end.
			 */
			close(1);
			dup(r0w1[1]);
			close(2);
			dup(r0w1[1]);
			close(r0w1[0]);
			close(r0w1[1]);

			/* exec the program */
			execl(SHELL, SHELL, "-c", command, NULL);

			/* if we get here, fail! */
			exit(1);
		}
		else /* parent */
		{
			/* close the write end of the pipe; the read end
			 * becomes readfd.
			 */
			close(r0w1[1]);
			readfd = r0w1[0];

			/* don't need the command string any more. */
			safefree(command);
			command = NULL;
		}
	}
	else /* writing but not reading */
	{
		/* close the writefd */
		close(writefd);
		writefd = -1;
	}

	return ElvTrue;
}


/* Reads text from the program's stdout, and returns the number of
 * characters read.  At EOF, it returns 0.  Note that this text
 * should be subjected to the same kinds of transformations as
 * textread().
 *
 * For UNIX, this is simply a read() from the pipe.
 */
int prgread(buf, nbytes)
	CHAR	*buf;	/* buffer where text should be placed */
	int	nbytes;	/* maximum number of characters to read */
{
	assert(readfd >= 0);
	return read(readfd, buf, (size_t)nbytes);
}

/* Clean up, and return the program's exit status.  The exit status
 * should be 0 normally.
 *
 * For UNIX, this involves closing the pipe, calling wait() to get the
 * program's exit status, and then deleting the temp file.
 */
int prgclose()
{
	int	status;

	/* close the readfd, if necessary */
	if (readfd >= 0)
	{
		close(readfd);
		readfd = -1;
	}

	/* close the writefd, if necessary */
	if (writefd >= 0)
	{
		close(writefd);
		writefd = -1;
	}

	/* wait for the program to die */
	while (wait(&status) != pid)
	{
	}

	/* delete the temp file, if there was one */
	if (*tempfname)
	{
		unlink(tempfname);
		*tempfname = '\0';
	}

	return status;
}
