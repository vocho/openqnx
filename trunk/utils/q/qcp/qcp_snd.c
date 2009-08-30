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





/*              15-Dec-89  1:43:46pm                                        */

/*--------------------------------------------------------------------------*/
/*  History: qcp_snd.c, V0.0, ??-Nov-86 11:36:31am, Dan Dodge,  Baseline    */
/*                      V0.1, 13-Jan-87 11:36:31am, Dan Hildebrand          */
/*----------------------------------------------------------------------------
     The following is the qcp protocol. It is initiated by the sender  but
     once started  is driven by the receiver. The sender contains one very
     large timeout to handle the case of a non-existant receiver.

     To send a file the sender first sends an escape sequence to  arm  the
     receiver. The  receiver now starts hunting for one of 5 escape codes.
     Start receive mode, start of data block, enquire receiver,  start  of
     remote message or cancel receive.  The sequences are:

    SYN SYN         - Receiver goes into receive file mode.
    SYN SOH         - Start of a data block.
    SYN ENQ         - Enquire request.
    SYN SO          - Start of a remote message block.
    SYN CAN CAN     - Abort file receive. Done receive mode.

     The SYN SOH is followed by a data block. The block structure is.

    SYN SOH <seq> <~seq> <len> <~len> <data....> <crc>
	      1      1     2      2      len       2

     The last data block in the transmission is indicated by a  length  of
     zero. This  scheme was chosen over one which used SYN EOT to indicate
     end of trasmission because the code was simpler at  both  the  sender
     and receiver since they could reuse their send/receive block code.

     Upon receipt of a SYN SOH the receiver will  read  the  seq  and  len
     values.  If

    <seq> != ~<~seq>  or  <len> != ~<~len>

     then the receiver starts hunting for another SYN escape sequence. The
     <seq> is an 8 bit number in which the top 5 bits are a segment number
     and  the  lower  3  bits  are a sequence number which indicates which
     piece of the segment this block is.  Each segment  has  8  pieces  (0
     thru 7).    The  sender always starts a segment with piece 0 and will
     send all 8 pieces without the need for any acknowledge.  Upon sending
     all pieces the sender will pause for the receiver to ack or  nak  the
     composite transmission before starting the next segment. The receiver
     will ack/nak for the following reasons.

     1. Piece 7 (the last one) is received. If any pieces are missing  the
     recevier  sends  one  packet  consisting of NAK <seq> <~seq> for each
     missing piece.  This packet will be 3*n chars long  where  n  is  the
     number of missing  pieces.   The sender will resend these pieces.  If
     all pieces are received ok the receiver will ACK the highest received
     piece (in this case 7) using ACK <seq> <~seq>. ( ACK 7 ~7 ).

     2. End of file is indicated by a zero length.  The  rules  of  1  are
     followed except the acknowledged piece may be less than 7.

     3. No data  is  received after a timeout value (several seconds).  In
     this case we re-ack the last received piece every few  seconds  until
     we receive  a  SYN. At this point we unget the SYN and drop back into
     hunt mode.

     In all cases the pieces are saved to disk before an ACK is sent. This
     ensures  that  the program does not have to handle receiving and disk
     writing at the same time. This greatly simplifies the program.

     The receiver follows the following simple logic.  Collect  data  into
     each of the 8 piece buffers.  Upon receiving the last piece or a zero
     length  piece  check  to  ensure  all  pieces  have been received (no
     holes). If so then save the data and ACK  the  highest  piece.    The
     receiver  will  now  wait  for  a  SYN  and until one is seen it will
     re-issue its ACK every couple of seconds.

     If there are missing pieces then each missing piece is NAKed.   If  a
     timeout  occurs, examine the piece buffers and NAK the missing pieces
     and wait for them to arrive.

     Thanks to Glen McCready for the file transmission timing.

----------------------------------------------------------------------------*/
/*
#ifdef __USAGE
%C  [device] SEnd [options]* src_file[,dst_file] ... [x=index_file] ..
%C  [device] REceive [options]* [-f forced_filename | -p prefix]
Options:
 -f filename     Force the received file to have this filename
 -l logfile      Log file transmission / receptions to this logfile
 -m              Supress making directories for received files
 -n              Receive only files newer than existing files
 -p prefix       Place this file or path prefix on received filenames
 -q              Be quiet during file transfer
 -r              Relaxed timing, double timeouts and quadruple retry counts
 -s packet_size  Specify an alternate packet_size (default: 2048 bytes)
 -t              Stamp today's date on received files
 -u              Unlink existing files that lack write permission
 -V, -v          Display error status while transferring files
 x=index_file    A list of filenames to send.
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
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
#include <gulliver.h>
#define EXT     extern      /* Let Main.c publics be extern for this module */
#include        "main.h"
#undef  EXT                 /* Force QCP externs to reside in this module   */
#define EXT
#include        "qcp.h"

