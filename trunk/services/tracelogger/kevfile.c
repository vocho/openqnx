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
 * kevfile.c
 *
 * kevfile operations
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <sys/trace.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/kercalls.h>
#include <time.h>
#include <sys/uio.h>
#include <unistd.h>
 
#include "kevfile.h"
#include "utils.h"

#define VERSION_MAJOR   1
#define VERSION_MINOR   01

#undef _TRACE_MK_HK
#define _TRACE_MK_HK(k)    k
enum {
	_TRACE_HEADER_KEYWORDS()
};

/* header ("keywords") creation */
#undef _TRACE_MK_HK
#define _TRACE_MK_HK(k)    _TRACE_HEADER_PREFIX #k _TRACE_HEADER_POSTFIX
#define NUM_TO_STR(n,a)    ((void) sprintf((a), "%lld", (long long) (n)), (a))
#define MACRO_TO_STR(s)    TMP_STR_RESOLVE(s)
#define TMP_STR_RESOLVE(s) #s


static const char* const head_keywords[] = {
	_TRACE_HEADER_KEYWORDS()
};

static size_t kwrite( kevfile_t *kev, const char *data, size_t n );
static int write_header( kevfile_t *kev, time_t  t_n );

kevfile_t *kevfile_open( const char *path, unsigned flags, size_t max_size, unsigned initial_buffers )
{
	kevfile_t		*new_kevfile;
	kevfile_buf_t	*kbuf;
	int				i;
	
	new_kevfile = malloc( sizeof(kevfile_t) );
	if ( new_kevfile == NULL ) {
		return NULL;
	}
	if ( flags & KEVFILE_MAP ) {
		info("Creating direct-mapping output file %s of %d bytes\n", path, max_size );
		new_kevfile->fd = shm_open( path, O_RDWR|O_CREAT|O_TRUNC, 0644 );
	} else {
		info("Creating regular output file %s", path );
		if ( max_size > 0 ) {
			info( " with a maximum size of %d bytes\n", max_size );
		} else {
			info( "\n" );
		}
		new_kevfile->fd = open( path, O_RDWR|O_CREAT|O_TRUNC, 0644 );
	}	
	if ( new_kevfile->fd == -1 ) {
		free( new_kevfile );
		return NULL;
	}	
	
	new_kevfile->path = strdup( path );
	if ( new_kevfile->path == NULL ) {
		free( new_kevfile );
		close( new_kevfile->fd );
		return NULL;
	}
	
	new_kevfile->flags = flags;
	new_kevfile->max_size = max_size;
	new_kevfile->num_buffers = 0;
	new_kevfile->buffers_logged = 0;
	new_kevfile->buffers_dropped = 0;
	new_kevfile->last_buffers_dropped = 0;
	new_kevfile->free_buffers = 0;
	new_kevfile->write_depth = 0;
	new_kevfile->write_depth = 0;
	
	if ( flags & KEVFILE_MAP ) {
		if ( -1 == ftruncate( new_kevfile->fd, new_kevfile->max_size ) ) {
			free( (void *)new_kevfile->path );
			close( new_kevfile->fd );
			free( new_kevfile );
			return NULL;
		}
		new_kevfile->addr = mmap( 0, new_kevfile->max_size, PROT_READ|PROT_WRITE, MAP_SHARED, new_kevfile->fd, 0 );
		if ( new_kevfile->addr == MAP_FAILED ) {
			free( (void *)new_kevfile->path );
			close( new_kevfile->fd );
			free( new_kevfile );
			return NULL;
		}
	}	
	
	new_kevfile->current_offset = 0;
	
	write_header( new_kevfile, time(NULL) );	

	for ( i = 0; i < initial_buffers; i++ ) {
		kbuf = malloc( sizeof(kevfile_buf_t) );
		if ( kbuf == NULL ) {
			break;
		}	
		kbuf->next = new_kevfile->free_buffers;
		if ( !(new_kevfile->flags & KEVFILE_MAP) ) {
			kbuf->data = malloc( sizeof(tracebuf_t) );
			if ( kbuf->data == NULL ) {
				free(kbuf);
				break;
			}
		}
		new_kevfile->free_buffers = kbuf;
		new_kevfile->num_buffers++;
	}
	if ( new_kevfile->num_buffers == 0 ) {
		kevfile_close( new_kevfile );
		return NULL;
	}
	new_kevfile->write_buffers = NULL;
	new_kevfile->write_tail = NULL;
	
	return new_kevfile;
}

