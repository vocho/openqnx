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





/* comp.h

   Header for the compression routines added to the Counterpane PRNG. 
*/

#ifndef YARROW_COMP_H
#define YARROW_COMP_H

#include "zlib.h"

/* Top level compression context */
typedef struct{
	uint8_t  *buf;
    uint32_t space;
	uint32_t spaceused;
} comp_ctx_t;

typedef enum comp_error_status {
	COMP_SUCCESS = 0,
	COMP_ERR_NULL_POINTER,
	COMP_ERR_LOW_MEMORY,
	COMP_ERR_LIB
} comp_error_status;

/* Exported functions from compress.c */
int comp_init(comp_ctx_t* ctx);
int comp_add_data(comp_ctx_t* ctx,Bytef* inp,uInt inplen);
int comp_end(comp_ctx_t* ctx);
int comp_get_ratio(comp_ctx_t* ctx,float* out);

#endif

