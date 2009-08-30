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





/*				11-Dec-89 10:27:59am										*/

/*--------------------------------------------------------------------------*/
/*  History: main.c, V0.0, ??-Nov-86 11:36:31am, Dan Dodge,	Baseline		*/
/*  				 V0.1, 13-Jan-87 11:36:31am, Dan Hildebrand				*/
/*----------------------------------------------------------------------------

     This file contains the command line parser and some of  the  commonly
     needed  routines  for  the various protocols that make up the Quantum
     Communications Protocol family. User written protocols should link in
     this module and include "main.h"  to  interface  cleanly  to  Quantum
     communications products.    Use  of  this command line interface also
     makes user written protocols  installable  into  the  /qterm/protocol
     file, thus  making  the  protocol  usable  from  within  Qterm. Other
     application programs ( such as bulletin board programs ) can refer to
     the /qterm/protocol file when a choice of protocol is needed.

     The  following  routines  must be defined and linked with this source
     module to produce a working protocol.    Please  refer  to  the  file
     "protocol.doc" for details.

     void help(void);				- Display help message
     int extra_option(char *);		- Process extra command line options
     void init_mdm(void);			- Initialize protocol variables
     void multi_send_check(void);	- Check if batch mode supported
     void done(int );				- Exit with status, shutdown protocol
     void final_init(void);			- Last init called before snd/rcv
     int snd(char *);				- Send a file
     receive_files( int argc, char *argv[] ) - Receive one or more files

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#ifndef __QNXNTO__
#include <sys/fsys.h>
#include <sys/dev.h>
#endif
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#ifndef __QNXNTO__
#include <i86.h>
#endif

#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
/* Give EXT a null definition so that externals become local to this module	*/
#define	EXT
#include "main.h"

#ifdef __QNXNTO__
#include <devctl.h>
#define MAX_TTY_NAME TTY_NAME_MAX 
#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_OSFLOW 0x0010
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST|_DEV_OSFLOW)
#endif

extern int timeout(int );

unsigned short ungot_char = 0xffff;
char device[ MAX_TTY_NAME+1 ];

#ifdef __QNXNTO__
#include <unistd.h>
#include <termios.h>
#include <libgen.h>
#endif

