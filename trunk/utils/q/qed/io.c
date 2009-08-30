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
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#ifndef __QNXNTO__
#include <sys/dev.h>
#endif
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"
#define EXT extern
#include "externs.h"

#define NULL_CHAR	0xfa
#define TAB_CHAR	0x10
#define HATCH_CHAR	0xb0
#define PARA_CHAR   0x14
#define SECT_CHAR	0x15
#define CONT_CHAR	0x1d
#define OVER_CHAR	0x1b
#define TAB_MARK	0x1f
#define LMARG_MARK	'L'
#define RMARG_MARK	'R'

#define NORM_ATTR				0x00
#define BLINK_ATTR				0x01
#define HIGHLIGHT_ATTR			0x02
#define INVERSE_ATTR			0x04
#define UNDERLINE_ATTR			0x08
#define HIGHLIGHT_BLINK_ATTR	0x03
         
#define MACRO_DISABLE_CHAR	0xa3


/*
 * These two variables are 8088/8086/80186 machine specific as is the structure
 * for accessing them.
 */

char null_char;
char tab_char;
char end_char;
unsigned line_attr[67];	/* allow up to a 66 line display */
char	del_line_supported = 0,
		ins_line_supported = 0;
unsigned startup_fill;

/*
 * This routine is called each time we break for input to ensure that the
 * screen is updated properly. It is not called during execution of a
 * command line even if there are multiple commands on the line. The one
 * exception is the VS command which implicity calls this routine to
 * force an update. ie:  u30 s/^/+/     - only displays end product
 *						 u30 s/^/+/vs0	- displays each iteration
 *
 * This routine does not actually touch the hardware and can be used as is.
 * However, if the output device is slow you may want to make modifications
 * at this higher level. There may not be sufficient information available
 * to do much optimization at the lower screen drivers.
 */

void
update_screen() {
	register int new_crow, new_ccol, new_srow, new_scol, cursor_type;
	register struct line *curln_ptr;
	signed char one;

	if(lastln) {					/* buffer not empty */
		curln_ptr = getptr(curln);
		if(curcol <= 0)
			curcol = index_to_col(curln_ptr->textp, curin);
		else
			curin = col_to_index(curln_ptr->textp, curcol);

		if((new_crow = curln - screen_row) < 0) {
			new_srow = lock_cursor ? curln : imax(1, curln - center_line);
			new_crow = curln - new_srow;
			}
		else if(new_crow >= screen_height - SCREEN_OFFSET) {
			new_srow = lock_cursor
							? imax(1, curln-(screen_height-SCREEN_OFFSET-1))
							: imax(1, curln - center_line);
			new_crow = curln - new_srow;
			}
		else
			new_srow = screen_row;

		lock_cursor = 0;
	
		if((new_ccol = curcol - screen_col) < 0) {
			new_scol = memory_mapped ? curcol : imax(1, curcol - 20);
			new_ccol = memory_mapped ? 0 : imax(0, curcol - new_scol);
			}
		else if(new_ccol >= screen_width) {
			new_scol = imax(1, curcol - (screen_width - (memory_mapped ? 1
																	: 20)));
			new_ccol = curcol - new_scol;
			}
		else
			new_scol = screen_col;

		if(new_srow != screen_row  ||  new_scol != screen_col) {
			if(new_scol != screen_col)
				firstp->lflags |= DIRTY_FLAG;
			redisp_line = 0;
			screen_row = new_srow;
			screen_col = new_scol;
			}
		}
	else
		new_crow = new_ccol = new_srow = 0;

	disp_stats();
	disp_screen_image(new_srow);

	cursor_type = opt.state==CMD ? INACTIVE : ACTIVE;
	move_cursor(new_crow + SCREEN_OFFSET, new_ccol, cursor_type,
				&cursor_row, &cursor_col, cursor_type);

	/*
	Turn position caching back on in case the map_term_line had to turn
	it off to display embedded control characters
	*/
	term_state.cache_pos = 1;

	new_ccol = index_to_col(firstp->textp, cmdin) - 1;
	cursor_type = opt.state==CMD ? ACTIVE : INACTIVE;
	one = 1;
	move_cursor(one, new_ccol, cursor_type,
				&one, &cmd_cursor_col, cursor_type);

	clr_flag = 0;
	}



/*
 * This routine like the one above does not touch the display driver and
 * may be used as is. However, it has been included to allow you to make
 * higer level decisions if necessary.
 */
