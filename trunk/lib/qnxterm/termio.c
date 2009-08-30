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




/*				26-Feb-90 10:33:30am										*/

/*--------------------------------------------------------------------------*/
/*  History: termio.c, V0.0, 21-Feb-90  8:56:19pm, Dan Hildebrand, Baseline	*/
/*--------------------------------------------------------------------------*/
#undef __INLINE_FUNCTIONS__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <termios.h>
#include <sys/term.h>
#include <errno.h>
#ifndef __QNXNTO__
#include <sys/dev.h>
#endif
#include <gulliver.h>


#define ct	__cur_term

#define	TERMIO_MAGIC	0x011a
struct termiohdr {			/* Header at the front of a terminfo file	*/
	short int
		magic,
		name_size,
		bool_count,
		num_count,
		offset_count,
		str_size;
	};

TERMINAL *__cur_term;

static char terminfo[] = "/usr/lib/terminfo";

/* Endianize a structure of 16 bit shorts.*/
static void
endianize_short_struct(void *data, int size)
{
	int i = 0;
	char *s = data;
	
	while(i < size){
		*((short *)(s + i)) = ENDIAN_LE16(*((short *)(s + i)));
		i += 2;
	}
}

static void 
_err( int num, int *rc ) {
	if ( rc ) {
		*rc = 0;
		errno = ENOENT;
		}
	else {
		fprintf( stderr, "Bad termio file(%d).\n", num );
		exit( EXIT_FAILURE );
		}
	return;
	}

static void
*safe_alloc( int amount ) {
	register void *p;

	p = calloc( amount, 1 );
	if ( p == NULL ) {
		fprintf( stderr, "term: No memory.\n" );
		exit( EXIT_FAILURE );
		}
	return( p );
	}

static void
key_scan( short *first, short *last ) {
	unsigned char *c;

	while( first < last ) {
		c = (unsigned char *) &ct->_strtab[0] + *first;
		/* Set one of 256 bits in the keystart array	*/
		if (*c) ct->keystart[ *c >> 4 ] |= (1 << (*c & 0x0f) );
		++first;
		}
	}