int
main( int argc, char *argv[] ) {
	int		opt, file_count = 0;

	pname = basename(argv[0]);
	if ( argc < 2 ) {
		Puterr( "insufficient arguments\n" );
		exit(1);
		}

	prefix[0] = '\0';	/* No prefix and forced file name off by default	*/
	logfile[0] = '\0';	/* No activity log file by default */
	force = newer = relaxed = today = verbose = quiet = qterm = FALSE;
	make_dir = TRUE;
	onesec = ONESEC;
	retry_limit = RETRY_MAX;

	if (optind==0) /* 16-bit */
		++optind;		/* Skip over command name	*/

	if ( *argv[optind] == '/' ) { 	/*-- Check for a device over-ride ----*/
		if ( ( mdm = open( argv[1], O_RDWR ) ) == -1 ) {
			Puterr( "unable to open device '%s'\n", argv[1] );
			exit( 1 );
			}
		++optind;		/* Skip over specified device name	*/
		}
	else { 	/*-- No device override so use stdin/stdout to communicate ----*/
		struct stat statbuf;

		strcpy( device, ttyname( 0 ) );
		if( fstat( 0, &statbuf ) != -1 && S_ISSOCK(statbuf.st_mode) ) {
			socket = 1;
			mdm = 0;
		    }
		else if ( ( mdm = open( device, O_RDWR ) ) == -1 ) {
			Puterr( "unable to open stdin as r/w\n" );
			exit( 1 );
			}
		use_stdin = TRUE;
		}

	signal( SIGALRM, (void *)&timeout );

	init_mdm();			/* Setup the selected modem port and initialize		*/
						/*	any variables associated with the protocol.		*/

	if		( ( *argv[optind] | ' ' ) == 'r')					sending = FALSE;
	else if ( ( *argv[optind] | ' ' ) == 's'  &&  argc > 2 )	sending = TRUE;
	else {
		Puterr( "Invalid argument '%s' (must be 'se' or 're')\n", argv[optind] );
		exit( 1 );
		}
	++optind;

	/*---- Ready for command line options ----*/
	while( ( opt = getopt( argc, argv, "FmnqrtuvVQf:l:p:s:" ) ) != -1 ) {
		switch( opt ) {
			case 'F':		flashfile = TRUE;			break;
			case 'f':		strcpy( prefix, optarg );
							force = TRUE;				break;
			case 'l':		strcpy( logfile, optarg );	break;
			case 'm':		make_dir = FALSE;			break;
			case 'n':		newer = TRUE;				break;
			case 'r':		relaxed = TRUE;
							onesec <<= 2;		/* Timeouts 4 times larger	*/
							retry_limit <<= 1;	/* Twice as many retries	*/
							break;
			case 's':		sub_packet_size = atoi( optarg );
							sub_packet_size >>= 3;	/* Divide by 8 for sub_packet	*/
							if ( !sub_packet_size ) {
								Puterr( "packet size too small\n" );
								exit( 1 );
								}
							if ( sub_packet_size > MAXSIZE ) {
								Puterr( "packet size too large (max %d)\n",
											MAXSIZE*8);
								exit( 1 );
								}
							break;
			case 't':		today = TRUE;					break;
			case 'u':		unlink_file = TRUE;				break;
			case 'V':
			case 'v':		verbose = TRUE;					break;
			case 'q':		quiet = TRUE;					break;
			case 'Q':		qterm = TRUE;					break;
			case 'p':		strcpy( prefix, optarg );		break;
			case '?': 		if ( !extra_option( (char *)&optopt ) ) {
								Puterr( "unknown option '%s'\n", optopt );
								exit( 1 );
								}
							break;
			default: 		++file_count;		/* Count filenames		*/
							continue;			/*	but don't null them	*/
			}
		}

	if ( file_count > 1 ) multi_send_check();	/* Check if multi-send OK	*/

	final_init();						/* Last changes before starting		*/

#ifndef __QNXNTO__
	if ( !use_stdin ) dev_mode( 0, 0, _DEV_MODES );		/* Raw console input for abort requests	*/
#endif
	if ( sending )	send_files(		argc - optind, (signed char **)(argv + optind) );
	else	 		receive_files(	argc - optind, (signed char **)(argv + optind) );

	done( 0 );				/* Protocol dependant termination routine		*/
        return 0;
	}

void
send_files( int argc, signed char *argv[] ) {
	FILE	*fp;
	int		i, rc;
	char	xfile_buf[110];	/* Place to read file name entries from xfile	*/

	for( i = 0; i < argc; ++i ) {
		if ( argv[i][0] == 'x' && argv[i][1] == '=' ) {
			multi_send_check();		/* Does the protocol do multi-send?		*/
			if ( !( fp = fopen( &argv[i][2], "r" ) ) ) {
				Puterr( "unable to open index '%s'\n", &argv[i][2] );
				exit( 1 );
				}
			while( fgetln( xfile_buf, 100, fp ) ) {
				rc = snd( xfile_buf );
				writelog( "SND", rc, xfile_buf );
				}
			fclose( fp );
			}
		else {
#ifdef DIAG
fprintf(stderr,"Sending %s\r\n",argv[i]);
#endif
			rc = snd( argv[i] );
			writelog( "SND", rc, argv[i] );
			if( !rc )	done( 1 );
			}
		}
	done( 1 );
	}

