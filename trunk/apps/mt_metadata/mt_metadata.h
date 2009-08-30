/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/

#ifndef MT_METADATA_H_
#define MT_METADATA_H_

#include <stdint.h>
#include <sys/mt_trace.h>

#define ALIGNMENT	0
#define _mt_meta_KER_EVENT(id, name, format)			pt0 = mt_meta_event(pt0, "kernel", name, id, format)
#define _incr_strcpy(pt, str)					strcpy(pt, str); pt += (strlen(str) + 1)


void * mt_meta_events (void *pt0);
void * mt_meta_event(void *pt0, char *channel, char *name, unsigned short id, char *format);
void * mt_meta_write_id(void *pt0, unsigned short id);
void * mt_meta_write_id29(void *pt0, unsigned short id, unsigned size);
void * mt_meta_write_header(void *pt0);
void * mt_meta_ker_events (void *pt0);

struct mt_meta_event_header {
	unsigned 	id			: 5;
	unsigned 	timestamp	: 27;
};
#if 0
union mt_meta_event_header {
	/* event header */

	struct mt_meta_event_header sep;

	uint32_t	both;
};
#endif

struct mt_meta_event_id29 {
	uint16_t	id;
	uint16_t	smallsize;
	uint32_t	fullsize;
	uint64_t	timestamp;
};





#endif /* MT_METADATA_H_ */
