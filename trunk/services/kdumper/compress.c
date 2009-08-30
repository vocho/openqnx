#include "kdumper.h"

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define GZ_OS_CODE 0x03
#define GZ_MAGIC_0 0x1f
#define GZ_MAGIC_1 0x8b

#define IN_SIZE		2048
#define OUT_SIZE	4096

static void		(*raw_write)(void *, unsigned);
static z_stream	stream;
static unsigned char		*inbuf, *outbuf;
static unsigned	crc;
static int		compress_enabled = 0;
static char		gz_header[] = { GZ_MAGIC_0, GZ_MAGIC_1, Z_DEFLATED, 0, 0, 0, 0, 0, 0, GZ_OS_CODE };


// These next two definitions are to prevent the libz zutil.c file from
// bringing in a whole whack of libc that we don't need
void free(void *p) {
}

void *calloc(size_t n, size_t size) {
	return NULL;
}


static void *zcalloc( void *opaque, unsigned items, unsigned size )
{
	paddr64_t	p;
	p=alloc_pmem(items * size, 0);
	if ( p == ~(paddr64_t)0 ) {
		return NULL;
	}
	return cpu_map(p, items * size, PROT_READ|PROT_WRITE, -1, NULL);
}

static void zcfree( void *opaque, void *ptr )
{
	/* can't free */
}

static void putLong (unsigned long x)
{
	int n;
	char c;
	for (n = 0; n < 4; n++) {
		c = (x & 0xff);
		raw_write(&c, 1);
		x >>= 8;
	}
}

void
compress_write(void *p, unsigned len) {
	int done = 0, err, remaining;
	if ( !compress_enabled ) {
		raw_write( p, len );
	}
	if(p == WRITE_CMD_ADDR) {
		if (len == WRITE_FINI) {
			for(;;) {
				len = OUT_SIZE - stream.avail_out;
				if ( len != 0 ) {
					raw_write( outbuf, len );
					stream.next_out = outbuf;
					stream.avail_out = OUT_SIZE;
				}
				if ( done ) break;
				err = deflate( &stream, Z_FINISH );
				if ( len == 0 && err == Z_BUF_ERROR ) err = Z_OK;
				done = (stream.avail_out != 0 || err == Z_STREAM_END);
				if ( err != Z_OK && err != Z_STREAM_END ) break;
			}
			putLong( crc);
			putLong( stream.total_in );
			raw_write( p, 1 );
			kprintf("COMPRESS DONE - total in %d total out %d %d%%\n",  
				stream.total_in, stream.total_out, stream.total_out*100/stream.total_in );
		}
	} else {
		for ( remaining = len; remaining > 0; ) {
			stream.next_in = inbuf;
			stream.avail_in = min(remaining,IN_SIZE);
			/* bounce buffer is required to prevent pte updates messing with crc! */
			memcpy( inbuf, p, stream.avail_in );
			p = (char *)p + stream.avail_in;
			remaining -= stream.avail_in;

			crc = crc32(crc, (const Bytef *)inbuf, stream.avail_in);
			while ( stream.avail_in != 0 ) {
				if ( stream.avail_out == 0 ) {
					stream.next_out = outbuf;
					raw_write( outbuf, OUT_SIZE );
					stream.avail_out = OUT_SIZE;
				}
				deflate( &stream, Z_NO_FLUSH );
			}
		}
	}
}

void
compress_start(void (*func)(void *, unsigned)) {
	raw_write = func;
	if ( compress_enabled ) {
		raw_write( gz_header, sizeof(gz_header) );
	}
}

void
compress_init(void) {
	int err;

	inbuf = zcalloc( 0, 1, IN_SIZE );
	if ( inbuf == NULL ) {
		kprintf("Couldn't allocate %d bytes for input buffer\n", IN_SIZE );
		return;
	}
	outbuf = zcalloc( 0, 1, OUT_SIZE );
	if ( outbuf == NULL ) {
		kprintf("Couldn't allocate %d bytes for output buffer\n", OUT_SIZE );
		return;
	}

	crc = crc32(0L, Z_NULL, 0);

	stream.avail_out = OUT_SIZE;
	stream.next_out = outbuf;

	stream.zalloc = zcalloc;
	stream.zfree = zcfree;

	err = deflateInit2(&stream, 1, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	if ( err != Z_OK ) {
		kprintf("Couldn't initialize compression library\n");
		return;
	}
	
	compress_enabled = 1;
}
