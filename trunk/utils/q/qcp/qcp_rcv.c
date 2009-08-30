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





/*              14-Dec-89  2:18:29pm                                        */

/*--------------------------------------------------------------------------*/
/*  History: qcp_rcv.c, V0.0, ??-Nov-86  1:20:08pm, Dan Dodge,  Baseline    */
/*                      V0.1, 13-Jan-87  1:20:08pm, Dan Hildebrand          */
/*----------------------------------------------------------------------------
     This source module contains the code for receiving files with the QCP
     protocol.
----------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <errno.h>
#include    <fcntl.h>
#include    <termios.h>
#include    <unistd.h>
#include    <utime.h>
#include    <time.h>
#include    <signal.h>
#include    <gulliver.h>
#ifdef __QNXNTO__
#include <devctl.h>
#endif

#define EXT     extern              /* Force all manifests to be external   */
#include        "main.h"
#include        "qcp.h"

#define MAX_INTER_FRAME_NOISE   1000

int     last_piece;
int     num_pieces_needed;
char    failure;
char    filename[  240 ];
static char fname[ 240 ];
int     fd = -1;

int     got_syn;

void sighangup(int i) {
    i=i; 
    delete_file();
    signal(SIGHUP, SIG_DFL);
    raise(SIGHUP);
    }

/* QCP receives file names, so command line names don't matter.         */
#if defined(__WATCOMC__) 
#pragma off(unreferenced)
#endif
void
receive_files( int argc, signed char *argv[] ) {
#if defined(__WATCOMC__)
#pragma on(unreferenced)
#endif
    int     i, r;
    uchar   c;


    flush_in();
    signal(SIGHUP, &sighangup);
    for(;;) {
	got_syn = 0;
	i = 0;                                      /* Set sync count to 0  */
	time( &start_time );
	r = rcv();
	switch( r ) {
	case 0: done( 0 );                          /* Abort on rcv fail    */
	case 1:
again:      if ( read_mdm( 1, 4*onesec, &c ) != 1 ) {
		puterr( "DONE " );
		done( 0 );
		}
	    else if ( c == SYN ) {                  /* Restart rx process   */
		got_syn++;
		proto_ungetc( c );
		continue;
		}
	    else if ( ++i <= 20 ) {                 /* Only 20 non-SYN chars*/
		if ( c != CAN ) {
		    putc( c, stderr );
		    fflush( stderr );
		    }
		goto again;
		}
	    else done( 0 );
	    break;
	case 2: continue;                           /* Skip to next file    */
	    }
	}
    }


