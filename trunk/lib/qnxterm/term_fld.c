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
#include <sys/term.h>
#include <sys/qnxterm.h>

#define t	term_state

#ifndef __STDC__
#define const
#endif

static int get_term_field( row, col, field, field_size, field_attr )
	unsigned	 row, col, field_size, field_attr;
	char *field;
	{
	register unsigned	ptr;
	unsigned			i;
	unsigned 			c;
	char				first,
						insert_switch;

	for(ptr = 0, first = 1, insert_switch = 0; ; ) {
		switch(c = term_key()) {
		case '\n':
		case '\r':
			return(K_TAB);

		case K_KPDPLUS:	/*	A7	*/
		case K_ESC:
			return(c);

		case K_ERASE:			/*	18	*/
			memset(field, '\0', field_size + 1);
			ptr = 0;
			term_printf(row, col, field_attr, "%-*s", field_size, field);
			term_cur(row, col);
			break;

		case K_BACKSP:
		case K_RUBOUT:		/*	7F	*/
			if(ptr > 0)
				--ptr;

		case K_DELETE:		/*	AC	*/
			if(*field)
				strcpy(field + ptr, field + ptr + 1);

			term_printf(row, col + ptr, field_attr, "%-*s",
										1 + strlen(field + ptr), field + ptr);
			term_cur(row, col + ptr);
			break;

		case K_KPDMINUS:		/*	A3	*/
			return(c);

		case K_LEFT:			/*	A4	*/
			if ( ptr > 0 ) --ptr;
			term_cur(row, col + ptr);
			break;

		case K_RIGHT:			/*	A6	*/
			++ptr;
cursor:
			if ( ptr > strlen( field ) ) ptr = strlen( field );
			term_cur(row, col + ptr);
			break;

		case K_INSERT:		/*	AB	*/
			insert_switch = (char) (insert_switch ? 0 : 1);
			break;

		case K_CTL_LEFT:		/*	B4	*/
			while(ptr > 0)
				if(field[ptr - 1] == ' ')
					--ptr;
				else
					break;

			while(ptr > 0)
				if(field[ptr - 1] != ' ')
					--ptr;
				else
					break;

			term_cur(row, col + ptr);
			break;

		case K_CTL_RIGHT:	/*	B6	*/
			i = strlen(field);
			while(ptr < i  &&  field[ptr] != ' ')
				++ptr;

			while(ptr < i  &&  field[ptr] == ' ')
				++ptr;

			term_cur(row, col + ptr);
			break;

		default:
			if ( c >= K_RESIZE && c < 0xf000 ) {
				if ( (c & (K_CLASS|K_MOUSE_BUTTONS)) ==
						(K_MOUSE_POS|K_MOUSE_BSELECT) &&
						t.mouse_row == row &&
						t.mouse_col >= col &&
						t.mouse_col <= col + field_size ) {
					if ( (c & K_MOUSE_ACTION) == K_MOUSE_CLICK ) {
						ptr = t.mouse_col - col;
						goto cursor;
						}
					else {
						continue;
						}
					}
				return( c );
				}
			if(ptr >= field_size  ||  c < ' ' || c >= 0x100 )
				{
				__putp( bell );
				continue;
				}
			else if(first  &&  !insert_switch)
				{
				i = strlen(field);
				memset(field, '\0', field_size + 1);
				field[0] = (char)c;
				ptr = 1;
				term_printf(row, col, field_attr, "%-*s",
					i > 1 ? i : 1, field);
				term_cur(row, col + 1);
				}
			else
				{
				term_type(row, col + ptr, (char *) &c, 1, field_attr);

				if(insert_switch)
					{
					for(i = field_size; i > ptr; --i)
						field[i] = field[i - 1];

					field[field_size] = '\0';
					term_printf(row, col + ptr + 1, field_attr, "%-*s",
									strlen(field + ptr + 1), field + ptr + 1);
					}

				field[ptr++] = (char)c;
				term_cur(row, col + ptr);
				}
			}
		first = 0;
		}
	}

/* Defalt is misspelled to avoid reserved word problems */
unsigned term_field( row, col, field, field_size, defalt, field_attr )
	int		row, col, field_size;
	unsigned field_attr;
	char * field;
	const char * defalt;
	{
	register unsigned	c;

	do
		{
		if(defalt)
			strncpy(field, defalt, field_size);
		else
			strncpy(field, "", field_size);

		/*
		 *	If field_size <= strlen(defalt), strncpy will not null terminate
		 *	field (screwy Unix definition!).
		 *	Force null termination.
		 */
		field[field_size] = '\0';

		term_printf(row, col, field_attr, "%-*s", field_size, field);
		term_cur(row, col);

		c = term_key();
		if( (K_F1 <= c  &&  c <= K_CTL_F10)      ||
			(K_ALT_F1 <= c  &&  c <= K_SHF_F12)	 ||
			(K_CTL_F11 <= c  &&  c <= K_CTL_F12) ||
			(K_ALT_F11 <= c  &&  c <= K_ALT_F12)	)
			return(c);

		switch(c)
			{
		case K_TAB:
		case K_BACKTAB:
		case K_CTL_TAB:
		case K_HOME:
		case K_UP:
		case K_PGUP:
		case K_KPDMINUS:
		case K_KPDPLUS:
		case K_END:
		case K_DOWN:
		case K_PGDN:
		case K_ALT_UP:
		case K_ALT_DOWN:
		case K_ESC:
			return(c);

		default:
			term_unkey(c);
			}

		c = get_term_field(row, col, field, field_size, field_attr);
		}
	while(K_KPDMINUS == c);

	return(c);
	}
