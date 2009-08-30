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




#include <gulliver.h>
#include <netinet/in.h>

#undef htonl
#undef htons
#undef ntohl
#undef ntohs

uint32_t htonl(uint32_t x)
{
	return ENDIAN_BE32(x);
}

uint32_t ntohl(uint32_t x)
{
	return ENDIAN_BE32(x);
}

uint16_t htons(uint16_t x)
{
	return ENDIAN_BE16(x);
}

uint16_t ntohs(uint16_t x)
{
	return ENDIAN_BE16(x);
}

__SRCVERSION("tohn.c $Rev: 153052 $");
