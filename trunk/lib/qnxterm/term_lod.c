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




#undef __INLINE_FUNCTIONS__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/qnxterm.h>
#include <errno.h>
#ifndef __QNXNTO__
#include <sys/osinfo.h>
#include <sys/magic.h>
#include <sys/dev.h>
#if 1 /* this should be removed later on, see note below with console_query */
	#include <sys/con_msg.h>
	#include <sys/kernel.h>
#endif
#endif
#include <sys/term.h>
#include <termios.h>

#define t	term_state
#define ct	__cur_term

struct t t;

#ifdef __QNXNTO__


static void __set(tcflag_t *ptr, int flag, unsigned set) {

	if(set)
		*ptr |= flag;
	else
		*ptr &= ~flag;
	}


#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_OSFLOW 0x0010
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST|_DEV_OSFLOW)

static unsigned dev_mode(int fd, unsigned mode, unsigned mask) {
	struct termios tios;
	unsigned omode = 0;

	if(tcgetattr(fd, &tios) == -1)
		return(-1);

	// Convert current flags into a mode
	if(tios.c_lflag & ECHO)   omode |= _DEV_ECHO;
	if(tios.c_lflag & ICANON) omode |= _DEV_EDIT;
	if(tios.c_lflag & ISIG)   omode |= _DEV_ISIG;
	if(tios.c_oflag & OPOST)  omode |= _DEV_OPOST;

	// Modify selected flags
	if(mask & _DEV_ECHO)  __set(&tios.c_lflag, ECHO|ECHOE|ECHOK|ECHONL, mode & _DEV_ECHO);
	if(mask & _DEV_EDIT)  __set(&tios.c_lflag, ICANON|IEXTEN, mode & _DEV_EDIT);
	if(mask & _DEV_EDIT)  __set(&tios.c_iflag, ICRNL,  mode & _DEV_EDIT);
	if(mask & _DEV_ISIG)  __set(&tios.c_lflag, ISIG,   mode & _DEV_ISIG);
	if(mask & _DEV_OPOST) __set(&tios.c_oflag, OPOST|ONLCR, mode & _DEV_OPOST);

	if(tcsetattr(fd, TCSANOW, &tios) == -1)
		return(-1);

	return(omode);
	}
#endif

void
term_init() {
	unsigned save_color = t.color, save_fill = t.fill;

	tcdrain( ct->Filedes ); /* Flush all queued output before changing things	*/
	__putp( init_1string );		/* Init the terminal					*/
	__putp( enter_ca_mode );	/* Enable cursor addressing				*/
	__putp( ena_acs );			/* Enable char sets for line drawing	*/

	/* Disable auto_margin if we can	*/
	if ( (t.save_am_mode || auto_right_margin)  &&  *exit_am_mode ) {
		__putp( exit_am_mode );
		t.save_am_mode = 1;
		auto_right_margin = 0;
		}

#if 0
	Allow ^c to go through to the application
	t.init_mode = dev_mode( ct->_inputfd, ct->_cc ? 0 : _DEV_ISIG, _DEV_MODES );
#else
	t.init_mode = dev_mode( ct->_inputfd, 0, _DEV_ECHO|_DEV_OPOST|_DEV_EDIT );
#endif
	
	ct->_cproxy_armed = ct->_dproxy_armed = ct->_mproxy_armed = 0;

	__keypad( 0, 1 );

	/* Clear cached control values so the term routines will re-assert them	*/
	if ( !ct->_cc ) t.col = t.row = 12345;
	t.old_attr = 0xff;
	t.color = t.fill = 0;
	t.region1 = t.num_rows;
	t.region2 = -1;
	term_color( save_color );
	term_fill( save_fill );
#ifndef __QNXNTO__
	if ( ct->_cc  &&  t.num_rows_save ) {
		console_size( ct->_cc, 0, t.num_rows, t.num_cols, NULL, NULL );
/*
		term_resize_on();
*/
		}
#endif
	}

void
term_restore() {
#ifndef __QNXNTO__
	if ( ct->_cc  &&  t.num_rows_save ) {
		console_size( ct->_cc, 0, t.num_rows_save, t.num_cols_save, NULL,NULL );
/*
		term_resize_off();
*/
		}
#endif
	term_flush();
	fflush( ct->_outputfp );
	__putp( exit_attribute_mode );
	__putp( exit_ca_mode );
	__putp( reset_1string );
	term_flush();
	fflush( ct->_outputfp );
	tcdrain( ct->Filedes );	/* Finish all output before we change post-processing	*/
	__resetty();
	}

int
term_load() {
	/* The term_setup function will support more options than term_load
	video_on
	snap screen on startup
	*/
	return( term_setup() );
	}


