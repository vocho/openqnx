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

static void
clear( row, col, len )
	int row, col, len;
	{
	term_cur( row, col );
	while( len > 0 ) {
		fwrite( "                    ", len > 20 ? 20 : len, 1, ct->_outputfp );
		len -= 20;
		}
	}

static void erase_eol() {
	register int i = t.row * columns + t.col;

	if ( i > t.screen_amount/2 ) return;
	if ( t.row < t.region1 ) t.region1 = t.row;
	if ( t.row > t.region2 ) t.region2 = t.row;
	__memsetw( t.scrbuf + i*2, (t.fill & 0x7700) | ' ', t.num_cols - t.col );
	if ( t.attrbuf ) memset( t.attrbuf + i, 0, t.num_cols - t.col );
	}

void
term_up( n )
	register int n;
	{
	if ( t.scrbuf ) {
		if ( t.row == 0 ) {
			if ( (n = t.row) == 0 ) return;
			}
		}
	if ( (t.row -= n) < 0 ) t.row = 0;		/* Adjust our current position	*/
	if ( t.row < t.region1 ) t.region1 = t.row;
	if ( t.row > t.region2 ) t.region2 = t.row;

	if ( !ct->_cc ) {		/* Not on console write, do serial output	*/
		int line;

		if ( line = t.line_set_on ) term_box_off();
		if ( n > 1 && t.cost_up_p < n * t.cost_up )
							__putp( __tparm( parm_up_cursor, n ) );
		else				while ( n-- ) __putp( cursor_up );
		if ( line ) term_box_on();
		}
 	}

void
term_down( n )
	register int n;
	{
	if ( t.scrbuf ) {
		if ( t.row == t.num_rows - 1 ) {
			if ( (n = t.num_rows - t.row - 1) == 0 ) return;
			}
		}
	if ( (t.row += n) >= t.num_rows-1 ) t.row = t.num_rows-1;
	if ( t.row < t.region1 ) t.region1 = t.row;
	if ( t.row > t.region2 ) t.region2 = t.row;

	if ( !ct->_cc ) {
		int line;

		if ( line = t.line_set_on ) term_box_off();
		if ( n > 1 && t.cost_down_p < n )
							__putp( __tparm( parm_down_cursor, n ) );
		else				while ( n-- ) fputc( '\012', ct->_outputfp );
		if ( line ) term_box_on();
		}
	}

void
term_left( n )
	register int n;
	{
	if ( t.scrbuf ) {
		if ( t.col - n < 0 ) {
			if ( (n = t.col) == 0 ) return;
			}
		}
	if ( (t.col -= n) < 0 ) t.col = 0;

	if ( !ct->_cc ) {
		int line;

		if ( line = t.line_set_on ) term_box_off();
		if ( n > 1 && t.cost_left_p < n )
							__putp( __tparm( parm_left_cursor, n ) );
		else				while ( n-- ) fputc( '\b', ct->_outputfp );
		if ( line ) term_box_on();
		}
	}

void
term_right( n )
	register int n;
	{
	if ( t.scrbuf ) {
		if ( t.col + n > t.num_cols - 1 ) {
			if ( (n = t.num_cols - t.col - 1) == 0 ) return;
			}
		}
	if ( (t.col += n) > t.num_cols-1 ) t.col = t.num_cols-1;

	if ( !ct->_cc ) {
		int line;

		if ( line = t.line_set_on ) term_box_off();
		if ( n > 1 && t.cost_right_p < n )
							__putp( __tparm( parm_right_cursor, n ) );
		else				while ( n-- ) __putp( cursor_right );
		if ( line ) term_box_on();
		}
#if 0
	if ( t.col + n > t.num_cols - 1 ) {
		if ( (n = t.num_cols - t.col - 1) == 0 ) return;
		}
	if ( n > 1 && t.cost_right_p < n * t.cost_right )
						__putp( __tparm( parm_right_cursor, n ) );
	else				while ( n-- ) __putp( cursor_right );
	if ( (t.col += n) > t.num_cols-1 ) t.col = t.num_cols-1;
#endif
	}

