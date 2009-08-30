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
#include <sys/term.h>
#include <sys/qnxterm.h>
#include <errno.h>

#define t	term_state
#define ct	__cur_term

int
term_insert_line( row, n )
	int row;
	register int n;
	{
	register int i = n;
	unsigned color;

	if ( !ct->_cc ) {
		if ( t.color != t.fill && ( !t.qnx_term || !*micro_row_address ) ) {
			color = t.color;
			term_color( t.fill );
			}
		else color = 0;

		if ( *insert_line ) {
			term_cur( row, 0 );
			if ( n > 1 && *parm_insert_line )
								__putp( __tparm( parm_insert_line, n ) );
			else				while ( i-- ) __putp( insert_line );
			}
		else if ( *scroll_reverse && ( row == 0 || *change_scroll_region ) ) {
			if ( n && row ) {
				__putp( __tparm( change_scroll_region, row, t.num_rows - 1 ) );
				t.col = 12345;
				}
			term_cur( row, 0 );
			while ( i-- ) __putp( scroll_reverse );
			if ( n && row ) {
				__putp( __tparm( change_scroll_region, 0, t.num_rows - 1 ) );
				t.col = 12345;
				term_cur( row, 0 );
				}
			}
		else {
			errno = EOPNOTSUPP;
			if(color) term_color(color);
			return( -1 );
			}
		if ( t.attrbuf ) {
			memmove( t.attrbuf + (row + n) * t.num_cols,
					 t.attrbuf + row * t.num_cols,
					((t.num_rows - row) - n) * t.num_cols );
			memset( t.attrbuf + row * t.num_cols, 0, n * t.num_cols );
			}
		if(color) term_color(color);
		}
	else
		term_cur( row, 0 );

	/* Area from insert line to the bottom of the screen is changed	*/
	if ( row < t.region1 ) t.region1 = row;
	t.region2 = t.num_rows - 1;
	if ( (row + n) > t.num_rows ) n = t.num_rows - row;
	if ( t.scrbuf && n ) {
		memmove( t.scrbuf + (row + n) * t.line_amount,
				 t.scrbuf + row * t.line_amount,
				((t.num_rows - row) - n) * t.line_amount );
		__memsetw( t.scrbuf + (row * t.line_amount),
					(t.fill & 0x7f00) | ' ', n * t.num_cols );
		}
	return( 0 );
	}

int
static del_line( row, n )
	int row;
	register int n;
	{
	register int i = n;
	unsigned color;

	if ( !ct->_cc ) {
		if ( t.color != t.fill && ( !t.qnx_term || !*micro_row_address ) ) {
			color = t.color;
			term_color( t.fill );
			}
		else color = 0;

		if ( *delete_line ) {
			term_cur( row, 0 );
			if ( n > 1 && *parm_delete_line )
								__putp( __tparm( parm_delete_line, n ) );
			else				while ( i-- ) __putp( delete_line );
			}
		else if ( row == 0 || *change_scroll_region ) {
			if ( n && row ) {
				__putp( __tparm( change_scroll_region, row, t.num_rows - 1 ) );
				t.col = 12345;
				}
			term_cur( t.num_rows - 1, 0 );
			while ( i-- ) {
				if ( *scroll_forward )
					__putp( scroll_forward );
				else
					fputc( 0x0a, ct->_outputfp );
				}
			if ( n && row ) {
				__putp( __tparm( change_scroll_region, 0, t.num_rows - 1 ) );
				t.col = 12345;
				}
			}
		else {
			errno = EOPNOTSUPP;
			if(color) term_color(color);
			return( -1 );
			}

		if ( t.attrbuf ) {
			memmove( t.attrbuf + row * t.num_cols,
					t.attrbuf + (row + n) * t.num_cols,
					(t.num_rows - row ) * t.num_cols );
			memset( t.attrbuf + t.screen_amount/2 - (n * t.num_cols),
					0, n * t.num_cols );
			}
		if(color) term_color(color);
		}

	/* Area from insert line to the bottom of the screen is changed	*/
	if ( row < t.region1 ) t.region1 = row;
	t.region2 = t.num_rows - 1;
	if ( (t.num_rows - row) - n < 0 ) n = row;
	if ( t.scrbuf && n ) {
		memmove( t.scrbuf + row * t.line_amount,
				t.scrbuf + (row + n) * t.line_amount,
				((t.num_rows - row) - n) * t.line_amount );
		__memsetw( t.scrbuf + t.screen_amount - (n * t.line_amount),
					(t.fill & 0x7f00) | ' ', n * t.num_cols );
		}
	return( 0 );
	}

