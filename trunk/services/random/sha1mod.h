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
    SHA-1 in C
    By Steve Reid <steve@edmweb.com>
    100% Public Domain
*/
/* Header portion split from main code for convenience (AYB 3/02/98) */

#ifndef SHA1_H

#define SHA1_H

/*
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#define LITTLE_ENDIAN /* This should be #define'd if true. */
#undef SHA1HANDSOFF /* Copies data before messing with it. */

//Context declaration
typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} sha1_ctx_t;

//Function forward declerations
void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(sha1_ctx_t* context);
void SHA1Update(sha1_ctx_t* context, unsigned char* data, unsigned int len);
void SHA1Final(unsigned char digest[20], sha1_ctx_t* context);

#endif
