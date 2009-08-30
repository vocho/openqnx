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

     help()							- Display help message
     extra_option( opt )			- Process extra command line options
     init_mdm()						- Initialize protocol variables
     multi_send_check()				- Check if batch mode supported
     final_init()					- Last routine called before send / receive
     snd( filename )				- Send a file
     receive_files( argc, argv )	- Receive one or more files

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <sys/fsys.h>
#include <sys/dev.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
/* Give EXT a null definition so that externals become local to this module	*/
#define	EXT
#include "main.h"

unsigned old_mode;
extern int __errno;

unsigned ungot_char = 0xffff;
char device[ MAX_TTY_NAME+1 ];

void
main( int argc, char *argv[] ) {
	int i, file_count = 0;
	register char *p;
	struct _dev_info_entry dinfo;

	if ( argc < 2 ) help();

	prefix[0] = '\0';	/* No prefix and forced file name off by default	*/
	logfile[0] = '\0';	/* No activity log file by default */
	force = newer = relaxed = today = verbose = quiet = qterm = FALSE;
	make_dir = TRUE;
	onesec = ONESEC;
	retry_limit = RETRY_MAX;

	/*-- Check for a device over-ride ----*/
	if ( !strncmp( argv[1], "/dev/", 5 ) ||
		 !strncmp( argv[1], "//", 2 )		) {
		if ( (mdm = open( argv[1], O_RDWR ) ) == -1) {
			fprintf( stderr, "qcp: unable to open device '%s'.\n", argv[1] );
			exit( -1 );
			}
		--argc;					/* Skip over the device argument			*/
		++argv;
		}
	/*-- No device override so use stdin/stdout to communicate ----*/
	else {
		if ( dev_info( 0, &dinfo ) == -1 ) {
			fprintf( stderr, "qcp: Stdin not a device.\n" );
			exit( -1 );
			}
		strncpy( device, dinfo.tty_name, MAX_TTY_NAME );
		if ( (mdm = open( device, O_RDWR ) ) == -1) {
			fprintf( stderr, "qcp: unable to open stdin as r/w.\n" );
			exit( -1 );
			}
		use_stdin = 1;
		}

	init_mdm();			/* Setup the selected modem port and initialize		*/
						/*	any variables associated with the protocol.		*/

	if		( ( argv[1][0] | ' ' ) == 'r')					sending = FALSE;
	else if ( ( argv[1][0] | ' ' ) == 's'  &&  argc > 2 )	sending = TRUE;
	else {
		fprintf( stderr, "Invalid argument '%s', must be 'se' or 're'.\n", argv[1] );
		exit( -1 );
		}
	argc -= 2;		/* Skip over command name and send/receive argument		*/
	argv += 2;

	/*---- Parse any +, - or = options ----*/
	for ( i = 0; i < argc; ++i ) {
		p = argv[ i ];
		switch( (*p << 8) + *(p + 1) ) {
			case 'f=':		strcpy( prefix, p+2 );
							force = TRUE;			break;
			case 'l=':		strcpy( logfile, p+2 );	break;
			case '-m':		make_dir = FALSE;		break;
			case '+n':		newer = TRUE;			break;
			case '+r':		relaxed = TRUE;
							onesec <<= 2;		/* Timeouts 4 times larger	*/
							retry_limit <<= 1;	/* Twice as many retries	*/
							break;
			case '+t':		today = TRUE;			break;
			case '+v':		verbose = TRUE;			break;
			case '-v':		quiet = TRUE;			break;
			case '+Q':		qterm = TRUE;			break;
			case 'p=':		strcpy( prefix, p+2 );	break;
			default:		if (	*p == '+' ||
									*p == '-' ||
									(*p != 'x' && *(p+1) == '=') ) {
								if ( !extra_option( p ) )
									fprintf( stderr, "%s : Unknown option\n", p );
								else break;		/* Null out the option	*/
								}
							++file_count;		/* Count filenames		*/
							continue;			/*	but don't null them	*/
			}
		*p = '\0';			/* Mark processed options as null strings	*/
		}

	if ( file_count > 1 ) multi_send_check();	/* Check if multi-send OK	*/

	final_init();						/* Last changes before starting		*/

	if ( sending )	send_files(		argc, argv );
	else	 		receive_files(	argc, argv );

	done( 0 );				/* Protocol dependant termination routine		*/
	}

void
send_files( argc, argv )
	int		argc;
	char	*argv[];
	{
	register FILE *fp;
	register int i, rc;
	char	xfile_buf[130];	/* Place to read file name entries from xfile	*/

	for( i = 0; i < argc; ++i ) {
		if ( argv[i][0] == '\0' ) continue;			/* A processed option	*/
		if ( argv[i][0] == 'x'  &&  argv[i][1] == '=' ) {
			multi_send_check();		/* Does the protocol do multi-send ?	*/
			if ( !(fp = fopen( &argv[i][2], "r" ) ) )
				fprintf( stderr, "Unable to open index '%s'.\n", &argv[i][2] );
			while( fgetln( xfile_buf, 100, fp ) ) {
				rc = snd( xfile_buf );
				writelog( "SND", rc, xfile_buf );
				if( !rc )	done( -1 );
				}
			fclose( fp );
			}
		else {
			rc = snd( argv[i] );
			writelog( "SND", rc, argv[i] );
			if( !rc )	done( -1 );
			}
		}
	done( TRUE );
	}

