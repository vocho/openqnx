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





#ifndef __YARROW_H
#define __YARROW_H

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>

#include "comp.h"
#include "sha1mod.h"

#define MAX_SOURCES       32
#define ENTROPY_SOURCES   10 
#define SLOWPOLLSOURCE    0
#define TOTAL_SOURCES     ENTROPY_SOURCES
#define COMP_SOURCES      ENTROPY_SOURCES
#define BACKTRACKLIMIT    428
#define K                 0
#define THRESHOLD         100

/* Error numbers */
typedef enum yarrow_ready_status 
{
	YARROW_READY = 33,	
	YARROW_NOT_READY = 0
} yarrow_ready_status;


/* Top level output state */
typedef struct
{
	uint8_t iv[20];
	uint8_t out[20];
} yarrow_gen_ctx_t;


/* yarrow_t state structure */
typedef struct
{
	/* Output State */
	yarrow_gen_ctx_t  outstate;
	uint32_t          index;
	uint32_t          numout;

	/* Entropy Pools (somewhat unlike a gene pool) */
	sha1_ctx_t        pool;

    uint32_t          max_pool_count;
    uint32_t          pool_count;
	uint32_t          poolSize[MAX_SOURCES];			
	uint32_t          poolEstBits[MAX_SOURCES];
	comp_ctx_t        comp_state[MAX_SOURCES];

	/* Status Flags */
	yarrow_ready_status ready;
    pthread_mutex_t   mutex;

} yarrow_t;



/* Error Codes */
typedef enum yarrow_error_status {
	YARROW_SUCCESS = 0,
	YARROW_ERR_REINIT,
	YARROW_ERR_WRONG_CALLER,
	YARROW_ERR_NOT_READY,
	YARROW_ERR_NULL_POINTER,
	YARROW_ERR_LOW_MEMORY,
	YARROW_ERR_OUT_OF_BOUNDS,
	YARROW_ERR_COMPRESSION,
	YARROW_ERR_NOT_ENOUGH_ENTROPY,
	YARROW_ERR_MUTEX,
	YARROW_ERR_TIMEOUT,
	YARROW_ERR_PROGRAM_FLOW
} yarrow_error_status;


/* yarrow.c */

yarrow_t *yarrow_create( void );
int yarrow_destory( yarrow_t *p );
int yarrow_add_source( yarrow_t *p, uint32_t *pool_no );
int yarrow_output( yarrow_t *p, uint8_t *outbuf, uint32_t outbuflen );
int yarrow_stretch( uint8_t *inbuf, uint32_t inbuflen, uint8_t *outbuf, 
                    uint32_t outbuflen );
int yarrow_input( yarrow_t *p, uint8_t *inbuf, uint32_t inbuflen, 
                  uint32_t poolnum, uint32_t estbits );
int yarrow_force_reseed( yarrow_t *p, uint64_t ticks );
int yarrow_allow_reseed( yarrow_t *p, uint64_t ticks );


#endif
