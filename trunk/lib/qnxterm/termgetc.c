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
#include <unistd.h>
#include <string.h>
#include <sys/term.h>
#include <signal.h>
#include <errno.h>
#ifndef __QNXNTO__
#include <sys/qioctl.h>
#include <sys/qnxterm.h>
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/timers.h>
#endif

#define ct	__cur_term

/*-------------------------------------------------------------------------
Input modes:

	_DELAY		- block and wait for input
	_NODELAY	- return ERR if no input pending
	_HALFDELAY	- Wait until character or timeout expired
	keypad( win, TRUE ) - return tokens for function keys
	keypad( win, FALSE ) - return individual keystrokes

If a lead-in character for a function key is typed, it will only be returned
after a delay to see if the rest of the function key is also coming in.

Handled by tty driver:
cbreak() mode - input character by character
nocbreak() mode - input line by line
echo() / noecho() control echo  of typed input
nodelay( win, FALSE) - getch is a blocking call, _inpmode = _DELAY
nodelay( win, TRUE)  - getch becomes non-blocking, _inpmode = _NODELAY
halfdelay( tenths ) - Wait up to tenths / second, _inpmode = _HALFDELAY
notimeout( win, FALSE)- Timeout between function key chars, _kpdmode=_KPD_DELAY
notimeout( win, TRUE)  - Never timeout between chars, _kpdmode = _KPD_ON
keypad( win, FALSE) - Return each character as typed, _kpdmode = _KPD_OFF
keypad( win, TRUE ) - Return function keys tokens, _kpdmode = _KPD_ON
----------------------------------------------------------------------*/

/*
Table to map offsets into the __cur_term->_strs structure into terminfo
defined key codes. There are four contiguous regions in the terminfo
structure, and by some conditional offsets, this one tables maps all
four areas.
*/
static unsigned table[] = {
	/* _ky_backspace */
	0407, 0526, 0515, 0525, 0512, 0510, 0402, 0514, 0517, 0516,
	0410, 0411, 0422, 0412, 0413, 0414, 0415, 0416, 0417, 0420,
	0421, 0406, 0513, 0511, 0404, 0533, 0522, 0523, 0405, 0520,
	0521, 0524, 0403,

	/* _ky_a1 */
	0534, 0535, 0536, 0537, 0540,

	/* _ky_btab */
	0541,

	/* _ky_beg */
	0542,
	0543, 0544, 0545, 0546, 0547, 0550, 0527, 0551, 0552, 0553,
	0554, 0555, 0556, 0557, 0560, 0561, 0562, 0532, 0563, 0564,
	0565, 0566, 0567, 0570, 0571, 0627, 0630, 0572, 0573, 0574,
	0575, 0576, 0577, 0600, 0601, 0602, 0603, 0604, 0605, 0606,
	0607, 0610, 0611, 0612, 0613, 0614, 0615, 0616, 0617, 0620,
	0621, 0622, 0623, 0624, 0625, 0626,
	0000,
	0423, 0424, 0425, 0426,
	0427, 0430, 0431, 0432, 0433, 0434, 0435, 0436, 0437, 0440,
	0441, 0442, 0443, 0444, 0445, 0446, 0447, 0450, 0451, 0452,
	0453, 0454, 0455, 0456, 0457, 0460, 0461, 0462, 0463, 0464,
	0465, 0466, 0467, 0470, 0471, 0472, 0473, 0474, 0475, 0476,
	0477, 0500, 0501, 0502, 0503, 0504, 0505, 0506, 0507,
	};

/*-----------------------------------------------------------------------
IMPORT: first - Pointer to first integer offset of string to match
		last - Pointer to first integer offset of string to match
		buf - Pointer to string to matched against
		match - Number of characters to match.
EXPORT:	Pointer to integer offset of fully matching string.
		NULL - No full matches found.
		complete = 0 - No matches were found.
		complete = 1 - A partial match was found.
		complete = 2 - A full match was found.
------------------------------------------------------------------------*/
static short int *
key_scan(	first, last, buf, match, complete )
	short int *first, *last;
	char *buf;
	int match, *complete; 	
	{
	register unsigned char *c;

	while( first < last ) {
		c = &ct->_strtab[0] + *first;			/* Point to string to match	*/
		if ( !strncmp( c, buf, match ) ) {		/* A partial match ?		*/
			if ( !strcmp( c, buf ) ) {			/* Yes, A full match ?		*/
				*complete = 2;					/* Yes, we're done			*/
				return( first );
				}
			*complete = 1;
			}
		++first;								/* Try next string			*/
		}
	return( NULL );								/* No full matches			*/
	}

