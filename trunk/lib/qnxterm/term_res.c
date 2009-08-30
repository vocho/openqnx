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
#include <sys/term.h>
#include <signal.h>
#include <errno.h>
#include <sys/qnxterm.h>
#ifndef __QNXNTO__
#include <sys/proxy.h>
#endif

#define t	term_state
#define ct	__cur_term

static void __term_sigwinch(int sig) {
	if(sig == SIGWINCH) {
		ct->_window_changed = 1;
	}
}

int
term_resize_on() {
	if ( ct->_input_winch ) {
		void (*old_handler)(int);

		old_handler = signal(SIGWINCH, __term_sigwinch);
		/*
		 * if someone else registered to get SIGWINCH, leave it
		 * alone, and do the proxy stuff
		 */
		if(old_handler == SIG_DFL || old_handler == SIG_IGN)
			return(0);
		ct->_input_winch = 0;
		signal(SIGWINCH, old_handler);
		}

	if ( !ct->_cc ) {
		char *p1, *p2;

/* @@@ change_res_horz	==	QNX/Xterm Mouse Prefix 1 */
		p1 = change_res_horz;
/* @@@ change_res_vert	==	QNX/Xterm Mouse Prefix 2 */
		p2 = change_res_vert;
/* @@@ max_micro_address		madde#	Yd#	Click Messages 1=QNX, 2=Xterm */
/* @@@ enter_micro_mode == send resize events */
/* @@@ exit_micro_mode == stop sending resize events */
		if ( t.qnx_term && *enter_micro_mode && *exit_micro_mode &&
							max_micro_address != -1 && (*p1 || *p2)) {
			if ( ct->_win_key_scan == NULL ) {
				ct->_win_key_scan = &term_window_scan;
				if ( *p1 ) {
					t.save_keystart1 = ct->keystart[ *p1 >> 4 ];
					ct->keystart[ *p1 >> 4 ] |= (1 << (*p1 & 0x0f) );
					}
				if ( *p2 ) {
					t.save_keystart2 = ct->keystart[ *p2 >> 4 ];
					ct->keystart[ *p2 >> 4 ] |= (1 << (*p2 & 0x0f) );
					}
				}
			
/* @@@ enter_micro_mode == send resize events */
			__putp( enter_micro_mode );
			t.scan_resize = 1;
			return(0);
			}
		return( -1 );
		}

#ifndef __QNXNTO__
	/* Create resize proxy	*/
	if ( !ct->_cproxy ) {
		if ( (ct->_cproxy = qnx_proxy_attach( 0, 0, 0, -1 )) == -1 ) {
			ct->_cproxy = 0;
			return( -1 );
			}
		ct->_cproxyr = qnx_proxy_rem_attach( ct->_outputfd_nid, ct->_cproxy );
		if ( ct->_cproxyr == -1 ) {
			qnx_proxy_detach( ct->_cproxy );
			ct->_cproxy = ct->_cproxyr = 0;
			return( -1 );
			}
		}

	/* We also need a keyboard read proxy now	*/
	if ( !ct->_dproxy ) {
		if ( (ct->_dproxy = qnx_proxy_attach( 0, 0, 0, -1 )) == -1 ) {
			qnx_proxy_detach( ct->_cproxy );
			qnx_proxy_rem_detach( ct->_outputfd_nid, ct->_cproxyr );
			ct->_cproxy = ct->_cproxyr = ct->_dproxy = 0;
			return( -1 );
			}
		ct->_dproxyr = qnx_proxy_rem_attach( ct->_inputfd_nid, ct->_dproxy );
		if ( ct->_dproxyr == -1 ) {
			qnx_proxy_detach( ct->_cproxy );
			qnx_proxy_rem_detach( ct->_outputfd_nid, ct->_cproxyr );
			qnx_proxy_detach( ct->_dproxy );
			ct->_cproxy = ct->_cproxyr = ct->_dproxy = ct->_dproxyr = 0;
			return( -1 );
			}
		}

	console_size( ct->_cc, 0, 0, 0, &t.num_rows_save, &t.num_cols_save );
	console_state( ct->_cc, 0, 0, _CON_EVENT_SIZE );
	console_arm( ct->_cc, 0, ct->_cproxyr, _CON_EVENT_SIZE );
	ct->_cproxy_armed = 1;
#endif
	return(0);
	}

int
term_resize_off() {
	if ( ct->_input_winch ) {
		signal(SIGWINCH, SIG_DFL);
		return(0);
	}
#ifndef __QNXNTO__
	if ( ct->_cproxy ) {
		console_arm( ct->_cc, 0, -1, 0 );
		qnx_proxy_detach( ct->_cproxy );
		qnx_proxy_rem_detach( ct->_outputfd_nid, ct->_cproxyr );
		ct->_cproxy = ct->_cproxyr = 0;
		return(0);
		}
	else
#endif
	if ( t.scan_resize ) {
		if ( !t.scan_mouse && ct->_win_key_scan ) {
			char *p1, *p2;

/* @@@ change_res_horz	==	QNX/Xterm Mouse Prefix 1 */
			p1 = change_res_horz;
/* @@@ change_res_vert	==	QNX/Xterm Mouse Prefix 2 */
			p2 = change_res_vert;
			ct->_win_key_scan = NULL;
			if ( *p1 ) ct->keystart[ *p1 >> 4 ] = t.save_keystart1;
			if ( *p2 ) ct->keystart[ *p2 >> 4 ] = t.save_keystart2;
			}
		__putp( exit_micro_mode );
		t.scan_resize = 0;
		return(0);
		}
	errno = EINVAL;
	return( -1 );
	}

