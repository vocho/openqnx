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





#ifndef __QNXNTO__
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/term.h>
#include <sys/qnxterm.h>
#include <sys/dev.h>
#include <sys/proxy.h>
#include <sys/console.h>
#include <signal.h>

#define t	term_state
#define ct	__cur_term

int
term_mouse_on() {
	char *p, *p2;
	unsigned i;
	struct _mouse_param param;

/* @@@ change_res_horz	==	QNX/Xterm Mouse Prefix 1 */
	p = change_res_horz;
/* @@@ change_res_vert	==	QNX/Xterm Mouse Prefix 2 */
	p2 = change_res_vert;
/* @@@ max_micro_address		madde#	Yd#	Click Messages 1=QNX, 2=Xterm */
	if ( t.qnx_term && max_micro_address != -1 && (*p || *p2)) {
		if ( ct->_win_key_scan == NULL ) {
			ct->_win_key_scan = &term_window_scan;
			if ( *p ) {
				t.save_keystart1 = ct->keystart[ *p >> 4 ];
				ct->keystart[ *p >> 4 ] |= (1 << (*p & 0x0f) );
				}
			if ( *p2 ) {
				t.save_keystart2 = ct->keystart[ *p2 >> 4 ];
				ct->keystart[ *p2 >> 4 ] |= (1 << (*p2 & 0x0f) );
				}
			}
		if ( !ct->_mouse_handler ) term_mouse_handler( term_mouse_default );
		t.scan_mouse = 1;
		if ( i = t.mouse_flags ) {
			t.mouse_flags = 0;
			term_mouse_flags( i, i );
			}
		return( 0 );
		}
	if ( !ct->_cc ) return( -1 );			/* Only on the console	*/
	if ( ct->_mproxy ) return( 0 );
	if ( (ct->_mm = mouse_open( ct->_inputfd_nid, getenv("TERM_MOUSE_NAME"),
			ct->_inputfd)) == NULL)
		return( -1 );	 	 /* No mouse present	*/

#if 1 /* this should be move/changed when touch screen support is added */
	if ( mouse_param( ct->_mm, 0, &param ) != -1 ) {
		if ( ( param.mode & _MSEFLG_TYPE_MASK ) != _MSEFLG_REL_ARB ) {
			mouse_close( ct->_mm );
			return( -1 );	/* Mouse type not relative */
			}
		}
#endif

	if ( (ct->_mproxy = qnx_proxy_attach( 0, 0, 0, -1 )) == -1 ) {
		ct->_mproxy = 0;
		return( -1 );
		}
	ct->_mproxyr = qnx_proxy_rem_attach( ct->_inputfd_nid, ct->_mproxy );
	if ( ct->_mproxyr == -1 ) {
		qnx_proxy_detach( ct->_mproxy );
		ct->_mproxy = ct->_mproxyr = 0;
		return( -1 );
		}

	/* We also need a keyboard read proxy now	*/
	if ( !ct->_dproxy ) {
		if ( (ct->_dproxy = qnx_proxy_attach( 0, 0, 0, -1 )) == -1 ) {
			qnx_proxy_detach( ct->_mproxy );
			qnx_proxy_rem_detach( ct->_inputfd_nid, ct->_mproxyr );
			ct->_mproxy = ct->_mproxyr = ct->_dproxy = 0;
			return( -1 );
			}
		ct->_dproxyr = qnx_proxy_rem_attach( ct->_inputfd_nid, ct->_dproxy );
		if ( ct->_dproxyr == -1 ) {
			qnx_proxy_detach( ct->_mproxy );
			qnx_proxy_rem_detach( ct->_inputfd_nid, ct->_mproxyr );
			qnx_proxy_detach( ct->_dproxy );
			ct->_mproxy = ct->_mproxyr = ct->_dproxy = ct->_dproxyr = 0;
			return( -1 );
			}
		ct->_dproxy_armed = 0;
		}

	mouse_flush( ct->_mm );
	ct->_mproxy_armed = 0;

	if ( !ct->_mouse_handler ) term_mouse_handler( term_mouse_default );
	t.mouse_click = 4;
	t.mouse_xskip = 3;
	t.mouse_yskip = 5;
	if ( p = getenv("TERM_MOUSE_CLICK") )  t.mouse_click = atoi(p);
	if ( p = getenv("TERM_MOUSE_REVERSE")) t.mouse_flags |= TERM_MOUSE_REVERSE;
	if ( p = getenv("TERM_MOUSE_XSKIP") )  t.mouse_xskip = atoi( p );
	if ( p = getenv("TERM_MOUSE_YSKIP") )  t.mouse_yskip = atoi( p );
	return( 0 );
	}

