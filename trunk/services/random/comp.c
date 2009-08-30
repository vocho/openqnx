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





#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <zlib.h>
#include <sys/types.h>
#include <inttypes.h>
#include <string.h>

#include "comp.h"

/* Might want to vary these by context */
#define BUFSIZE     16384 /* 16K */
#define OUTBUFSIZE  16800 /* = inbufsize*1.01 + 12 (See zlib docs) */
#define SHIFTSIZE   (BUFSIZE/4) 

#define MIN(a,b) (((a)<(b))?(a):(b))

/* Initialize these routines */
int comp_init( comp_ctx_t* ctx )
{
	ctx->buf = calloc( BUFSIZE, 1 );
	if(ctx->buf == NULL) 
        return COMP_ERR_LOW_MEMORY;

    ctx->space = BUFSIZE;
	ctx->spaceused = 0;

	return COMP_SUCCESS;
}


int comp_add_data( comp_ctx_t* ctx, uint8_t* inp, uint32_t inplen )
{
	uint32_t shifts;
	uint32_t blocksize;
	uint8_t* buf;

	buf = ctx->buf;

	if( inplen+SHIFTSIZE > ctx->space )
	{
		blocksize = MIN( inplen, ctx->space );
		memmove( buf, inp, blocksize );
		ctx->spaceused = blocksize;
	}
	else
	{
		if( inplen + ctx->spaceused > ctx->space ) 
		{
			shifts = (uint32_t)ceil((inplen+ctx->spaceused-ctx->space)/(float)SHIFTSIZE);
			blocksize = MIN(shifts*SHIFTSIZE,ctx->spaceused);
			memmove(buf,buf+blocksize,ctx->space-blocksize);
			ctx->spaceused = ctx->spaceused - blocksize;
		}
		memmove(buf+ctx->spaceused,inp,inplen);
		ctx->spaceused += inplen;
	}

	return COMP_SUCCESS;
}


int comp_get_ratio(comp_ctx_t* ctx,float* out)
{
	uint8_t *inbuf,*outbuf;
	uLong insize,outsize;
	int resp;

	*out = 0;

	if(ctx->spaceused == 0) {return COMP_SUCCESS;}

	inbuf = ctx->buf;
	outbuf = (uint8_t*)malloc(OUTBUFSIZE);
	if(outbuf==NULL) {return COMP_ERR_LOW_MEMORY;}

	insize = ctx->spaceused;
	outsize = OUTBUFSIZE;

	resp = compress(outbuf,&outsize,inbuf,insize);
	if(resp==Z_MEM_ERROR) {return COMP_ERR_LOW_MEMORY;}
	if(resp==Z_BUF_ERROR) {return COMP_ERR_LIB;}

	*out = (float)outsize/(float)insize;

	/* Thrash the memory and free it */
	memset(outbuf,0x00,OUTBUFSIZE);
	memset(outbuf,0xFF,OUTBUFSIZE);
	memset(outbuf,0x00,OUTBUFSIZE);
	free(outbuf);

	return COMP_SUCCESS;
}


int comp_end(comp_ctx_t* ctx)
{
	if(ctx == NULL) {return COMP_SUCCESS;} /* Since nothing is left undone */

	free( ctx->buf );
	ctx->buf = NULL;

	return COMP_SUCCESS;
}