void
move_cursor(signed char new_row, signed char new_col, signed char new_type,
			signed char *old_row, signed char *old_col, signed char old_type)	{

	if(old_type == INACTIVE && *old_row > 1 && (new_type == ACTIVE
			|| *old_row != new_row || *old_col != new_col)) {
		put_screen(*old_row, *old_col, 0, attributes[TEXT_AREA]);
		}

	if(new_type == ACTIVE) {
		unbreakable();
		locate_cursor(new_row, new_col);
		breakable();
		}
	else if(new_row > 1) {
		put_screen(new_row, new_col, 0, attributes[TEXT_AREA] | INVERSE_ATTR);
		}

	*old_row = new_row;
	*old_col = new_col;
	}



/*
 * This routine is called by update screen to display a new screen image.
 * Only those lines which have changed are displayed. This routine calls
 * on another routine DISP_LINE_IMAGE to interface to the display hardware.
 * It is therefore also relatively indepandant of the display hardware.
 * There are two criteria for displaying a line.
 *
 * 1. The line number is greater than redisp_line. Redisp_line will
 *    be set to the lowest new line which was inserted or deleted.
 *    This affects all lines below. It is sometimes set to zero when
 *    I want every line to be redisplayed.
 * 2. The NEW_FLAG or DIRTY_FLAG is set (the line is new or has been changed)
 *
 */
void
disp_screen_image(int ln)
	{
	register struct line *p;
	int row, nrows;
	static int old_ln;

	if(ln > lastln  ||  ln < 0) {
		putln(stderr, "** load screen image **\n",0);
		return;
 		}

	/*
	 * Display the command line first.
	 */

	p = firstp;
	if(p->lflags & DIRTY_FLAG) {
		if(opt.state == CMD)
			disp_line_image(1, p->textp, attributes[CMD_AREA], 0);
		else
			hatch_cmd_line();
		p->lflags &= ~DIRTY_FLAG;
		}

	row = SCREEN_OFFSET;
	if(lastln) {
		if( !memory_mapped  &&  old_ln ) {
			/*
			 *	Take care of deleted lines above the displayed screen
			 *	and the screen scrolling up or down.
			 */
			if( num_delete_lines  &&  delete_line < old_ln ) {
				num_delete_lines -= nrows = imin( num_delete_lines,
													old_ln - delete_line );
				delete_line = old_ln -= nrows;
				}
			if( num_delete_lines == 0 )
				delete_line = lastln + 1;

			if( ( nrows = ln - old_ln ) > 0  &&
											del_line_supported) {
				if( nrows < 2 * screen_height / 3 )
					del_term_line( 2, nrows );
				}
			else if( ( nrows = old_ln - ln ) > 0  &&
												ins_line_supported)
				if( nrows < 2 * screen_height / 3 )
					ins_term_line( 2, nrows );
			}
		old_ln = ln;

		p = getptr(ln);

		nrows = imin(lastln - ln + 1, screen_height - SCREEN_OFFSET) +
																SCREEN_OFFSET;
		for(row = SCREEN_OFFSET ; row < nrows ; ++row) {
			if(!memory_mapped) {
				if( ln == delete_line  &&
									num_delete_lines < screen_height - row
											&&  del_line_supported )
					del_term_line( row, num_delete_lines );
				if( p->lflags & NEW_FLAG  &&  ln < lastln  &&  redisp_line  &&
							redisp_line <= ln && ins_line_supported )
					ins_term_line( row, 1 );
				}
			if(redisp_line <= ln  ||  (p->lflags & DIRTY_FLAG)) {
				disp_line_image(row, p->textp,
						nmarks  &&  ln >= marker1  &&  ln <= marker2
									? attributes[TEXT_AREA] | INVERSE_ATTR
									: attributes[TEXT_AREA], p->lflags);
				p->lflags &= ~( NEW_FLAG | DIRTY_FLAG );
				}
			++ln;
			p = p->next;
			}
		}

	if(redisp_line != lastln + 1)
		while(row < screen_height) {
			disp_line_image(row, "", attributes[TEXT_AREA], 0);
	        ++row;
			}

	redisp_line = lastln + 1;
	num_delete_lines = 0;
	}



/*
 * This routine hatches the command line when you drop into
 * the text area.
 */