void kevfile_close( kevfile_t *kev )
{
	kevfile_buf_t *kbuf;
	if ( kev->flags & KEVFILE_MAP ) {
		munmap( kev->addr, kev->max_size );
		ftruncate( kev->fd, kev->current_offset );
	}
	else {
		for ( kbuf = kev->write_buffers; kbuf != NULL; kbuf = kev->write_buffers ) {
			kev->write_buffers = kbuf->next;
			free( kbuf->data );
			free( kbuf );
		}
	}
	for ( kbuf = kev->free_buffers; kbuf != NULL; kbuf = kev->free_buffers ) {
		kev->free_buffers = kbuf->next;
		if ( !(kev->flags & KEVFILE_MAP) ) {
			free( kbuf->data );
		}
		free( kbuf );
	}
	close( kev->fd );
	free( (void *)kev->path );
	free( kev );
}

#define FLUSH_IOVS 10
int kevfile_flush( kevfile_t *kev )
{
	kevfile_buf_t *kbuf, *next, *gather_list = NULL;
	iov_t	iovs[FLUSH_IOVS]; /* RUSH */
	int n = 0, niovs, write_depth, buffers_dropped, last_buffers_dropped, nbytes;
	
	InterruptLock( &kev->buf_spin );
	write_depth = kev->write_depth;
	gather_list = kev->write_buffers;
	if ( gather_list != NULL ) {
		kev->write_buffers = kev->write_tail = NULL;
		kev->write_depth = 0;
	}
	InterruptUnlock( &kev->buf_spin );
	
	debug("start flush, write_depth is %d\n", write_depth );
	do {
		for ( nbytes = 0, niovs = 0, kbuf = gather_list; niovs < FLUSH_IOVS && kbuf != NULL; kbuf = kbuf->next ) {
			hack("doing buffer index %d sequence %d!!\n", _TRACE_GET_BUFFNUM(kbuf->off), _TRACE_GET_BUFFSEQ(kbuf->off) );
			SETIOV(&iovs[niovs], kbuf->data, kbuf->nbytes );
			niovs++;
			n++;
			nbytes += kbuf->nbytes;
		}
		if ( writev( kev->fd, iovs, niovs ) < nbytes ) {
			return -1;
		}
		/* TODO - maybe it would be better not to lock/unlock so often */
		for ( ; gather_list != kbuf; gather_list = next ) {
			hack("done with buffer index %d sequence %d!!\n", _TRACE_GET_BUFFNUM(gather_list->off), _TRACE_GET_BUFFSEQ(gather_list->off) );
			InterruptLock( &kev->buf_spin );
			next = gather_list->next;
			gather_list->next = kev->free_buffers;
			kev->free_buffers = gather_list;
			InterruptUnlock( &kev->buf_spin );
		}
	} while( gather_list != NULL );
	
	InterruptLock( &kev->buf_spin );
	write_depth = kev->write_depth;
	buffers_dropped = kev->buffers_dropped;
	last_buffers_dropped = kev->last_buffers_dropped;
	kev->last_buffers_dropped = kev->buffers_dropped;
	InterruptUnlock( &kev->buf_spin );
	
	if ( buffers_dropped > last_buffers_dropped ) {
		fprintf( stderr, "Help, we're dropping buffers! (%d dropped so far!)\n", buffers_dropped );
	}
	if ( n < write_depth ) {
		fprintf( stderr, "Help, we're not keeping up (%d/%d accumulated in the time we took to write %d)\n", write_depth, kev->num_buffers, n );
	}
	debug("end flush, wrote %d buffers, write_depth is now %d\n", n, write_depth );
	return n;
}

kevfile_buf_t *kevfile_buffer_get( kevfile_t *kev )
{
	kevfile_buf_t	*kbuf;
	InterruptLock( &kev->buf_spin );
	kbuf = kev->free_buffers;
	if ( kbuf == NULL ) {
		InterruptUnlock( &kev->buf_spin );
		return NULL;
	}
	kev->free_buffers = kbuf->next;
	if ( kev->flags & KEVFILE_MAP ) {
		kbuf->data = &kev->addr[kev->current_offset]; /* RUSH */
	}
	InterruptUnlock( &kev->buf_spin );
	kbuf->nbytes = 0;
	kbuf->next = NULL;
	return kbuf;
}	

int kevfile_buffer_put( kevfile_t *kev, kevfile_buf_t *buf )
{
	InterruptLock( &kev->buf_spin );
	kev->current_offset += buf->nbytes;
	if ( kev->flags & KEVFILE_MAP ) {
		/* since we have no need to explicitly write the buffer, we just bung it back on the free list */
		buf->next = kev->free_buffers;
		kev->free_buffers = buf;
	} else {
		if ( kev->write_buffers == NULL ) {
			kev->write_buffers = buf;
			kev->write_tail = buf;
		} else {
			kev->write_tail->next = buf;
			kev->write_tail = buf;
		}
		kev->write_depth++;
		if ( kev->write_depth > kev->max_write_depth ) {
			kev->max_write_depth = kev->write_depth;
		}
	}
	InterruptUnlock( &kev->buf_spin );
	return 0;
}	

unsigned kevfile_space_left( kevfile_t *kev )
{
	int n;
	if ( kev->max_size == 0 ) {
		return UINT_MAX;
	}
	n = kev->max_size - kev->current_offset;
	return n < 0 ? 0 : (unsigned)n;
}

