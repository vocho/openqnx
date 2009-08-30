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






/*
#ifdef __USAGE
%C - full-screen patch utility (QNX)

%C	[-bp] filename [offset]
%C	-m [-bp] pid selector offset
Options:
 -b       Browse only (don't allow save).
 -p       Pause before starting.
 -m       Patch memory instead of a disk file. A pid, selector and
          offset must be specified as operands.
Where:
 filename is the name of the file or disk to patch.
 offset   is the beginning address (hexadecimal) to position at.
          If 'filename' is a block device, then offset may be the
          name of a file or directory (the beginning address will
          be the first block of that file or directory).
#endif

#ifdef __USAGENTO
%C - full-screen patch utility (QNX)

%C	[-bp] filename [offset]
Options:
 -b       Browse only (don't allow save).
 -p       Pause before starting.
Where:
 filename is the name of the file or disk to patch.
 offset   is the beginning address (hexadecimal) to position at.
          If 'filename' is a block device, then offset may be the
          name of a file or directory (the beginning address will
          be the first block of that file or directory).
#endif
*/

#ifdef __QNXNTO__
#define _FILE_OFFSET_BITS 64
#endif

#include <unistd.h>
#include <time.h>
/* #include <term.h> */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <lib/compat.h>
#include <util/diskman.h>
#include <util/stdutil.h>
#include <sys/debug.h>
#include <util/qnx4dev.h>
#ifndef __QNXNTO__
	#include <sys/disk.h>
	#include <sys/fsys.h>
	#include <sys/kernel.h>
	#include <sys/seginfo.h>
#endif

#include <util/fs_qnx4_util.h>

#include <sys/qnxterm.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#ifdef __QNXNTO__
#define _BLOCK_SIZE (512L)
#undef MEM_SUPPORTED
#undef FSYS_STAT_SUPPORTED
#else
#define MEM_SUPPORTED
#define FSYS_STAT_SUPPORTED
#endif

#define CLRline(l) 		{term_cur(l,0); term_clear(1); term_flush();}

#define ESCAPE			(0x1B)
#define MENUline		 0
#define PRMTline		 1
#define ROW(i)			(2+i+(i/4))
#define ERRORline       21
#define FILEline		22
#define MARKline		23

#define BEST			1
#define GOOD			2
#define FAIR			3
#define MAXfound		1000

#define ABSOLUTE_ADDR	0
#define BLOCK_ADDR		1
#define SEGMENT_ADDR	2

#define FILE_IO			0
#define BLOCK_IO		1
#define MEM_IO			2

#define FILEtype		0
#define DIRtype			1
#define COPYtype		2
#define APPENDtype		3

typedef off_t _offval_t;

#ifdef FSYS_STAT_SUPPORTED
struct _fsys_stat fst;
#endif

typedef struct {
	_offval_t off;
	_offval_t blk;
} offset_t;

char *get_line(char *);
int  conv_addr(offset_t *, char *);
int  calc_offset(offset_t *,char *,int);
long  prepXblk(long, char *);
void  do_copy(int, long, char *);
int   compare_offset(offset_t, offset_t);
void  inc_offset(offset_t *, _offval_t);
extern char *basename(char *);
void build_xblk_list();
void get_buffer(char *, offset_t);
void display(char *, offset_t);
void check_dirty();
int find_pattern();
int get_pattern(char *);
void edit(unsigned, unsigned);
int get_addr();
void end_mark();
void put_buffer(char *, offset_t);
void put_addr(int, int, offset_t, char);
void put_data(unsigned, unsigned, unsigned,unsigned char, unsigned);
void showmark();
int hexval(char);
void errmsg( char *, char * );
int dev_ischars (int);
char getans( int, char *);
void savemarked( int, char * );
int chkXblk(long, long, char *);
void do_link( int, long, char * );
void link_1(int, long, long, long);
void copy_1(FILE *, long, long);
int verify_fsys( char * );
int addr(char *);

unsigned equiv[]  = { K_PGUP, 'p', K_PGDN, 'n', K_HOME, 'h', K_END, 'l', 0 };
char *cmds_menu[] = {
	"Edit", "Next", "Prev", "Home", "Lastblk", "Goto", "Find", "Continue", "Save", "Addr", "Quit", NULL
	};
char *restricted_menu[] = {
	"Edit", "Next", "Prev", "Home", "Lastblk", "Goto", "Find", "Continue", "Addr", "Quit", NULL
	};
char *block_menu[] = {
	"Edit", "Next", "Prev", "Home", "Lastblk", "Goto", "Find", "Continue", "Save", "Addr", "Display", "Quit", NULL
	};
char *mark_menu[] = {
	"Edit", "Next", "Prev", "Home", "Lastblk", "Goto", "Find", "Continue", "Save", "Addr", "Display", "Mark", "Quit", NULL
	};
char *svmark_menu[] = {
	"Append_Off_Disk", "Copy_Off_Disk", "Directory_Save", "File_Save", "Remove_marks", /*"Quit_MarkMenu",*/ NULL
	};

unsigned x_equiv[]  = { K_PGUP, 'p', K_PGDN, 'n', K_HOME, '1', K_END, 'l', 0 };
char *x_menu[] = {
	"Next_Xblock", "Prev_Xblock", "1st", "Last", "Save_to_lost+found", "Quit_XblockMenu", NULL
	};

char *match[] = {
	"NOT a match", "BEST", "Good", "Fair", NULL
	};

struct foundLIST {
	char	 q;
	offset_t x;
	} foundLIST[MAXfound+1];

char  hex[] = "0123456789ABCDEF";
char  line[3*16 + 3 + 16];
char  pattern[80];

char  buffer[_BLOCK_SIZE];
int   buffer_errno=EOK; /* errno from last attempt to read buffer */

char  d_buffer[_BLOCK_SIZE];
char  x_buffer[_BLOCK_SIZE];
char  dirname[_POSIX_PATH_MAX+1];
char *dn, *fn, filenm[80];
char  oln[81];
char *offptr;

#ifdef MEM_SUPPORTED
struct _seginfo		 seginfo;
struct _osinfo		 osdata;
#endif

struct stat			 stats;
offset_t offset;
offset_t totlen;
offset_t lastblk;
offset_t file_base;
_offval_t mark = -1;
_offval_t emark = -1;
unsigned protected, pid;
unsigned cur_row, cur_col;
_offval_t segment, displacement;
unsigned perms = O_RDWR;
unsigned found;	/* index into foundList[] */
unsigned best;
unsigned good;
int pat_len;
int len = 256;
char block;
char io_type;
char addr_type;
char ascii;
char dirty;
char *cmd;
int		fd;
int		pauseFLAG,
		browseFLAG;
int  saved_errno;	/* used to store errno after errors but before printing
                       the error message */

/* lib/mig4nto doesn't have the right type for blkno.  So just make a
 * wrapper for spatch */
int
_block_read (int fd, _offval_t blk, int nblks, char *buf)
{
	blk --;                     /* readblock starts at blkno 0 */
	if (blk < 0 || blk > UINT_MAX) { /* readblock's blknum is unsigned */
		errno = EINVAL;
		return -1;
	}
	return readblock(fd, 512, blk-1, nblks, buf);
}