void
hatch_cmd_line() {
	register char *p, hatch_c;
	unsigned o, n, w, d, r = 0;

	hatch_c = memory_mapped ? HATCH_CHAR : ' ';
	for(p = rbuff ; p < &rbuff[LINE_LENGTH]; ++p)
		*p = hatch_c;

	*p = '\0';

	/* READ_ONLY file ? */
	if (file_attrs)
		if (!S_ISREG(file_attrs)  ||  !(file_attrs & S_IWRITE))
			r = 6;

	w = (right_margin > 80 ? 80 : right_margin) - left_margin;
	n = strlen(curfile);
	d = (file_type == 0) ? 0 : ((file_type == 1) ? 5 : 6);
	o = (n + d + r >= w) ? 0 : left_margin + (w - n - d - r)/2;

	if(o + n + d + r >= 70)
		n = 70 - o;
	memcpy(rbuff + o, curfile, n);
	if(file_type == 1)
		memcpy(rbuff + o + n, "(DOS)", 5);
	if(file_type == 2)
		memcpy(rbuff + o + n, "(QNX2)", 6);
	if (r)
		memcpy(rbuff + o + n + d, " (R/O)", 6);

	rbuff[left_margin - 1] = LMARG_MARK;
	rbuff[right_margin - 1] = RMARG_MARK;
#ifndef __STDC__
	if(get_account_flags(tbuff, 0xffff))
		memcpy(rbuff, tbuff, strlen(tbuff));
#endif
	disp_line_image(1, rbuff, memory_mapped ? attributes[CMD_AREA]
											: INVERSE_ATTR, 0);
	}


/***************************************************************************
 *                                                                         *
 *  HERE STARTS THE VERY DISPLAY SPECIFIC ROUTINES                         *
 *                                                                         *
 ***************************************************************************/
 


/*
 * This routine displays one entire line image
 * It is a fine example of dangerous assumptions. It makes certain
 * variables of type register knowing what registers they will be
 * put in the 8088. Then based upon that fact repeat store instructions
 * are used. Its fast but not the least bit portable.
 */
#if defined(__WATCOMC__)
#pragma off(unreferenced);
#endif 
void
disp_line_image(int row, char *text, unsigned attr, char lflags)
#if defined(__WATCOMC__)
#pragma on(unreferenced);
#endif 
	{
	char *sp, *tp;					/* sp in reg di  and  tp in si */
	int col, last_col, left_col;	/* col in reg dx and last_col in cx */
	char dbuff[133], c;


	unbreakable();
	tp = text;
	left_col = (row > 1  ||  opt.state != CMD) ? screen_col : 1;
/*	last_col = left_col + screen_width;
	// Crashed when screen_width > 132 -- wojtek
*/
	last_col = left_col + imin( 132, screen_width );

	sp = dbuff;

/*	sp_save = sp;	// This variable isn't used -- wojtek */

	if(tab_len == 0) tab_len = 0x03;
	null_char = opt.opt_b ? (memory_mapped ? NULL_CHAR : '.' ) : ' ';
	tab_char  = opt.opt_t ? (memory_mapped ? TAB_CHAR  : '>' ) : ' ';
	end_char = !(memory_mapped  ||  term_state.qnx_term) ? null_char
				: lflags & PARA_FLAG ? PARA_CHAR 
						: lflags & CONT_FLAG ? CONT_CHAR
											 : lflags & OVER_FLAG ? OVER_CHAR
																  : null_char;
	for(col = 1 ; *tp  &&  col < left_col ; ++col)
		if(*tp++ == '\t')
			while(col & tab_len)
				if(++col >= left_col  &&  col < last_col) {
/*
					asm("mov al,<null_char>");
					asm("stob");
 */
					*sp++ = null_char;
					}

	for(col = imax(col, left_col) ; *tp  &&  col < last_col ; ++col) {
/*
		asm("lodb");
		asm("cmp al,#09");
		asm("je TAB1");
			asm("stob");
			continue;
		asm("TAB1:");
			asm("mov al,<tab_char>");
			asm("stob");
 */
		if((c = *tp++) != '\t')
			{
			*sp++ = c;
			continue;
			}

		*sp++ = tab_char;

		while((col & tab_len)  &&  col < last_col) {
			++col;
/*
			asm("mov al,<null_char>");
			asm("stob");
 */
			*sp++ = null_char;
			}
		}

	if((last_col -= col) > 0) {
/*
		asm("mov al,<end_char>");
		asm("stob");
 */
		*sp++ = end_char;
		if(--last_col) {
/*
			asm("mov al,<null_char>");
			asm("rep stob");
 */
			while(last_col-- != 0)
				*sp++ = null_char;
			}
		}


	if(row > 1  &&   opt.opt_l  &&  (attr & INVERSE_ATTR)) {		/* limit inverse video to limits */

		last_col =	imin(left_col + screen_width - 1, limit2) -
					(col = imax(left_col, limit1)) + 1;

		if(col > left_col)
			/*
			 *	Display the portion to the left of the tagged block
			 *	(if there is any on the screen), ...
			 */
			put_term_line(row, 0, dbuff, col - left_col, attr ^ INVERSE_ATTR);
		if(last_col > 0)
			/*
			 *	... then display any tagged block on the screen, ...
			 */
			put_term_line(row, col - left_col, dbuff + col - left_col,
				last_col, attr);
		if(left_col + screen_width - 1 > limit2)
			/*
			 * ... then display anything to the right of the block.
			 */
			put_term_line(row, col - left_col + imax(0, last_col),
				dbuff + col - left_col + imax(0, last_col),
				screen_width - (col - left_col + imax(0, last_col)),
				attr ^ INVERSE_ATTR);
		}
	else
		put_term_line(row, 0, dbuff, screen_width, attr);

	line_attr[row] = attr;
	breakable();
	}