extern int fd;
int old_mode;
unsigned old_c_lflag, old_c_oflag, old_c_iflag;

#ifdef __QNXNTO__
#include <devctl.h>
#define MAX_TTY_NAME TTY_MAX_NAME
#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_OSFLOW 0x0010
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST|_DEV_OSFLOW)
#endif

int extra_option( signed char *opt ) {
    switch( *opt ) { default: return 0; }
    }


void
init_mdm() {
    struct termios termios;
    speed_t baud;
#ifdef __QNXNTO__
    struct termios termios_p;
#endif
    tcgetattr( mdm, &termios );
    baud = min(termios.c_ispeed, termios.c_ospeed);
#ifdef __QNXNTO__
    old_c_lflag=termios.c_lflag;
    old_c_iflag=termios.c_iflag;
    old_c_oflag=termios.c_oflag;
    if(!use_stdin){
    	tcgetattr( mdm, &termios_p);
   		termios_p.c_cc[VMIN]  =  1;
    	termios_p.c_cc[VTIME] =  0;
    	termios_p.c_lflag &= ~( ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL );
    	termios_p.c_oflag &= ~( OPOST|ONLCR|OCRNL );
    	termios_p.c_iflag &= ~( ICRNL|INLCR|IXON|IXOFF );
    	tcsetattr(mdm, TCSADRAIN, &termios_p);
    }
#else
    old_mode = dev_mode( mdm, 0, _DEV_MODES ); // this'll fail on a socket
#endif
    use_crc = TRUE;                         /* Use CRCs by default          */
//  sub_packet_size = baud >= 9600 ? 8192 >> 3 : 2048 >> 3; // Set dflt sub_packet size
    sub_packet_size = 256; // Set dflt sub_packet size
    }

void
multi_send_check() {;       /* QCP Supports multi-send so don't abort with  */
    }                       /*  an error message.                           */

void
done( int status ) {
#ifdef __QNXNTO__
  struct termios termios_p;
#endif
    if(status !=0) {
	time_t t = time(NULL)+1;

	while( t <= time(NULL)) cancel();   
	}
#ifdef __QNXNTO__
	tcgetattr(mdm, &termios_p);
    termios_p.c_cc[VMIN]  =  1;
    termios_p.c_cc[VTIME] =  0;
    termios_p.c_lflag = old_c_lflag;
    termios_p.c_oflag = old_c_oflag;
    termios_p.c_iflag = old_c_iflag;
    tcsetattr( mdm, TCSADRAIN, &termios_p);
#else
    dev_mode( mdm, old_mode, _DEV_MODES ); // this'll fail on a socket
#endif
    exit( 0 );
    }

void
final_init() {
    putmsg( sending ? "QCP Send:\n" : "QCP Receive:\n" );
    }