#ifndef __QNXNTO__
/* @@@	something like this should be in console.c, but since it isn't it is
		here. When a real one becomes available, this should be removed
*/
static int
_console_query_screentype(struct _console_ctrl *cc) {
        union _console_msg {
                struct _console_query          query;
                struct _console_query_reply    query_reply;
                } msg;

        msg.query.type = _CONSOLE_QUERY;
        msg.query.handle = cc->handle;
        msg.query.console = 0;
        msg.query.zero[0] = 0;
        msg.query.zero[1] = 0;

        if(Send(cc->driver, &msg, &msg, sizeof(msg.query), sizeof(msg.query_reply)) == -1) {
                return(-1);
                }

        if(msg.query_reply.status != 0) {
                errno = msg.query_reply.status;
                return(-1);
                }

        return(msg.query_reply.screen_type);
        }
#endif

int
term_setup() {
	int rc;
#ifndef __QNXNTO__
	unsigned char attr;
#endif
	register char *p, *p1;
	static char def_chars[] = "|-+++++++++#:_-+'#o<>v^##";
	static char qnx_chars[] = "³ÄÚ¿ÀÙÂÁÃ´ÅÛ±Üßøñú\036²è";

	if ( ct || t.terminfo ) {
		errno = EBUSY;
		return( -1 );
		}

	__setupterm( NULL, fileno ( stdout ), &rc );
	if ( rc != 1 ) return( -1 );		/* Problems...	*/

	/* Initialize term_state structure	*/
	strcpy( t.name, ct->_termname );
	t.version = TERM_VERSION;
	t.cache_attr = t.cache_pos = 1;
#ifndef __QNXNTO__
	t.scrbuf = NULL;						/* Console video buffer			*/
#endif
	t.attrbuf = NULL;						/* Terminal attribute buffer	*/
	t.num_rows = lines;
	t.num_cols = columns;
	t.line_amount	=	columns * 2;		/* # of bytes in a line		*/
	t.screen_amount	=	columns * lines * 2;/* # of bytes in a screen	*/
	t.color = t.fill = 0x0700;
#if 0
	if ( !strcmp ( t.name, "qnx" ) && !has_print_wheel ) {
		hard_copy = 0;
		has_print_wheel = 1;
		col_addr_glitch = 1;
		/* micro_row_address = "\033!%p1%02d"; */
	}
#endif
/* @@@ hc@, daisy == QNX extentions to terminfo */
	if ( !hard_copy && has_print_wheel )
		t.qnx_term = 1;
	t.is_mono = !__has_colors();		/* Learn if monochrome	*/
/* @@@ cr_cancels_micro_mode == don't do console-type calls */
#ifndef __QNXNTO__
	if ( t.qnx_term && !cr_cancels_micro_mode ) {
		if ( ct->_cc = console_open( ct->Filedes, O_RDWR ) ) {
			if ( (t.scrbuf = malloc( t.screen_amount )) == NULL ) {
				errno = ENOMEM;
				return( -1 );
				}
			if ( term_relearn_size() == -1 ) return( -1 );
			if(		console_read( ct->_cc, 0, 0, t.scrbuf, t.screen_amount,
					&t.row, &t.col, NULL) == -1 ||
					console_write( ct->_cc, 0, 0, t.scrbuf, 2,
					NULL, NULL, NULL) == -1) {
				free( t.scrbuf );
				t.scrbuf = NULL;
				console_close( ct->_cc );
				ct->_cc = NULL;
				}
			else {
				t.is_mono = _console_query_screentype( ct->_cc ) & 0x01;
				if ( !t.is_mono ) {	/* Learn initial colors	*/
					if ( t.col )
						rc = t.row * t.num_cols + t.col - 1;
					else
						rc = ( max( 1, t.row ) - 1 ) * t.num_cols;
					attr = t.scrbuf[rc * 2 + 1];
					t.fill = t.color = ((attr << 8) & 0x7700) | 0x8000;
					}
				}
			}
		}
#endif

	if ( ct->_tty ) {
		t.region1 = t.num_rows;		/* Min/max y values for screen update	*/
		t.region2 = -1;
		if ( term_relearn_size() == -1 ) return( -1 );
		}
	
	memcpy(&t.box_vertical, def_chars, sizeof(def_chars)-1);
	if(ct->_cc)
		memcpy(&t.box_vertical, qnx_chars, sizeof(qnx_chars)-1);
	else {
		if(t.qnx_term && col_addr_glitch) {
			for(p = qnx_chars, p1 = &t.box_vertical;
						p < qnx_chars + sizeof(qnx_chars)-1; p++, p1++) {
				if(*p >= ' ') *p1 = *p;
				}
			}
		p = acs_chars;
		if ( *p ) {
			if ( strlen( p ) & 1 ) {			/* Must be even number of chars	*/
				fprintf( stderr, "Bad acsc string.\n" );
				return( -1 );
				}
			/* Extract line drawing characters into something we can use	*/
			while( rc = (int) *p++ ) {
				if( *p == '\0' )
					break;
				switch ( rc ) {
					case 'O':	t.box_solid_block = *p;		break;
					case 'a':	t.box_shade_block = *p;		break;
					case 'j':	t.box_bot_right = *p;		break;
					case 'k':	t.box_top_right = *p;		break;
					case 'l':	t.box_top_left = *p;		break;
					case 'm':	t.box_bot_left = *p;		break;
					case 'n':	t.box_cross = *p;			break;
					case 't':	t.box_left_tee = *p;		break;
					case 'u':	t.box_right_tee = *p;		break;
					case 'v':	t.box_bot_tee = *p;			break;
					case 'w':	t.box_top_tee = *p;			break;
					case 'q':	t.box_horizontal = *p;		break;
					case 'x':	t.box_vertical = *p;		break;
					case 'o':	t.box_top_solid_block = *p;	break;
					case 's':	t.box_bot_solid_block = *p;	break;
					case 'f':	t.box_degree = *p;			break;
					case 'g':	t.box_plus_minus = *p;		break;
					case '~':	t.box_bullet = *p;			break;
					case 'h':	t.box_board = *p;			break;
					case 'i':	t.box_lantern = *p;			break;
					case '`':	t.box_diamond = *p;			break;
					case ',':	t.box_arrow_left = *p;		break;
					case '+':	t.box_arrow_right = *p;		break;
					case '.':	t.box_arrow_down = *p;		break;
					case '-':	t.box_arrow_up = *p;		break;
					}
				++p;
				}
			}
		}
	
	/* Precompute cost of various motions									*/
	/* Assumptions:															*/
	/* 	Single motion is cheaper than parameterized motion					*/
	/*	Any terminal with parameterized positioning also					*/
	/*	has single motion ( parameter can be "precast" into defined string	*/
	/*	Bug: The strlen does not take into account padding characters		*/
	/*		so optimimum cost calculations only work for no-padding devices	*/
	t.cost_move		= strlen( __tparm( cursor_address, 10, 10 ) );
	t.cost_down		= 1;		/* Line feed */
	t.cost_left		= 1;		/* Backspace */
	t.cost_up		= *cursor_up ? strlen( cursor_up ) : 1000;
	t.cost_right	= *cursor_right ? strlen( cursor_right ) : 1000;
	/* Assume single digit motion vertically	*/
	t.cost_up_p		= *parm_up_cursor ? strlen( __tparm( parm_up_cursor, 1 ) ) : 1000;
	t.cost_down_p	= *parm_down_cursor ? strlen( __tparm( parm_down_cursor, 1 ) ) : 1000;
	/* Assume double digit motion horizontally	*/
	t.cost_left_p	= *parm_left_cursor ? strlen( __tparm( parm_left_cursor, 10 ) ) : 1000;
	t.cost_right_p	= *parm_right_cursor ? strlen( __tparm( parm_right_cursor, 10 ) ) : 1000;

	__savetty();
	term_init();
	atexit( &term_restore );
	/*
	Set a pointer into magic memory so that a ditto process can "see"
	what happens on a serial terminal and initialize the necessary
	variables to allow another process to walk through the term and terminfo
	structures.
	*/
	term_state.ptr_size = sizeof( char * );
	term_state.int_size = sizeof( int );
	term_state.terminfo = (void *) ct;
#ifndef __QNXNTO__
#ifdef __386__
	__MAGIC.termst   = &term_state;
#else
	__MAGIC.sptrs[5] = &term_state;
#endif	
#endif
	return( 0 );
	}