/*----------------------------------------------------------------------------
     This  routine processes the incoming packets until the entire file is
     received.

IMPORT: none.
EXPORT: 0 - The receive failed.
	1 - The receive succeeded.
	2 - The receive was skipped because the file already existed with
	    the same or newer date.
----------------------------------------------------------------------------*/
int rcv() {
    register int timeouts;
    int     noise, status;
    uchar   c;

    seg_num = 0;
    failure = FALSE;
    last_piece = -1;
    num_pieces_needed = 1;
    seg_bfr_init();

#ifdef DIAG
	fprintf(stderr,"rcv()\r\n");
#endif
    /*  Let transmitter know we are ready to receive.    */
    if ( !got_syn && !ack( ACK, 0, 1, NULL ) ) {
#ifdef DIAG
	fprintf(stderr,"NOACK1; returning 0\r\n");
#endif
		puterr( "NOACK1 ");
		cancel();
		return 0;
	}

#ifdef DIAG
	fprintf(stderr,"rcv(): starting loop\r\n");
#endif

    ++seg_num;
    for( timeouts = noise = 0; timeouts < retry_limit; ++noise ) {
#ifdef DIAG
	fprintf(stderr,"rcv(): waiting up to 5s for opening SYN\r\n");
#endif
	if ( read_mdm( 1, 5*onesec, &c ) == 1 ) {   /* Get opening SYN      */
	    if ( c != SYN ) continue;               /* Throw away junk      */
#ifdef DIAG
	fprintf(stderr,"rcv(): got SYN; waiting for next char\r\n");
#endif
	    if ( read_mdm( 1, 5*onesec, &c ) == 1 ) {
		switch( c ) {
		case SYN:                           /* Startup frame        */
#ifdef DIAG
	fprintf(stderr,"rcv(): got SYN SYN (startup frame)\r\n");
#endif
		    timeouts = noise = 0;
		    if ( fd == -1 ) {
			seg_num = 0;    /* Let tx side know we are ready    */
#ifdef DIAG
	fprintf(stderr,"rcv(): sending ACK\r\n");
#endif
			if ( !ack( ACK, 0, 1, NULL ) ) {
			    puterr( "NOACK2 ");
			    cancel();
			    return 0;
			    }
			++seg_num;
			}
		    else proto_ungetc( c );
		    continue;

		case CAN:                           /* Cancel frame         */
#ifdef DIAG
	fprintf(stderr,"rcv(): got SYN CAN (cancel frame)\r\n");
#endif
		    timeouts = noise = 0;
		    if ( read_mdm( 1, 5*onesec, &c ) == 1 ) {
			if ( c == CAN ) {
			    /*cancel(); we just got shutdown, don't send a cancel*/
			    return 0;
			    }
			proto_ungetc( c );
			}
		    continue;
			    
		case SOH:                           /* Data frame           */
#ifdef DIAG
	fprintf(stderr,"rcv(): got SYN SOH (data frame)\r\n");
#endif
		    timeouts = noise = 0;
		    if ( rcv_piece( TRUE ) ) {      /* Special piece rxed ? */
			status = save_segment();
			if ( status == 2 ) return 2;        /* File skipped */
			if ( status == 1 ) {
			    writelog("RCV", !failure, fname);
			    if ( failure ) {
				delete_file();
				cancel();
				}
			    else {
				time( &stop_time );
				if ( stop_time == start_time ) ++stop_time;
				putmsg( "100%% ok  %5ld CPS  %0ld:%2.02ld minutes\n",
				    fbytes / ( stop_time - start_time ),
				    ( stop_time - start_time ) / 60,
				    ( stop_time - start_time ) % 60 );
				close_fd();
				date_stamp();
				*fname = '\0';
				if ( abort_check() ) done( 0 );
				}
			    return !failure;
			    }
			}
		    continue;

		case SO:                        /* Remote message frame     */
#ifdef DIAG
	fprintf(stderr,"rcv(): got SYN SO (remote message frame)\r\n");
#endif
		    timeouts = noise = 0;
		    rcv_piece( FALSE );         /* Receive the message      */
		    delete_file();              /* Throw away any data      */
		    return 0;                   /* Force a failure          */

		default:
		    puterr( "HDR(%02x) ", (unsigned) c );
		    continue;
		    }
		}
	    }   /* Of read opening SYN  */
#ifdef DIAG
fprintf(stderr,"rcv(): timed out waiting for SYN\r\n");
#endif
	puterr( "TO(SYN) " );
	if ( save_segment() == 2 ) return 2;    /*  Skip file       */
	++timeouts;
	}   /* Of for ( timeouts....    */

    /*  The transmitter has died     */
    puterr( "TRANS_DEATH " );
    delete_file();
    /*cancel(); If the transmitter has died, why cancel him? */
    return 0;
    }