/*----------------------------------------------------------------------------

IMPORT: src_fname - Pathname of file to send
EXPORT: 0 - Error
	Non-zero means OK.
----------------------------------------------------------------------------*/
int snd( signed char *src_fname ) {
    char    buf[2], *dst_fname;
    int     i, status, result;
    struct  first_piece *ptr;

    /* Is an alternate receive file name specified ?                        */
    if ( dst_fname = strchr( src_fname, ',') )  *dst_fname++ = '\0';
    else                                        dst_fname = src_fname;

    if ( ( fd = open( src_fname, O_RDONLY ) ) == -1 ) {
	put_remote( "%s: Unable to open '%s', skipping.\n", pname, src_fname );
	return 1;
	}

#ifdef DIAG
	fprintf(stderr,"src_fname='%s', dst_fname='%s'\r\n",src_fname,dst_fname);
#endif

    seg_num = piece_num = 0;
    seg_bfr_init();
    ptr = (struct first_piece *) &segment_table[0].data;

    if ( fstat( fd, &statbuf ) == -1 ) {
	put_remote("%s: Unable to get fstat info, skipping '%s'.\n", pname, src_fname);
	return 1;
	}

    if ( S_ISDIR( statbuf.st_mode ) ) {                 /* @@@ Why not ?    */
	put_remote("%s: Cannot transmit directories, skipping '%s'.\n", pname, src_fname);
	return 1;
	}

    stat_to_dir_entry( &statbuf, &ptr->dir_info );
    /* Also pass upper 8 bits of uid/gid    */
    ptr->uid_ext = statbuf.st_uid >> 8;
    ptr->gid_ext = statbuf.st_gid >> 8;

    if ( strcmp( src_fname, dst_fname ) )       /* Name change?             */
	putmsg( "Sending %s to %s - ", src_fname, dst_fname );
    else
	putmsg( "Sending %s - ", src_fname );
    buf[0] = buf[1] = SYN;                      /* Start up remote qcp      */
    for ( i = 0; i < retry_limit; ++i ) {
		int ack_rc;
#ifdef DIAG
	fprintf(stderr,"flush_in()\r\n");
#endif
	flush_in();
#ifdef DIAG
fprintf(stderr,"writing SYN to modem (retries=%d)\r\n",i);
#endif
	write_mdm( 2, buf );
#ifdef DIAG
fprintf(stderr,"waiting for remote answer\r\n");
#endif
	if ( (ack_rc=wait_ack()) >= 0 ) break;           /* Wait for remote qcp      */
#ifdef DIAG
fprintf(stderr,"ack_rc=%d\r\n",ack_rc);
#endif
	}
#ifdef DIAG
fprintf(stderr,"finished waiting for remote; retries = %d\r\n",i);
#endif
    if ( i >= retry_limit ) {
	put_remote( "%s: Unable to detect remote %s.\n", pname, pname );
	return 0;
	}
    time( &start_time );
    ++seg_num;
    fbytes = statbuf.st_size;
    rbytes = 0;
    strcpy( ptr->dir_info.fname, dst_fname );
#define NCPFN 16 /* from qnx2 lfsys.h */
    segment_table[0].data_header.len =
	    (sizeof(struct first_piece) - NCPFN) + strlen( dst_fname );
    ptr->flags = (uchar) (newer ? SENDING_NEWER : 0) | IS_4;
#ifdef DIAG
fprintf(stderr,"sending filename and info\r\n");
#endif
    status = send_segment( 1, 0 );          /* Send filename and info       */
#ifdef DIAG
fprintf(stderr,"returned from send_segment()\r\n");
#endif
    switch( status ) {
	case 1:
#ifdef DIAG
fprintf(stderr,"OK; send 0%% message to remote \r\n");
#endif
putmsg( "  0%%\b\b\b\b" );  /* Indicate things have started */
#ifdef DIAG
fprintf(stderr,"Sending segments of file to remote \r\n");
#endif
		if ( status = send_segments( fd ) ) {/* Send file's contents*/
		    time( &stop_time );
		    if ( stop_time == start_time ) ++stop_time;
		    if( status == 2 )
			putmsg( "skipped\n" );
		    else
                    {
			putmsg( "100%% ok  %5ld CPS  %ld:%2.02ld minutes\n",
			    fbytes / (stop_time - start_time),
			    ( stop_time - start_time ) / 60,
			    ( stop_time - start_time ) % 60 );
                    }
		    result = TRUE;          /* Success                      */
		    }
		else result = FALSE;        /* Failure                      */
		break;

	case 2:
#ifdef DIAG
fprintf(stderr,"Remote says to skip this file\r\n");
#endif
	putmsg( "skipped\n" );    /* Remote side already has file */
		result = TRUE;              /* Call it a success            */
		break;

	default:
#ifdef DIAG
fprintf(stderr,"Remote came back with unknown code\r\n");
#endif
	   puterr( "Unknown status (%02x)\n", status );
		    result = FALSE;
		    break;
	}
    if ( !result ) putmsg( "Send failed\n" );
    close_fd();
    if ( abort_check() ) done( 1 );
    return( result );
    }
