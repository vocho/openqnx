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
	P1003.2/9,4.72 xargs - Construct argument list(s) and execute utility.
	
	Very straightforward.  executes a utility with a command line formed 
	from standard input stream.   The input stream is tokenized by white
	space.  quoting(\") and escaping(\\) are respected.  

	There are two contraints -- number of arguments and total
	command line length -- which are used to decide when to
	execute a command.  

	If a single argument would cause an overflow of the command
	buffer, an error message is printed and the argument is
	ignored.
	
	The default size is {LINE_MAX}
	The default number of args is 255.

	
	The SV version of xargs allows the user to specify something like:

		echo "source object" | xargs cp /usr/steve/{} /usr/frank/{}
	
	which would copy /usr/steve/source to /usr/frank/source
				and  /usr/steve/object to /usr/frank/object
	
	Steve McPolin.
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <process.h>
#include <limits.h>
#include <sys/wait.h>

#define	XARGS_OK	0
#define	XARGS_ERROR	1

#define	TXT(s)		s

#define	DEFAULT_NARGS	255
#define	DEFAULT_LINE	LINE_MAX
#define	DEFAULT_CMD	"echo "

int	returnValue = 0;
int	exitOnOverflow = 0;
int	npar = 1;
int	verbose = 0;
FILE	*infile;

int getword(char *buf, int maxline) {
	int tchar;
	int count = 0;
	int state = 0;
	while (count < maxline){
		tchar = fgetc(infile);
		if (tchar == ' ' && state == 0){
			/* Ignore starting blank spaces */
			count--;
		}else if (tchar == '\n' && count > 0 || tchar == -1){
			break;
		}else{
			state = 1;
			buf[count] = tchar;
		}
		count++;
	}
	buf[count] = 0;
	return count;
}

static int nrunning =0;

int wait_children() 
{ 
	int	status,pid;
	while (nrunning > 0){
		pid = wait(&status);
		nrunning--;
	}
	return 0;
}

int runcmd2(char *cmd, char **args, int len){
	int pid;
	int i;
	int status;
	/* Ensure we're always within our bounds for running procs */
	if (nrunning >= npar){
		nrunning--;
		pid = wait(&status);
	}
	
	/* Be a little verbose if needed */
	if (verbose){
		fprintf(stderr, "%s ", cmd);
		for(i = 1; i < len-1;i++){
			fprintf(stderr, ",%s", args[i]);
		}
		fprintf(stderr, ")\n");
	}
	
	/* Spawn this command, and count it */
	pid = spawnvpe(P_NOWAIT,cmd,args,0);
	nrunning++;
	
	if (pid == -1){
		perror("spawnvpe");
		exit(1);
	}
	return 0;
}

int xins(int asize, int maxargs, char **argv, int argc) 
{
	char **targv = malloc(sizeof(char*)*(argc+1));
	int i;
	char *wordbuf = malloc(asize);
	if (!wordbuf || !targv){
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	while (getword (wordbuf, asize)){
		/* Build original command line */
		for(i = 0; i < argc; i++){
			targv[i] = malloc(strlen(argv[i])+2);
			if (!targv[i]){
				fprintf(stderr, "Out of memory\n");
				exit(1);
			}
			snprintf(targv[i], strlen(argv[i])+1, "%s", argv[i]);
		}
		
		/* Replace the insertion key with wordbuf references */
		for (i=1; i < argc; i++) {
			if (strcmp(targv[i], "{}") == 0){
				free(targv[i]);
				targv[i] = wordbuf;
			}
		}
		
		runcmd2(argv[0], targv, argc);
		
		/* Free our command line */
		for(i = 0; i < argc; i++){
			if (targv[i] != wordbuf){
				free(targv[i]);
			}
			targv[i] = 0;
		}
		
	}
	wait_children();
	return 0;
}

int xargs(int asize, int maxargs, char **argv, int argc) {
	int		i = 0;
	int		rc= XARGS_OK;
	char *cmd = argv[0];
	int		arglen = 0;
	int csize = 0;
	int dumpnow = 0;
	int exitflag = 0;
	/* Allocate our buffers */
	char **targv = malloc(sizeof(char*)*maxargs);
	char *wbuf = malloc(asize);
	char *tbuf = NULL;
	
	if (!targv || !wbuf){
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	if (getword(wbuf, asize) == 0){
		return 0;
	}
	
	maxargs += argc;
	while(wbuf || i < argc){
		/* If we're at our max args, run this command */
		if (arglen == maxargs || dumpnow){
			int j;
			
			/* Terminate */
			targv[i] = 0;
			
			runcmd2(cmd, targv, arglen);
			
			/* Free our buffers */
			for(j = 0; j < arglen; j++){
				free(targv[j]);
				targv[j] = 0;
			}
			
			/* Reset state variables */
			arglen = 0;
			i = 0;
			csize = 0;
			dumpnow = 0;
			if (exitflag){
				break;
			}
		}
		
		/* If we need to add the command and flags from the xargs cmd line */
		if (i < argc){
			targv[arglen] = malloc(strlen(argv[i])+2);
			if (!targv[arglen]){
				fprintf(stderr, "Out of memory\n");
				exit(1);
			}
			snprintf(targv[arglen], strlen(argv[i])+1, "%s", argv[i]);
			csize += strlen(targv[arglen]);
		}else if (tbuf){
			/* On our last pass, we ran out of room */
			/* Pick up what's left and add it first before reading more */
			targv[arglen] = tbuf;
			csize += strlen(tbuf);
			tbuf = NULL;
		}else{
			/* If we're now accepting input from stdin */
			int size = getword(wbuf, asize);
			
			/* If we're at the end of our input */
			if (size == 0){
				exitflag = 1;
				dumpnow = 1;
			}
			if (wbuf && !exitflag){
				if (size > asize - csize){
					tbuf = malloc(size+2);
					if (!tbuf){
						fprintf(stderr, "Out of memory\n");
						exit(1);
					}
					snprintf(tbuf, size+1, "%s", wbuf);
					dumpnow = 1;
				}else{
					targv[arglen] = malloc(size+2);
					if (!targv[arglen]){
						fprintf(stderr, "Out of memory\n");
						exit(1);
					}
					snprintf(targv[arglen], size+1, "%s", wbuf);
					csize += size;
				}
			}
				
		}
		
		i++;
		arglen++;
	}
	
	if (targv){
		free(targv);
	}
	if (wbuf){
		free(wbuf);
	}
	if (tbuf){
		free(tbuf);
	}
	
	wait_children();
	return rc;
}


int main (int argc, char **argv, char **envp) 
{
	extern	int	optind;
	extern	char	*optarg;
	int		nargs = DEFAULT_NARGS;
	int		asize = DEFAULT_LINE;
	int		c;
	int		fd;
	int		ins=0;
	int		x;
	char	*buf;

	int		maxArgEnv = ARG_MAX - 2048;
	char	*defarg[2];

	fd = dup(0);
	close(0);
	open("/dev/tty",O_RDWR);
	if ((infile=fdopen(fd,"r")) == NULL) {
		perror("xargs");
		return XARGS_ERROR;
	}

	while (*envp)
	{
		maxArgEnv -= strlen (*envp);
		envp++;
	}

	// Fix for PR8622: xargs not working properly with a quoted input.
	for (x=0; x < argc; x++){
    	if (strchr(argv[x], ' ')){
        	buf = (char *)malloc(strlen(argv[x]) * sizeof(char));
        	sprintf(buf, "\'%s\'", argv[x]);
        	argv[x] = buf;
    	}
	} 

	while ((c=getopt(argc,argv,"xtn:s:**P:vi")) != -1) 
	{
		switch (c) 
		{
		case	'x':
			exitOnOverflow = 1;
			break;

		case	'v':
		case	't':	
			verbose = 1;	
			break;
		case	'n':	
			nargs = strtol(optarg,NULL,10);	
			if (nargs <= 0)
			{
				fprintf(stderr,"xargs: numargs must be > 0.\n");
				return XARGS_ERROR;
			}
			break;
		case	's':
			asize = strtol(optarg,NULL,10);
			// limit our command line length so that we dont exceed the limit
			// on our argv and envp
			if ((asize <= 0) || (asize > min(DEFAULT_LINE,maxArgEnv)))
			{
				fprintf(stderr,"xargs warning: maxsize must be <= %d. using maxsize = %d.\n",
					min(DEFAULT_LINE,maxArgEnv), min(DEFAULT_LINE,maxArgEnv));
				asize = min(DEFAULT_LINE,maxArgEnv);
			}
			break;
		case	'i':
			ins = 1;
			break;
		case	'P': 
			npar = strtol(optarg, 0, 10);
			if (npar <= 0) 
			{
				fprintf(stderr,"xargs: invalid number of processes (%d) ignored\n",npar);
				return XARGS_ERROR;
			}
			break;
		default:
			return XARGS_ERROR;
		}
	}
	if (argc == optind) 
	{	
		defarg[0] = DEFAULT_CMD;
		defarg[1] = NULL;
		argv = defarg;
		argc = 1;
		optind = 0;
	}
	if (ins)
		xins(asize,nargs,argv+optind,argc-optind);
	else
		xargs(asize,nargs,argv+optind,argc-optind);
	return returnValue;
}