int
main(argc, argv)
char argc, *argv[];
	{
	int		 c, n, error = 0;
	char	*p, *p1, **menu;
#ifdef MEM_SUPPORTED
	int		spatchmem=0;
#endif

	cmd = basename(argv[0]);

	if ( argc < 2 ) {
		fprintf(stderr,"spatch: missing command line arguments\n");
		return 1;
	}

	while((c = getopt(argc, argv, "=:bpm")) != -1)
		{
		switch(c)
			{
#ifdef MEM_SUPPORTED
			case 'm':
				spatchmem = 1;
				break;
#endif

			case '=':
				break;

			case 'p':
				++pauseFLAG;
				break;

			case 'b':
				browseFLAG = 1;
				perms = O_RDONLY;
				break;

			case '?':
				error++;
				break;
			}
		}

	if( error )
		exit( EXIT_FAILURE );

	if( pauseFLAG )
		{
		printf( PAUSEphrase );
		fflush( stdout );
		while( getchar() != '\n' )			/* Consume newline */
			;
		}

	if(term_load() != 0) {
		sprintf( buffer, "%s: Can't read terminfo entry", cmd);
		perror(buffer);
		exit( EXIT_FAILURE );
		}
	term_clear(TERM_CLS_SCR);
	term_state.cache_pos=0;


#ifdef MEM_SUPPORTED
	qnx_osinfo((nid_t)0, &osdata);
	protected = (unsigned) (osdata.sflags & _PSF_PROTECTED);

	if (spatchmem)
		{
		if ((argc - optind) < 3)
			{
			fprintf(stderr, "wrong number of arguments\n\n");
			exit(EXIT_FAILURE);
			}
		pid = strtol(argv[optind], 0, 0);
		segment = (_offval_t)atoll(argv[++optind]);
		if (protected)
			offset.off = (_offval_t)atoll(argv[++optind]);
		else
			offset.off = (((long) segment) << 4) + (_offval_t)atoll(argv[++optind]);

		io_type = MEM_IO;
		addr_type = SEGMENT_ADDR;
		qnx_segment_info(PROC_PID, pid, segment, &seginfo);
		file_base.blk=file_base.off= 0;
		totlen.blk=lastblk.blk=0;
		totlen.off = lastblk.off = 999999999L;
		menu = (browseFLAG) ? restricted_menu : cmds_menu;
		sprintf(filenm, "MEM: pid %d, segment %04llx, offset %04llx", pid,
				(long long)segment, (long long)offset.off);
	}
	else
#endif
	{
		n = ((argc-optind) >= 2 && (strequal(argv[optind],"file") || strequal(argv[optind],"FILE"))) ? 1 : 0;
		if( (optind+n+1) < argc )
			offptr = argv[optind+n+1];
	
		fn = argv[optind+n];
#ifdef FSYS_STAT_SUPPORTED
		if( fsys_stat(fn, &fst) == (-1) ) {
#endif
			if( stat(fn, &stats) == (-1) ) {
				saved_errno=errno;
				term_flush();
  				term_cur( term_state.num_rows - 1, 0 );
  				term_clear( TERM_CLS_EOL );
  				term_restore();
				fflush(stdout);
				sprintf( buffer, "%s: Unable to stat '%s'", cmd, fn );
				errno=saved_errno;
				perror( buffer );
				exit( EXIT_FAILURE );
				}
			if( S_ISDIR(stats.st_mode) ) {
				/* 
				 This will only allow the user to look at the returned
				 stat information but some people might be interested
				 in that information.
				*/
				perms = O_RDONLY;
				/* Old funky directory support -- deprecated.
				term_cum( term_state.num_rows - 1, 0 );
				term_clear( TERM_CLS_EOL );
				term_restore();
				sprintf( buffer, "%s: Unable to fsys_stat the directory '%s'", cmd, fn );
				perror( buffer );
				exit( EXIT_FAILURE );
				*/
				}
#ifdef FSYS_STAT_SUPPORTED
		} else {
/* funky directory support -- did this ever work? zzx
			if( S_ISDIR(fst.st_mode) ) {
				offptr = NULL;
				build_blk_list(ENDIAN_DADDR_T(fst.st_first_xtnt.xtnt_blk), ENDIAN_SIZE_T(fst.st_first_xtnt.xtnt_size));
				fn = get_disk_name( dn = fn, dirname );
				}
*/
			}
#endif
	
		if( (fd = open(fn, perms)) == -1 ) {
			term_cur( term_state.num_rows - 1, 0 );
			term_clear( TERM_CLS_EOL );
			term_restore();
			sprintf( buffer, "%s: Unable to open '%s'", cmd, fn );
			perror( buffer );
			exit( EXIT_FAILURE );
			}
	
#ifdef FSYS_STAT_SUPPORTED
		if( dn != NULL )
			build_xblk_list();				/* blocks from the xblock lists */
#endif
	
		fstat( fd, &stats );
		if( block = S_ISBLK(stats.st_mode) )
		{
			file_base.blk=(dn != NULL) ? 0L : 1L;
			file_base.off=0;
			io_type = (char) BLOCK_IO;
			addr_type = (char) ((dn != NULL) ? ABSOLUTE_ADDR : BLOCK_ADDR);
			strcpy( filenm, "BLOCK: " );
			menu = (browseFLAG) ? restricted_menu : (dn != NULL) ? block_menu : mark_menu;
		} else {  /* FILE */
			io_type = (char) FILE_IO;
			addr_type = (char) ABSOLUTE_ADDR;
			strcpy( filenm, "FILE: " );
			menu = (browseFLAG) ? restricted_menu : cmds_menu;
		}
	
		if( (n = strlen(fn)) < 72 )
			strcat( filenm, fn );
		else {
			strcat( filenm, ".../" );
			strcat( filenm, basename(fn) );
		}

		if( dn != NULL && (n = strlen(filenm)) < 55 ) {
			strcat( filenm, " DIR: " );
			n += 6;
			if( (n + strlen(dn)) < 72 )
				strcat( filenm, dn );
			else {
				strcat( filenm, ".../" );
				strcat( filenm, basename(dn) );
			}
		}
	
		if( found ) {
			term_flush();
			term_cur( term_state.num_rows - 1, 0 );
			term_clear( TERM_CLS_EOL );
			term_restore();
			fprintf(stderr,"zzx: internal error - check early src\n");
			fprintf(stderr,"zzx: found=%d\n",found);
			exit(1);
		} else {
			if (io_type==BLOCK_IO) {
				totlen.blk=stats.st_size;
				totlen.off=256; /* last screen is last half of last block */
				lastblk.blk=totlen.blk;
			} else {
				// lseek(fd, 0, SEEK_END) doesn't work with /dev/mem.
				// use st_size instead (see PR52327).
				totlen.off = stats.st_size;
				totlen.blk = 0;
			}

			//	if( (lastblk.off = ((totlen.off+511) / 256) * 256) < 0 )
			if( (lastblk.off = (totlen.off / 256) * 256) < 0 )
				lastblk.off = 0;

			//fprintf(stderr,"\nlastblk.blk=%x, lastblk.off=%x, totlen.blk=%x, totlen.off=%x\n",
			//lastblk.blk,lastblk.off,totlen.blk,totlen.off);
			//sleep(10);
		}
		calc_offset(&offset,offptr,TRUE);	/* also row and col */
	}

	get_buffer(buffer, offset);

	display(buffer, offset);

	for(p = *menu; ; ) {
		if((p1 = term_menu(MENUline, 0, (const char * const *)menu, p, TERM_NORMAL, equiv, 1)) == 0)
			continue;

		switch(tolower(*(p = p1))) {

		case 'a':
			addr_type = (char) ( (addr_type + 1) % 2 );
			display(buffer, offset);
			break;

		case 'c':
			check_dirty();
			find_pattern();
			break;

		case 'd':
			addr_type = (char) ( (addr_type + 1) % 3 );
			display(buffer, offset);
			break;

		case 'e':
			edit(cur_row, cur_col);
			break;

		case 'f':
			check_dirty();
			get_pattern("? ");
			find_pattern();
			break;

		case 'g':
			check_dirty();
			if ( get_addr() )
				get_buffer(buffer, offset);
			display(buffer, offset);
			break;

		case 'h':
			check_dirty();
			offset = file_base;
			get_buffer(buffer, offset);
			display(buffer, offset);
			break;

		case 'l':
			check_dirty();
			offset = lastblk;
			get_buffer(buffer, offset);
			display(buffer, offset);
			break;

		case 'm':
			check_dirty();
			if( mark == (-1L) )
				mark = offset.blk + offset.off / 512;
			else
				end_mark();
			display(buffer, offset);
			break;

		case 'n':
			check_dirty();
			inc_offset(&offset,256);
			if( compare_offset(offset,lastblk)>0 )
				offset = lastblk;
			get_buffer(buffer, offset);
			display(buffer, offset);
			break;

		case 'p':
			check_dirty();
			inc_offset(&offset,-256);
			if (compare_offset(offset,file_base)<0) 
				offset = file_base;
			get_buffer(buffer, offset);
			display(buffer, offset);
			break;

		case 's':
			put_buffer(buffer, offset);
			break;

		case 'q':
			check_dirty();
			term_clear(4);
			term_restore();
			return 0;
			}
		}

	}

