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
 * crypt:	a relatively simple salted encryption routine.
 *			produces reasonable results.
 *



 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "login.h"

/* TF NOTE: 
   This function should not be required.  The crypt() 
   function should be found in the NTO C library and 
   in the QNX unix.lib compatability library.

   There is a qnx_crypt() for old QNX compatability
   and the standard crypt() for general Unix compatability
*/
#if 0
char *crypt(const char *pw, const char *salt)
{
static char buf[14];
char bits[67];
int i;
int j, rot;
char	salt0 = salt[0];
char	salt1 = salt[1];


	memset(bits,0,sizeof bits);
	if (salt1 == 0)
		salt1 = salt0;
	rot = (salt1 * 4 - salt0) % 128;
	for (i=0; *pw && i < 8; i++) {
		for (j=0; j < 7; j++)
			bits[i+j*8] = (*pw & (1 << j) ? 1 : 0);
		bits[i+56] = (salt[i / 4] & (1 << (i % 4)) ? 1 : 0);
		pw++;
	}
	bits[64] = (salt0 & 1 ? 1 : 0);
	bits[65] = (salt1 & 1 ? 1 : 0);
	bits[66] = (rot & 1 ? 1 : 0);
	while (rot--) {
		for (i=65; i >= 0; i--)
			bits[i+1] = bits[i];
		bits[0] = bits[66];
	}
	for (i=0; i < 12; i++) {
		buf[i+2] = 0;
		for (j=0; j < 6; j++)
			buf[i+2] |= (bits[i*6+j] ? (1 << j) : 0);
		buf[i+2] = base_64(buf[i+2]);
	}
	buf[0] = salt0;
	buf[1] = salt1;
	buf[13] = '\0';
	return(buf);
}

/** Neutrino Crypt function **/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*	
	From Tanenbaum's book "Computer Networks",
	coded in C from pascal. Pg 398 1st edition 1981

	Partially based on minix code for crypt in order
	to guarantee consistancy w/ other Unix DES type
	crypts.

	TODO: Finish modifying the tables to be functions
	      so as to reduce the data size.
*/

struct block {
	unsigned char b_data[64];
};

struct ordering {
	unsigned char o_data[64];
};

static struct block key;

/*
 Standard DES encryptions tables
*/
/* 
uint8_t InitialTr(uint8_t index) {
	uint8_t start, row, col; 

	col = index % 8;
	row = index / 8;

	if (row < 4) 
		start = 58 + (row * 2);
	else
		start = 57 + ((row-4) * 2);  

	return start + (col * 8);
}
*/
const static struct ordering InitialTr = { {
	58,50,42,34,26,18,10, 2, 
	60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6, 
	64,56,48,40,32,24,16, 8,
	57,49,41,33,25,17, 9, 1, 
	59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5, 
	63,55,47,39,31,23,15, 7, }
};

/*
uint8_t FinalTr(uint8_t index) {
	uint8_t row, col; 

	col = (index % 8) / 2;
	row = index / 8;

	start = ((index % 2) == 0) ? 40 : 8;
	start -= row;

	return start + (col * 8);
}
*/
const static struct ordering FinalTr = { {
	40, 8,48,16,56,24,64,32,
	39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30,
	37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28,
	35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26,
	33, 1,41, 9,49,17,57,25, }
};

/*
uint8_t swap(uint8_t index) {
	uint8_t offset, start;

	offset = (index % 32);
	if (offset < 32) 
		start = 33;
	else
		start = 1;

	return start + offset;
}
*/
const static struct ordering swap = { {
	33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32, }
};

const static struct ordering KeyTr1 = { {
	57,49,41,33,25,17, 9, 1,
	58,50,42,34,26,18,10, 2,
	59,51,43,35,27,19,11, 3,
	60,52,44,36,63,55,47,39,
	31,23,15, 7,62,54,46,38,
	30,22,14, 6,61,53,45,37,
	29,21,13, 5,28,20,12, 4, }
};

const static struct ordering KeyTr2 = { {
	14,17,11,24, 1, 5, 3,28,
	15, 6,21,10,23,19,12, 4,
	26, 8,16, 7,27,20,13, 2,
	41,52,31,37,47,55,30,40,
	51,45,33,48,44,49,39,56,
	34,53,46,42,50,36,29,32, }
};

/* 
 These two can be funcitionized as well 
*/
const static struct ordering etr = { {
	32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9,
	 8, 9,10,11,12,13,12,13,14,15,16,17,
	16,17,18,19,20,21,20,21,22,23,24,25,
	24,25,26,27,28,29,28,29,30,31,32, 1, }
};

const static struct ordering ptr = { {
	16, 7,20,21,29,12,28,17, 1,15,23,26, 5,18,31,10,
	 2, 8,24,14,32,27, 3, 9,19,13,30, 6,22,11, 4,25, }
};

//We really need to find the algorithm for this ...
const static unsigned char s_boxes[8][64] = {
{	14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	 0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	 4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13,
},

{	15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	 3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	 0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9,
},

{	10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	 1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12,
},

{	 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	 3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14,
},

{	 2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	 4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3,
},

{	12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	 9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	 4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13,
},

{	 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	 1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	 6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12,
},

{	13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	 1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	 7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	 2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11,
},
};

static int rots(int i) { 
	if (i==0 || i==1 || i==8 || i==15)	
		return(1); 
	else
		return(2); 
}
/*
const static int rots[] = {
	1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,
};
*/

