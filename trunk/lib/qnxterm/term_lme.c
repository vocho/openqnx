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
#include <string.h>
#include <sys/qnxterm.h>

#define t	term_state

static int
lower( c )
	unsigned c;
	{
	if(c >= 'A'  &&  c <= 'Z') c |= ' ';
	return( c );
	}


static void clear_one( p, attr )
char		*p;
unsigned	 attr;
	{
	if( *p ) {
		term_type( -1, -1, p, 1, (attr | 0x0002) );
		term_type( -1, -1, p+1, 0, attr );
		}
	}

char *term_lmenu( row, col, menu, pos, attr, special, flags)
	int row, col;
	unsigned attr, flags;
	const char * const *menu;
	const char *pos;
	unsigned *special;
	{
	register unsigned i, c;
	register int entry;
	register char *p;
	unsigned *u;
	static char *last_menu;
	static int last_entry, old_entry;

	for( entry=0, p=(char *)pos; menu[entry] && *menu[entry] && *p != *menu[entry]; )
		++entry;
/*
	if( menu[ entry ] == NULL || *menu[i] == 0 )
		*p = *menu[0];
*/
	if( (flags & TERM_MENU_DISPLAY)  ||
		 menu[0] != last_menu  ||  entry != old_entry ) {
	
		for( i = 0; menu[i]  &&  *menu[ i ]; ++i ) {
			if ( (p = (char *)menu[ i ]) == pos )
				term_type( row + i, col, p, 0, (attr | 0x0004) );
			else {
				term_cur( row + i, col );
				clear_one( p, attr );
				}
			if ( p == pos ) old_entry = entry = i;
			}
		last_entry = i-1;
		last_menu = (char *)menu[entry];
		}
		
	for( p = (char *)pos ; ;) {
		term_cur( row + entry, col );

		c = term_key();
		for ( u = special ; u  &&  *u ; ++u )	/* Character substitution*/
			if ( *u++ == c ) {
				c = (unsigned) *u;
				break;
				}
		t.exit_char = c;

		switch( c ) {

		case K_DOWN:
		case K_TAB:
			clear_one( menu[ entry++ ], attr );
			if ( !menu[ entry ] || *menu[entry] == '\0' ) entry = 0;
			term_type( row + entry, col, menu[ entry ], 0, attr | 0x0004 );
			break;

		case K_UP:
			clear_one( menu[ entry ], attr );
			if( --entry < 0 ) entry = last_entry;
			term_type(row + entry, col, menu[ entry ], 0, attr | 0x0004);
			break;

		case '\n':
		case '\r':
			return( (char *)menu[ old_entry = entry ] );

		case K_KPDMINUS:
			if ( flags & TERM_MENU_NO_CANCEL ) break;
			return( 0 );

		default:
			if ( (c & (K_CLASS|K_MOUSE_BUTTONS|K_MOUSE_ACTION)) ==
					(K_MOUSE_POS|K_MOUSE_BSELECT|K_MOUSE_CLICK) &&
					!( flags & TERM_MENU_NO_MOUSE ) &&
					t.mouse_row >= row && t.mouse_col >= col ) {
				for( i = 0; menu[i]  &&  *menu[i]; ++i ) {
					if ( t.mouse_row == row + i &&
								t.mouse_col < col + strlen( menu[i] ) ) {
						break;
						}
					}
				}
			else {
				for( i = 0; menu[i]  &&  *menu[i]; ++i ) {
					if( lower(*menu[i]) == lower(c)) {
						break;
						}
					}
				}
			if ( menu[i] && *menu[i] ) {
				term_cur(row + entry,  col);
				clear_one( menu[ entry ], attr );
				term_type(row + i, col, menu[ i ], 0, attr | 0x0004);
				return( (char *)menu[ old_entry = entry = i ] );
				}

			if ( flags & TERM_MENU_UNKNOWN ) return( 0 );
			}
		}
	}