#ifndef __QNXNTO__
void
__term_clear_buffer( row )
	int	row;
	{
	register int amount;
	register char *p;

	if ( !t.scrbuf ) return;
	if ( !(amount = ((lines - row) * t.line_amount)/2) ) return;
	p = t.scrbuf + (t.line_amount * row);
	__memsetw( p, ' ' | (t.fill & 0x7700), amount );
	if ( row < t.region1 ) t.region1 = row;
	t.region2 = t.num_rows - 1;
	if ( p = t.attrbuf ) {
		p += t.num_cols * row;
		memset( p, 0, amount );
		}
	}
#endif

void
term_flush() {
#ifndef __QNXNTO__
	if(t.mouse_cursor) {
		term_mouse_hide();
		t.mouse_cursor = 1;
		}
	if ( ct->_cc ) {
		if ( t.region1 != t.num_rows  ||  t.region2 != -1 ) {
			console_write( ct->_cc, 0, t.region1 * t.line_amount,
				t.scrbuf + t.region1 * t.line_amount,
				(t.region2 - t.region1 + 1) * t.line_amount,
				&t.row, &t.col, NULL );
			t.region1 = t.num_rows;
			t.region2 = -1;
			}
		}
	else
#endif
		fflush( ct->_outputfp );
#ifndef __QNXNTO__
	if(t.mouse_cursor) {
		t.mouse_cursor = 0;
		term_mouse_move(-1, -1);
		}
#endif
	}

