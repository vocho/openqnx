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
#include <stdlib.h>
#include <string.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

#define EXEC_COMMAND	0
#define READ_CURLINE	1
#define READ_ABSLINE	2
#define READ_DATA		3
#define DISPLAY_SCREEN	4

#define DEFLT_USER_PRIORITY 8

/*
struct line {
	char *next;
	char *prev;
	char lflags;
	char textp[0];
	} ;
*/

struct message {
	char msg_code;
	unsigned msg_line;
	char msg_data[LINE_LENGTH];
	} ;

#if defined(__WATCOMC__)
#pragma off(unreferenced);
#endif 
int exec_sys_cmd(unsigned under_glob, unsigned under_until)
#if defined(__WATCOMC__)
#pragma on(unreferenced);
#endif 
	{
	char				 supress = 0;

	if(msg_tid)
		return(ERROR15);

	if(*lp == '!') {
		++lp;
		supress = 1;
		}

	*(lp + strlen(lp) - 1) = '\0';	/* Remove the trailing \n */

	unbreakable();
	term_restore();

	if(supress == 0) {
		clear_screen1(0);
		setdown();
		term_flush();
		}

	cmd_status = system(lp);
	term_init();
	lp = "\n";	/* To force a fetch of a new line */

	if(supress == 0) {
		prnt_row = 1;
		firstp->lflags |= DIRTY_FLAG;
		breakable();
		printf("ED: Press <Enter> to continue");
		fflush(stdout);
		}
	else
		breakable();

	term_fill(attributes[2]);
	fflush(stdout);

	return(OK);
	}

void
msg_input(){}
/*
msg_input(under_glob, under_until)
unsigned under_glob, under_until;
	{
	register char *p, msg_code;
	unsigned reply_len;

	p = lbuff;
	msg_tid = Exc_bits[1];
	Exc_bits[1] = Exc_allow[1] = 0;	/. Ignore any new user exceptions ./

	while(receive(msg_tid, p, LINE_LENGTH) == msg_tid) {
		msg_code = p->msg_code;
		p->msg_code = OK;
		reply_len = sizeof(struct message);

		if(msg_code == EXEC_COMMAND) {
			strcat(lbuff, "\n");
			lp = p->msg_data;
			p->msg_code = exec_line(under_glob, under_until);
			reply_len = 1;	/. For efficiency ./
			}
		else if(msg_code == READ_CURLINE)
			strcpy(p->msg_data, getptr(curln)->textp);
		else if(msg_code == READ_ABSLINE)
			if(p->msg_line <= lastln)
				strcpy(p->msg_data, getptr(p->msg_line)->textp);
			else
				p->msg_code = ERROR1;
		else if(msg_code == READ_DATA)
			memcpy(p->msg_data, &nladdrs, ((unsigned)&opt_e) - ((unsigned)&nladdrs) + 1);
		else if(msg_code == DISPLAY_SCREEN) {
			redisp_line = 0;
			clr_flag = 1;
			update_screen();
			}
		else
			p->msg_code = -1;

		reply(msg_tid, p, reply_len);
		}

	msg_tid = 0;
	}
*/