/*----------------------------------------------------------------------------
     This will return TRUE if we have received a complete  set  of  pieces
     and  we  do  not believe that the sender is going to send us any more
     data until we ACK or NAK him. In the case of a timeout we also return
     TRUE since the sender is not sending and must be kicked.  Returning a
     TRUE kicks the sender.  The routine save_segment() will determine how
     best to kick him (ACK or NAK's).

IMPORT: data = TRUE, receive a data frame, return a 0 or 1
	data = FALSE, receive a message frame, return a -1
EXPORT: TRUE - All OK or a timeout.
	FALSE - Bad packet.
----------------------------------------------------------------------------*/
int rcv_piece( uchar data ) {
    struct  piece_entry *p;
    int     i;
    unsigned short tmp;

    if( read_mdm(sizeof(header)-2, 5*onesec, &header.seq) != sizeof(header)-2) {
	puterr( "TO(HDR) " );
	return 1;   /*  Timeout */
	}

	header.len = ENDIAN_LE16( header.len );
	header.clen = ENDIAN_LE16( header.clen );

    if ( (uchar) header.seq != (uchar) ~header.cseq ) {
	puterr( "SEQ#(%02x != ~%02x) ", (unsigned)header.seq, (unsigned)header.cseq);
	return 0;
	}

    if ( (unsigned short)header.len != (unsigned short)~header.clen  &&  header.len <= MAXSIZE ) {
	puterr( "LEN(%04x != ~%04x)  ", header.len, header.clen );
	return 0;
	}

    if ( (uchar)(header.seq & 0xf8) != (uchar) ((seg_num << 3) & 0xf8 ) ) {
	puterr( "SEG#(%02x != %02x) ", (header.seq>>3) & 0x1f, seg_num );
	return 0;
	}

    last_piece = max( last_piece, (int) (piece_num = (uchar) (header.seq & 0x07)));

    p = &segment_table[ piece_num ];
    if( !p->rcv_ok ) --num_pieces_needed;   /* Yes we dec on a good seq before the data */

    p->data_header = header;
    i = read_mdm(   header.len + 2, 5*onesec, p->data );
    if ( i != header.len+2 ) {
	puterr( "Timeout(DAT) " );
	return 1;
	}

    if ( abort_check() ) {
	delete_file();
	done( 0 );
	}

    memcpy( &tmp, &p->data[header.len], 2 );
		tmp = ENDIAN_LE16( tmp );
    if ( (short unsigned)( compute_crc_16( header.len, p->data ) + tmp) != 0 )
	puterr( "BadCRC(%d vs %d ", compute_crc_16(header.len, p->data), -tmp );
    else
	p->rcv_ok = TRUE;

    if ( !data ) {
	putmsg( p->data );                  /* Output null terminated text  */
	return 0;                           /* Always abort for messages    */
	}

    return(     piece_num == 7      ||                  /* All pieces       */
		!num_pieces_needed  ||                  /* No pieces needed */
		(!header.len ? 1 : 0)   );              /* Empty header     */
    }

/*----------------------------------------------------------------------------
     Figure out if we have received a complete set of pieces.  If  so  ACK
     them, otherwise  NAK  the missing pieces. It will return -1 after the
     last segment of the transmission is saved, or if the transmitter  has
     died (detected  only  on  ACK), or if the file can't be opened. -2 is
     returned if the file already exists  and  +newer  is  asserted.    To
     summarize,  a  negative  number indicates the receive is finished and
     the global "failure" indicates why.

IMPORT: Global receive buffer.
EXPORT: 0 - Segment saved, ready for next.
	1 - Unable to open file or done or tx side died.
	2 - File being downloaded is not newer than existing file, skip it.
----------------------------------------------------------------------------*/
int save_segment() {
    register struct piece_entry *sp1;
    register uchar nak_flag = 0;
    int tlast_piece = last_piece;

    if ( last_piece == -1 )
	tlast_piece = 7;        /*  If nothing received, NAK all    */
    else if ( segment_table[last_piece].data_header.len  &&  fd != -1 )
	tlast_piece = 7;        /*  If not first or last, check all */

    for( sp1 = &segment_table[0]; sp1 <= &segment_table[tlast_piece]; ++sp1 ) {
	if ( !sp1->rcv_ok ) {
	    puterr( "NAK(%02x) ",
		((seg_num << 3) + (sp1 - &segment_table[0]) & 0xFF) );
/*  The transmitter can never be detected as dying on a NAK. Don't set more */
	    ack( NAK, sp1 - &segment_table[0], 0, NULL );
	    ++nak_flag;
	    }
	}
    num_pieces_needed = (int) nak_flag;

    if ( !nak_flag ) {                          /*  No holes. ACK highest   */
	/* No file open yet, so assume data packet is file info             */
	if ( fd == -1 ) {
	    switch( open_file() ) {
		case  0:    return 1;   /* Unable to open the file      */
		case  1:    putmsg( "  0%%\b\b\b\b" );      break;
		case  2:    putmsg( "skipped\n" );
			    seg_num = 0;
			    ack( SI, 0, 0, NULL );  /* Tell txer to skip it */
			    return 2;   /* We already have the file     */
		}
	    }
	else save_data( last_piece );

	seg_bfr_init();
	if( !ack( ACK, last_piece, segment_table[last_piece].data_header.len, NULL ) )
	    return 1;       /* Transmitter died */
	++seg_num;
	last_piece = -1;
	num_pieces_needed = 8;
	}

    return( !nak_flag && !segment_table[tlast_piece].data_header.len );
    }

