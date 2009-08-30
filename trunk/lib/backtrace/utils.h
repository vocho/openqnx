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

#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#include <string.h>

#ifndef _BT_LIGHT

#define _bt_memcpy  memcpy
#define _bt_memmove memmove

#else

void *_BT(memcpy)(void *dest, const void *src, size_t n);
void *_BT(memmove)(void *dest, const void *src, size_t n);

#endif

#endif
