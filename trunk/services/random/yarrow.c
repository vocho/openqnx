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
	yarrow.c

	Core routines for the Counterpane Yarrow PRNG
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/neutrino.h>
#include <inttypes.h>
#include <dirent.h>
#include <string.h>

#include "sha1mod.h"
#include "comp.h"

#include "yarrow.h"

#define _MAX(a,b) (((a)>(b))?(a):(b))
#define _MIN(a,b) (((a)<(b))?(a):(b))


/* Utility functions forward declerations */
static void yarrow_slow_init( yarrow_t *p );
static void yarrow_do_sha1( yarrow_t *p, yarrow_gen_ctx_t *ctx );
static void yarrow_make_new_state( yarrow_t *p, yarrow_gen_ctx_t *ctx, 
                                   uint8_t *state);
static void trash_mem(void* mem,uint32_t len);
static void bubble_sort(uint32_t* data,uint32_t len);


int yarrow_slow_poll( uint8_t *buffer, int buffer_size )
{
    int j;
    int count;
    int min;
    uint64_t clk;
    DIR *dir;
    struct dirent* dir_entry;
    sha1_ctx_t context;
    uint8_t digest[20];

    count = 0;

    /* Pull about 1 second worth of randomness from the timers */
    for( j=0; ( j < 100  ) && ( count < buffer_size ); j++ )
    {
        clk = ClockCycles();

        SHA1Init( &context );
        SHA1Update( &context, (uint8_t *)&clk, sizeof( clk ) );
        SHA1Final( digest, &context );

        min = _MIN( buffer_size - count, sizeof( digest ) );
        memcpy( buffer + count, digest, min );
        count += min;

        delay( 10 );
    }


    /* Open /proc and /dev and hash on everything found within */

    dir = opendir( "/proc" );
    if( dir == NULL )
        return count;

    while( count < buffer_size )
    {
        dir_entry = readdir( dir );
        if( dir_entry == NULL )
            break;

        SHA1Init( &context );
        SHA1Update( &context, dir_entry->d_name, dir_entry->d_namelen );
        SHA1Final( digest, &context );

        min = _MIN( buffer_size - count, sizeof( digest ) );
        memcpy( buffer + count, digest, min );
        count += min;
    }

    closedir( dir );

    dir = opendir( "/dev" );
    if( dir == NULL )
        return count;

    while( count < buffer_size )
    {
        dir_entry = readdir( dir );
        if( dir_entry == NULL )
            break;

        SHA1Init( &context );
        SHA1Update( &context, dir_entry->d_name, dir_entry->d_namelen );
        SHA1Final( digest, &context );

        min = _MIN( buffer_size - count, sizeof( digest ) );
        memcpy( buffer + count, digest, min );
        count += min;
    }

    closedir( dir );

    return count;
}




yarrow_t * yarrow_create( void ) 
{
    yarrow_t *p;
	uint32_t i;
    int ret;
    pthread_mutexattr_t mutex_attr;

	/* Assign memory */
	p = calloc( sizeof( yarrow_t ), 1 );
	if( p == NULL )
        return NULL;

    /* Initialize the secret state. */
    SHA1Init( &p->pool );

    /* Create our mutex and lock it */
    pthread_mutexattr_init( &mutex_attr );
    pthread_mutexattr_setrecursive( &mutex_attr, PTHREAD_RECURSIVE_ENABLE );
    pthread_mutex_init( &p->mutex, NULL );
    pthread_mutex_lock( &p->mutex );

    /* Does a slow poll and then calls yarrow_make_state(...) */
    yarrow_slow_init( p );	

    /* Initialize compression routines */
    for(i=0;i<COMP_SOURCES;i++) 
    {
        ret = comp_init( ( p->comp_state ) + i );
        if( ret != COMP_SUCCESS) 
        {
            pthread_mutex_destroy( &p->mutex );
            free( p );
            return NULL;
        }
    }

    p->pool_count = 0;
    p->max_pool_count = MAX_SOURCES;
    p->ready = YARROW_READY;

    pthread_mutex_unlock( &p->mutex );

    return p;
}


int yarrow_destroy( yarrow_t *p ) 
{
	uint32_t i;

	if( p == NULL)
        return YARROW_SUCCESS;

    pthread_mutex_lock( &p->mutex );

	p->ready = YARROW_NOT_READY;
	
	for(i=0;i<COMP_SOURCES;i++)
	{
		comp_end((p->comp_state)+i);
	}

    pthread_mutex_destroy( &p->mutex );
	free( p );
	
	return YARROW_SUCCESS;
}