void show_error(int error)
{
	static char errorbuf[80];

	term_cur(ERRORline,0);
	term_clear(1);
	if (error==EOK) return;
	
	sprintf(errorbuf,"ERROR: %s\n",strerror(errno));
	term_type(ERRORline,0,errorbuf,0,TERM_INVERSE);
}

void
display(char *b, offset_t off) {
	offset_t	 addr = off;
	unsigned	 row;
	unsigned	 ln;
	int			 n;
	char		*p1 = b, *p2;

	show_error( buffer_errno );

	for(row = ln = 0 ; row < 16 ; ++row, addr.off += 16) {
		for(n = 0, p2 = line; p2 < line + 3*16 + 1; ++p1, ++ln) {
			if( ln < len ) {
				*p2++ = hex[(*p1 >> 4) & 0xf];
				*p2++ = hex[*p1 & 0xf];
				}
			else {
				*p2++ = '-';
				*p2++ = '-';
			}
			*p2++ = ' ';
			if( (++n % 8) == 0 )
				*p2++ = ' ';
			}

		*p2++ = ' ';

		for(ln -= 16, p1 -= 16 ; p2 < line + (3*16 + 3 + 16) ; ++p2, ++p1, ++ln) {
			if( ln < len )
				*p2 = (char) ( (*p1 >= ' ' && *p1 <= '~') ? *p1 : '.' );
			else
				*p2 = '-';
			}

		put_addr(ROW(row), 0, addr, ':');
		term_type(ROW(row), 12, line, sizeof(line), TERM_NORMAL);
		}

	put_data(cur_row, cur_col, ascii, b[cur_row*16 + cur_col], TERM_INVERSE);
	term_type( FILEline, 0, filenm, 0, TERM_NORMAL );
	showmark();
	}


void
put_addr(int realrow, int realcol, offset_t addr, char c)
	{
	char  sbuf[51];

	if(addr_type == ABSOLUTE_ADDR) {
		sprintf(sbuf,"%09llx%c ", (long long)(addr.blk*512 + addr.off), c);
		}
	else if(addr_type == BLOCK_ADDR) {
		sprintf(sbuf, "%06llx:%03llx%c",
				(long long)addr.blk+addr.off/512,
				(long long)addr.off%512, c);
		}
#ifdef MEM_SUPPORTED
	else {
		normalize(offset.off);
		sprintf(sbuf, "%04llx:%04llx: ", (long long)segment, (long long)addr.off);
		}
#endif

	term_type( realrow, realcol, sbuf, 0, TERM_NORMAL );
	}


void
edit(row, col)
unsigned row, col;
	{
	int		 c;
	char	 odd = 0,
			 *p;

	put_data(row, col, ascii, buffer[row*16 + col], TERM_NORMAL);

	for(;;) {
		term_cur(ROW(row), ascii ? 63 + col : ((col<=7)?12:13) + 3*col + odd);

		switch( c = term_key() ) {
			case K_HOME:
				row = col = 0;
				break;

			case K_END:
				row = col = 15;
				break;

			case K_UP:
				if(row > 0)
					--row;
				break;

			case K_DOWN:
				if(row < 16-1)
					++row;
				break;

			case K_LEFT:
				if(col > 0)
					--col;
				else if(row > 0) {
					col = 15;
					--row;
					}
				break;

			case K_RIGHT:
				if(col < 16-1)
					++col;
				else if(row < 16-1) {
					col = 0;
					++row;
					}
				break;

			case K_TAB:
				ascii = (char) (! ascii);
				break;

			case ESCAPE:
			case K_KPDPLUS:
				cur_row = row;
				cur_col = col;
				put_data(row, col, ascii, buffer[row*16 + col], TERM_INVERSE);
				return;

			default:
				p = &buffer[row*16 + col];
				if(ascii)
					*p = (char) c;
				else if((c = (char) hexval(c)) < 16) {
					if(odd == 0) {
						*p = (char) ((*p & 0x0f) | (c << 4));
						odd = 1;
						}
					else {
						*p = (char) ((*p & 0xf0) | c);
						odd = 0;
						}
					}
				else
					break;

				dirty = 1;
				put_data(row, col, 0, *p, TERM_NORMAL);
				put_data(row, col, 1, *p, TERM_NORMAL);

				if(odd == 0) {
					if(col < 16-1)
						++col;
					else if(row < 16-1) {
						col = 0;
						++row;
						}
					}
				continue;
			}

		odd = 0;
		}
	}


int
hexval(c)
char c;
	{

	if(c >= '0'  &&  c <= '9')
		return(c - '0');

	c = (char) tolower(c);
	if(c >= 'a'  &&  c <= 'f')
		return(c - 'a' + 10);

	return(16);
	}



