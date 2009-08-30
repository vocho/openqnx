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




//#include <sys/dispatch.h>
#include <malloc.h>
#include <sys/dispatch.h>
#include "dispatch.h"

void *_dispatch_vector_find(void *vec,int num) {
	message_vec_t			*vector = vec;
	int						i = 0;

	while(i < num) {
		if(!(vector->flags & _VEC_VALID)) return vector;	
		vector++; i++;
	}
	return NULL;
}

void _dispatch_vector_free(void *vec, int index) {
	message_vec_t			*vector = vec;

	vector[index].flags &= ~_VEC_VALID;
}

void *_dispatch_vector_grow(void *vec, int newnum) {

	return realloc(vec, sizeof(message_vec_t) * newnum);
}

__SRCVERSION("vec_util.c $Rev: 153052 $");
