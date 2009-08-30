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
	char c;
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

static int
offset( menu, entry )
	char **menu;
	int entry;
	{
	register int i, off = 0;

	for ( i = 0; i < entry; ++i )
		off += strlen( menu[i] ) + 1;
	return( off );
	}

char *term_menu(row, col, menu, pos, attr, special, flags)
	int row, col;
	unsigned attr, flags;
	const char * const *menu;
	const char *pos;
	unsigned *special;
	{
	unsigned *u;
	char *p;
	static char *last_menu;
	register unsigned i, c;
	register int entry;
	static int last_entry, old_entry;

	for( entry = 0, p = (char *)pos; *p != *menu[ entry ]; ++entry ) ;

	if( (flags & TERM_MENU_DISPLAY) ||
		menu[0] != last_menu  ||  entry != old_entry ) {
	
		term_cur( row, col );
		for( i = 0; menu[i]  &&  *menu[ i ]; ++i ) {
			p = (char *)menu[ i ];
			if ( p == pos )
				term_type( -1, -1, p, 0, (attr | 0x0004) );
			else
				clear_one( p, attr );
			term_type( -1, -1, " ", 1, attr );
			if ( p == pos ) old_entry = entry = i;
			}
		last_entry = i-1;
		last_menu = (char *)menu[ entry ];
		if ( !(flags & TERM_MENU_NO_CEOL) )	term_clear( 1 );
		}
		
	for( p = (char *)pos;; ) {
		term_cur( row, col + offset( menu, entry ) );

		c = term_key();
		for( u = special ; u  &&  *u; ++u )
			if ( *u++ == c ) {
				c = (unsigned) *u;
				break;
				}

		t.exit_char = c;
		switch(c) {
		case K_RIGHT:
		case K_TAB:
			clear_one( menu[ entry++ ], attr );
			if ( !menu[entry]  ||  *menu[entry] == '\0' ) entry = 0;
			term_type( row, col + offset( menu, entry ),
						menu[ entry ], 0, attr | 0x0004 );
			break;

		case K_LEFT:
			clear_one( menu[ entry ], attr );
			if( --entry < 0 ) entry = last_entry;
			term_type( row, col + offset( menu, entry ),
						menu[ entry ], 0, attr | 0x0004);
			break;

		case '\n':
		case '\r':
			return( (char *)menu[ old_entry = entry ] );

		case K_KPDMINUS:
			if ( flags & TERM_MENU_NO_CANCEL ) break;
			return( NULL );

		default:
			if ( (c & (K_CLASS|K_MOUSE_BUTTONS|K_MOUSE_ACTION)) ==
					(K_MOUSE_POS|K_MOUSE_BSELECT|K_MOUSE_CLICK) &&
					!( flags & TERM_MENU_NO_MOUSE ) &&
					t.mouse_row == row && t.mouse_col >= col ) {
				for( c = col, i = 0; menu[i]  &&  *menu[i]; ++i ) {
					if ( t.mouse_col < c ) {
						continue;
						}
					c += strlen( menu[i] );
					if ( t.mouse_col < c ) {
						break;
						}
					c++;
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
				term_cur( row, col + offset( menu, entry ) );
				clear_one( menu[ entry ], attr );
				term_type( row, col + offset( menu, i ),
							menu[ i ], 0, attr | 0x0004 );
				return( (char *)menu[ old_entry = entry = i ] );
				}

			if ( flags & TERM_MENU_UNKNOWN ) return( NULL );
			}
		}
	}