int read_mdm( int nchars, int timeout, uchar *buf ) {
	int		n, i, ungot = 0;

#ifdef DIAG
fprintf(stderr,"read_mdm(%d, %d, buffer)\r\n",nchars, timeout);
#endif

	if ( ungot_char != 0xffff ) {		/* Process unget buffer first	*/
		--nchars;
		*buf++ = (uchar) ungot_char;
		ungot_char = 0xffff;
		ungot = 1;
		}
	if ( timeout == -1 ) timeout = 0;
	n = 0;
#ifndef __QNXNTO__
	if ( !socket ) {
		while ( nchars && ( i = dev_read( mdm, buf, nchars, nchars, 0, timeout, 0, 0 ) ) ) {
			if ( i == -1 ) puterr( "read_mdm(): %s\n", strerror(errno) );
			buf += i; nchars -= i; n += i;
			}
	    }
	else
#endif
		{
		int s=0;
		struct timeval tv;
		fd_set rfd;

		FD_ZERO( &rfd );
		FD_SET( mdm, &rfd );
		memset(&tv,0,sizeof(tv));
		tv.tv_sec = timeout / onesec;

#ifdef DIAG
fprintf(stderr,"select tv.tv_src = %d\r\n",tv.tv_sec);
#endif
		while( nchars && (s = select( 1 + mdm, &rfd, 0, 0, &tv )) == 1 ) {
#ifdef DIAG
fprintf(stderr,"select returned - chars waiting. Set timeout for %d seconds\r\n",timeout/onesec);
#endif
			alarm( timeout / onesec );
#ifdef DIAG
fprintf(stderr,"reading...\r\n");
#endif
			while ( nchars && ( i = read( mdm, buf, nchars ) ) ) {
				if ( i == -1 ) {
					if( errno == EINTR ) {
#ifdef DIAG
					fprintf(stderr,"read_mdm(): alarm timeout\n");
#endif
						goto timeout;
					}
					if( errno != EAGAIN )
						puterr( "read_mdm(): %s\n", strerror(errno) );
					break;
					}
				else {
#ifdef DIAG 
fprintf(stderr,"read_mdm got %d chars [0x%02x 0x%02x 0x%02x...]\r\n",i,buf[0],buf[1],buf[2]);
#endif
					buf += i; nchars -= i; n += i;
					}
				}
			alarm( 0 );
		    }
timeout:
		if ( s == -1 ) puterr( "read_mdm() / select(): %s\n", strerror(errno));
		}

#ifdef DIAG
fprintf(stderr,"read_mdm returning %d(+%d ungot) chars\r\n",n,ungot);
#endif

	return( n + ungot );
	}

void
writelog( signed char *str, int rc, signed char *msg ) {
	int		i = 0;
	time_t	tt;
	FILE	*fp;

	if ( *logfile && *msg ) {
		time( &tt );
		while( i++ < 5 ) {	/* Try 5 times to open the potentially busy file*/
			if ( !( fp = fopen( logfile, "a" ) ) ) {
				wait_ticks( 10 );			/* 0.5 seconds between attempts	*/
				continue;
				}
			fprintf( fp, "%s: %24.24s  %s %d  %s\n", pname, ctime( &tt ), str, rc, msg );
			fclose( fp );
			break;
			}
		if ( i == 5 )
			fprintf( stderr, "Unable to append to logfile '%s'.\n", logfile );
		}
	}

void
write_mdm( unsigned nchars, void *buf ) {
#ifdef DIAG
{
	char *cb=buf;

	fprintf(stderr,"write_mdm(%d, '0x%02x 0x%02x 0x%02x...'\r\n",nchars,cb[0],cb[1],cb[2]);
}
#endif
	write( mdm, buf, nchars );
	}

void
putmsg( signed char *fmt, ... ) {
	va_list	arglist;

#ifndef DIAG
	if ( quiet  ||  use_stdin ) return;
#else 
fprintf(stderr,"putmsg() about to do a vprintf... quiet=%d, use_stdin=%d\r\n",quiet,use_stdin);
#endif
	va_start( arglist, fmt );
	vprintf( fmt, arglist );
	fflush( stdout );
	}

void
puterr( signed char *fmt, ... ) {
	va_list arglist;

#ifndef DIAG
	if ( quiet || use_stdin || !verbose ) return;
#endif
	va_start( arglist, fmt );
	vfprintf( stderr, fmt, arglist );
	}

void
Puterr( signed char *msg, ... ) {
	va_list	arglist;

	va_start( arglist, msg );
	fprintf( stderr, "%s: ", pname );
	vfprintf( stderr, msg, arglist );
	va_end( arglist );
	}

void
wait_ticks( int time ) {
	if ( time ) delay( time * 50 );		/* Delay units of 50 milliseconds 	*/
	}

