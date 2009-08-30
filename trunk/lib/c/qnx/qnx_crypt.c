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
 * crypt: a relatively simple salted encryption routine.
 *	  produces reasonable results.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
qnx_base_64(short x)
{
    if (x <= 9)  return x + '0';
    x -= 9;
    if (x <= 26) return x + 'A';
    x -= 26;
    return x + 'a';
}

char *
qnx_crypt(char *pw, char *salt)
{
    static char buf[14];
    char bits[72];
    int i, j, rot;

    memset(bits, 0, sizeof bits);
    if (salt[1] == 0)
	salt[1] = salt[0];
    rot = (salt[1] * 4 - salt[0]) % 128;
    for (i = 0; *pw && i < 8; i++) {
	for (j = 0; j < 7; j++)
	    bits[i + j * 8] = (*pw & (1 << j) ? 1 : 0);
	bits[i + 56] = (salt[i / 4] & (1 << (i % 4)) ? 1 : 0);
	pw++;
    }
    bits[64] = (salt[0] & 1 ? 1 : 0);
    bits[65] = (salt[1] & 1 ? 1 : 0);
    bits[66] = (rot & 1 ? 1 : 0);
    while (rot--) {
	for (i = 65; i >= 0; i--)
	    bits[i + 1] = bits[i];
	bits[0] = bits[66];
    }
    for (i = 0; i < 12; i++) {
	buf[i + 2] = 0;
	for (j = 0; j < 6; j++)
	    buf[i + 2] |= (bits[i * 6 + j] ? (char)(1 << j) : 0);
	buf[i + 2] = (char)qnx_base_64(buf[i + 2]);
    }
    buf[0] = salt[0];
    buf[1] = salt[1];
    buf[13] = '\0';
    return buf;
}

__SRCVERSION("qnx_crypt.c $Rev: 153052 $");