int
term_mouse_off() {
	unsigned i;

	if ( t.mouse_cursor ) term_mouse_hide();
	if ( t.scan_mouse ) {
		if ( !t.scan_resize && ct->_win_key_scan ) {
			char *p1, *p2;

/* @@@ change_res_horz	==	QNX/Xterm Mouse Prefix 1 */
			p1 = change_res_horz;
/* @@@ change_res_vert	==	QNX/Xterm Mouse Prefix 2 */
			p2 = change_res_vert;
			ct->_win_key_scan = NULL;
			if ( *p1 ) ct->keystart[ *p1 >> 4 ] = t.save_keystart1;
			if ( *p2 ) ct->keystart[ *p2 >> 4 ] = t.save_keystart2;
			}
		if ( i = t.mouse_flags ) {
			term_mouse_flags( i, 0 );
			t.mouse_flags = i;
			}
		t.scan_mouse = 0;
		}
	else if ( ct->_mproxy ) {
		qnx_proxy_detach( ct->_mproxy );
		qnx_proxy_rem_detach( ct->_inputfd_nid, ct->_mproxyr );
		ct->_mproxy = ct->_mproxyr = 0;
		mouse_close( ct->_mm );
		}
	else {
		errno = EINVAL;
		return( -1 );
		}
	return(0);
	}


static unsigned
flag( unsigned flags, unsigned oldflags, unsigned bits,
			char *stron, char *stroff ) {
	if ( ( stron && !*stron) || ( stroff && !*stroff ) ) {
		flags &= ~bits;
		}
	else {
		if ( ( flags ^ oldflags ) & bits ) {
			if ( flags & bits ) {
				if ( stron ) {
					__putp( __tparm( stron ) );
					}
				}
			else {
				if ( stroff ) {
					__putp( __tparm( stroff ) );
					}
				}
			}
		}
	return( flags );
	}
		

unsigned
term_mouse_flags( unsigned mask, unsigned bits ) {
	unsigned flags, saved;

	flags = t.mouse_flags;
	t.mouse_flags &= ~mask;
	t.mouse_flags |= bits & mask;
	saved = t.mouse_flags;

	if ( t.scan_mouse ) {
		saved = flag( saved, flags, 
			TERM_MOUSE_FOLLOW|TERM_MOUSE_HELD|TERM_MOUSE_MOVED,
			micro_left, micro_right );

		saved = flag( saved, flags, TERM_MOUSE_RELEASE, parm_up_micro, micro_up );

		saved = flag( saved, flags, 
			TERM_MOUSE_SELECT|TERM_MOUSE_ADJUST|TERM_MOUSE_MENU,
			(char *)0, micro_down );

		saved = flag( saved, flags, TERM_MOUSE_SELECT, parm_left_micro, (char *)0 );

		saved = flag( saved, flags, TERM_MOUSE_ADJUST, parm_down_micro, (char *)0 );

		saved = flag( saved, flags, TERM_MOUSE_MENU, parm_right_micro, (char *)0 );
		}

	t.mouse_flags = saved;

	return( flags );
	}

int
term_mouse_default( unsigned *key, struct mouse_event *me ) {
	if ( term_mouse_process( key, me ) ) return( 1 );
	term_mouse_move( -1, -1 );
	return( 0 );
	}

int
term_mouse_handler( int (*handler)(unsigned *key, struct mouse_event *) ) {
	if ( !handler && t.mouse_cursor )	term_mouse_hide();
	ct->_mouse_handler = handler;
	return( 1 );
	}

