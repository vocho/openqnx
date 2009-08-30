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




#include <string.h>

LIBC_WEAK(straddstr, __straddstr);

int straddstr(const char *str, int len, char **pbuf, size_t *pmaxbuf) {
	int					nbytes;
	char				*buf;

	if(len == 0 && str) {
		len = strlen(str);
	}
	nbytes = len;
	if((buf = *pbuf) && *pmaxbuf > 0) {
		if(str) {
			while(*pmaxbuf > 1 && nbytes--) {
				(*pmaxbuf)--;
				*buf++ = *str++;
			}
		}
		*buf = '\0';
		*pbuf = buf;
	}
	return len;
}

__SRCVERSION("straddstr.c $Rev: 167420 $");