void
get_buffer(char *buffer, offset_t offset)
{
	offset_t l;
	int	 i;

	dirty = 0;

	buffer_errno=EOK;
	
//  zzx
//  fprintf(stderr,"get_buffer(*buf, %d:%d)\n",offset.blk,offset.off);

	switch(io_type) {
		_offval_t seek_pos;

	case FILE_IO:
		if (-1!=(seek_pos=lseek( fd,offset.blk*512L+offset.off, SEEK_SET ))) {
			if (seek_pos==(offset.off+offset.blk*512L)) {
				if (-1==(len = read( fd, buffer, 256))) {
	                buffer_errno=errno;
					len=0L;
				}
			} else {
				/* seek succeeded but to wrong location! */
				buffer_errno=EINVAL;
				len=0L;
			}
		} else {
			buffer_errno=errno;
			len=0L;
		}

		/* put nulls for any remaining space */
		for( i = len; i < 256; ++i )
			buffer[i] = 0;

		break;

	case BLOCK_IO:
		l.blk = offset.blk+offset.off / _BLOCK_SIZE;
		l.off = offset.off % _BLOCK_SIZE;

		if( foundLIST[0].x.blk!=0 && foundLIST[0].x.off!=0 ) {
//			fprintf(stderr,"l.off=%d, found=%d\n",l.off,found);
			l.off = min( l.off, found );
			l = foundLIST[l.off].x;
//			fprintf(stderr,"l=%d:%d\n",l.blk,l.off);
			}
		if (-1!=_block_read(fd, l.blk, 1, d_buffer)) {
			memcpy(buffer, &d_buffer[l.off], 256);
		} else {
			buffer_errno=errno;
			memset(buffer,0,256);
		}
		break;

#ifdef MEM_SUPPORTED
	case MEM_IO:
		normalize(offset.off);
		if (__qnx_debug_xfer(PROC_PID, pid, _DEBUG_MEM_RD, buffer, 256, offset.off, segment) != 0)
			memset(buffer, 0, 256);
		break;
#endif
	    }
	}


void
put_buffer(char *buffer, offset_t offset)
	{
	_offval_t l;

	dirty = 0;

	switch(io_type) {
	case FILE_IO:
		lseek( fd, offset.off, SEEK_SET );
		write(fd, buffer, len);
		break;

	case BLOCK_IO:
		l = offset.blk;
		if( foundLIST[0].x.off!=0 && foundLIST[0].x.blk!=0 ) {
			l = max( l, 1L );
			l = min( l, found );
			l = foundLIST[l].x.blk+foundLIST[l].x.off/512L;
			}
		_block_read(fd, l, 1, d_buffer);
		memcpy(&d_buffer[offset.off], buffer, 256);
		block_write(fd, l, 1, d_buffer);
		break;

#ifdef MEM_SUPPORTED
	case MEM_IO:
		normalize(offset.off);
		break;
#endif
	}
	}

int
get_addr() {
	char *p;
	int   flag = FALSE;

	switch(addr_type) {

	case ABSOLUTE_ADDR:
		p = get_line("ADDRESS? ");
		if( *p == 0 )
			return( 0 );
		calc_offset(&offset, p, FALSE);
		break;

	case BLOCK_ADDR:
		if( *(p = get_line("BLOCK:OFFSET (or -filename)? ")) == '-')
			flag = *p++;
		if( *p == 0 )
			return( 0 );
		calc_offset(&offset,p, flag);
		break;

	case SEGMENT_ADDR:
		p = get_line("SEGMENT:OFFSET? ");
		calc_offset(&offset, p, FALSE);
		break;
		}

	cur_row = (unsigned) (offset.off & 0xff) >> 4;
	cur_col = (unsigned) ( offset.off & 0x0f );

	offset.off  &= 0xfffffff00LL; /* limit to 36bits due to too many
								   * display assumptions */

	return( 1 );
	}


int conv_addr(offset_t *off, char *p)
{
	_offval_t  	 n1 = 0, n2 = 0;
	int		 i, c = 0;

	while(*p && (i = hexval(c = *p++)) < 16)
		n1 = (n1 << 4) + i;

	if(c == ':')
		while(*p && (i = hexval(c = *p++)) < 16)
			n2 = (n2 << 4) + i;

	switch (addr_type) {
	  case ABSOLUTE_ADDR:	off->off= n1 + n2; break;
	  case BLOCK_ADDR:		off->blk=n1; off->off=n2; break;
	  case SEGMENT_ADDR:	segment = n1; off->blk=0; off->off=n2; break;
	  }

	return 0;
}


#ifdef MEM_SUPPORTED
normalize(n)
    _offval_t n;
	{
	_offval_t n1;

	if(protected) {
		displacement = n;
		return;
		}

	n1 = 16 * segment;

	if(n < n1  ||  n - n1 > 0x0000ffffL) {
		segment = (offset.off / 0x0000ffffL) * 0x1000;
		n1 = 16 * segment;
		}

	displacement = n - n1;
	}
#endif


int
addr(char *p)
	{
	for( ; *p; ++p )
		if( hexval(*p) >= 16 && *p != ':' )
			return( 0 );

	return( 1 );
	}


int calc_offset(offset_t *result, char *offptr, int fileok)
{
	offset_t l={0,0};

	if( offptr == NULL ) {
		*result=file_base;
		return 0;
	}

	if( fileok && io_type == BLOCK_IO ) {
		close( fd );
#ifdef FSYS_STAT_SUPPORTED
		if( fsys_stat(offptr, &fst) != (-1) ) {
			l.blk = ENDIAN_DADDR_T(fst.st_first_xtnt.xtnt_blk);
		}
   		else
#endif
		if( ! addr(offptr) ) {
			sprintf( oln, "could not find file or directory on: %s", fn );
			errmsg( offptr, oln );
			l = offset;
			}
		if( (fd = open(fn, perms)) == (-1) ) {
			term_cur( term_state.num_rows - 1, 0 );
			term_clear( TERM_CLS_EOL );
			sprintf( buffer, "%s: Unable to re-open '%s'", cmd, fn );
			perror( buffer );
			exit( EXIT_FAILURE );
			}
		}

	if( l.blk == 0L && l.off==0L )
		conv_addr(&l, offptr);

	*result=((compare_offset(l,file_base)>0)?l:file_base);
	return 0;
	}


int
get_pattern(char *prmt) {
	char *p1, *p2;

	if( *(p1 = get_line(prmt)) == 0 )
		return( 0 );

	for(p2 = pattern ; *p1 ; ++p1) {
		while(*p1 == ' ') ++p1;
		if(*(p1 + 1) == ' '  ||  *(p1 + 1) == '\0')
			*p2++ = *p1;
		else {
			*p2 = (char) hexval(*p1++);
			*p2 = (char) ( (*p2 << 4) + hexval(*p1) );
			++p2;
			}
		}

	if(p2 > pattern)
		pat_len = p2 - pattern;

	return(pat_len);
	}