/*----------------------------------------------------------------------------
     This is used for ACKS, NAKS etc... It returns 0  if  the  transmitter
     times out.   This only happens on an ACK that is not at EOF. A 3 byte
     frame is built and sent that  contains  the  response  code  and  the
     sub-packet number  that  is being acked or nacked. If a message is to
     be sent, the ack packet length is extended with a byte count and  the
     message.

IMPORT: code - The control code to imbed within the ack frame.
	    0 - Use only the message.
	    ACK - OK
	    NAK - Error
	    SI - Skip to the next file
	piece - The number of the sub-packet to ack / nak.
	more - Flag indicating that next frame of data is wanted.
	msg - Pointer to an error message. Set to NULL for normal operation.
		If pointing to a message, msg will be sent and qcp terminated.

EXPORT: True / False indication of SYN response to ACK.
----------------------------------------------------------------------------*/
int ack( uchar code, uchar piece, uchar more, uchar *msg ) {
    static uchar ack_msg[100], c;
    int     n;

    ack_msg[1] = (uchar) ((seg_num << 3) + piece);  /* Place seg and !seg into frame */
    ack_msg[2] = (uchar) ~ack_msg[1];

    if ( msg ) {                            /* If a message, send it too    */
	ack_msg[0] = SO;                    /* Indicate msg                 */
	n = strlen( msg ) + 1;              /* Send string length + null    */
	ack_msg[3] = (uchar) n;             /* Maximum of 256 bytes         */
	ack_msg[4] = (uchar) ~n;
	strcpy( &ack_msg[5], msg );         /* Place message into frame     */
	write_mdm( 5 + n, &ack_msg );
	putmsg( "QCP Aborted.\n" );
	delete_file();
	done( 0 );
	}

    ack_msg[0] = code;                      /* Indicate control code        */
    write_mdm( 3, ack_msg );                /* Send ack / nak / skip        */

    if ( more && code == ACK ) {
	c = 0;
	for ( n = 0; n < retry_limit  &&  c != SYN; ++n ) {
	    if ( abort_check() ) {
		delete_file();
		done( 0 );
		}
	    if ( read_mdm( 1, 5*onesec, &c ) != 1 ) puterr( "ACK-TO " );
	    else if ( c == SYN ) break;
	    flush_in();
	    puterr( "RE-ACK " );
	    write_mdm( 3, ack_msg );
	    }

	if ( n >= retry_limit ) {
	    failure = TRUE;
	    puterr( "NO-ACK " );
	    return 0;
	    }
	proto_ungetc( SYN );                /* Force hunt mode again        */
	}
    return 1;
    }

/*----------------------------------------------------------------------------
     Open the file and set the directory entry. If the +newer command line
     option is in effect, the file may not be opened if it already  exists
     with the same or newer date.

IMPORT: Received block containing directory entry of file.
EXPORT: 0 - Cannot open file.
	1 - File opened, ready to proceed.
	2 - We already have a file with this date or newer.
----------------------------------------------------------------------------*/
static char err1[]  = "Unable to open file '%s'.\n";
static char err2[]  = "Bad filename '%s'.\n";
static char err3[]  = "Unable to get date from '%s'.\n";
static char err4[]  = "Unable to modify dir entry '%s'.\n";
static char err5[]  = "Disk write error, transfer incomplete.\n";
static char err6[]  = "Cannot receive a directory.\n";
static char err7[]  = "Cannot set rxed file date.\n";
#ifndef __QNXNTO__
static char err8[]  = "Cannot truncate existing file.\n";
#endif