void
term_cur( row, col )
	int row;
    int col;
    {
	int i = 0;
	register int c_off, r_off;
	int line = 0;

	/* If keeping a video buffer and not running on a terminal, skip output	*/
	if ( ct->_cc ) goto finish;

	if ( line = t.line_set_on ) term_box_off();

	/* Force addressing	*/
	if ( !t.cache_pos  ||  t.col == 12345 ) goto full_move;

	r_off = row - t.row;
	c_off = col - t.col;
	if ( r_off == 0  &&  c_off == 0 ) goto finish2;		/* Nothing to do	*/

	/* Add cost of column relative motion	*/
	if ( t.col && col == 0 ) i = 1;		/* Carriage return takes us here	*/
	else if ( c_off > 1 )	i = t.cost_right_p ? t.cost_right_p : ( c_off*t.cost_right );
	else if ( c_off < -1 )	i = t.cost_left_p ? t.cost_left_p : ( -c_off*t.cost_left );
	else if ( c_off == 1 )	i = t.cost_right;
	else if ( c_off == -1 )	i = t.cost_left;

	/* Compute cost for row relative motion	*/
	if ( r_off > 1 ) 		i += t.cost_down_p ? t.cost_down_p : ( r_off*t.cost_down );
	else if ( r_off < -1 ) 	i += t.cost_up_p ? t.cost_up_p : ( -r_off*t.cost_up );
	else if ( r_off == 1 ) 	i += t.cost_down;
	else if ( r_off == -1 ) i += t.cost_up;

	if ( i > t.cost_move ) {		/* Do full cursor addressing			*/
		if ( col == 0 ) { 				/* Do "cheap" moves before full move*/
			if ( *cursor_home ) {
				__putp( cursor_home );
				if ( row == 0 )	goto finish;	/* Top left corner			*/
				else if ( row == 1 ) {			/* 2nd line, left margin	*/
					fputc( '\012', ct->_outputfp );		/* Line feed				*/
					goto finish;
					}
				/* If row more than 1, do full cursor addressing	*/
				}
			else if ( row == lines - 1 && *cursor_to_ll ) {	/* Bottom left	*/
				__putp( cursor_to_ll );
				goto finish;
				}
			}
full_move:
		__putp( __tgoto( cursor_address, col, row ) );
		}
	else {						/* Do relative, parameterized motion		*/

		/* Do column motion	*/
		if ( col == 0 && t.col ) fputc( '\r', ct->_outputfp );
		else {
			if ( c_off > 0 )		term_right( c_off );
			else if ( c_off < 0 )	term_left( -c_off );
			}

		/* Do row motion	*/
		if ( r_off > 0 )			term_down( r_off);
		else if ( r_off < 0 )		term_up( -r_off	);
		}

finish:
	t.row = row;				/* Record our current position	*/
	t.col = col;
	if ( row < t.region1 ) t.region1 = row;
	if ( row > t.region2 ) t.region2 = row;
finish2:
	if ( line ) term_box_on();
	}

void
term_home() {
	term_cur( 0, 0 );
	}

void
term_clear( type )
	int type;
	{
	register int i;
	unsigned color = 0;

	if ( !ct->_cc && t.color != t.fill &&
			( !t.qnx_term || !*micro_row_address ) ) {
		color = t.color;
		term_color( t.fill );
		}

	switch( type ) {
		case TERM_CLS_EOL:		/* clear eol */
			if ( !ct->_cc ) {
				if ( *clr_eol )	__putp( clr_eol );
				else {
					clear( t.row, t.col, columns - t.col );
					term_cur( t.row, t.col );
					}
				}
			if ( t.scrbuf ) erase_eol();
			break;

		case TERM_CLS_EOS:		/* clear eos */
			if ( !ct->_cc ) {
				if ( *clr_eos )	__putp( clr_eos );
				else {
					for( i = t.row; i < lines; ++i )
						clear( i, t.col, columns - t.col );
					term_cur( t.row, t.col );
					}
				}
			if ( t.scrbuf ) {
				erase_eol();
#ifndef __QNXNTO__
				if ( t.row < lines )
					__term_clear_buffer( t.row + 1 );
#endif
				}
			break;

		case TERM_CLS_SCR:	/* clear screen and home	*/
		default:
			if ( !ct->_cc ) {
				__putp( clear_screen );
				__putp( cursor_home );
				}
			t.row = t.col = 0;
#ifndef __QNXNTO__
			__term_clear_buffer( 0 );
#endif
			break;
			}
	if ( color ) term_color( color );
	}