int
find_pattern() {
	offset_t offset_save = offset;
	int		 c = 0;
	char	*ext_buffer = &buffer[256],
			 chk = 0,
			*p1;

	if(pat_len) {
		p1 = &buffer[cur_row*16 + cur_col];
		inc_offset(&offset,256);
		get_buffer(ext_buffer, offset);
		inc_offset(&offset,-256);

		for(;;) {
			if(++p1 >= ext_buffer) {
				p1 = buffer;
				inc_offset(&offset,256);
				if(compare_offset(offset,lastblk)>0) {
					term_type( PRMTline, 30, "NOT FOUND: Type any key to continue...", 38, 0 );
					c = term_key();
					term_cur(PRMTline, 0);
					term_clear(1);
					break;
					}
				if((offset.off & 0x100) == 0)
					{                  
					put_addr(PRMTline, 0, offset, ':');
					term_flush();
					}

				if((++chk & 0x07) == 0  &&  dev_ischars(0)) {
					term_key();
					term_cur(PRMTline, 0);
					term_clear(1);
					break;
					}

				memcpy(buffer, ext_buffer, 256);
				inc_offset(&offset,256);
				get_buffer(ext_buffer, offset);
				inc_offset(&offset,-256);
				}

			if(memcmp(p1, pattern, pat_len) == 0) {
				term_cur(PRMTline, 0);
				term_clear(1);
				put_data(cur_row, cur_col, ascii, buffer[cur_row*16 + cur_col], TERM_NORMAL);
				cur_row = (p1 - buffer) / 16;
				cur_col = (p1 - buffer) % 16;
				if(compare_offset(offset,offset_save)!=0)
					display(buffer, offset);
				else
					put_data(cur_row, cur_col, ascii, buffer[cur_row*16 + cur_col], TERM_INVERSE);
				return(1);
				}
			}
		}

	offset=offset_save;
	get_buffer(buffer, offset);
	return(0);
	}


void
put_data(row, col, area, data, attr)
unsigned row, col, area, attr;
unsigned char data;
	{
	unsigned char	 c;
	int				 base;

	if(area) {
		c = (data < ' '  ||  data >= 0x7f) ? '.' : data;
		term_type(ROW(row), 63 + col, &c, 1, attr);
		}
	else {
		base = (col <= 7) ? 12 : 13;
		c = hex[(data >> 4) & 0xf];
		term_type(ROW(row), base + 3*col, &c, 1, attr);
		c = hex[data & 0xf];
		term_type(ROW(row), base + 3*col + 1, &c, 1, attr);
		}
	}


void
check_dirty() {
	if(dirty && browseFLAG == 0) {
		if( 'Y' == getans(PRMTline, "You have not saved your changes to this screen. Do you wish them saved? (y/n)? "))
			put_buffer(buffer, offset);
		dirty = 0;
		}
	}


void
end_mark() {
	offset_t offset_save = offset;
	_offval_t fst, lst;
	char	*p, *p1;

	term_cur(MENUline, 0);
	term_clear(1);
	emark = offset.blk + offset.off / 512;
	fst = min(mark,emark);
	lst = max(mark,emark);

	mark = fst;
	emark = lst;
	showmark();

	for(p = *svmark_menu; ; ) {
		if((p1 = term_menu(MENUline, 0, (const char * const *)svmark_menu, p, TERM_NORMAL, NULL, 1)) == 0)
			continue;

		switch(tolower(*(p = p1))) {

		case 'a':						/* append to a file on another disk */
			savemarked(APPENDtype, "Append_Off_Disk");
			goto donesave;
		case 'c':						/* copy to a file on another disk */
			savemarked(COPYtype, "Copy_Off_Disk");
			goto donesave;
		case 'd':						/* save marked area to lost+found */
			savemarked(DIRtype, "Save_as_DIRECTORY");
			goto donesave;
		case 'f':						/* save marked area to lost+found */
			savemarked(FILEtype, "Save_as_FILE");
		case 'r':						/* remove marks */
donesave: ;
			mark = (-1);
		case 'q':						/* quit MarkMenu */
			emark = (-1);
			term_cur(MENUline, 0);
			term_clear(1);
			offset=offset_save;
			get_buffer(buffer, offset);
			display(buffer, offset);
			return;
			}
		}
	}

                             
void savemarked( int type, char *descr ) {
	_offval_t	 l, o, lastreal = lastblk.blk+(lastblk.off / 512L);
	int		 i;
	char	*p, *p1;

	if('Y' == getans(PRMTline, "Scan the disk for an Xblock describing the marked area (y/N)? "))
		{
		found = best = 0;
		memset( foundLIST, 0, sizeof(foundLIST) );
		for( l = 1L; l < lastreal && found < MAXfound; ++l ) {
			if( dev_ischars(STDIN_FILENO) ) {
				do	{
					getchar();
					} while( dev_ischars(STDIN_FILENO) );
				break;
				}
			if( (l % 10L) == 1 ) {
				CLRline(PRMTline);
				sprintf( oln, "Scanning: %lld  [matches - BEST: %d  Good: %d  Fair: %d]  (any key to stop)", (long long)l, best, good, (found-best-good) );


				term_type( PRMTline, 0, oln, 0, TERM_NORMAL );
				term_flush();
				}
			_block_read(fd, l, 1, x_buffer);
			if( foundLIST[found].q = chkXblk(mark,emark,x_buffer) ) {
				foundLIST[found].x.blk = l;
				foundLIST[found].x.off = 0;
				if( foundLIST[found].q == BEST )
					++best;
				if( foundLIST[found].q == GOOD )
					++good;
				++found;
				}
			}

		if( found ) {					/* Xblock found */
			CLRline(MENUline);
			x_menu[4] = descr;

			for(l = foundLIST[i=0].x.blk, o = 0L, p = *x_menu; ; ) {
				CLRline(PRMTline);
				sprintf(oln, "FOUND - %d Xblocks (%d BEST matches): This is a %s match.", found, best, match[(unsigned)foundLIST[i].q] );


				term_type( PRMTline, 0, oln, 0, TERM_NORMAL );
				term_flush();
				if (addr_type==BLOCK_ADDR) {
					offset.blk=l; offset.off=o;
				} else {
					offset.blk=0; offset.off= (l*512L)+o;
				}
				get_buffer(buffer, offset);
				display(buffer, offset);
				if((p1 = term_menu(MENUline, 0, (const char * const *)x_menu, p, TERM_NORMAL, x_equiv, 1)) == 0)
					continue;

				switch(tolower(*(p = p1))) {

				case 'a':						/* append area to another disk */
				case 'c':						/* copy area to another disk */
				case 's':						/* save area to lost+found */
					CLRline(PRMTline);
					if( type == COPYtype )
						do_copy( type, l, x_buffer );
					else if( type == APPENDtype )
						do_copy( type, l, x_buffer );
					else
						do_link( type, l, x_buffer );
				case 'q':						/* quit xmenu */
					CLRline(PRMTline);
					memset( foundLIST, 0, sizeof(foundLIST) );
					return;
				case 'n':						/* next Xblock */
					if( o ) {
						if( i < (found-1) ) {
							l = foundLIST[++i].x.blk;
							o = 0;
							}
						}
					else
						o = 256;
					break;
				case 'p':						/* previous Xblock */
					if( i ) {
						if( o )
							o = 0;
						else {
							o = 256;
							l = foundLIST[--i].x.blk;
							}
						}
					else
						o = 0;
					break;
				case '1':						/* first Xblock */
					o = 0;
					l = foundLIST[i=0].x.blk;
					break;
				case 'l':						/* last Xblock */
					o = 0;
					l = foundLIST[i=found-1].x.blk;
					break;
					}
				}
			}
		else {							/* Xblock not found */
			sprintf( oln, "Sorry, no Xblock found: Save marked area as a %s to lost+found (y/N)? ", (type==FILEtype)?"FILE":"DIRECTORY" );
			if( getans(PRMTline,oln) != 'Y' )
				return;
			}
		}
										/* save marked area */
	if( type == COPYtype )				/* copy off disk */
		do_copy( type, 0L, NULL );
	else if( type == APPENDtype )		/* copy off disk */
		do_copy( type, 0L, NULL );
	else								/* link into lost+found */

		do_link( type, 0L, NULL );
	}