int
term_mouse_process( unsigned *ch, struct mouse_event *me ) {
	int dx, dy;
	unsigned button;
	unsigned c = *ch;
	unsigned i;
	static unsigned key;
	static long timestamp;
	static unsigned timekey;
	static int click;

	if(c == K_RESIZE) {
		if ( t.mouse_row >= t.num_rows )	t.mouse_row = t.num_rows-1;
		if ( t.mouse_col >= t.num_cols )	t.mouse_col = t.num_cols-1;
		}
	if(c != 0 && (c & K_CLASS) != K_MOUSE_EVENT) return( 0 );
	if ( c ) {
		if ( c & K_MOUSE_BUTTONS ) {
			if(me) ct->_mm->buttons |= me->buttons; /* make sure buttons are ok */
			i = ct->_mm->buttons & (_MOUSE_LEFT|_MOUSE_MIDDLE|_MOUSE_RIGHT);

			if ( i == _MOUSE_LEFT || i == _MOUSE_MIDDLE || i == _MOUSE_RIGHT ) {
				c &= ~K_MOUSE_BUTTONS;
				if ( ct->_ctrl_held )			c |= K_MOUSE_BMENU;
				else if ( ct->_shift_held )		c |= K_MOUSE_BADJUST;
				else 							c |= K_MOUSE_BSELECT;
				}
			else {
				if ( t.mouse_flags & TERM_MOUSE_REVERSE ) {
					key = c;
					c &= ~(K_MOUSE_BLEFT|K_MOUSE_BRIGHT);
					c |= (key & K_MOUSE_BLEFT) >> 2;
					c |= (key & K_MOUSE_BRIGHT) << 2;
					}
				if( (i & _MOUSE_MIDDLE) == 0 && ct->_shift_held ) {
					if(c & K_MOUSE_BSELECT) c |= K_MOUSE_BADJUST;
					c &= ~K_MOUSE_BSELECT;
					}
				}
			}
		key = c;
		}
	else 
		c = key;
	
	button = (c ^ t.mouse_old_buttons) & K_MOUSE_BUTTONS;

	if(c & (K_MOUSE_XMASK|K_MOUSE_YMASK)) {
		key &= ~(K_MOUSE_XMASK|K_MOUSE_YMASK);
		if(me) dx = me->dx;
		else {
			dx = (c & K_MOUSE_XMASK) >> 4;
			if ( dx ) dx = 1 << (dx-1);
			if ( c & K_MOUSE_XDIR ) dx = -dx;
			}
		t.mouse_dx += dx;
		while ( t.mouse_dx > t.mouse_xskip ) {
			t.mouse_col++;
			t.mouse_dx -= t.mouse_xskip + 1;
			}
		while ( t.mouse_dx < 0 ) {
			t.mouse_col--;
			t.mouse_dx += t.mouse_xskip + 1;
			}
		if ( t.mouse_col < 0 ) 					t.mouse_col = 0;
		else if ( t.mouse_col >= t.num_cols )	t.mouse_col = t.num_cols-1;

		if(me)	dy = me->dy;
		else {
			dy = (c & K_MOUSE_YMASK);
			if(dy) dy = 1 << (dy-1);
			if(c & K_MOUSE_YDIR) dy = -dy;
			}
		t.mouse_dy -= dy;
		while ( t.mouse_dy > t.mouse_yskip ) {
			t.mouse_row++;
			t.mouse_dy -= t.mouse_yskip + 1;
			}
		while ( t.mouse_dy < 0 ) {
			t.mouse_row--;
			t.mouse_dy += t.mouse_yskip + 1;
			}
		if ( t.mouse_row < 0 ) 					t.mouse_row = 0;
		else if ( t.mouse_row >= t.num_rows )	t.mouse_row = t.num_rows-1;
	
		if(	(	(t.mouse_flags & TERM_MOUSE_MOVED) ||
				(	(t.mouse_flags & TERM_MOUSE_HELD) &&
					(c & K_MOUSE_BUTTONS)	)	) &&
			(	(t.mouse_row != t.mouse_old_row) ||	
				(t.mouse_col != t.mouse_old_col)	)	) {
			*ch = c & K_MOUSE_BUTTONS | K_MOUSE_POS | K_MOUSE_CURSOR;
			click = timekey = 0;
			return button;
			}
		}

	if(	!me || 
		(	timekey && 
			(	me->timestamp - timestamp > t.mouse_click ||
				button != timekey) ) )
		click = timekey = 0;
	if ( me ) timestamp = me->timestamp;
	if ( click > 14 ) click = 14;
	if ( button ) {
		if ( button & K_MOUSE_BSELECT ) {
			t.mouse_old_buttons ^= K_MOUSE_BSELECT;
			if ( c & K_MOUSE_BSELECT ) {
				if(t.mouse_flags & TERM_MOUSE_SELECT) {
					timekey = K_MOUSE_BSELECT;
					*ch = K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BSELECT|++click<<4;
					return button & (K_MOUSE_BADJUST|K_MOUSE_BMENU);
					}
				}
			else {
				if ( t.mouse_flags & TERM_MOUSE_RELEASE ) {
					*ch = K_MOUSE_POS | K_MOUSE_RELEASE | K_MOUSE_BSELECT;
					return button & (K_MOUSE_BADJUST|K_MOUSE_BMENU);
					}
				}
			}
		if ( button & K_MOUSE_BADJUST ) {
			t.mouse_old_buttons ^= K_MOUSE_BADJUST;
			if(c & K_MOUSE_BADJUST) {
				if(t.mouse_flags & TERM_MOUSE_ADJUST) {
					timekey = K_MOUSE_BADJUST;
					*ch = K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BADJUST|++click<<4;
					return button & K_MOUSE_BMENU;
					}
				}
			else {
				if(t.mouse_flags & TERM_MOUSE_RELEASE) {
					*ch = K_MOUSE_POS | K_MOUSE_RELEASE | K_MOUSE_BADJUST;
					return button & K_MOUSE_BMENU;
					}
				}
			}
		if ( button & K_MOUSE_BMENU ) {
			t.mouse_old_buttons ^= K_MOUSE_BMENU;
			if(c & K_MOUSE_BMENU) {
				if(t.mouse_flags & TERM_MOUSE_MENU) {
					timekey = K_MOUSE_BMENU;
					*ch = K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BMENU|++click<<4;
					return( 0 );
					}
				}
			else {
				if(t.mouse_flags & TERM_MOUSE_RELEASE) {
					*ch = K_MOUSE_POS | K_MOUSE_RELEASE | K_MOUSE_BMENU;
					return( 0 );
					}
				}
			}
		}	
	*ch = 0;
	return( 0 );
	}
#endif