/*
 * This routine puts the character or attribute in 'c' at 'location'.
 * This routine is only used for
 * displaying the very top line of the screen (options and stats).
 */
void
put_screen(unsigned row, unsigned col, unsigned c, unsigned attr)
	{
	put_term_char(row, col, c, attr);
	}



/*
 * This routine moves the active cursor
 */
void
locate_cursor(char row, char col)
	{
	term_cur(row, col);
	}


/*
 * This routine is called every time we need an input character from
 * the keyboard. The one exception is the carriage return required
 * to clear an error. It calls getchar() directly.
 */

int get_term_char() {
	unsigned c;
	static int x_off = 0;
	static int y_off = 0;

	for(;;) {
		/* Process residual mouse movements before getting more	input	*/
		if ( x_off > 0 ) {
			--x_off;
			return( translate_key( K_RIGHT ) );
			}
		if ( x_off < 0 ) {
			++x_off;
			return( translate_key( K_LEFT ) );
			}

		if ( y_off > 0 ) {
			--y_off;
			return( translate_key( K_UP ) );
			break;
			}
		if ( y_off < 0 ) {
			++y_off;
			return( translate_key( K_DOWN ) );
			}

		c = term_key();

		if ( c == K_RESIZE ) {
			windows_size();
			continue;
			}

		if ( (c & K_CLASS) != K_MOUSE_EVENT ) {		/* Not a mouse event	*/
			return( c > 0x00ff ? translate_key( c ) : c );
			}

		if ( c & K_MOUSE_XMASK ) {					/* X mouse movement		*/
			if ( c & K_MOUSE_BLEFT ) {				/* Drag operation		*/
				if ( c & K_MOUSE_XMASK == 0 )
					x_off = 0;
				else
					x_off = 1 << ( ((c & K_MOUSE_XMASK) >> 4)-1 );
				if ( c & K_MOUSE_XDIR ) x_off = -x_off;
				}
			}

		if ( c & K_MOUSE_YMASK ) {					/* Y mouse movement		*/
			if ( c & K_MOUSE_BLEFT ) {				/* Drag operation		*/
				if ( c & K_MOUSE_YMASK == 0 )
					y_off = 0;
				else
					y_off = 1 << ( (c & K_MOUSE_YMASK)-1 );
				if ( c & K_MOUSE_YDIR ) y_off = -y_off;
				y_off /= 4;
    			}
			}

		}		/* Of for(;;)	*/
	}


/*
 * This routine is called right after the editor loads
 * You may have to stuff attributes	[STATUS_AREA]
									[CMD_AREA]
									[TEXT_AREA]
 */