/*-----------------End of Interface Routines--------------------------------*/

void
seg_bfr_init() {
    int     n;
    for( n = 0; n < NPIECES; ++n )  segment_table[n].rcv_ok = FALSE;
    }

/*----------------------------------------------------------------------------
     This  routine  will  take  an error message and convey it through the
     line protocol to the other side. In this manner, even though QCP  may
     be  using it's own stdin / stdout for the communications link, it can
     send an error message across to the console where an operator may  be
     present.

IMPORT: msg - Pointer to a message to display.
----------------------------------------------------------------------------*/
void
put_remote( signed char *fmt, ... ) {
    va_list arglist;
    char line[200];
    register struct piece_entry *p;

#ifdef DIAG
fprintf(stderr,"put_remote called (%s)\r\n",fmt);
#endif

    fflush( stdout );
    va_start( arglist, fmt );
    if ( !use_stdin ) {                 /* We are running with a console    */
	vsprintf( line, fmt, arglist );
	putmsg( line );                 /* so use normal output routine     */
	return;
	}
    p = &segment_table[0];              /* Point to a send buffer           */
    strcpy( p->data, "\nRemote: " );
    vsprintf( p->data + 9, fmt, arglist );
    if ( sending ) {                /* Send msg to remote protocol engine.  */
	p->rcv_ok = FALSE;
	p->data_header.len = strlen( p->data ) + 1;
	send_segment( 1, TRUE );
	}
    else ack( 0, 0, 0, p->data );       /* Ack will do an exit( 1 );        */
    }

int send_segments( int fd ) {
    struct  piece_entry *p;
    int     stp, status;

    for( stp = 0; stp == 0; ) {
	for( p = &segment_table[0]; p < &segment_table[NPIECES]; ++p ) {
	    p->rcv_ok = FALSE;
	    /* We try and send full 256 bytes packets for public networks   */
	    /* 8, 256 byte subpackets are sent, making a 2K Kbyte packet    */
	    if ( !( p->data_header.len = read( fd, p->data,
			    sub_packet_size - sizeof(p->data_header)-2 ) ) ) {
		++p;            /* Last buffer not full, send what we have  */
		stp = 1;
		break;
		}
	    rbytes += (long) p->data_header.len;
	    }

	if ( ( status = send_segment( p-&segment_table[0], FALSE ) ) != 1 )
	    return( status );

	time( &stop_time );
	if ( stop_time == start_time ) ++stop_time;
        if (fbytes!=0)
	putmsg( "%3u%%  %5ld CPS\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
	    min( 100, (unsigned)(rbytes*100L/fbytes) ),
            rbytes / ( stop_time - start_time ) );
	if ( abort_check() ) done( 1 );
	}
    return 1;
    }

