/*
 * new_montt_trace.c
 *
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

#if 0
#include "mt_kertrace.h"

extern void *privateptr;
extern struct montt_data_ctrl;
extern struct montt_info;


void montt_init_traceset(struct montt_data_ctrc *ptdc) {

	ltt_subbuffer_header_t *pthd;

	/* double-check status */
	if (ptdc->status != -1)
		kprintf("error: montt_init_traceset got nonempty traceset\n");

	pthd = ptdc->pt_begin;

	/* fill header */
	montt_write_header(pthd);

	ptcd->pt_current = pthd;	/* pointer to next byte available for writing */
	ptdc->status = 0;	/* not emply any more */

	return;
}

int montt_write_header(ltt_subbuffer_header_t *ltt_hd) {

	ltt_hd->cycle_count_begin = 0;     /* Cycle count at subbuffer start */
	ltt_hd->cycle_count_end = 0;       /* Cycle count at subbuffer end */
	ltt_hd->magic_number = 0x00D6B7ED;          /*
									 * Trace magic number.
									 * contains endianness information.
									 */
	ltt_hd->major_version = 0;
	ltt_hd->minor_version = 1;
	ltt_hd->arch_size = 0;              /* Architecture pointer size */
	ltt_hd->alignment = 0;              /* LTT data alignment */
	ltt_hd->start_time_sec = 0;        /* NTP-corrected start time */
	ltt_hd->start_time_usec = 0;
	ltt_hd->start_freq = 0;            /*
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
	++ltt_hd;
	//if (++ltt_hd != ltt_hd->header_end)
	return 1;

}


void montt_trace_task_create(pid_t pid, tid_t tid, int priority) {
	void		*pt0;
	uint8_t		*pt1;
	uint16_t	*pt2;
	uint32_t	*pt4;

	pt4 = montt_get_tracing_pointer();

	pt4++ = pid;
	pt4++ = tid;
	pt4++ = priority;

	/* set current pointer !!! */

	/* check if traceset full */

}

void * montt_get_tracing_pointer() {
	struct montt_info		*ptinfo;
	struct montt_data_ctrl	*ptdc;
	struct montt_data_ctrl	*ptdc_empty, *ptdc_current;
	void					*pt0;
	int						i;

	ptinfo++ = privateptr->montt_trace;	/* after the info structure */
	ptdc = ptinfo;

	/* get cpuid */
	//...
	ptcd += cpuid * TRACESETS_PER_CPU;	/* at first data_ctrl of cpuid */

	/* check if there are currently filling or empty trace-sets for this cpu */
	ptdc_empty = ptcd_current = NULL;
	for (i = 0; i < TRACESETS_PER_CPU; ++i) {
		/* consider using switch rather than ifs */
		if (ptdc->status == -1)
			ptdc_empty = ptdc;
		if (ptdc->status == 0) {
			if (unlikely(ptcd_current != NULL))
				kprintf("error: more than one currently filling trace-sets per cpu\n");
			ptcd_current = ptdc->pt_current;
			/* should we break now or continue for safety check? */
		}
		ptdc++;
	}
	if (ptcd_current == NULL) {
		if (unlikely(ptcd_empty == NULL))
			kprintf("error: all trace-sets appear to be full!\n");
		else
			montt_init_traceset(ptdc_empty);
			pdtc_current = ptdc_empty->ptr_current;
	}

	/* HACK ALERT : all above is the same for all events: make it a function! */

	/* add event and increment ptr_current */
	// to be continued ...

	return;
}









void alloc_buffer() {




}

#endif
