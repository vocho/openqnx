/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
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

#ifndef _SIGNATURE_H_INCLUDED
#define _SIGNATURE_H_INCLUDED

#include <stdint.h>

typedef struct {
	uint16_t val;
	uint16_t mask;
} bt_signature_u16_t;

typedef struct {
	uint32_t val;
	uint32_t mask;
} bt_signature_u32_t;

#define BT_SIG_MATCH(name, data)   _bt_sig32_check(name, name ## _len, data)

#define BT_SIG32_MATCH(name, data) _bt_sig32_check(name, name ## _len, data)

#define BT_SIG16_MATCH(name, data) _bt_sig16_check(name, name ## _len, data)

#define BT_SIG_LEN(name) static const int name ## _len=sizeof(name)/sizeof(name[0])

static inline int
_bt_sig16_check (const bt_signature_u16_t sig[], int siglen, uint16_t *data)
{
	int i;
	for (i=0; i<siglen; i++) {
		if ((data[i] & sig[i].mask) != (sig[i].val & sig[i].mask))
			return 0;
	}
	return 1;
}

static inline int
_bt_sig32_check (const bt_signature_u32_t sig[], int siglen, uint32_t *data)
{
	int i;
	for (i=0; i<siglen; i++) {
		if ((data[i] & sig[i].mask) != (sig[i].val & sig[i].mask))
			return 0;
	}
	return 1;
}

#endif