int yarrow_add_source( yarrow_t *p, uint32_t *pool_no )
{
    int  idx;

    if( p->pool_count >= p->max_pool_count )
        return -1;

    pthread_mutex_lock( &p->mutex );

    idx = p->pool_count;
    p->pool_count ++;

    p->poolSize[idx] = 0;
    p->poolEstBits[idx] = 0;
    
    pthread_mutex_unlock( &p->mutex );

    *pool_no = idx;
    return 0;
}


/* Provide output */
int yarrow_output( yarrow_t *p, uint8_t *outbuf, uint32_t outbuflen ) 
{
	uint32_t i;

    pthread_mutex_lock( &p->mutex );

	for( i=0; i<outbuflen; i++,p->index++,p->numout++ ) 
	{
		/* Check backtracklimit */
		if(p->numout > BACKTRACKLIMIT) 
		{
			yarrow_do_sha1( p, &p->outstate );	
			yarrow_make_new_state( p, &p->outstate, p->outstate.out );
		}

		/* Check position in iv */
		if( p->index >= 20 ) 
		{
			yarrow_do_sha1( p, &p->outstate );
		}

		/* Output data */
		outbuf[i] = (p->outstate.out)[p->index];
	}

    pthread_mutex_unlock( &p->mutex );

	return YARROW_SUCCESS;
}


/* Take some "random" data and make more "random-looking" data from it */
int yarrow_stretch( uint8_t *inbuf, uint32_t inbuflen, uint8_t *outbuf, uint32_t outbuflen ) 
{
	long int left,prev;
	sha1_ctx_t ctx;
	uint8_t dig[20];

	if(inbuflen >= outbuflen) 
	{
		memcpy(outbuf,inbuf,outbuflen);
		return YARROW_SUCCESS;
	}
	else  /* Extend using SHA1 hash of inbuf */
	{
		SHA1Init(&ctx);
		SHA1Update(&ctx,inbuf,inbuflen);
		SHA1Final(dig,&ctx);
		for(prev=0,left=outbuflen;left>0;prev+=20,left-=20) 
		{
			SHA1Update(&ctx,dig,20);
			SHA1Final(dig,&ctx);
			memcpy(outbuf+prev,dig,(left>20)?20:left);
		}
		trash_mem(dig,20*sizeof(uint8_t));
		
		return YARROW_SUCCESS;
	}

	return YARROW_ERR_PROGRAM_FLOW;
}


/* Add entropy to the YARROW from a source */
int yarrow_input( yarrow_t *p, uint8_t *inbuf, uint32_t inbuflen, 
                  uint32_t poolnum, uint32_t estbits ) 
{
	int resp;

	if( poolnum >= p->pool_count ) 
        return YARROW_ERR_OUT_OF_BOUNDS;

    pthread_mutex_lock( &p->mutex );

	/* Add to entropy pool */
	SHA1Update(&p->pool,inbuf,inbuflen);
	
	/* Update pool size, pool user estimate and pool compression context */
	p->poolSize[poolnum] += inbuflen;
	p->poolEstBits[poolnum] += estbits;

	if(poolnum<COMP_SOURCES)
	{
		resp = comp_add_data((p->comp_state)+poolnum,inbuf,inbuflen);
		if( resp != COMP_SUCCESS )
        {
            pthread_mutex_unlock( &p->mutex );
            return YARROW_ERR_COMPRESSION;
        }
	}

    pthread_mutex_unlock( &p->mutex );

	return YARROW_SUCCESS;
}



int yarrow_force_reseed( yarrow_t *p, uint64_t ticks ) 
{
	int i;
	uint64_t start;
	uint64_t now;
	uint8_t buf[64];
	uint8_t dig[20];

    pthread_mutex_lock( &p->mutex );

	ClockTime( CLOCK_REALTIME, NULL, &start );
    start = ( start / 1000000 );

	do
	{
		/* Do a couple of iterations between time checks */
		yarrow_output( p, buf, 64 );
		SHA1Update( &p->pool,buf, 64 );
		yarrow_output( p, buf, 64 );
		SHA1Update( &p->pool,buf, 64 );
		yarrow_output( p, buf, 64 );
		SHA1Update( &p->pool,buf, 64 );
		yarrow_output( p, buf, 64 );
		SHA1Update( &p->pool,buf, 64 );
		yarrow_output( p, buf, 64 );
		SHA1Update( &p->pool, buf, 64 );
	
        /* Set up now */
        ClockTime( CLOCK_REALTIME, NULL, &now );
        now = ( now / 1000000 );

	} while ( ( now - start ) < ticks );

	SHA1Final(dig,&p->pool);
	SHA1Update(&p->pool,dig,20); 
	SHA1Final(dig,&p->pool);

	/* Reset secret state */
	SHA1Init(&p->pool);
	yarrow_make_new_state( p, &p->outstate, dig );

	/* Clear counter variables */
	for( i=0; i<p->pool_count; i++) 
	{
		p->poolSize[i] = 0;
		p->poolEstBits[i] = 0;
	}

    pthread_mutex_unlock( &p->mutex );

	/* Cleanup memory */
	trash_mem( dig, sizeof( dig ) );
	trash_mem( buf, sizeof( buf ) );

	return YARROW_SUCCESS;
}