/*----------------------------------------------------------------------------

IMPORT: npieces - Number of pieces to send from the buffer.
	msg - Flag indicating a message is contained in data buffer
EXPORT: 0 - Send failed because of timeout, etc.
	1 - Send succeeded.
	2 - Remote side requested file skip because date was newer.
----------------------------------------------------------------------------*/
int send_segment( int npieces, int msg ) {
    struct  piece_entry *p1, *p2;
    int missed;
    unsigned short n, crc, header_len, header_clen;

    p2 = &segment_table[npieces];

resend:
    piece_num = 0;
    seq = (uchar) (seg_num << 3);           /* In case we don't enter loop */
    for ( p1 = &segment_table[0]; p1 < p2; ++p1, ++piece_num ) {
	if ( p1->rcv_ok )   continue;
	seq = (uchar) ((seg_num << 3) + piece_num);
	p1->rcv_ok = TRUE;                      /* NAK will clear this */
	p1->data_header.syn = SYN;
	p1->data_header.soh = (uchar) (msg ? SO : SOH);
	p1->data_header.seq =  (uchar)  seq;
	p1->data_header.cseq = (uchar) ~seq;

  /* send the len/clen as LE */
	header_len = p1->data_header.len;
	header_clen = (unsigned short)~p1->data_header.len;
	p1->data_header.len = ENDIAN_LE16( header_len );
	p1->data_header.clen = ENDIAN_LE16( header_clen );


	crc = -compute_crc_16( header_len, p1->data );
	crc = ENDIAN_LE16( crc );
  memcpy( p1->data + header_len, &crc, sizeof( crc ) );
	write_mdm( sizeof(p1->data_header) + header_len + 2, &p1->data_header );

	p1->data_header.len = header_len;
	p1->data_header.clen = header_clen;
	}

    if ( msg ) return 0;                        /* Messages get no ack      */

    switch( n = wait_ack() ) {
    case -1:    goto resend;                /* NAK seq !seq, ...        */
    case -2:    puterr( "CAN " ); return 0; /* SYN CAN CAN              */
    case -3:    puterr( "TMO " ); return 0; /* Timeout                  */
    case -4:    puterr( "NXT " ); return 2; /* Advance to next file     */
    default:    ++seg_num;                  /* ACK seq !seq             */
		missed = piece_num - (n + 1);
		if ( missed > 0 ) {     /* Receiver missed a trailing piece or two */
		    puterr( "MIS " );
		    p2 = &segment_table[n + 1];
		    memmove( &segment_table[0], p2, missed*sizeof(*p2) );
		    p2 = &segment_table[missed];
		    seg_bfr_init();
		    goto resend;
		    }
		break;
	}
    return 1;
    }

int wait_ack() {
    unsigned char buf[200], nak_flag = 0, b0;

    if ( read_mdm( 3, 20*onesec, buf ) != 3 ) return -3;    /* Timeout  */

again:
#ifdef DIAG
fprintf(stderr,"read_mdm got 0x%02x 0x%%02x 0x%%02x\r\n",buf[0],buf[1],buf[2]);
#endif

    if ( abort_check() ) done( 1 );
    b0 = buf[0];
    if ( b0 == ACK  ||  b0 == NAK  ||  b0 == SO  ||  b0 == SI ) {
	if ( (uchar) buf[1] == (uchar) ~buf[2] ) {
	    if ( b0 == SO ) {                   /* Error message received   */
		if ( read_mdm( 2, 2*onesec, &buf[3] ) != 2 ) return -3;
		if ( (uchar) buf[3] == (uchar) ~buf[4] ) {  /* Check string length  */
		    if ( read_mdm( buf[3], 5*onesec, &buf[5] ) != buf[3] )
			return -3;
		    putmsg( &buf[5] );  /* Display error message and exit   */
		    cancel();
		    exit( 1 );
		    }
		}
	    if ( b0 == SI ) {           /* Send_next_file command received  */
		return -4;
		}
	    if ( (uchar) (buf[1] & 0xf8) == (uchar) ((seg_num << 3) & 0xf8) )
		switch( b0 ) {
		    case ACK:
			puterr( "ACK(%2.2x) ", buf[1] & 0xff );
			return( buf[1] & 0x07 );            /* ACK received */

		    case NAK:
			++nak_flag;
			segment_table[ buf[1] & 0x07 ].rcv_ok = FALSE;
			puterr( "NAK(%2.2x) ", buf[1] & 0xff );
			return -1;                      /* NAK received */

		    default:
			puterr( "NOT-ACK " );
			return( buf[1] & 0x07 );
		    }
	    puterr( "ACK?(%2.2x) ", b0 & 0xff );
	    }
	}

    if( b0 == SYN  &&  buf[1] == CAN  &&  buf[2] == CAN )
	return -2;                          /* CAN received */

    if ( b0 >= ' '  &&  b0 < 0x7f ) puterr( "?%c ", b0 );
    else                            puterr( "?%2.2X ", b0 & 0xff );

    buf[0] = buf[1];    /* Shuffle down one byte and get next byte      */
    buf[1] = buf[2];
    /* @@@ Long timeout */
    if ( read_mdm( 1, 10*onesec, &buf[2]) != 1 )    return -3;  /* Timeout  */

    goto again;         /* Try for another ack / nak / skip */
    }

void
cancel() {
    uchar buf[3];

    buf[0] = SYN; buf[1] = buf[2] = CAN;
    write_mdm( 3, buf );
    write_mdm( 3, buf );
    }
