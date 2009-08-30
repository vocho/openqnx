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
 * April 1980 by  D. T. Dodge
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

#if defined(__QNX__) && !defined(__QNXNTO__)
#undef SIGTSTP		/* QNX4 doesn't support this correctly */
#endif

static int status = 0;
unsigned old_stkptr;

sigjmp_buf env;

void
do_ed() {
	breakable();
	signal(SIGINT, &rip_out);
	while(status != EOF) {
		if((lp = cmd_input(lbuff)) == NULL)
			quit(0);
		else
			status = exec_line(NOGLOB, 0);
		}
	quit(0);
	}

#ifdef SIGTSTP
void term_stop(int signo) {
	sigset_t		mask;
	extern void __ungetch(int c);

	term_cur(term_state.num_rows - 1, 0);

	term_restore();

	sigemptyset(&mask);
	sigaddset(&mask, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	signal(SIGTSTP, SIG_DFL);
    kill(getpid(), SIGTSTP);
	/* We don't get here until we are send a SIGCONT */
	signal(SIGTSTP, term_stop);
	term_init();
	prnt_row = 1;
	firstp->lflags |= DIRTY_FLAG;
	__ungetch(0);
	siglongjmp(env, 0);
	}
#endif


int main( int argc, char *argv[] ) {
	int arg;
	int opt;

	while ( ( opt = getopt( argc, argv, "br" ) ) != -1 )	{
		switch(opt)	{
			case 'b':	++browse_flag;		continue;
			case 'r':	++restrict_flag;	continue;
			case '?':	exit( -1 );
			}
		}

#ifdef SIGTSTP
	if(signal(SIGTSTP, SIG_IGN) == SIG_DFL)
		signal(SIGTSTP, term_stop);
#endif
	signal(SIGQUIT, SIG_IGN);

	if(initbuffer() != OK) {
		fprintf( stderr,"Not enough memory to load editor\n");
		perror( "" );
		return(-1);
		}

	if(argc > 1) {
		if((status=_read(argv[optind], 0, 0)) <= ERROR) {
			puterr(status);
			}
		else {
			if(lastln)
				curln = 1;
			}

		set_fn(curfile, argv[optind++]);
		}

	if(browse_flag) {
		++restrict_flag;
		*curfile = '\0';
		}

	openerrflag = 1;
	dirty = 0;
	status = OK;
	escape_char = 1;
	for(arg = optind ; arg < argc  &&  status != EOF ; ++arg) {
		strcat(strcpy(lp = lbuff, argv[arg]), "\n");
		if((status = exec_line(NOGLOB, 0)) == EOF)
			return(0);
		}

	/*
	 * Turn off edit on the tty so we get all chars immediatly.
	 */
#if 0
	get_attr(stdin, lbuff);
	escape_char = lbuff.tty_ESCAPE;
#endif
	unbreakable();

	for(;;) {
		clr_flag = 1;
		firstp->lflags |= DIRTY_FLAG;
		if ( sigsetjmp( env , 1 ) == 0 )	/* Enter edit after setting up siglongjmp	*/
			do_ed();

		macro_flags = 1;
		macro_ptr = 0;
		macro_level = 0;

		if(learn_ptr) {
			learn_ptr[learn_index++] = '\n';
			learn_ptr[learn_index] = '\0';
			install(*learn_ptr, learn_ptr + 1, 0x1);
			free(learn_ptr);
			learn_ptr = NULL;
			}

		if(ed_fp)  {			/* break or abort during a file r/wf*/
			ed_close();
			}

		if(x_fp) {		/* break or abort during a file execute */
			fclose(x_fp);
			x_fp = 0;
			}

		brk1();
		}
	}

void
rip_out(int signo) {
/*
	printf( "BREAK!\n" );
	fflush( stdout );
*/
	signo = signo;
	siglongjmp( env, 1 );
	}

int exec_line(int under_glob, int under_until)
	{
	int curln_save, stat;

	do {
		curln_save = curln;
		stat = exec_cmd(under_glob, under_until);
		} while(*lp  &&  *lp != '\n'  &&  stat == OK);

	if(stat <= ERROR) {
		cc_reg = FALSE;
		if(!under_until) {
			puterr(stat);
			curln = imin(curln_save, lastln);
			}
		}

	return( stat );
	}



/*
 * Fix the users tty so it is the same as it was upon entry
 * then exit.
 */
void
quit( int value) { 
	setdown();
//	clear_screen1(0);	/* clear users screen */
	fflush(stdout);
	exit(value);
	}