void
do_link( int type, long blk, char *buf )
	{
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) buf;

										/* xblock described file */
	if( blk && (blk = prepXblk(blk, buf)) ) {
		_block_read(fd, blk, 1, buf);
		link_1(type, ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk), ENDIAN_SIZE_T(x->xblk_first_xtnt.xtnt_size), blk);
		}
	else								/* simple file */
		link_1(type, mark, emark-mark+1L, 0L);
	}


void link_1(int type, long first, long fsize, long xblk)
	{
	union qnx4fs_dir_entry *ie = (union qnx4fs_dir_entry *) d_buffer;
	union qnx4fs_dir_entry *de = (union qnx4fs_dir_entry *) d_buffer;
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) x_buffer;
	int		 i, lfndx = 0;
	long	 blk, rootdir, lfblk, lfbeg, lfend;

	if( _block_read(fd, QNX4FS_ROOT_BLOCK, 1, d_buffer) == (-1) ) {
		errmsg( "", "could not read QNX4FS_ROOT_BLOCK: file not linked" );
		return;
		}
	if( verify_fsys(d_buffer) ) {		/* valid file system ? */
		errmsg( "", "invalid file system (see documentation on 'dinit -r'): file not linked" );
		return;
		}
                          
	rootdir = ENDIAN_DADDR_T(ie->d_inode.i_first_xtnt.xtnt_blk);	/* locate for later use */

	ie += 4;							/* lost+found inode in QNX4FS_ROOT_BLOCK */
	lfbeg = ENDIAN_DADDR_T(ie->d_inode.i_first_xtnt.xtnt_blk);
	lfend = ENDIAN_DADDR_T(ie->d_inode.i_first_xtnt.xtnt_blk) + ENDIAN_SIZE_T(ie->d_inode.i_first_xtnt.xtnt_size) - 1L;
										/* find an empty slot in lost+found */
	for( lfblk = lfbeg; lfblk <= lfend; ++lfblk ) {
		if( _block_read(fd, lfblk, 1, d_buffer) == (-1) )
			continue;
		for( lfndx = 0; lfndx < 8; ++lfndx ) {
			if( ((de+lfndx)->d_inode.i_status & (QNX4FS_FILE_USED | QNX4FS_FILE_LINK)) == 0 )
				break;
			}
		if( lfndx < 8 )
			break;
		}

	if( lfblk > lfend ) {				/* none empty: try for overwrite */
		for( lfblk = lfbeg; lfblk <= lfend; ++lfblk ) {
			if( _block_read(fd, lfblk, 1, d_buffer) == (-1) )
				continue;
			for( lfndx = 0; lfndx < 8; ++lfndx ) {
				if( lfblk == lfbeg && lfndx < 2 )
					continue;			/* pass over . and .. */
				errmsg( "", "No empty directory entries in lost+found - OK to overwrite this entry (Y/n)?" );
				strncpy( oln, (de+lfndx)->d_inode.i_fname, sizeof(de->d_inode.i_fname));
				oln[sizeof(de->d_inode.i_fname)] = 0;
				if( getans(PRMTline, oln) != 'N' )
					break;
				}
			if( lfndx < 8 )
				break;
			}
		}
	if( lfblk > lfend ) {				/* too bad: couldn't find a slot */
		errmsg( "", "No available slots in lost+found: file not linked" );
		return;
		}

	de += lfndx;						/* create directory entry in lost+found */
	memset( de, 0, sizeof( *de ) );
	sprintf( de->d_inode.i_fname, "%04lx", first );
	de->d_inode.i_size = ENDIAN_SIZE_T(fsize * _BLOCK_SIZE);
 	de->d_inode.i_first_xtnt.xtnt_blk  = ENDIAN_DADDR_T(first);
  	de->d_inode.i_first_xtnt.xtnt_size = ENDIAN_SIZE_T(fsize);
  	de->d_inode.i_xblk = ENDIAN_DADDR_T(xblk);
  	de->d_inode.i_ftime = ENDIAN_TIME_T(time( NULL ));
  	de->d_inode.i_mtime = de->d_inode.i_ftime;  // no endian conv necessary
  	de->d_inode.i_atime = de->d_inode.i_ftime;  // "
  	de->d_inode.i_ctime = de->d_inode.i_ftime;  // "
    de->d_inode.i_num_xtnts = ENDIAN_NXTNT_T(1);
  	de->d_inode.i_gid = ENDIAN_GID_T(getgid());
  	de->d_inode.i_uid = ENDIAN_UID_T(getuid());
  	de->d_inode.i_nlink = ENDIAN_NLINK_T(1);

	if( type == DIRtype ) {
  		de->d_inode.i_mode = ENDIAN_MODE_T(S_IFDIR | S_IRWXU);
  		/* next line was: ++de->d_inode.i_nlink; */
        de->d_inode.i_nlink= ENDIAN_NLINK_T(ENDIAN_NLINK_T(de->d_inode.i_nlink)+1);
		}
	else
  		de->d_inode.i_mode = ENDIAN_MODE_T(S_IFREG | S_IRUSR | S_IWUSR);
	de->d_inode.i_status = QNX4FS_FILE_USED;

	if( blk = xblk ) {					/* calc size and num_xtnts */
		do	{
			if( _block_read(fd, blk, 1, x_buffer) == (-1) )
				break;
			for(i=0; i < x->xblk_num_xtnts && i < QNX4FS_MAX_XTNTS_PER_XBLK; ++i)
				{
				/* line below was: ++de->d_inode.i_num_xtnts; */
                de->d_inode.i_num_xtnts=ENDIAN_NXTNT_T(ENDIAN_NXTNT_T(de->d_inode.i_num_xtnts)+1);

				if( ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size) )
					de->d_inode.i_size += ENDIAN_SIZE_T((ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size) * _BLOCK_SIZE));
				}
			} while( blk = ENDIAN_DADDR_T(x->xblk_next_xblk) );
		}

	block_write(fd, lfblk, 1, d_buffer);	/* directory entry for new file/dir */

	if( _block_read(fd, rootdir, 1, d_buffer) != (-1) ) {
		de = (union qnx4fs_dir_entry *) d_buffer;
											/* make lost+found active */
											/* (even if there is already one) */
		if( (de += 6)->d_inode.i_status == QNX4FS_FILE_BUSY &&
							(de->d_inode.i_gid & QNX4FS_FILE_LINK) &&
							ENDIAN_DADDR_T(de->d_link.l_inode_blk) == QNX4FS_ROOT_BLOCK &&
							de->d_link.l_inode_ndx == 4 ) {
			de->d_inode.i_status = QNX4FS_FILE_LINK;
			de->d_inode.i_gid = 0;
			block_write(fd, rootdir, 1, d_buffer);
			if( _block_read(fd, QNX4FS_ROOT_BLOCK, 1, d_buffer) != (-1) ) {
				de = (union qnx4fs_dir_entry *) d_buffer;
				/* next line was: ++de->d_inode.i_nlink; */
                de->d_inode.i_nlink = ENDIAN_NLINK_T(ENDIAN_NLINK_T(de->d_inode.i_nlink)+1);
				block_write(fd, QNX4FS_ROOT_BLOCK, 1, d_buffer);
				}
			}
		}
	if( type == DIRtype ) {				/* if new entry is for a directory */
										/* 1) bump links in lost+found inode */
		if( _block_read(fd, QNX4FS_ROOT_BLOCK, 1, d_buffer) != (-1) ) {
			de = (union qnx4fs_dir_entry *) d_buffer;
			de += 4;					/* point at lost+found inode */
			/* next line was: ++de->d_inode.i_nlink; */
            de->d_inode.i_nlink = ENDIAN_NLINK_T(ENDIAN_NLINK_T(de->d_inode.i_nlink)+1);
			block_write(fd, QNX4FS_ROOT_BLOCK, 1, d_buffer);
			}
										/* 2) make first . and .. valid */
		if( _block_read(fd, first, 1, d_buffer) != (-1) ) {
			de = (union qnx4fs_dir_entry *) d_buffer;
			memset( de, 0, sizeof(*de) );
			*de->d_inode.i_fname = '.';
			strcpy( &de->d_inode.i_fname[2], "I\003QNX" );
			*((long *) &de->d_inode.i_fname[8]) = fsize;
			de->d_link.l_inode_blk = ENDIAN_DADDR_T(lfblk);
			de->d_link.l_inode_ndx = lfndx;
			de->d_inode.i_status = QNX4FS_FILE_LINK;

			++de;						/* .. */
			memset( de, 0, sizeof(*de) );
			*de->d_inode.i_fname = '.';
			*(de->d_inode.i_fname+1) = '.';
			de->d_link.l_inode_blk = ENDIAN_DADDR_T(lfbeg);
			de->d_inode.i_status = QNX4FS_FILE_LINK;

			block_write(fd, first, 1, d_buffer);
			}
		}

	sprintf( oln, "%04lx", first );		/* tell user the file's name */
	errmsg( oln, "Saved(linked) in lost+found as:" );
	}