static struct utimbuf utimes;   /* Used to date stamp file after rxing  */

int open_file() {
    struct  first_piece *file_info;
    char    *p, do_newer = newer;
    unsigned device, tx_uid, tx_gid;    /* Transmitted uid and gid  */

    file_info = (struct first_piece *) &segment_table[0].data[0];
    if ( !make_file_name( fname, file_info->dir_info.fname ) ) {
	put_remote( err2, file_info->dir_info.fname );
	return 1;
	}
    strcpy( filename, fname );
    if ( file_info->flags & SENDING_NEWER ) do_newer = TRUE;
    putmsg( "File %s - ", fname );
    fbytes = ( (long)file_info->dir_info.fnum_blks+1) * 512;
    rbytes = 0L;

    p = fname;
    if ( *p == '[' )                        /* Skip over the node number    */
	while( *p  &&  *p++ != ']' ) ;
    device = ( *p == '$' );

    if ( !device  &&  (file_info->dir_info.fattr & _DIRECTORY) ) {
	put_remote( err6, fname );
	failure = TRUE;
	close_fd();
	return 0;
	}

    file_info->dir_info.fseconds += TEN_YEARS;  /* Convert to Posix base    */

    /* If file exists ... */
    if ( stat( fname, &statbuf ) == 0 ) {
	if ( do_newer ) {
	    if ( file_info->dir_info.fseconds != 0L ) {
		if ( statbuf.st_mtime >= file_info->dir_info.fseconds ) {
		    fd = -1;    /* Skip over this file, we already have it. */                  return( 2 );                    }
		}
	    }
	}
    else if( errno != ENOENT) {     /* Any error except file doesn't exist */
	put_remote( err3, fname );
	failure = TRUE;
	fd = -1;
	return( 0 );
	}

    if ( unlink_file ) unlink( fname );

    if ( ( fd = open( fname, O_WRONLY|O_CREAT, 0666 ) ) == -1 ) {
		put_remote( err1, fname );
		*fname = '\0'; failure = TRUE; fd = -1;
		return 0;
	}

    statbuf.st_mode = compute_mode( file_info->dir_info.fattr,
				    file_info->dir_info.fgperms,
				    file_info->dir_info.fperms      );

    tx_uid = file_info->dir_info.fowner;    /* Get least sig 8 bits     */
    tx_gid = file_info->dir_info.fgroup;

    /* Undo the inversion done for 2.15 compatibility   */
    if ( tx_gid == 255 )    tx_gid = 0;
    else if ( tx_gid == 0 ) tx_gid = 255;

    if ( tx_uid == 255 )    tx_uid = 0;
    else if ( tx_uid == 0 ) tx_uid = 255;

    if ( file_info->flags & IS_4 ) {        /* A QNX4.0 system sending? */
	/* Or in the upper 8 bits of the uid/gid numbers                */
	tx_uid |= (file_info->uid_ext << 8);
	tx_gid |= (file_info->gid_ext << 8);
	}

    if ( !flashfile && fchmod( fd, statbuf.st_mode ) == -1 && fchown( fd, tx_gid, tx_uid ) == -1 ) {
	put_remote( err4, fname );      /* Unable to set dir entry  */
	failure = TRUE;
	close_fd();
	return 0;
	}

#ifndef __QNXNTO__
    if ( !flashfile && ltrunc( fd, 0, SEEK_SET ) == -1 ) {
	put_remote( err8, fname );
	failure = TRUE;
	close_fd();
	return 0;
	}
#endif

    /* Get today's date into both date fields   */
    if ( today ) time( (time_t *)&file_info->dir_info.fseconds );
    /* Preserve the original file date for later file stamping  */
    utimes.actime = file_info->dir_info.fseconds;
    utimes.modtime = file_info->dir_info.fseconds;
    return 1;                       /* Open for write           */
    }

void
date_stamp() {
    if ( !flashfile && utime( filename, &utimes ) == -1 ) {
	put_remote( err7, fname );
	failure = TRUE;
	close_fd();
	}
    }

