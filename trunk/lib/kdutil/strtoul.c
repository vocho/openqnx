/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <stdlib.h>
#include <sys/types.h>

/*
	Override the C library's definition because we don't need all of
	its fluff.
*/

#define hexstr(p) (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))

static int
radix_value( char c ) {
	if(c >= '0'  &&  c <= '9') return(c - '0');
	if(c >= 'a'  &&  c <= 'f') return(c - 'a' + 10);
	if(c >= 'A'  &&  c <= 'F') return(c - 'A' + 10);
	return( 37 );
}

paddr_t
strtopaddr(const char *nptr, char **endptr, int base) {
	const char	*p;
	const char	*startp;
	int 		digit;
	paddr_t		value;

	if(endptr != NULL) *endptr = (char *)nptr;
	p = nptr;
	if(base == 0) {
	    if(hexstr(p))	 base = 16;
	    else if(*p == '0') base = 8;
	    else		 base = 10;
	}
	if(base == 16) {
	    if(hexstr(p))  p += 2;	/* skip over '0x' */
	}
	startp = p;
	value = 0;
	for(;;) {
	    digit = radix_value(*p);
	    if(digit >= base) break;
	    value = value * base + digit;
	    ++p;
	}
	if(p == startp)  p = nptr;
	if(endptr != NULL) *endptr = (char *)p;
	return value;
}

unsigned long
strtoul(const char *nptr, char **endptr, int base) {
	//Assuming that sizeof(paddr_t) >= sizeof(unsigned long)
	return strtopaddr(nptr, endptr, base);
}

__SRCVERSION("strtoul.c $Rev: 153052 $");