void errmsg( char *ln0, char *ln1 )
	{
	CLRline(PRMTline);
	term_type( PRMTline, 0, ln0, 0, TERM_NORMAL );
	CLRline(MENUline);
	term_type( MENUline, 0, ln1, 0, TERM_HILIGHT );
	term_type( MENUline, 55, "(ANY key to continue)...", 0, TERM_INVERSE );
	term_flush();
	term_key();
	CLRline(MENUline);
	CLRline(PRMTline);
	}


char
getans( int row, char *ln )
	{
	int		 n;
	char	 a;

	CLRline(row);
	term_type( PRMTline, 0, ln, n=strlen(ln), TERM_NORMAL );
	term_cur( row, n+2 );
	term_flush();
	a = toupper(term_key());
	CLRline(row);
	return( a );
	}


char *get_line(char *prmt) {
	int		 n;

	term_type( PRMTline, 0, prmt, (n = strlen(prmt)), TERM_INVERSE );
	term_field(PRMTline, n+1, line, (sizeof(line)-1), NULL, TERM_NORMAL);
	term_cur(PRMTline, 0);
	term_clear(1);

	return( line );
	}


int
chkXblk(long fst, long lst, char *buf)
	{
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) buf;
	int		 i;

	if( *x->xblk_signature != 'I' || strcmp(x->xblk_signature, "IamXblk") )
		return( 0 );

	if(x->xblk_num_xtnts == 0 || x->xblk_num_xtnts > QNX4FS_MAX_XTNTS_PER_XBLK ||
											ENDIAN_SIZE_T(x->xblk_xtnts[0].xtnt_size) == 0L )
		return( 0 );

	if( ENDIAN_DADDR_T(x->xblk_prev_xblk)== 0L )	/* if( 1st xblk ) ... */
		{
		if( ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk) <= fst &&
		  lst < (ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk) + ENDIAN_SIZE_T(x->xblk_first_xtnt.xtnt_size)) ) {
			if( ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk) == fst &&
			  lst == (ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk) + ENDIAN_SIZE_T(x->xblk_first_xtnt.xtnt_size) - 1L) )
				return( BEST );
			if( ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk) == fst )
				return( GOOD );
			return( FAIR );
			}
		}

	for( i = 0; i < QNX4FS_MAX_XTNTS_PER_XBLK; ++i )
		{
		if( i >= x->xblk_num_xtnts )
			break;

		if( ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk) <= fst &&
		  lst < (ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk) + ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size)) ) {
			if( ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk) == fst &&
			  lst == (ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk) + ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size) - 1L) )
				return( BEST );
			if( ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk) == fst )
				return( GOOD );
			return( FAIR );
			}
		}

	return( 0 );
	}

void
showmark()
	{
	_offval_t l;
	char buf[81];

	term_cur(MARKline, 0);
	term_clear(1);
	if( mark != (-1L) ) {
		if( emark == (-1L) )
			sprintf( buf, "Beginning MARK: 0x%llX", (long long)mark );
		else {
			l = emark - mark + 1L;
			sprintf( buf, "MARK: 0x%llX for %lld block%c",
					 (long long)mark, (long long)l, (l>1) ? 's' : ' ' );
			}
		term_type( MARKline, 0, buf, 0, TERM_NORMAL );
		}
	}


void do_copy( int type, long blk, char *buf )
	{
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) buf;
	FILE	*fp;
	int		 i;
	char	*p;

	do	{
		if( *(p = get_line("New filename (<Enter> to quit)? ")) == 0 )
			return;
		if( (fp = fopen(p, (type == APPENDtype) ? "a" : "w")) == NULL )
			errmsg( p, strerror(errno) );
		} while( fp == NULL );
	
	if( blk == 0L )
		copy_1(fp, mark, emark);
	else if( blk = prepXblk(blk, buf) ) {
		do	{
			if( _block_read(fd, blk, 1, buf) == (-1) )
				break;
			if( ENDIAN_SIZE_T(x->xblk_first_xtnt.xtnt_size) )
				copy_1(fp, ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk),
					ENDIAN_DADDR_T(x->xblk_first_xtnt.xtnt_blk)+ENDIAN_SIZE_T(x->xblk_first_xtnt.xtnt_size)-1);
			for(i=0; i < x->xblk_num_xtnts && i < QNX4FS_MAX_XTNTS_PER_XBLK; ++i)
				{
				if( ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size) )
					copy_1(fp, ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk),
						ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk)+ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size)-1);
				}
			} while( blk = ENDIAN_DADDR_T(x->xblk_next_xblk) );
		}

	fclose( fp );
	}