static void transpose(struct block *data, const struct ordering *t, int n) {
	struct block x;

	x = *data;

	while (n-- > 0) {
		data->b_data[n] = x.b_data[t->o_data[n] - 1];
	}
}

static void rotate(struct block *key) {
	unsigned char *p = key->b_data;
	unsigned char *ep = &(key->b_data[55]);
	int data0 = key->b_data[0], data28 = key->b_data[28];

	while (p++ < ep) *(p-1) = *p;
	key->b_data[27] = data0;
	key->b_data[55] = data28;
}

const static struct ordering *EP = &etr;

static void f(int i, struct block *key, struct block *a, struct block *x) {
	struct block e, ikey, y;
	int k;
	unsigned char *p, *q, *r;

	e = *a;
	transpose(&e, EP, 48);

	//for (k = rots[i]; k; k--) rotate(key);
	for (k = rots(i); k; k--) rotate(key);
	ikey = *key;
	transpose(&ikey, &KeyTr2, 48);
	p = &(y.b_data[48]);
	q = &(e.b_data[48]);
	r = &(ikey.b_data[48]);
	while (p > y.b_data) {
		*--p = *--q ^ *--r;
	}
	q = x->b_data;
	for (k = 0; k < 8; k++) {
		int xb, r;

		r = *p++ << 5;
		r += *p++ << 3;
		r += *p++ << 2;
		r += *p++ << 1;
		r += *p++;
		r += *p++ << 4;

		//TODO: Replace this with a function
		xb = s_boxes[k][r];

		*q++ = (xb >> 3) & 1;
		*q++ = (xb>>2) & 1;
		*q++ = (xb>>1) & 1;
		*q++ = (xb & 1);
	}
	transpose(x, &ptr, 32);
}

void setkey(const char *k) {

	key = *((struct block *) k);
	transpose(&key, &KeyTr1, 56);
}

void encrypt(char *blck, int edflag) {
	struct block *p = (struct block *) blck;
	int i;

	//If the user wants to decrypt, we don't
	//support that, so UNIX98 says return ENOSYS
	if (edflag == 1) {
		errno = ENOSYS;
		return;
	}

	transpose(p, &InitialTr, 64);
	for (i = 15; i>= 0; i--) {
		int j = edflag ? i : 15 - i;
		int k;
		struct block b, x;

		b = *p;
		for (k = 31; k >= 0; k--) {
			p->b_data[k] = b.b_data[k + 32];
		}
		f(j, &key, p, &x);
		for (k = 31; k >= 0; k--) {
			p->b_data[k+32] = b.b_data[k] ^ x.b_data[k];
		}
	}
	transpose(p, &swap, 64);
	transpose(p, &FinalTr, 64);
}

char *crypt(const char *pw, const char *insalt)
{
	struct ordering new_etr;
	static char result[16];
	char pwb[66], salt[3];
	char *cp, *p;
	int i;

	p = pwb;

	/* Generate the 64 bit encryption buffer based on the users
	   initial 8 character password entry. The buffer is a binary
	   01 combination used for the encrypt function */
	while (*pw && p < &pwb[64]) {
		int j = 7;

		while (j--) {
			*p++ = (*pw >> j) & 01;
		}
		pw++;
		*p++ = 0;
	}
	while (p < &pwb[64]) *p++ = 0;

	setkey(p = pwb);

	while (p < &pwb[66]) *p++ = 0;

	new_etr = etr;
	EP = &new_etr;

	/* If there is no salt, then make a default one */
	if (!insalt || insalt[0] == 0 || insalt[1] == 0) {
		salt[0] = salt[1] =  '*';
	}
	else {
		salt[0] = insalt[0];
		salt[1] = insalt[1];
	}
	salt[2] = '\0';

	/* Add in the salt (assumed [a-zA-Z0-9./]) to the result */
	for (i = 0; i < 2; i++) {
		char c;
		int j;

		result[i] = c = salt[i];

		/* If c was a lower case letter */
		if ( c > 'Z') {	
			c -= 6 + 7 + '.';	
		}
		/* If c was an upper case letter */
		else if ( c > '9') {
			c -= 7 + '.';
		}
		/* If c was a digit, '.' or '/'. */
		else {
			c -= '.';				
		}

		/* Now guaranteed that 0 <= c <= 63 */

		for (j = 0; j < 6; j++) {
			if ((c >> j) & 01) {
				int t = 6*i + j;
				int temp = new_etr.o_data[t];
				new_etr.o_data[t] = new_etr.o_data[t+24];
				new_etr.o_data[t+24] = temp;
			}
		}
	}

/* This will never happen because we check the salt above
	if (result[1] == 0) {
		result[1] = result[0];
	}
*/

	/* Run through 25 iterations of mashing the buffer */
	for (i = 0; i < 25; i++) {
		encrypt(pwb,0);
	}
	EP = &etr;

	p = pwb;
	cp = result+2;
	while (p < &pwb[66]) {
		int c = 0;
		int j = 6;

		while (j--) {
			c <<= 1;
			c |= *p++;
		}

		/* Change the perturbed character to something
		   that is in the right range again */

		/* Force the character >= '.' */
		c += '.';				
		/* If not in [./0-9] force it upper */
		if (c > '9') {
			c += 7;
		}
		/* If not in [A-Z], force it lower */
		if (c > 'Z') {
			c += 6;
		}
		*cp++ = c;
	}
	*cp = 0;
	return result;
}

#endif