void
__key_timeout( int i ) {
	i = i; /* keep compiler quiet about unused variables */
	ct->_was_timeout = 1;
	}

#ifdef __QNXNTO__
int dev_read( int fd, void *buf, unsigned bytes,
                  unsigned minimum, unsigned itime, unsigned timeout,
                  pid_t proxy, int *triggered) {

	if(proxy  ||  triggered) {
		fprintf(stderr, "termgetc.c : Unsupported call to dev_read in Neutrino.\n");
		exit(1);
	}

	return(readcond(fd, buf, bytes, minimum, itime, timeout));
}
#endif

int
__getch() {
	register short int *first1;
	int i, complete;
	unsigned off1, off2, off3, offset = 0, n, event;
	unsigned char ch;
#ifndef __QNXNTO__
	struct mouse_event me;
	pid_t pid;
	struct itimerspec timer;
	struct itimercb timercb;

	int bitc;
	long bits[2];
#endif

	fflush( ct->_outputfp );		/* All output complete before input 	*/
again:
	if ( ct->_ungotten )
		return( ct->_ungot_queue[ --ct->_ungotten ] );

	/* Parse previous leftovers before trying to read more	*/
	if ( i = ct->_chars_on_queue ) {
		ch = ct->_input_queue[--i];
		}
	else {
		memset( ct->_input_queue, 0, INP_QSIZE );
		if ( !ct->_tty ) {				/* Read from a file if not a tty	*/
			if ( read( ct->_inputfd, &ch, 1 ) != 1 ) return( EOF );
			}
		else {
#ifndef __QNXNTO__
			if ( ct->_window_changed ) {
window_changed:
				ct->_window_changed = 0;
				if(ct->_mouse_handler) {
					event = K_RESIZE;
					while((*ct->_mouse_handler)(&event, NULL)) event = 0;
					}
				errno = EOK;
				return( K_RESIZE );
				}
#endif
			if ( !ct->_dproxy && !ct->_cproxy && !ct->_mproxy ) {
				if ( ct->_nonblock ) {
					n = dev_read( ct->_inputfd, &ch, 1, 0, 0, 0, 0, NULL );
					}
				else {
					n = dev_read( ct->_inputfd, &ch, 1,
						ct->_min, ct->_time, ct->_timeout, 0, NULL );
					}
#ifndef __QNXNTO__
				if ( n != 1 ) {
					if ( ct->_window_changed )
						goto window_changed;
					return( EOF );
					}
#endif
				}
#ifndef __QNXNTO__
			else {
				if ( ct->_dproxy  &&  !ct->_dproxy_armed ) {
					dev_read( ct->_inputfd, NULL, 0,
						1, 0, 0, ct->_dproxyr, &ct->_dproxy_armed );
					}
				if ( ct->_mproxy  &&  !ct->_mproxy_armed ) {
					mouse_read( ct->_mm, NULL, 0, ct->_mproxyr, &ct->_mproxy_armed );
					}
				if ( ct->_cproxy  &&  !ct->_cproxy_armed ) {
					console_state( ct->_cc, 0, 0, _CON_EVENT_SIZE );
					console_arm( ct->_cc, 0, ct->_cproxyr, _CON_EVENT_SIZE );
					ct->_cproxy_armed = 1;
					}

				if ( ct->_nonblock )			/* Just poll the keyboard	*/
					pid = Creceive( 0, NULL, 0 );
				else {							/* Block and wait for key	*/
					if ( ct->_timeout ) {
						if ( ct->_timerid == -1 ) { /* Create timer if not present */
							memset( &timercb, 0, sizeof( timercb ) );
							timercb.itcb_event.evt_value = SIGDEV;
							ct->_timerid = mktimer( TIMEOFDAY, _TNOTIFY_SIGNAL, &timercb );
							}
						ct->_was_timeout = 0;
						signal( SIGDEV, __key_timeout );
						n = ct->_timeout / 10;
						timer.it_value.tv_sec = (long)n;
						timer.it_value.tv_nsec = (long)(ct->_timeout % 10) * 100000000L;
						timer.it_interval.tv_sec = (long)(n + 1);
						timer.it_interval.tv_nsec = 0L;
						reltimer( ct->_timerid, &timer, NULL );
						}
					pid = Receive( 0, NULL, 0 );
					if ( ct->_timeout ) {			/*	Cancel timer	*/
						timer.it_value.tv_sec = 0;
						timer.it_value.tv_nsec = 0;
						timer.it_interval.tv_sec = 0;
						timer.it_interval.tv_nsec = 0;
						reltimer( ct->_timerid, &timer, NULL );
						if ( pid == -1 && errno == EINTR && ct->_was_timeout ) {
							errno = EOK;
							return( EOF );
							}
						}
					}

				if ( pid == -1 && errno == EINTR && ct->_window_changed )
					goto window_changed;

				if ( pid == ct->_cproxy ) {
					console_state( ct->_cc, 0, 0, _CON_EVENT_SIZE );
					ct->_cproxy_armed = 0;
					if(ct->_mouse_handler) {
						event = K_RESIZE;
						while((*ct->_mouse_handler)(&event, NULL)) event = 0;
						}
					return( K_RESIZE );
					}
				if ( pid == ct->_mproxy ) {
					mouse_read( ct->_mm, &me, 1, ct->_mproxyr, &ct->_mproxy_armed );
					event = K_MOUSE_EVENT;
					event |= (unsigned) (((unsigned)(me.buttons & 0x07)) << 8);

					/* Mouse motion companding */
					if ( me.dx < 0 ) {
						event |= K_MOUSE_XDIR;
						me.dx = -me.dx;
						}
					for( bitc = 15; bitc > -1; --bitc )
						if ( me.dx & (1<<bitc) ) break;
					++bitc;
					event |= (( (bitc>7) ? 7 : bitc) & 0x07) << 4;

					if ( me.dy < 0 ) {
						event |= K_MOUSE_YDIR;
						me.dy = -me.dy;
						}
					for( bitc = 15; bitc > -1; --bitc )
						if ( me.dy & (1<<bitc) ) break;
					++bitc;
					event |= ( ((bitc>7) ? 7 : bitc) & 0x07);
					if ( ct->_mouse_handler ) {
						if ( ct->_console_input ) {
							bits[0] = bits[1] = 0;
							if ( qnx_ioctl( ct->_inputfd, QCTL_DEV_CTL,
										&bits[0], 8, &bits[0], 4 ) == 0 ) {
								ct->_shift_held = ( bits[0] & 0x010000L ) != 0;
								ct->_ctrl_held = ( bits[0] & 0x020000L ) != 0;
								ct->_alt_held = ( bits[0] & 0x040000L ) != 0;
								ct->_extend_key = 0;
								}
							}
						if ( event & K_MOUSE_XDIR ) me.dx = -me.dx;
						if ( event & K_MOUSE_YDIR ) me.dy = -me.dy;
						if ( i = ( *ct->_mouse_handler )( &event, &me ) ) {
							while( i ) {
								n = 0;
								i = ( *ct->_mouse_handler ) ( &n, &me );
								if( n ) __ungetch( n );
								}
							}
						}

					if ( event == 0 && !ct->_key_null ) goto again;
					return( event );
					}
				if ( pid == ct->_dproxy ) {
					if ( (n = dev_read( ct->_inputfd, &ch, 1,
						0, 0, 0, ct->_dproxyr, &ct->_dproxy_armed ) ) != 1 ) {
						if ( n == 0 ) goto again;
						return( EOF );
						}
					}
				if ( pid == -1 ) {
					return( EOF );
					}
				
				/*
					If an unknown message, call user function which will
					do a read_msg to resolve this.
				*/
				if (	pid != ct->_dproxy &&
						pid != ct->_mproxy &&
						pid != ct->_cproxy &&
						( pid != -1 || !ct->_nonblock ) ) {
					event = term_receive ( pid );
					if( event == 0 && !ct->_key_null ) goto again;
					return( event );
					}
				}
#endif
			}
		}

	ct->_input_queue[i++] = ch;
	ct->_chars_on_queue = (char) i;
	ch = ct->_input_queue[0];

	/*
	 * Quick check to speed up input processing.
	 * If not a function key, or kpd mode off, return each rxed char.
	 */
	if (	!(ct->keystart[ ch >> 4 ] & ( 1 << (ch & 0x0f) ))  ||
			ct->_kpdmode == _KPD_OFF  /* ||  ct->_nonblock */ ) {
		goto done;		/* Not a function key	*/
		}

	/* Compute offsets for ranges 2, 3 and 4 of function keys	*/
	off1 = &ct->_strs._kpad_local - &ct->_strs._ky_backspace;
	off2 = &ct->_strs._prtr_non - &ct->_strs._ky_a1;
	off3 = &ct->_strs._entr_xon_mode - &ct->_strs._ky_btab;
	for( ;; ) {
		/*
		For the 'i'  characters we have so far, at least one string in
		the keyboard set should match to 'i' characters. If none match
		to 'i' characters, then we'll never match.
		*/
		complete = 0;
		if ( first1 = key_scan(	&ct->_strs._ky_backspace,
				&ct->_strs._kpad_local, ct->_input_queue, i, &complete ) ) {
			offset = first1 - &ct->_strs._ky_backspace;
			break;
			}
		if ( first1 = key_scan(	&ct->_strs._ky_a1,
				&ct->_strs._prtr_non, ct->_input_queue, i, &complete ) ) {
			offset = (first1 - &ct->_strs._ky_a1) + off1;
			break;
			}
		if ( first1 = key_scan(	&ct->_strs._ky_btab,
				&ct->_strs._entr_xon_mode, ct->_input_queue, i, &complete ) ) {
			offset = (first1 - &ct->_strs._ky_btab) + off1 + off2;
			break;
			}
		if ( first1 = key_scan(	&ct->_strs._ky_beg,
				&ct->_strs._clr_bol, ct->_input_queue, i, &complete ) ) {
			offset = (first1 - &ct->_strs._ky_beg) + off1 + off2 + off3;
			break;
			}
		if ( ct->_win_key_scan ) {
			event = (*ct->_win_key_scan)(ct->_input_queue, i, &complete);
			if ( complete == 2 ) {
				ct->_chars_on_queue = 0;
#ifndef __QNXNTO__
				if(ct->_mouse_handler) {
					if(i = (*ct->_mouse_handler)(&event, NULL)) {
						while(i) {
							n = 0;
							i = (*ct->_mouse_handler)(&n, NULL);
							if(n) __ungetch(n);
							}
						}
					}
#endif
				if ( event || ct->_key_null ) return( event );
				goto again;
				}
			}
		if ( !complete ) break;		/* No match at all	*/
		if ( !ct->_tty ) {			/* Not a tty, read the file		*/
			if ( read( ct->_inputfd, &ch, 1 ) != 1 ) return( EOF );
			}
		else {
			if ( ct->_nonblock ) {
				n = dev_read( ct->_inputfd, &ch, 1, 0, 0, 0, 0, NULL );
				}
			else {
				n = dev_read( ct->_inputfd, &ch, 1, 
							1, 0, ct->_time, 0, NULL );
				}
			if ( n != 1 ) break;
			}
	
		ct->_input_queue[i++] = ch;
		++ct->_chars_on_queue;

		if ( i >= INP_QSIZE ) {	/* Overflowed without matching	*/
			fprintf( stderr, "getch: overflow\n" );
			break;
			}
		}

	if ( first1 ) {		/* Found a full match	*/
		ct->_chars_on_queue = 0;
		return( table[ offset ] );
		}
	/*  danh - 8sep92
	If polling the keyboard, and some lead in keys arrived but it is not a
	complete function key yet, then return as if no keys had come in and do
	not consume the data that has arrived so far.
	*/
	if ( ct->_nonblock && (complete != 0) )
		return( EOF );
done:					/* Timeout or error condition	*/
	--ct->_chars_on_queue;
	ch = ct->_input_queue[0];	/* Return first typed character	*/
	/* Slide input buffer down one character	*/
	memmove( &ct->_input_queue[0], &ct->_input_queue[1], INP_QSIZE-1 );
	ct->_input_queue[INP_QSIZE-1] = 0;
	return( ch );
	}

void
__ungetch( int c ) {
	ct->_ungot_queue[ ct->_ungotten ] = c;
	if ( ct->_ungotten < INP_QSIZE-1 ) ++ct->_ungotten;
	}

void
__halfdelay( int tenths ) {
	ct->_inpmode = ( ct->_timeout = tenths ) ? _HALFDELAY : _DELAY;
	ct->_nonblock = 0;
	}