void
__setupterm( char *term, int fd, int *rc ) {
	FILE	*fp;
	register char	*p;
	int		amount, pos, c;
	short	*iptr;
	struct	termiohdr hdr;
	union {
#ifndef __QNXNTO__
		struct _dev_info_entry	dinfo;
#endif
		char	path[ 100 ];
	} temp;
	char	sm_path[ 6 ];
	char	tname[ 20 ];

	if ( !term ) term = getenv( "TERM" );
	if ( !term ) {
		if ( rc ) {
			*rc = 0;
			errno = EINVAL;
			}
		else {
			fprintf( stderr, "TERM environment variable not set.\n" );
			exit( EXIT_FAILURE );
			}
		return;
		}
	strncpy( tname, term, 19 );
	p = getenv( "TERMINFO" );
	strcpy( temp.path, p ? p : terminfo );
	sm_path[0] = '/';
	sm_path[1] = tname[0];
	sm_path[2] = '/';
	sm_path[3] = '\0';
	strcat( temp.path, sm_path );
	strcat( temp.path, tname );

	if ( !(fp = fopen( temp.path, "r" ) ) ) {
		strcpy( temp.path, terminfo );		/* Failed so try default path	*/
		strcat( temp.path, sm_path );
		strcat( temp.path, tname );
		if ( !(fp = fopen( temp.path, "r" ) ) ) {
#ifdef __QNXNTO__
			strcpy( temp.path, "/proc/boot/");
			strcat( temp.path, tname );
			if ( !(fp = fopen( temp.path, "r" ) ) )
#endif
			{
				if ( rc ) {
					*rc = 0;
					errno = ENOENT;
					}
				else {
					fprintf( stderr, "Bad terminal setting.\n" );
					exit( EXIT_FAILURE );
					}
				return;
				}
			}
		}

	if ( 1 != fread( &hdr, sizeof( hdr ), 1, fp ) ) {
		_err(1,rc);
		return;
		}

	hdr.magic = ENDIAN_LE16(hdr.magic);
	hdr.name_size = ENDIAN_LE16(hdr.name_size);
	hdr.bool_count = ENDIAN_LE16(hdr.bool_count);
	hdr.num_count = ENDIAN_LE16(hdr.num_count);
	hdr.offset_count = ENDIAN_LE16(hdr.offset_count);
	hdr.str_size = ENDIAN_LE16(hdr.str_size);
	
	pos = sizeof( hdr );
	if ( hdr.magic != TERMIO_MAGIC ) {
		_err(2,rc);
		return;
		}
	ct = safe_alloc( sizeof( TERMINAL ) + hdr.str_size );
	ct->_version = _TERMINFO_VERSION;
	ct->Filedes = fd;					/* Specified output device	*/
	if ( fd == fileno( stdout ) ) {
		ct->_outputfp = stdout;
		}
	else {
		if ( !(ct->_outputfp = fdopen( fd, "w" ) ) ) {
			if ( rc ) {
				*rc = 0;
				/* errno set by fdopen() */
				}
			else {
				fprintf( stderr, "Can't open output stream.\n" );
				exit( EXIT_FAILURE );
				}
			return;
			}
		}
	ct->_inputfd = fileno( stdin );	/* Stdin device		*/
#ifndef __QNXNTO__
	if ( dev_info( ct->_inputfd, &temp.dinfo ) != -1) {
		ct->_inputfd_nid = temp.dinfo.nid;	/* Memorize nid for later	*/
		ct->_console_input = !strcmp( temp.dinfo.driver_type, "console" );
		ct->_input_winch = (temp.dinfo.flags & _DEV_WILL_WINCH) != 0;
		}
	if ( ct->_inputfd == ct->Filedes ) {
		ct->_outputfd_nid = ct->_inputfd_nid;
		}
	else {
		if ( dev_info( ct->Filedes, &temp.dinfo ) != -1) {
			ct->_outputfd_nid = temp.dinfo.nid;	/* Memorize nid for later	*/
			}
		}
#endif
	ct->_tty = (char) isatty( fd );
	ct->terminal_name = safe_alloc( hdr.name_size );
	if ( 1 !=
		fread( (char *) &ct->terminal_name[0], hdr.name_size, 1, fp ) ) {
			_err(3,rc);
			return;
			}
	pos += hdr.name_size;

	amount = min( sizeof( struct _bool_struct ), hdr.bool_count );
	p = (char *) &ct->_bools;
	while( amount-- ) {
		if ( (c = getc( fp )) == -1 ) {
			_err( 4, rc );
			return;
			}
		if ( c == 2 ) c = 0;	/* Map cancelled entries into not-present	*/
		*p++ = c;
		}
	pos += hdr.bool_count;
	if ( pos & 0x0001 ) ++pos;			/* Word align next read		*/
	/* We may not have read as much as this field contained, so skip ahead	*/
	fseek( fp, (long) pos, 0 );

	amount = min( sizeof( struct _num_struct ), hdr.num_count*2 );
	memset( &ct->_nums, 0xff, sizeof( struct _num_struct ) );
	if ( 1 != fread( (char *) &ct->_nums, amount, 1, fp ) ) {
		_err(5,rc);
		return;
		}
	endianize_short_struct(&ct->_nums, amount);
	pos += hdr.num_count*2;
	fseek( fp, (long) pos, 0 );

	amount = min( sizeof( struct _strs ), hdr.offset_count*2 );
	memset( &ct->_strs, 0xff, sizeof( struct _strs ) );
	if ( 1 != fread( (char *) &ct->_strs, amount, 1, fp)) {
		_err(6,rc);
		return;
		}
	endianize_short_struct(&ct->_strs, amount);
	pos += hdr.offset_count*2;
	fseek( fp, (long) pos, 0 );
	if ( 1 != fread( (char *) &ct->_strtab[0], hdr.str_size, 1, fp ) ) {
		_err(7,rc);
		return;
		}
	fclose( fp );

	p = &ct->_strtab[0];			/* Find a null byte to point to		*/
	while ( *p ) ++p;				/* Scan forward until really null	*/
	/* Set all -1 or -2 offsets to offset to point to a null byte		*/
	for (	iptr = (short *) &ct->_strs;
			iptr < (short *) &ct->sgr_mode; ++iptr )
		if ( *iptr == -1  ||  *iptr == -2 )
			*iptr = (short) (p - &ct->_strtab[0]);

	/*
	Mark in the keystart array which binary values represent the start
	of function keys. Because the strings which define the functions are
	in 4 regions of the struct, we need to explicitly scan all four.
	*/
	key_scan( &ct->_strs._ky_backspace, &ct->_strs._kpad_local );
	key_scan( &ct->_strs._ky_a1, &ct->_strs._prtr_non );
	key_scan( &ct->_strs._ky_btab, &ct->_strs._entr_xon_mode );
	key_scan( &ct->_strs._ky_beg, &ct->_strs._clr_bol );

	if ( !strcmp( cursor_address, "\033Y%p1%' '%+%c%p2%' '%+%c" ) )
		ct->_cup_type = _VT52_CUP;
	else if ( !strcmp( cursor_address, "\033[%i%p1%d;%p2%dH" ) )
		ct->_cup_type = _VT100_CUP;
	else if ( !strcmp( cursor_address, "\033[%i%p1%d;%p2%dH$<5>" ) )
		ct->_cup_type = _VT100_CUP;

	ct->_pad_char = (char) (*pad_char ? *pad_char : '\0');
	strncpy( ct->_termname, tname, 15 );
	ct->_min = 1;
	ct->_time = ct->_timeout = ct->_nonblock = 0;
	ct->_timerid = -1;
	if ( p = getenv( "LINES" ) )	lines = atoi( p );
	if ( p = getenv( "COLUMNS" ) )	columns = atoi( p );
	if ( rc ) *rc = 1;
	setvbuf( ct->_outputfp, NULL, _IOFBF, 2000 );
	}