void
device_setup() {
	register char *p;

	devno = 0;		/* Stdin	*/

	macro_disable_char = MACRO_DISABLE_CHAR;
	attributes[STATUS_AREA] = NORM_ATTR;
	attributes[CMD_AREA]    = UNDERLINE_ATTR;
	attributes[TEXT_AREA]   = NORM_ATTR;

	macro_file = "/usr/lib/ed.macros";

	if ( p = getenv("EDMACROS") ) {
		macro_file = malloc( strlen( p ) + 1 );
		strcpy( macro_file, p );
		}

	if(term_load() < 0) {
		fprintf(stderr,"TERM environment variable not set or no term file for it.\n");
		exit( -1 );
		}
	startup_fill = term_state.fill;

/*	term_mouse_on();	*/
	term_resize_on();
	ins_line_supported = !term_insert_line( 0, 0 );
	del_line_supported = !term_delete_line( 0, 0 );

	set_memory_mapped();
	#if 1 /* We're going to reallocate when resized -- wojtek, 24 Nov 95 */
		screen_height = term_state.num_rows;
		screen_width  = imin( 132, term_state.num_cols );
		init_terminal();
	#else

		if ( memory_mapped ) {
			/* Allocate space for maximum screen size	*/
		#if 1
			screen_height = imax( 50, (unsigned)term_state.num_rows);
			screen_width  = imax( 80, (unsigned)term_state.num_cols);
		#else
			screen_height = 50;
			screen_width = 80;
		#endif
			init_terminal();
			windows_size();
			}
		else {
			screen_height = imin( 66, (unsigned)term_state.num_rows);
			screen_width  = imin(132, (unsigned)term_state.num_cols);
			init_terminal();
			}
	#endif
	}


/*
 * This is called on each keyboard BREAK.
 * The main body of the editor does a lot of house cleaning but
 * a call to a user function is provided perhaps to cleanup
 * any input/output state machine.
 */
void
brk1(){
	;
	}

void
clear_screen1(int supress) {
	register int i;

	for(i = 0 ; i <= 66 ; ++i)
		line_attr[i] = 0x7fff;

	if(supress == 0)
		clear_term_screen();
	}

void
setdown() {
	if(memory_mapped) {
		term_fill(startup_fill);
		term_clear(3);
		}
	term_resize_off();
	term_restore();
	term_clear(3);
	term_flush();
/*	set_term_attr(NORM_ATTR);	@@@	*/
	}

void
#if defined(__WATCOMC__)
#pragma off(unreferenced);
#endif
zap_func(int func_num)
#if defined(__WATCOMC__)
#pragma on(unreferenced);
#endif
	{
	}

/*
 * A few additions to give user more control over the display of the screen
 */


void
put_stat(int col, char *str, int width)
	{
	register int n;

	n = strlen(str);

	/*
	 *	Don't update the column counter unless you are in command mode.
	 */
	if(col == 16  &&  width == 5  &&  opt.state != CMD  &&  !clr_flag
#ifndef __QNXNTO__
													&& dev_ischars(0) != 0 )
#else
													&& tcischars(0) != 0 )
#endif
		return;

	put_term_line(0, col, str, n, attributes[STATUS_AREA]);
	if(n < width)
		put_term_line(0, col+n, "        ", width-n, attributes[STATUS_AREA]);
	}

void
put_option(int opt_index, char value)
	{
	register int c, col;

	c = (int) opt_chars[opt_index];
	col = 22 + 3*opt_index;

	put_screen(0, col, c, attributes[STATUS_AREA]);
	put_screen(0, ++col, value ? '+' : '-', value && (c=='i' || c=='n' || c=='l')
						? HIGHLIGHT_BLINK_ATTR | attributes[STATUS_AREA]
						: attributes[STATUS_AREA]);
	put_screen(0, ++col, ' ', attributes[STATUS_AREA]);
	}

void
windows_size() {

	/* realloc the es buffer - added by wojtek, 24 Nov 95 */
	extern char *es;
	char *newes;
	enum { MINSSIZE = 3 * 80 };
	size_t ssize; unsigned cols;
	if ( ( cols = term_state.num_cols ) > 132 )
		/* Don't support more than 132 columns */
		cols = 132;
	if ( ( ssize = cols * term_state.num_rows ) < MINSSIZE )
		/* Make it at least large enough to hold 3 lines and 80 cols */
		ssize = MINSSIZE;
	if ( ( newes = realloc( es, ssize * 2 ) ) != NULL ) {
		/* If we don't have enough memory, let's keep previous size */
		es = newes;
		/* End of my additions - wojtek */
		screen_height = term_state.num_rows;
		screen_width = cols;
		clear_screen1(0);
		clr_flag = 1;
		mark_line(0);
		firstp->lflags |= DIRTY_FLAG;
		update_screen();
	}	}