void
copy_1(FILE *fp, long beg, long end) {
	long l;

	CLRline(MENUline);
	for( l = beg; l <= end; ++l ) {
		sprintf( oln, "block: %ld", l );
		term_type( MENUline, 0, oln, 0, TERM_NORMAL );
		term_flush();
		if( _block_read(fd, l, 1, d_buffer) <= 0 ) {
			if( getans(MENUline, "ERROR reading block: continue (Y/n)? ") == 'N' )
				break;
			}
		fwrite( d_buffer, _BLOCK_SIZE, 1, fp );
		}
	}


long prepXblk( long blk, char *buf )
	{
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) buf;
	long  beg, l;

	_block_read(fd, blk, 1, buf);

	for( beg = blk; ENDIAN_DADDR_T(x->xblk_prev_xblk); beg = l ) {
		if( _block_read(fd, l = ENDIAN_DADDR_T(x->xblk_prev_xblk), 1, buf) <= 0 )
			break;
		if( *x->xblk_signature != 'I' || strcmp(x->xblk_signature, "IamXblk") )
			break;
		if(x->xblk_num_xtnts == 0 || x->xblk_num_xtnts > QNX4FS_MAX_XTNTS_PER_XBLK ||
				ENDIAN_SIZE_T(x->xblk_xtnts[0].xtnt_size) == 0L || ENDIAN_DADDR_T(x->xblk_next_xblk) != beg )
			break;
		}

	for( l = beg; ENDIAN_DADDR_T(x->xblk_next_xblk); l = ENDIAN_DADDR_T(x->xblk_next_xblk) ) {
		if( _block_read(fd, ENDIAN_DADDR_T(x->xblk_next_xblk), 1, buf) <= 0 )
			break;
		if( *x->xblk_signature != 'I' || strcmp(x->xblk_signature, "IamXblk") )
			break;
		if(x->xblk_num_xtnts == 0 || ENDIAN_SIZE_T(x->xblk_xtnts[0].xtnt_size) == 0L )
			break;
		}

	_block_read(fd, l, 1, buf);
	if( ENDIAN_DADDR_T(x->xblk_next_xblk) ) {
		x->xblk_next_xblk = ENDIAN_DADDR_T(0L);
		block_write(fd, l, 1, buf);
		}

	return( beg );
	}


int
verify_fsys( char *rbuf )
/*
 *	Definition:	Check the disk for a valid (minimally so) file system.
 *
 *  Return:     0 if ok, else (-1)
 */
	{
	union qnx4fs_dir_entry	*de = (union qnx4fs_dir_entry *) rbuf;
	union qnx4fs_dir_entry	*ie = (union qnx4fs_dir_entry *) rbuf;
	int		 status = 0;
	static char slname[17] = "/";

	/*
	 *	Block 2 is the root block.  It is formed as a directory file
	 *	with 4 entries which are the:
	 *
	 *		1) directory entry of the root directory ("/")
	 *		2) the inode information for the special inodes file ("/.inodes")
	 *		3) the inode information for the boot image ("/.boot")
	 *		4) the inode information for the boot image ("/.altboot")
	 */

	if( memcmp( de->d_inode.i_fname, slname, 16 ) )
		status |= 0x01;
	if( ENDIAN_NLINK_T(de->d_inode.i_nlink) < 3 )
		status |= 0x02;
	if( (de->d_inode.i_status & QNX4FS_FILE_USED) == 0 )
		status |= 0x04;
	if( de->d_inode.i_status & QNX4FS_FILE_INODE )
		status |= 0x08;

	++ie;
	if( (ie->d_inode.i_status & QNX4FS_FILE_INODE) == 0 )
		status |= 0x010;
	if( ENDIAN_NLINK_T(ie->d_inode.i_nlink) < 1 )
		status |= 0x020;

	return( status );
	}


#ifdef FSYS_STAT_SUPPORTED
build_blk_list(long blk, long size)
	{
	while( size-- > 0L )
		foundLIST[found].x.off=0L;
		foundLIST[found++].x.blk = blk++;
	}
#endif


#ifdef FSYS_STAT_SUPPORTED
void
build_xblk_list()
	{
	struct qnx4fs_xblk *x = (struct qnx4fs_xblk *) x_buffer;
	int  i;
	_offval_t blk;

	if( blk = fst.st_xblk ) {
		do	{
			if( _block_read(fd, blk, 1, x_buffer) == (-1) )
				break;
			if( *x->xblk_signature != 'I' || strcmp(x->xblk_signature, "IamXblk") )
				break;
			for(i=0; i < x->xblk_num_xtnts && i < QNX4FS_MAX_XTNTS_PER_XBLK; ++i)
				{
				build_blk_list(ENDIAN_DADDR_T(x->xblk_xtnts[i].xtnt_blk), ENDIAN_SIZE_T(x->xblk_xtnts[i].xtnt_size));
				}
			} while( blk = ENDIAN_DADDR_T(x->xblk_next_xblk) );
		}
	}
#endif


int  compare_offset(offset_t a, offset_t b)
{
	/* must normalize a and b into a common form */
	offset_t a_norm, b_norm;

	a_norm.blk = a.blk+a.off/512;
	a_norm.off = a.off%512;

	b_norm.blk = b.blk+b.off/512;
	b_norm.off = b.off%512;
       
	/* if a>b, return 1 ;  if a<b, return -1 ;  if a==b, return 0 */
#if 0
	fprintf(stderr,"comparing %lld:%lld to %lld:%lld\n",
            (long long)a.blk,(long long)a.off,
            (long long)b.blk,(long long)b.off);
	fprintf(stderr,"comparing %lld:%lld to %lld:%lld\n",
            (long long)a_norm.blk,(long long)a_norm.off,
            (long long)b_norm.blk,(long long)b_norm.off);
#endif
	if (a_norm.blk>b_norm.blk) return 1;
	if (a_norm.blk<b_norm.blk) return -1;
	/* same block */
	if (a_norm.off>b_norm.off) return 1;
	if (a_norm.off<b_norm.off) return -1;
	/* exactly the same */
	return 0;
}

void   inc_offset(offset_t *ptr, _offval_t n) 
{

// fprintf(stderr,"incrementing %lld:%lld by %lld\n",(long long)ptr->blk,(long long)ptr->off,(long long)n);

	ptr->off+=n;

	if (addr_type==BLOCK_ADDR) {
		ptr->blk+=ptr->off/512L;
		ptr->off%=512L;
		if (ptr->off<0) {
			ptr->blk--;
			ptr->off=512L+ptr->off; /* ptr->off is negative */
		}
	}

	if (ptr->off<0) ptr->off=0;
	if (ptr->blk<0) ptr->blk=0;

// fprintf(stderr,"done: %lld:%lld\n",(long long)ptr->blk,(long long)ptr->off);
}


		