/* Save the data to wherever.  */
void
save_data( int last_piece ) {
    int     n, size;

    for( n = 0; n <= last_piece; ++n ) {
	size = segment_table[n].data_header.len;
	if ( size && write( fd, segment_table[n].data, size ) != size ) {
	    put_remote( err5 );         /* @@@ Tell user exact error ?  */
	    delete_file();
	    done( 1 );
	    }
	rbytes += (long) size;
	}
    time( &stop_time );
    if ( stop_time == start_time ) ++stop_time;
    putmsg( "%3d%%  %5ld CPS\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
	min( 100, (unsigned)((rbytes*100L)/fbytes) ),
	rbytes / ( stop_time - start_time ) );
    if ( abort_check() ) {
	delete_file();
	done( 0 );
	}
    }

void
delete_file() {
    close_fd();
    if ( *fname ) unlink( fname );
    }

void
close_fd() {
    if ( fd != -1 ) {
	close( fd );
	fd = -1;
	}
    }

/*
 * Open Modes and File Attributes/Permissions for QNX2.15
 */

#define Q_READ          0x01
#define Q_WRITE         0x02
#define Q_APPEND        0x04
#define Q_CREATE        0x04
#define Q_EXECUTE       0x08
#define Q_BLOCK         0x08
#define Q_MODIFY        0x10
#define Q_DIRECTORY     0x20

void
stat_to_dir_entry( struct stat *s, struct old_dir_entry *d ) {
    d->fseconds = (time_t) (s->st_mtime - TEN_YEARS);
    ndtood( d->fseconds, d->fdate );
    d->fowner = s->st_uid;          /* Preserve group and member    */
    d->fgroup = s->st_gid;
    /* 0 and 255 are reversed in meaning between QNX 2.15 and 4.0   */
    if ( s->st_uid == 0 ) d->fowner = 255;
    else if ( s->st_uid == 255 ) d->fowner = 0;
    if ( s->st_gid == 0 ) d->fgroup = 255;
    else if ( s->st_gid == 255 ) d->fgroup = 0;
    d->ftype = 0;
    d->fattr = Q_MODIFY;
    if ( s->st_mode & S_IREAD  ) d->fattr |= Q_READ;
    if ( s->st_mode & S_IWRITE ) d->fattr |= Q_WRITE;
    if ( s->st_mode & S_IEXEC  ) d->fattr |= Q_EXECUTE;
    d->fgperms = 0;
    if ( s->st_mode & S_IRGRP  ) d->fgperms |= Q_READ;
    if ( s->st_mode & S_IWGRP  ) d->fgperms |= Q_WRITE;
    if ( s->st_mode & S_IXGRP  ) d->fgperms |= Q_EXECUTE;
    d->fperms = 0;
    if ( s->st_mode & S_IROTH  ) d->fperms |= Q_READ;
    if ( s->st_mode & S_IWOTH  ) d->fperms |= Q_WRITE;
    if ( s->st_mode & S_IXOTH  ) d->fperms |= Q_EXECUTE;
/*  d->fnum_xtnt = s->st_num_xtnts; */
    d->fnum_blks = s->st_size / 512;
    }

/* Convert QNX permission bits into a POSIX mode_t value    */
mode_t
compute_mode( uchar attr, uchar gperm, uchar perm ) {
    mode_t  mode = 0;

    if ( attr  & Q_READ     ) mode |= S_IREAD;
    if ( attr  & Q_WRITE    ) mode |= S_IWRITE;
    if ( attr  & Q_EXECUTE  ) mode |= S_IEXEC;
    if ( gperm & Q_READ     ) mode |= S_IRGRP;
    if ( gperm & Q_WRITE    ) mode |= S_IWGRP;
    if ( gperm & Q_EXECUTE  ) mode |= S_IXGRP;
    if ( perm  & Q_READ     ) mode |= S_IROTH;
    if ( perm  & Q_WRITE    ) mode |= S_IWOTH;
    if ( perm  & Q_EXECUTE  ) mode |= S_IXOTH;
    return( mode );
    }