int abort_check() {
	char c;
//#ifndef __QNXNTO__   /* for now, disable all this code under nto */


	#ifdef __QNXNTO__
        int n; char buf[16];
        fcntl(0,F_SETFL,O_NONBLOCK);
        if (use_stdin || (read(0,&c,1)<1) ) return 0;
        fcntl(0,F_SETFL,0);
	#else
	if ( use_stdin || !dev_ischars(0) ) return 0;
        read(0,&c,1);
        #endif

	if ( c == ' ' || c == 27 || c == '\r' ) {

	#ifdef __QNXNTO__
        fcntl(0,F_SETFL,O_NONBLOCK);
     
        /* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
        do {
            n=read(0,&buf,sizeof(buf));
        } while (n!=0 && n!=-1);

        fcntl(0,F_SETFL,0);
	#else 
		while( dev_ischars(0) ) read( 0, &c, 1 );
	#endif

		printf( "\nStop %s (y/n) ? ", sending ? "sending" : "receiving" );
		fflush( stdout );
		read( 0, &c, 1 );
	#ifdef __QNXNTO__
		{
		int n; char buf[16];
        fcntl(0,F_SETFL,O_NONBLOCK);
     
        /* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
        do {
            n=read(0,&buf,sizeof(buf));
        } while (n!=0 && n!=-1);
     
        fcntl(0,F_SETFL,0);
		}
	#else 
		while( dev_ischars(0) ) read( 0, &c, 1 );
	#endif
		printf( "\n" );
		fflush( stdout );
		return( c|' ' == 'y' );
		}
//#endif
	return 0;
	}

/*----------------------------------------------------------------------------
     This  routine  translates a received filename into a filename allowed
     by the current command line options. If the "-f forced_filename"  option
     is set,  the  specified  filename  will  be  placed into "new".  If a
     filename prefix was specified with the -p option, any node and  drive
     information  on  the  received  filename  will  be  removed  and  the
     specified path will be prepended to the filename in "new".   Provided
     the make_dir flag is enabled, any necessary directories to accomodate
     the new  filename  will be created. When a prefix is specified, a '^'
     in the received filename is not tolerated.

IMPORT:	old - Pointer to old filename.
EXPORT: new - Pointer to new filename.
		TRUE - Filename is valid.
		FALSE - Filename is not valid.
----------------------------------------------------------------------------*/
int make_file_name( signed char *new, signed char *old ) {
	char	*p;
	int		len = strlen( old );

	strcpy( new, prefix );					/* Set prefix into new name		*/
	if ( force ) return 1;					/* If force flag, then all done	*/
	if ( !len ) return 0;					/* Must be a file name			*/
	p = old;								/* Start with old name			*/
	if ( *prefix ) {		/* No directory climbing if a prefix was set	*/
		if ( strchr( old, '^' ) ) return 0;	/* QNX 2.15 style		*/
		for( p = old; *p; p++ )
			if ( *p == '.'  &&  *(p+1) == '.' ) return 0;
		p = old;					/* Start with old name					*/
		if ( *p == '[' )			/* Skip over the QNX 2.15 node number	*/
			while( *p && *p++ != ']' );
		if ( *p == '/'  &&  *(p+1) == '/' ) {
			p += 2;					/* Skip over the QNX4.0 node number		*/
			while( *p && *p++ != '/' );
			}
		if ( *p == '$' ) return 0;			/* Can't prepend a device name	*/
		if ( *p && *(p+1) == ':') p += 2;	/* Skip over the drive number	*/
		if ( *p == '/' ) ++p;				/* Skip any path info			*/
		if ( *p == 0) return 0;				/* Nothing left...				*/
		}
	strcat( new, p );	/* Place what's left of old name into the new name	*/
	if ( make_dir != FALSE && qnx_create_path_to( new ) == -1 ) return 0;
	return 1;
	}

/*	A useful debugging routine	*/
void
dump( signed char *msg, int n, uchar *buf ) {
	register int i;

	putmsg( "\n%s\ncount = %d\n ", msg, n );
	for ( i = 0; i < n; ++i ) putmsg( " %02x", buf[i] );
	printf( "\n" );
	fflush( stdout );
	}

void
flush_in() {
	tcflush( mdm, TCIFLUSH );
	}

void
proto_ungetc( uchar ch ) {
	ungot_char = (unsigned) ch;
	}

char *
fgetln( signed char *buf, int length, FILE *fp ) {
	int		len;

	if ( (signed char *)(fgets( buf, length, fp )) != buf ) return 0;
	if ( (len = strlen( buf )) > 0 ) {
		if ( buf[ len-1 ] == '\n' ) buf[ len-1 ] = '\0';
		}
	return( buf );
	}

int
timeout(int sig) {
	sig = sig;
	return -1;
}