/* @@@ Macro candidate	*/
void
__set_curterm( TERMINAL *nterm ) {
	ct = nterm;
	}

/* @@@ Macro candidate	*/
void
__del_curterm( TERMINAL *oterm ) {
	if ( oterm->_outputfp != stdout ) fclose( oterm->_outputfp );
#ifndef __QNXNTO__
	if ( oterm->_cc ) console_close( oterm->_cc );
#endif
	free( oterm->terminal_name );
	free( oterm );
	if ( oterm == ct ) ct = (TERMINAL *)0;
	}

void
__restartterm( char *term, int fd, int *rc ) {
	term = term;						/* shut up the compiler */
	fd = fd;
	rc = rc;
	}

void
__tputs( char *p, int count, int (*putc)() ) {
	int delay = 0;

	while( *p ) {
		if ( *p == '$'  &&  *(p+1) == '<'  &&  *(p+2) != '\0' ) {
			p += 2;		/* Skip over "$<"	*/
			while( *p  &&  *p != '>' ) {
				if ( *p >= '0'  &&  *p <= '9' ) delay = delay*10 + (*p - '0');
				++p;
				}
			if ( *p == '>' ) {
				delay *= count;		/* Scale by line count	*/
				while( delay-- ) (*putc)( ct->_pad_char, ct->_outputfp );
				}
			}
		else
			(*putc)( *p, ct->_outputfp );
		++p;
		}
	if( ct->_cc ) fflush( ct->_outputfp );
	}

/* @@@ Macro candidate	*/
void __putp( str )
	char *str;
	{
	__tputs( str, 1, &fputc );
	}

void __savetty() {
	tcgetattr( ct->Filedes, &ct->Otty );
	}

void __resetty() {
	tcsetattr( ct->Filedes, TCSADRAIN, &ct->Otty );
	}

int
__has_colors() {
	/* Indicate color support	*/
	return( *set_color_pair || (*set_foreground && *set_background) );
	}

void
__keypad( int win, int mode ) {
	ct->_time = 0;
	ct->_kpdmode = mode ? _KPD_ON : _KPD_OFF;
	__notimeout ( win, 0 );
	if( *keypad_xmit && *keypad_local ) {
		__putp( keypad_xmit );		/* Enable the application keypad */
		}
	}

void
__notimeout( int win, int mode ) {
	int delay;

	win = 0;
	if ( ct->_kpdmode != _KPD_OFF ) {
		if ( mode ) {
			delay = 0;
			}
		else {
	/* Enable keypad key processing for non-qnx terminals. A QNX terminal	*/
	/* doesn't need this because all function keys start with 0xff and the	*/
	/* user never needs to manually type an 0xff.							*/
/* @@@ hc@, daisy == QNX extentions to terminfo */
			if ( (delay = print_rate) == -1 || !has_print_wheel || hard_copy )
				delay = 3;
			}
		ct->_time = delay;
		ct->_kpdmode = delay ? _KPD_DELAY : _KPD_ON;
		}
	}
