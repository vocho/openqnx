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
#include <malloc.h>
#include <string.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

unsigned mgetchar() {
	register unsigned c;
	register struct macro_entry *mp;

loop:
	if(macro_ptr) {
		if(c = *macro_ptr++) {
			if(c == INPUT_CHAR) {
				locate_cursor(msgrow, msgcol);
				if((c = getchar()) == '\r') {
					while(((unsigned) *macro_ptr) == INPUT_CHAR)
						++macro_ptr;
					goto loop;
					}
				else {
					put_screen(msgrow, msgcol++, c, attributes[CMD_AREA]);
					return(c);
					}
				}
			}
		else {
			macro_flags = macro_stk[--macro_level].macro_flags;
			macro_ptr = macro_stk[macro_level].macro_ptr;
			goto loop;
			}

		if(c == macro_disable_char  &&  *macro_ptr)
			return(*macro_ptr++);
		}
	else if((c = get()) == macro_disable_char)
			return(get());

	if(c != EOF  &&  (mp = lookup(c))) {
		if(macro_level >= MACRO_LEVELS) {
			macro_flags = 1;
			macro_ptr = 0;
			macro_level = 0;
			putmsg("Macro level overflow");
/*			asm("mov bp,<Exc_fp>");	@@@	*/
			return(0);
			}
		macro_stk[macro_level].macro_flags = macro_flags;
		macro_stk[macro_level++].macro_ptr = macro_ptr;
		macro_flags = mp->mflag;
		macro_ptr = mp->mstr;
		
		goto loop;
		}

	return(c);
	}


struct macro_entry *
lookup(char c)
	{
	register struct macro_entry *p;

	for(p = hash_tab[c & (HASH_TAB_SIZE - 1)] ; p ; p = p->mlink)
		if(p->mchar == c  &&  macro_flags)
			return(p);

	return( NULL );
	}


int install( char c, char *str, char recursive)
	{
	register struct macro_entry *p, *temp_p;
	register int n;

	for(p = (struct macro_entry *)&hash_tab[c & (HASH_TAB_SIZE - 1)];
												p->mlink ; p = p->mlink)
		if(p->mlink->mchar == c) {
			temp_p = p->mlink->mlink;
			free(p->mlink);
			if((p->mlink = temp_p) == NULL)
				break;
			}

	if(p = p->mlink = calloc(1, sizeof(struct macro_entry) - 1 + (n = strlen(str)))) {
		if(*str) {
			p->mchar = c;
			p->mflag = recursive;
			memcpy(p->mstr, str, n);
			p->mstr[n-1] = '\0';	/* Get rid of trailing CR */
			}
		return(OK);
		}

	return(ERROR0);
	}


int get() {
	register int c;

	macro_level = 0;
	macro_ptr = 0;
	macro_flags = 1;

	c = get_term_char();

	if(learn_ptr)
		if(learn_index >= LEARN_BUF_SIZE) {
			putmsg("Learn buffer overflow");
			while(msg_char != '\n'  &&  msg_char != '\r')
				msg_char = getchar();
/*			asm("mov bp,<Exc_fp>");	@@@	*/
			}
		else
			learn_ptr[learn_index++] = c;

	return(c);
	}