read_mdm( nchars, timeout, buf )
	register int nchars, timeout;
	register uchar *buf;
	{
	register int n, ungot = 0;

	if ( ungot_char != 0xffff ) {		/* Process unget buffer first	*/
		--nchars;
		*buf++ = (uchar) ungot_char;
		ungot_char = 0xffff;
		ungot = 1;
		}
	if ( timeout == -1 ) timeout = 0;
	n = dev_read( mdm, buf, nchars, nchars, 0, timeout, 0, 0 );
	if ( n == -1 ) perror( "read_mdm()" );
	if ( verbose )
		printf( "read(%d)=%d, c=%02x\n", nchars, n, *buf );
	return( n + ungot );
	}

void
writelog( str, rc, msg )
	char *str, *msg;
	int rc;
	{
	int i = 0;
	unsigned long tt;
	register FILE *fp;

	if ( logfile[0] && *msg ) {
		time( &tt );
		while( i++ < 5 ) {	/* Try 5 times to open the potentially busy file*/
			if ( !( fp = fopen( logfile, "a" ) ) ) {
				wait_ticks( 10 );			/* 0.5 seconds between attempts	*/
				continue;
				}
			fprintf( fp, "%s  %s %d  %s\n", ctime( &tt ), str, rc, msg );
			fclose( fp );
			break;
			}
		if ( i == 5 )
			fprintf( stderr, "Unable to append to Logfile '%s'.\n", logfile );
		}
	}

void
write_mdm( nchars, buf )
	unsigned	nchars;
	void		*buf;
	{
	write( mdm, buf, nchars );
	}

void
putmsg( char *fmt, ... ) {
	va_list	arglist;

	if ( quiet  ||  use_stdin ) return;
	va_start( arglist, fmt );
	vprintf( fmt, arglist );
	fflush( stdout );
	}

void
puterr( char *fmt, ... ) {
	va_list arglist;

	if ( !quiet  &&  verbose  &&  !use_stdin ) {
		va_start( arglist, fmt );
		vprintf( fmt, arglist );
		fflush( stdout );
		}
	}

void
wait_ticks( int time ) {
	if ( time ) delay( time * 50 );		/* Delay units of 50 milliseconds 	*/
	}

abort_check() {
	register int c;

	if ( use_stdin  ||  !char_waiting( stdin ) ) return( FALSE );
	if ( (c = getc( stdin ) ) == ' ' || c == 27 || c == '\r' ) {
		printf( "\nStop %s (y/n) ? ", sending ? "sending" : "receiving" );
		fflush( stdout );
		c = getc( stdin );
		printf( "\n" );
		fflush( stdout );
		return( (c == 'y')  ||  (c == 'Y') );
		}
	return( FALSE );
	}

/*----------------------------------------------------------------------------
     This  routine  translates a received filename into a filename allowed
     by the current command line options. If the f=forced_filename  option
     is set,  the  specified  filename  will  be  placed into "new".  If a
     filename prefix was specified with the p= option, any node and  drive
     information  on  the  existing  filename  will  be  removed  and  the
     specified path will be prepended to the filename in "new".   Provided
     the make_dir flag is enabled, any necessary directories to accomodate
     the new  filename  will be created. When a prefix is specified, a '^'
     in the received filename is not tolerated.

IMPORT:	old - Pointer to old filename.
EXPORT: new - Pointer to new filename.
		TRUE - Filename is valid.
		FALSE - Filename is not valid.
----------------------------------------------------------------------------*/
make_file_name( new, old )
	char	*new, *old;
	{
	register char	*p;
	register int len = strlen( old );

	*new = '\0';
	strcpy( new, prefix );					/* Set prefix into new name		*/
	if ( force ) return( TRUE );			/* If force flag, then all done	*/
	if ( !len ) return( FALSE );			/* Must be a file name			*/
#ifdef NEVER
	watch out for '..' in addition to '^' @@@
	if ( prefix  &&  strchr( old, '^' ) )	/* If a prefix specified then	*/
		return( FALSE );					/* 	climbing of dirs not allowed*/
#endif
	p = old;								/* Start with old name			*/
	if ( prefix[0] ) {						/* If a prefix is specified...	*/
		if ( *p == '[' )					/* Skip over the node number	*/
			while( *p  &&  *p++ != ']' ) ;
		if ( *p == '$' ) return( FALSE );	/* Can't prepend a device name	*/
/* @@@	Watch for /dev device name	*/
		if ( *p && *(p+1) == ':') p += 2;	/* Skip over the drive number	*/
		if ( *p == '/' ) ++p;				/* Skip any path info			*/
		if ( *p == 0) return( FALSE );		/* Nothing left...				*/
		}
	strcat( new, p );	/* Place what's left of old name into the new name	*/
	if ( qnx_create_path_to( new ) == -1 ) return( FALSE );
	return( TRUE );
	}

/*	A useful debugging routine	*/
void
dump( msg, n, buf )
	char	*msg;
	int		n;
	uchar	*buf;
	{
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
proto_ungetc( ch )
	uchar ch;
	{
	ungot_char = (unsigned) ch;
	}

char *
fgetln( buf, length, fp )
	char *buf;
	int length;
	FILE *fp;
	{
	register int len;

	if ( fgets( buf, length, fp ) != buf ) return( 0 );
	if ( (len = strlen( buf )) > 0 ) {
		if ( buf[ len-1 ] == '\n' ) buf[ len-1 ] = '\0';
		}
	return( buf );
	}

char_waiting( fp )
	FILE *fp;
	{
	return( FALSE );
	}

void
raw( int fd ) {
	old_mode = dev_mode( fd, 0, _DEV_MODES );
	}

void
restore_opt( fd )
	int fd;
	{
	dev_mode( fd, old_mode, _DEV_MODES );
	}