/* static functions */
static int write_header_keyvalue( kevfile_t *kev, const char *k, const char *v )
{
	int l, n, r;

	l = 0;

	n = strlen( _TRACE_HEADER_PREFIX );
	r = kwrite( kev, _TRACE_HEADER_PREFIX, n);
	if( r != n ) {
		return -1;
	}
	l += n;
	
	n = strlen( k );
	r = kwrite( kev, k, n );
	if ( r != n ) {
		return -1;
	}
	l += n;

	n = strlen( _TRACE_HEADER_POSTFIX );
	r = kwrite( kev, _TRACE_HEADER_POSTFIX, n);
	if( r != n ) {
		return -1;
	}
	l += n;
	
	n = strlen( v );
	r = kwrite( kev, v, n );
	if ( r != n ) {
		return -1;
	}
	l += n;

	return( l );
}

static int write_header_str( kevfile_t *kev, int keyword, const char *s )
{
	int l, n, r;
	
	n = strlen( head_keywords[keyword] );
	l = strlen( s );
	r = kwrite( kev, head_keywords[keyword], n );
	if ( r != n ) {
		return -1;
	}
	r = kwrite( kev, s, l );
	if ( r != l ) {
		return -1;
	}
	return( n + l );
}

static int write_header( kevfile_t *kev, time_t  t_n )
{
struct  utsname u_n;
char	s_a[24];
char    *timestr, *nl;
char	*syspage_p;

	write_header_str( kev, HEADER_BEGIN,   "");
	write_header_str( kev, FILE_NAME,      kev->path );
	timestr = ctime(&t_n);
	if(timestr != NULL && (nl = strchr(timestr, '\n')) != NULL) {
		*nl = '\0';
		write_header_str( kev, DATE,           timestr);
	}
	write_header_str( kev, VER_MAJOR,      MACRO_TO_STR(VERSION_MAJOR));
	write_header_str( kev, VER_MINOR,      MACRO_TO_STR(VERSION_MINOR));

#ifdef __LITTLEENDIAN__
	write_header_str( kev, LITTLE_ENDIAN, "TRUE");
#else
	write_header_str( kev, BIG_ENDIAN,    "TRUE");
#endif
	write_header_str( kev, ENCODING,       "16 byte events");

	timestr = ctime((time_t*)&(SYSPAGE_ENTRY(qtime)->boot_time));
	if(timestr != NULL && (nl = strchr(timestr, '\n')) != NULL) {
		*nl = '\0';
		write_header_str( kev, BOOT_DATE,           timestr);
	}

#if (defined(__MIPS__) || defined(__SH__) )
	write_header_str( kev, CYCLES_PER_SEC, NUM_TO_STR(SYSPAGE_ENTRY(qtime)->cycles_per_sec >> CLOCKCYCLES_INCR_BIT, s_a));
#else
	write_header_str( kev, CYCLES_PER_SEC, NUM_TO_STR( SYSPAGE_ENTRY(qtime)->cycles_per_sec, s_a));
#endif
	write_header_str( kev, CPU_NUM,        NUM_TO_STR(_syspage_ptr->num_cpu, s_a));
	if (!uname(&u_n)) {
		write_header_str( kev, SYSNAME,      u_n.sysname);
		write_header_str( kev, NODENAME,     u_n.nodename);
		write_header_str( kev, SYS_RELEASE,  u_n.release);
		write_header_str( kev, SYS_VERSION,  u_n.version);
		write_header_str( kev, MACHINE,      u_n.machine);
	}
	write_header_str( kev, SYSPAGE_LEN,    NUM_TO_STR(_syspage_ptr->total_size, s_a));
	if(g_extra_attributes) {
		struct attributes *attr;
		for(attr = g_extra_attributes; attr != NULL; attr = attr->next) {
			write_header_keyvalue( kev, attr->key, attr->value);
		}
	}
	write_header_str( kev, HEADER_END,     "");
		
	/* this may look weird, but you can't do a write directly from the syspage_ptr */
	syspage_p = alloca( _syspage_ptr->total_size );	
	memcpy( syspage_p, (void *)_syspage_ptr, _syspage_ptr->total_size );
	kwrite( kev, (void*)syspage_p, (size_t)_syspage_ptr->total_size);

	return 0;
}

static size_t kwrite( kevfile_t *kev, const char *data, size_t n )
{
	int r, l = min( kevfile_space_left(kev), n ); /* need to expand space left outside of macro */
	if ( kev->flags & KEVFILE_MAP ) {
		memcpy( &kev->addr[kev->current_offset], data, l );
		r = l;
	} else {
		r = write( kev->fd, data, l );
		if ( r < 0 ) {
			return r;
		}
	}
	kev->current_offset += r;
	return r;
}

__SRCVERSION("kevfile.c $Rev: 153052 $");
