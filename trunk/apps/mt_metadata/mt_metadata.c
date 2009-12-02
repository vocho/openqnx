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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/time.h>
#include "mt_metadata.h"
#include <string.h>

#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <inttypes.h>


char *fname = "metadata_0";
int metafic;
struct timeval mt_meta_time;
uint64_t		clk_cycles;


/** here are defined all events of the kernel (file kernel_[n]) */
void * mt_meta_ker_events (void *pt0) {
	/* usage
	 * 		_mt_meta_KER_EVENT(id, 	name,	format);
	 * or :
	 * 		char next_id = 1;
	 * 		_mt_meta_KER_EVENT(next_id++,	name,	format);
	 */

	/* This dummy_check trace contains no information besides the timestamp.
	 * Its purpose is to measure delay between two consecutive tracing calls
	 * using timestamps to quantify intrusive factor and delay due to tracing.
	 */
	_mt_meta_KER_EVENT(0,	"dummy_check",		"");

	/* task/thread events */
	_mt_meta_KER_EVENT(1, 	"task_create",		"pid %u tid %u priority %c");
	_mt_meta_KER_EVENT(2, 	"task_suspend",		"pid %u tid %u");
	_mt_meta_KER_EVENT(3, 	"task_resume",		"pid %u tid %u");
	_mt_meta_KER_EVENT(4, 	"task_delete",		"pid %u tid %u status %d");
	_mt_meta_KER_EVENT(5, 	"task_periodicity",	"pid %u tid %u period %llu");
	_mt_meta_KER_EVENT(6, 	"task_priority",	"pid %u tid %u priority %c");
	_mt_meta_KER_EVENT(7,	"task_info",		"pid %u tid %u state %c priority %c");
	_mt_meta_KER_EVENT(9,	"task_cycle_done",	"pid %u tid %u");

	/* semaphore events */
	_mt_meta_KER_EVENT(10,	"sem_init",			"&sem %p value %d pid %u tid %u");
	_mt_meta_KER_EVENT(11,	"sem_P",			"&sem %p value %d pid %u tid %u");
	_mt_meta_KER_EVENT(12,	"sem_V",			"&sem %p value %d pid %u tid %u");
	_mt_meta_KER_EVENT(13,	"mutex_init",		"&mutex %p pid %u tid %u");
	_mt_meta_KER_EVENT(14,	"mutex_lock",		"&mutex %p pid %u tid %u owner %u");
	_mt_meta_KER_EVENT(15,	"mutex_unlock",		"&mutex %p pid %u tid %u owner %u");
	_mt_meta_KER_EVENT(19,	"sync_destroy",		"&sync %p value %d pid %u tid %u");

	/* IRQ events */
	_mt_meta_KER_EVENT(20,	"IRQ_entry",		"irq %u");
	_mt_meta_KER_EVENT(21,	"IRQ_exit",			"irq %u");
	_mt_meta_KER_EVENT(22,	"IRQ_handler_entry","irq %u");
	_mt_meta_KER_EVENT(23,	"IRQ_handler_exit",	"irq %u");
	_mt_meta_KER_EVENT(24,	"IRQ_timer_entry",	"");
	_mt_meta_KER_EVENT(25,	"IRQ_timer_exit",	"");


	/* Event for debug: used to see which function and line emitted next trace,
	 * and allows to display some text.
	 */
	_mt_meta_KER_EVENT(0xFFFE,"var_debug",		"val1 %u val2 %u val3 %p");
	_mt_meta_KER_EVENT(0xFFFF,"debug",			"calling_function %s line %u text %s");

	return pt0;
}