/* If we have enough entropy, allow a reseed of the system */
int yarrow_allow_reseed( yarrow_t *p, uint64_t ticks ) 
{
	uint32_t temp[MAX_SOURCES];
	uint32_t i,sum;
	float ratio;
	int resp;


    pthread_mutex_lock( &p->mutex );

	for( i=0; i<p->pool_count; i++ )
	{
		/* Make sure that compression-based entropy estimates are current */
		resp = comp_get_ratio((p->comp_state)+i,&ratio);
		if(resp!=COMP_SUCCESS) {return YARROW_ERR_COMPRESSION;}

		/* Use 4 instead of 8 to half compression estimate */
		temp[i] = _MIN( (int)( ratio*p->poolSize[i]*4), (int)p->poolEstBits[i] ); 
	}

	bubble_sort( temp, MAX_SOURCES );

	for( i=K,sum=0; i<MAX_SOURCES; sum+=temp[i++] ); 
    
    pthread_mutex_unlock( &p->mutex );

	if( sum>THRESHOLD ) 
		return yarrow_force_reseed( p, ticks );
	else 
		return YARROW_ERR_NOT_ENOUGH_ENTROPY;

	return YARROW_ERR_PROGRAM_FLOW;
}



/******************************************************************************
  Static Local Functions
 ******************************************************************************/

/* All error checking should be done in the function that calls these */
static void yarrow_do_sha1( yarrow_t *p, yarrow_gen_ctx_t *ctx ) 
{
	sha1_ctx_t sha;

	SHA1Init(&sha);
	SHA1Update(&sha,ctx->iv,20);
	SHA1Update(&sha,ctx->out,20);
	SHA1Final(ctx->out,&sha);
	p->index = 0;
}



static void yarrow_make_new_state( yarrow_t *p, yarrow_gen_ctx_t *ctx, 
                                   uint8_t *state ) 
{
	sha1_ctx_t sha;

	memcpy(ctx->iv,state,20);
	SHA1Init(&sha);
	SHA1Update(&sha,ctx->iv,20);
	SHA1Final(ctx->out,&sha);
	p->numout = 0;
	p->index = 0;
}


/* Initialize the secret state with a slow poll */
#define SPLEN 65536  /* 64K */

static void yarrow_slow_init( yarrow_t *p )
/* This fails silently and must be fixed. */
{
	sha1_ctx_t* ctx = NULL;
	uint8_t* bigbuf = NULL;
	uint8_t  buf[20];
	int32_t polllength;

	bigbuf = (uint8_t *)calloc( SPLEN, 1 );
    if( bigbuf == NULL )
        goto cleanup_slow_init;
	ctx = (sha1_ctx_t *)calloc( sizeof( sha1_ctx_t ), 1 );
    if( ctx == NULL )
        goto cleanup_slow_init;


	/* Initialize the secret state. */
	/* Init entropy pool */
	SHA1Init(&p->pool);
	/* Init output generator */
	polllength = yarrow_slow_poll( bigbuf, SPLEN );
	SHA1Init(ctx);
	SHA1Update(ctx,bigbuf,polllength);
	SHA1Final(buf,ctx);
	yarrow_make_new_state( p, &p->outstate,buf );

cleanup_slow_init:
    trash_mem( buf, sizeof( buf ) );
    free( bigbuf );
    free( ctx );

	return;
}

static void trash_mem(void* mem,uint32_t len)
/* This function should only be used on data in RAM */
{
	/* Cycle a bit just in case it is one of those weird memory units */
	/* No, I don't know which units those would be */
	memset(mem,0x00,len);
	memset(mem,0xFF,len);
	memset(mem,0x00,len);
}

/* In-place modifed bubble sort */
static void bubble_sort(uint32_t *data,uint32_t len) 
{
	uint32_t i,last,newlast,temp;

	last = len-1; 
	while(last!=-1) 
	{
		newlast = -1;
		for(i=0;i<last;i++) 
		{
			if(data[i+1] > data[i]) 
			{
				newlast = i;
				temp = data[i];
				data[i] = data[i+1];
				data[i+1] = temp;
			}
		}
		last = newlast;
	}		
}
