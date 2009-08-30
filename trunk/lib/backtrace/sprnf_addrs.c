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

#ifndef _BT_LIGHT

#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include "backtrace.h"

#define XLATE_ADDRS() \
	do { if (memmap && !xlated) { \
		xlated=1; \
		reladdrs=alloca(sizeof(*reladdrs)*addrslen); \
		offsets=alloca(sizeof(*offsets)*addrslen); \
		indexes=alloca(sizeof(*indexes)*addrslen); \
		filenames=alloca(sizeof(*filenames)*addrslen); \
		if (reladdrs && offsets && indexes && filenames) { \
			bt_translate_addrs(memmap, addrs, addrslen, \
							   reladdrs, offsets, indexes, filenames); \
			} } } while(0)


int
bt_sprnf_addrs (bt_memmap_t *memmap, bt_addr_t *addrs, int addrslen,
				char *fmt, char *out, size_t outlen, char *seperator)
{
	int i;
	char *p;
	bt_addr_t addr;
	int written;
	int complete_written=0;
	int complete_count=0;
	int xlated=0;
	bt_addr_t *reladdrs=0;
	bt_addr_t *offsets=0;
	int *indexes=0;
	char **filenames=0;

	if (addrs==0 || addrslen<0 || fmt==0 || out==0 || outlen<0) {
		errno=EINVAL;
		return -1;
	}
	written = 0;
	if (seperator == 0) seperator="\n";

	for (i=0; i<addrslen; i++) {
		p=fmt;
		addr=addrs[i];
		if (i)
			written+=snprintf(out+written,max(0,outlen-written),
							  "%s", seperator);
		while (p[0] != 0) {
			if (p[0] == '%' && p[1] != 0) {
				p++;
				switch (*p) {
					case '%':
						written+=snprintf(out+written,max(0,outlen-written),
										  "%%");
						break;

					case 'a':
						written+=snprintf(out+written, max(0,outlen-written),
										  "%p", (void*)addr);
						break;

					case 'l':
						XLATE_ADDRS();
						if (reladdrs)
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "%p", (void*)(reladdrs[i]));
						else
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "--");
						break;

					case 'o':
						XLATE_ADDRS();
						if (offsets)
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "%p", (void*)(offsets[i]));
						else
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "--");
						break;

					case 'f':
						XLATE_ADDRS();
						if (filenames) {
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "%s", filenames[i]);
						} else {
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "--");
						}
						break;

					case 'I':   /* index of memory map */
						XLATE_ADDRS();
						if (indexes)
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "%d", indexes[i]);
						else
							written+=snprintf(out+written,
											  max(0,outlen-written),
											  "--");
						break;

					default:
						written+=snprintf(out+written, max(0,outlen-written),
										  "%%%c", (int)*p);
						break;
				}
			} else {
				if (written < outlen-1) {
					out[written]=*p;
					out[written+1]=0;
				}
				written++;
			}
			p++;
		}
		if (written >= outlen) {
			out[complete_written]=0;
			return complete_count;
		}
		complete_written=written;
		complete_count++;
	}
	return complete_count;
}

#endif