int
term_delete_line( int row, int n )
	{
	if ( ( n = del_line( row, n) ) != -1 )
		term_cur( row, 0 );
	return( n );
	}

int
term_scroll_up() {
	int i;

	if ( ( i = del_line( 0, 1) ) != -1 )
		term_cur( t.num_rows - 1, 0 );
	return( i );
	}

int
term_scroll_down() {
	return term_insert_line( 0, 1 );
	}

int
term_delete_char( n )
	register int n;
	{
	register char *p;
	register int i = n;
	unsigned color;

	if ( t.col + n > t.num_cols ) n = t.num_cols - t.row;
	if ( t.row < t.region1 ) t.region1 = t.row;
	if ( t.row > t.region2 ) t.region2 = t.row;
	if ( t.scrbuf && n ) {
		p = t.scrbuf + t.row * t.line_amount + t.col * 2;
		memmove( p, p+(n*2), ((t.num_cols - t.col) - n)*2 );
		__memsetw( t.scrbuf + t.row*t.line_amount + t.num_cols*2 - n*2, 
				(t.fill & 0x7f00) | ' ', n );
		}
	if ( !ct->_cc ) {
		if ( *delete_character == '\0' ) {
			errno = EOPNOTSUPP;
			return( -1 );
			}

		if ( t.color != t.fill && ( !t.qnx_term || !*micro_row_address ) ) {
			color = t.color;
			term_color( t.fill );
			}
		else color = 0;

		if ( n > 1 && *parm_dch )
							__putp( __tparm( parm_dch, n ) );
		else				while ( i-- ) __putp( delete_character );
		if ( t.attrbuf ) {
			p = t.attrbuf + t.row * t.num_cols + t.col;
			memmove( p, p+n, (t.num_cols - t.col) - n );
			memset( t.attrbuf + t.row*t.num_cols + t.num_cols - n, 0, n );
			}
		if(color) term_color(color);
		}
	return( 0 );
	}

int term_insert_char( n )
	register int n;
	{
	register char *p;
	register int i = n;
	unsigned color;

	if ( t.row < t.region1 ) t.region1 = t.row;
	if ( t.row > t.region2 ) t.region2 = t.row;
	if ( t.scrbuf && n ) {
		p = t.scrbuf + t.row * t.line_amount + t.col * 2;
		memmove( p+(n*2), p, ((t.num_cols - t.col) - n)*2 );
		__memsetw( t.scrbuf + t.row*t.line_amount + t.col*2, 
				(t.fill & 0x7f00) | ' ', n );
		}
	if ( !ct->_cc ) {
		if ( *insert_character == '\0' ) {
			errno = EOPNOTSUPP;
			return( -1 );
			}

		if ( t.color != t.fill && ( !t.qnx_term || !*micro_row_address ) ) {
			color = t.color;
			term_color( t.fill );
			}
		else color = 0;

		if ( n > 1 && *parm_ich )
							__putp( __tparm( parm_ich, n ) );
		else				while ( i-- ) __putp( insert_character );
		if ( t.attrbuf ) {
			p = t.attrbuf + t.row * t.num_cols + t.col;
			memmove( p+(n*2), p, (t.num_cols - t.col) - n );
			memset( t.attrbuf + t.row*t.num_cols + t.col, 0, n );
			}
		if(color) term_color(color);
		}
	return( 0 );
	}

int
term_insert_on() {
	if ( *enter_insert_mode == '\0' ) {
		errno = EOPNOTSUPP;
		return( -1 );
		}
	__putp( enter_insert_mode );
	return( 0 );
	}

void
term_insert_off() {
	__putp( exit_insert_mode );
	}