#ifdef __386__
/* This has been replaced with an inline pragma inside sys/qnxterm.h	*/
void *
__memsetw( void *s, unsigned c, size_t length ) {
	register unsigned short *s1 = (unsigned short *) s;

	/* @@@ Define assembly pragma for this!	*/
	while( length-- ) *s1++ = (unsigned short)c;
	return( s );
	}
#endif

#ifndef __STDC__
void *
__memsetw( s, c, length )
	char *s;
	int c, length;
	{
	memsetw( s, c, length );
	}
#endif

#ifndef __QNXNTO__
/*---------------------------------------------*/
#include <string.h>
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/dev_msg.h>
#include <errno.h>

static
int dev_size(fd, set_rows, set_columns, r, c)
int fd;
int set_rows, set_columns;
int *r, *c;
        {
        union _dev_msg {
                struct _dev_size         size;
                struct _dev_size_reply   size_reply;
                } msg;

		memset( &msg, 0, sizeof(msg.size) );
        msg.size.type =    _DEV_SIZE;
        msg.size.fd =      fd; 
        msg.size.rows =    set_rows;
        msg.size.cols =    set_columns;

        if(Sendfd(fd, &msg, &msg, sizeof(msg.size), sizeof(msg.size_reply)) == -1) {
                errno = EINVAL;
                return(-1);
                }

        if(msg.size_reply.status) {
			errno = msg.size_reply.status;
			return(-1);
			}

		if(r)    *r =    msg.size_reply.oldrows;
		if(c) *c = msg.size_reply.oldcols;

	    return(0);
        }
/*---------------------------------------------*/
#endif

/*---
	On a screen resize, this routine will reset the variables inside the
	term and terminfo routines to the appropriate values. The screen buffer
	will also be reallocated and cleared to the previously specified fill
	color. It is up to the application to put something reasonable on the
	screen after the resize. Any data that was present would have been
	cleared.
---*/
int
term_relearn_size() {
	char newlines[10];
	int changed = 0;
	int rows = -1, cols = -1;

#ifdef __QNXNTO__
	tcgetsize( ct->Filedes, &rows, &cols);
	if(rows > 0 && cols > 0) { 
		t.num_rows = rows;
		t.num_cols = cols;
		}
#else
	dev_size( ct->Filedes, -1, -1, &rows, &cols );
	if(rows > 0 && cols > 0) { 
		t.num_rows = rows;
		t.num_cols = cols;
		}
	else if ( ct->_cc )
		console_size( ct->_cc, 0, 0, 0, &t.num_rows, &t.num_cols );
#endif
	if ( lines != t.num_rows ) {
		itoa( lines = t.num_rows, newlines, 10 );
		setenv( "LINES", newlines, 1 );
		changed = 1;
		}
	if ( columns != t.num_cols ) {
		itoa( columns = t.num_cols, newlines, 10 );
		setenv( "COLUMNS", newlines, 1 );
		changed = 1;
		}
	/*
	Since term_flush was called within term_key, the region1/region2
	variables were already cleared. Since the screen length has changed,
	we must set them back to indicate a cleared status.
	*/
	t.region1 = t.num_rows;
	t.region2 = -1;

#ifndef __QNXNTO__
	t.line_amount	=	columns * 2;		/* # of bytes in a line		*/
	t.screen_amount	=	columns * lines * 2;/* # of bytes in a screen	*/

	if ( changed  &&  t.scrbuf ) {
		if ( (t.scrbuf = realloc( t.scrbuf, t.screen_amount )) == NULL ) {
			errno = ENOMEM;
			return( -1 );
			}
		if ( t.attrbuf ) {
			if ( (t.attrbuf = realloc(t.attrbuf, t.screen_amount/2)) == NULL ) {
				errno = ENOMEM;
				return( -1 );
				}
			}
		__memsetw( t.scrbuf, (t.fill & 0x7700) | ' ', t.screen_amount/2 );
		}
#endif
	return( changed );
	}

#ifdef __QNXNTO__
void *__memsetw( void *s, unsigned c, size_t n ) {
	short unsigned *up = s, u = c;

	while(n) {
		*up++ = u;
		--n;
	}

	return(s);
}
#endif