int main () {

    void	*ptbeg, *ptend;
	ltt_subbuffer_header_t *pthd;
	size_t	fsize;

    ptbeg = malloc(4*1024);
    if (ptbeg == NULL)
    	perror("allocation went wrong\b");
    ptend = ptbeg;
	pthd = (ltt_subbuffer_header_t *) ptbeg;

	/* write the binary */

	ptend = mt_meta_write_header(ptend);

	ptend = mt_meta_events(ptend);

	ptend = mt_meta_ker_events(ptend);



	remove(fname);

	/* file open */
	if((metafic = open(fname, O_WRONLY /*| O_APPEND*/ | O_CREAT,
			S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
		printf("USR Cannot open file '%s'.\n", fname);
		exit(1);
	}

	if (ptbeg != pthd)
		printf("pointers are not the same!\n");
	fsize = (ptend - ptbeg);
	printf("size = %u (header size = %u)\n", fsize, sizeof(ltt_subbuffer_header_t));
	pthd->buf_size = fsize;

    write (metafic, ptbeg, fsize);


    close(metafic);
    free(ptbeg);

	return 0;
}

/**
 * redefines the metadata events
 * begins the writing at start pointer pt0, and returns end pointer
 */
void * mt_meta_events (void *pt0) {

	uint8_t		*pt1;
	uint16_t	*pt2;

	/* event marker id (id29) */

	/* header 26 */
	/* "metadata":9 "core_marker_id":15 24+7=26 */
	pt0 = mt_meta_write_id29(pt0, 0, 26);

	/* marker id 0 */
	_incr_strcpy(pt0, "metadata");
	_incr_strcpy(pt0, "core_marker_id");
	pt2 = (uint16_t *) pt0;
	*(pt2++) = 0;	/* id */
	pt1 = (uint8_t *) pt2;
	*(pt1++) = sizeof(int);
	*(pt1++) = sizeof(long);
	*(pt1++) = sizeof(void *);
	*(pt1++) = sizeof(size_t);
	*(pt1++) = ALIGNMENT;	/* alignement */

	pt0 = pt1;

	/* marker format 1 */
	pt0 = mt_meta_write_id(pt0, 1);
	_incr_strcpy(pt0, "metadata");
	_incr_strcpy(pt0, "core_marker_id");
	_incr_strcpy(pt0, "channel %s name %s event_id %hu int #1u%zu long #1u%zu pointer #1u%zu size_t #1u%zu alignment #1u%u");

	/* event marker format */
	pt0 = mt_meta_write_id(pt0, 0);
	_incr_strcpy(pt0, "metadata");
	_incr_strcpy(pt0, "core_marker_format");
	pt2 = pt0;
	*(pt2++) = 1;	/* id */
	pt1 = (uint8_t *) pt2;
	*(pt1++) = sizeof(int);
	*(pt1++) = sizeof(long);
	*(pt1++) = sizeof(void *);
	*(pt1++) = sizeof(size_t);
	*(pt1++) = ALIGNMENT;	/* alignment */

	pt0 = pt1;

	/* marker format 1 */
	pt0 = mt_meta_write_id(pt0, 1);
	_incr_strcpy(pt0, "metadata");
	_incr_strcpy(pt0, "core_marker_format");
	_incr_strcpy(pt0, "channel %s name %s format %s");

	return pt0;

}

/**
 * writes the event definition (any non-metadata event)
 * called from mt_meta_ker_events
 */
void * mt_meta_event(void *pt0, char *channel, char *name, unsigned short id, char *format) {

	uint8_t		*pt1;
	uint16_t	*pt2;

	printf("id = %u, channel = %s, name = %s, format = %s\n", id, channel, name, format);

	/* 0 */
	pt0 = mt_meta_write_id(pt0, 0);
	_incr_strcpy(pt0, channel);
	_incr_strcpy(pt0, name);
	pt2 = pt0;
	*(pt2++) = id;	/* id */
	pt1 = (uint8_t *) pt2;
	*(pt1++) = sizeof(int);
	*(pt1++) = sizeof(long);
	*(pt1++) = sizeof(void *);
	*(pt1++) = sizeof(size_t);
	*(pt1++) = ALIGNMENT;	/* alignment */

	pt0 = pt1;

	/* 1 */
	pt0 = mt_meta_write_id(pt0, 1);
	_incr_strcpy(pt0, channel);
	_incr_strcpy(pt0, name);
	_incr_strcpy(pt0, format);

	return pt0;
}

/** writes the 32 bit event header (id & small timestamp) */
void * mt_meta_write_id(void *pt0, unsigned short id) {

	/* only events 0 and 1 will be found in metadata,
	 * no need for further check*/

	uint32_t	both, temp;
	uint32_t	*pt4;

	clk_cycles = ClockCycles();

	both = (uint32_t) clk_cycles;

	both &= 0x07FFFFFF; /* resetting first 5 bits (to 0) */

	temp = (id << 27); /* shifting id left */
	temp &= 0xF8000000;	/* resetting last 27 bits (to 0) */ /* 0x07FFFFFF 0xF8000000 */
	both |= temp;	/* joining both */

	pt4 = (uint32_t *) pt0;
	*(pt4++) = both;

	return pt4;

}

/** writes the event header with full timestamp (n°29) */
void * mt_meta_write_id29(void *pt0, unsigned short id, unsigned size) {

	pt0 = mt_meta_write_id(pt0, 29);

	/* alternate method for id 29 */
	uint16_t	*pt2;
	uint64_t	*pt8;
	pt2 = pt0;

	//clk_cycles = ClockCycles();
	*(pt2++) = id;			/* id */
	if (size < 0xFFFF) {	/* likely */
		/* smallsize is enough */
		*(pt2++) = size;	/* smallsize */
		pt8 = (uint64_t *) pt2;
	} else {
		/* fullsize required */

		*(pt2++) = 0xFFFF;	/* smallsize */
		uint32_t	*pt4;
		pt4 = (uint32_t *) pt2;
		*(pt4++) = size;	/* fullsize */

		pt8 = (uint64_t *) pt4;
	}
	*(pt8++) = clk_cycles;	/* mt_meta_timestamp (full) */

	return pt8;

}


/** writes buffer header (contains some QNX specific code) */
void * mt_meta_write_header(void *pt0) {

	ltt_subbuffer_header_t	 *ltt_hd;

	gettimeofday(&mt_meta_time, NULL);
	ltt_hd = pt0;

	ltt_hd->cycle_count_begin = 0;     /* Cycle count at subbuffer start */
	ltt_hd->cycle_count_end = 0;       /* Cycle count at subbuffer end */
	ltt_hd->magic_number = 0x00D6B7ED;          /*
									 * Trace magic number.
									 * contains endianness information.
									 */
	ltt_hd->major_version = 0;
	ltt_hd->minor_version = 1;
	ltt_hd->arch_size = 0;              /* Architecture pointer size */
	ltt_hd->alignment = ALIGNMENT;              /* LTT data alignment */
	ltt_hd->start_time_sec = mt_meta_time.tv_sec;        /* NTP-corrected start time */
	ltt_hd->start_time_usec = mt_meta_time.tv_usec;
	ltt_hd->start_freq = SYSPAGE_ENTRY( qtime )->cycles_per_sec;            /*
									 * Frequency at trace start,
									 * used all along the trace.
									 */
	ltt_hd->freq_scale = 0;            /* Frequency scaling (divisor) */
	ltt_hd->lost_size = 0;             /* Size unused at end of subbuffer */
	ltt_hd->buf_size = 0xFFFFFFFF;     /* Size of this subbuffer */
	ltt_hd->events_lost = 0;           /*
									 * Events lost in this subbuffer since
									 * the beginning of the trace.
									 * (may overflow)
									 */
	ltt_hd->subbuf_corrupt = 0;        /*
									 * Corrupted (lost) subbuffers since
									 * the begginig of the trace.
									 * (may overflow)
									 */

	return ltt_hd->header_end;
}
