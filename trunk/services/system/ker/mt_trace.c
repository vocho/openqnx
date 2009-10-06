/*
 * services/system/ker/mt_trace.c
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

/*	includes	*/

#include "mt_kertrace.h"
#include "kertrace.h"
#include <sys/time.h>		/* for struct timeval */
#include <sys/neutrino.h>	/* for TraceEvent */
#include <inttypes.h>		/* for ClockCycles */
#include <sys/trace.h>		/* for TraceEvent */

/*	variables	*/
static struct timeval	ts;
//static uint64_t		clk_cycles;	/* this is now a local variable */
//static uint64_t		clk_cycles29; /* was used when chosing header according to last full timestamp */
static uint64_t			last_clk_cycles;	/* now chosing header according to last timestamp */
static mt_ker_info_t	mt_buf_info;
static mt_trace_filters_t mt_filters = {
	.task = 1,
	.sem = 1,
	.irq = 1,
	.debug = 0
};

/****************************************************************************/
/**************************** binary tracesets ******************************/
/****************************************************************************/

int mt_buffer_init(mt_data_ctrl_t *ptdc) {

	uint8_t		*pt1;
	ptdc->status = -1;
	ptdc->buf_remaining = 23;
	ptdc->buf_begin = (void *) (ptdc + 1);
	pt1 = (uint8_t *) ptdc->buf_begin;
	pt1 += (unsigned) (_MT_TRACESET_SIZE * _MT_BUFFER_FULL);
	ptdc->buf_lim = (void *) pt1;

	ptdc->data_current = mt_write_header(ptdc->buf_begin);

	return 0;
}

/* initialise single data_control structue (multiple-tracesets) */
int mt_new_buffer_init(mt_data_ctrl_t *ptdc, int remaining) {

	uint8_t		*pt1;

	ptdc->status = -1;

	pt1 = (uint8_t *) (ptdc + (remaining + 1)); /* pt1 is now after all data_ctrl structures */
	pt1 += ((_MT_TRACESETS_PER_CPU - 1 - remaining) * _MT_TRACESET_SIZE); /* to correspondig traceset */
	ptdc->buf_begin = pt1;

	pt1 += (unsigned) (_MT_TRACESET_SIZE * _MT_BUFFER_FULL); /* full position (!= max size) */
	ptdc->buf_lim = (void *) pt1;
#if 0
	/* we don't want the header yet any more --> else statement*/
	ptdc->data_current = mt_write_header(ptdc->buf_begin);
#else
	/* the header will be rewritten on demand */
	ptdc->data_current = ptdc->buf_begin;
#endif
	ptdc->buf_remaining = remaining;
	//ptdc->buf_remaining = 24;	/* debug value */

	return 0;
}

/* initialise the whole set of data_control structues + ker_info */
int mt_buffers_init(void *base_addr) {

	mt_data_ctrl_t		*ptdc;
	int					i;

	ptdc = base_addr;
	last_clk_cycles = 0;

	mt_buf_info.first = base_addr;

	for (i = (_MT_TRACESETS_PER_CPU - 1); i >= 0; --i) {
		mt_new_buffer_init(ptdc++, i);
	}

	mt_buf_info.last = (ptdc - 1);
	mt_buf_info.in_use = mt_buf_info.first;
	//mt_buf_info.in_use = mt_buf_info.last;

	return 0;
}

/* wites normal event header (id & short timestamp in 32 bits) */
void * mt_write_id(void *pt0, unsigned short id, uint64_t clk_cycles) {

	uint32_t	both, temp;
	uint32_t	*pt4;

	if (id < 32) {
		//both = (uint32_t) ts.tv_sec;
		both = (uint32_t) clk_cycles;

		both &= 0x07FFFFFF; /* resetting first 5 bits (to 0) */

		temp = (id << 27); /* shifting id left */
		temp &= 0xF8000000;	/* resetting last 27 bits (to 0) */
		both |= temp;	/* joining both */

		pt4 = pt0;
		*pt4++ = both;

		return pt4;
	} else
		return mt_write_id31(pt0, id, clk_cycles);
}

/* wites an event header no 29 with full timestamp and sizes */
void * mt_write_id29(void *pt0, unsigned short id, unsigned size, uint64_t clk_cycles) {

	/* alternate method for id 29 */

	uint16_t	*pt2;
	uint64_t	*pt8;

	// clk_cycles29 = clk_cycles; /* bakup of timestamp (full) */

	pt0 = mt_write_id(pt0, 29, clk_cycles);

	pt2 = (uint16_t *) pt0;
	*pt2++ = id;			/* id */
	if (size < 0xFFFF) {	/* likely */
		/* smallsize is enough */
		*pt2++ = size;	/* smallsize */
		pt8 = (uint64_t *) pt2;
	} else {
		/* fullsize required */

		*pt2++ = 0xFFFF;	/* smallsize */
		uint32_t	*pt4;
		pt4 = (uint32_t *) pt2;
		*pt4++ = size;	/* fullsize */

		pt8 = (uint64_t *) pt4;
	}

	*(pt8++) = clk_cycles;	/* timestamp (full) */

	return pt8;

}
/* writes the event header with big ID*/
void * mt_write_id31(void *pt0, unsigned short id, uint64_t clk_cycles) {

	pt0 = mt_write_id(pt0, 31, clk_cycles);

	uint16_t	*pt2;
	pt2 = pt0;

	*pt2++ = id;

	return pt2;
}

/* called by mt_buffer_init, writes ltt_subbuffer_header
 * at pt0, which must point to a traceset's begining
 * returns a pointer to the end of the header */
void * mt_write_header(void *pt0) {

	gettimeofday(&ts, NULL);

	ltt_subbuffer_header_t *ltt_hd;
	ltt_hd = (ltt_subbuffer_header_t  *) pt0;

	ltt_hd->cycle_count_begin = 0;     /* Cycle count at subbuffer start */
	ltt_hd->cycle_count_end = 0;       /* Cycle count at subbuffer end */
	ltt_hd->magic_number = 0x00D6B7ED;          /*
									 * Trace magic number.
									 * contains endianness information.
									 */
	ltt_hd->major_version = 2;
	ltt_hd->minor_version = 3;
	ltt_hd->arch_size = 0;              /* Architecture pointer size */
	ltt_hd->alignment = _MT_ALIGNMENT;/* LTT data alignment */
	ltt_hd->start_time_sec = ts.tv_sec;        /* NTP-corrected start time */
	ltt_hd->start_time_usec = ts.tv_usec;
	ltt_hd->start_freq = SYSPAGE_ENTRY( qtime )->cycles_per_sec; /*
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

	return (void *) ltt_hd->header_end;
}

void mt_filter_tracing_debug(char val) {
	mt_filters.debug = val;
	return;
}

void mt_list_task_info()
{
	PROCESS* pProcess;
	THREAD* pThread;
	int iProc, iThread;

	for (iProc=1; iProc<process_vector.nentries; iProc++)
		if (VECP(pProcess, &process_vector, iProc))
			for (iThread=0; iThread<pProcess->threads.nentries; iThread++)
				if (VECP(pThread, &pProcess->threads, iThread))

					mt_trace_task_info(pProcess->pid, pThread->tid, pThread->state, pThread->priority); /* pThread->real_priority */
}

void mt_send_flush_pulse()
{
	if (mt_flush_evt_channel)
	{
		//kprintf("Send flush pulse (%d, %d)\n", mt_flush_evt_channel, mt_controller_thread);

		struct sigevent event;
		event.sigev_notify = SIGEV_PULSE;
		event.sigev_coid = mt_flush_evt_channel;
		event.sigev_priority = 10;
		event.sigev_code = _MT_FLUSH_PULSE_CODE;

		intrevent_add(&event, mt_controller_thread, clock_isr);
	}
}
/****************************************************************************/
/*************************** tracing functions ******************************/
/****************************************************************************/

#if 0	/* here is a probe sample*/

#ifdef _mt_LTT_TRACES_	/* PDB */
	mt_TRACE_DEBUG("info");
	mt_trace_dummy_check();
#endif

#endif	/* probe sample */

/* This dummy_check trace contains no information besides the timestamp.
 * Its purpose is to measure delay between two consecutive tracing calls
 * using timestamps to quantify intrusive factor and delay due to tracing.
 * May also help finding interruptions during tracing, if detected, kernel locks
 * or interruption locks should be used.
 * This function also stands for sample, it contains usage comments.
 */
void mt_trace_dummy_check() {

	mt_TRACE_FUNK_top(0, 0);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	/* Add here tracing values (none for dummy_check)
	 * use 'void *pt0' (already declared and used)
	 * to write at the adress pointed to by it.
	 */


	/* Don't foget to set pt0 to next writable byte !! */

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_create(unsigned pid, unsigned tid, unsigned char priority) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(1, 9);

	uint8_t		*pt1;
	uint32_t	*pt4;

	pt4 = pt0;
	*pt4++ = pid;
	*pt4++ = tid;
	pt1 = pt4;
	*pt1++ = priority;

	pt0 = pt1;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_suspend(unsigned pid, unsigned tid) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(2, 8);

	uint32_t	*pt4;
	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_resume(unsigned pid, unsigned tid) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(3, 8);

	uint32_t	*pt4;
	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_delete(unsigned pid, unsigned tid, int status) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(4, 12);

	uint32_t	*pt4;
	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;
	*pt4++ = status;

	pt0 = pt4;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_periodicity(unsigned pid, unsigned tid, unsigned long long period) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(5, 16);

	uint32_t	*pt4;
	uint64_t	*pt8;

	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;
	pt8 = (uint64_t *) pt4;
	*pt8++ = period;

	pt0 = pt8;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_priority(unsigned pid, unsigned tid, unsigned char priority) {

	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(6, 9);

	uint8_t		*pt1;
	uint32_t	*pt4;

	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;
	pt1 = (uint8_t *) pt4;
	*pt1++ = priority;

	pt0 = pt1;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_task_info(unsigned pid, unsigned tid, unsigned char state, unsigned char priority)
{
	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(7, 10);

	uint8_t		*pt1;
	uint32_t	*pt4;

	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;
	pt1 = (uint8_t *) pt4;
	*pt1++ = state;
	*pt1++ = priority;

	pt0 = pt1;

	mt_TRACE_FUNK_bottom;
}
void mt_trace_task_sig_return(unsigned pid, unsigned tid)
{
	if (!mt_filters.task) return;

	mt_TRACE_FUNK_top(9, 8);

	uint32_t	*pt4;

	pt4 = pt0;

	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;

	mt_TRACE_FUNK_bottom;
}

void mt_trace_sem_init(void *sem, int value, unsigned pid, unsigned tid) {

	if (!mt_filters.sem)  return;

	mt_TRACE_FUNK_top(10, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = sem;
	pt4 = ptpt;
	*pt4++ = value;
	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_sem_P(void *sem, int value, unsigned pid, unsigned tid) {

	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(11, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = sem;
	pt4 = ptpt;
	*pt4++ = value;
	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_sem_V(void *sem, int value, unsigned pid, unsigned tid) {

	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(12, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = sem;
	pt4 = ptpt;
	*pt4++ = value;
	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_mutex_init(void *mutex, unsigned pid, unsigned tid) {
	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(13, 12);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = mutex;
	pt4 = ptpt;
	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_mutex_lock(void *mutex, unsigned pid, unsigned tid, unsigned owner) {
	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(14, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = mutex;
	pt4 = ptpt;
	*pt4++ = pid;
	*pt4++ = tid;
	*pt4++ = owner;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_mutex_unlock(void *mutex, unsigned pid, unsigned tid, unsigned owner) {
	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(15, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = mutex;
	pt4 = ptpt;
	*pt4++ = pid;
	*pt4++ = tid;
	*pt4++ = owner;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void mt_trace_sync_destroy(void *sem, int value, unsigned pid, unsigned tid) {

	if (!mt_filters.sem) return;

	mt_TRACE_FUNK_top(19, 16);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	void		**ptpt;
	uint32_t	*pt4;

	ptpt = pt0;
	*ptpt++ = sem;
	pt4 = ptpt;
	*pt4++ = value;
	*pt4++ = pid;
	*pt4++ = tid;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}


void rdecl
mt_trace_irq_entry(unsigned irq) {
	if (!mt_filters.irq) return;

	mt_TRACE_FUNK_top(20, 4);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	uint32_t	*pt4;

	pt4 = pt0;
	*pt4++ = irq;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void rdecl
mt_trace_irq_exit(unsigned irq) {
	if (!mt_filters.irq) return;

	mt_TRACE_FUNK_top(21, 4);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	uint32_t	*pt4;

	pt4 = pt0;
	*pt4++ = irq;
	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void rdecl
mt_trace_irq_handler_entry(unsigned irq) {
	if (!mt_filters.irq) return;

	mt_TRACE_FUNK_top(22, 4);	/* parameters are mt_TRACE_FUNK_top(id, size) */

	uint32_t	*pt4;

	pt4 = pt0;
	*pt4++ = irq;

	pt0 = pt4;
	mt_TRACE_FUNK_bottom;
}
void rdecl
mt_trace_irq_handler_exit(unsigned irq) {
	if (!mt_filters.irq) return;

	if (irq) /* irq timer = 0 and we don't want it now */
	{
		mt_TRACE_FUNK_top(23, 4);	/* parameters are mt_TRACE_FUNK_top(id, size) */

		uint32_t	*pt4;

		pt4 = pt0;
		*pt4++ = irq;
		pt0 = pt4;
		mt_TRACE_FUNK_bottom;
	}
}
void rdecl
mt_trace_irq_timer_entry() {
	if (!mt_filters.irq) return;

	mt_TRACE_FUNK_top(24, 0);	/* parameters are mt_TRACE_FUNK_top(id, size) */
	mt_TRACE_FUNK_bottom;
}
void rdecl
mt_trace_irq_timer_exit() {
	if (!mt_filters.irq) return;

	mt_TRACE_FUNK_top(25, 0);	/* parameters are mt_TRACE_FUNK_top(id, size) */
	mt_TRACE_FUNK_bottom;
}


#if 1
void mt_trace_var_debug(unsigned v1, unsigned v2, unsigned *p) {

	if (!mt_filters.debug) return;

	mt_TRACE_FUNK_top(0xFFFE, 12);

	uint32_t	*pt4, **ptpt;

	pt4 = pt0;

	*pt4++ = v1;
	*pt4++ = v2;
	ptpt = pt4;
	*ptpt++ = p;

	pt0 = ptpt;

	mt_TRACE_FUNK_bottom;
}
#endif

void mt_trace_debug(const char *func, unsigned line, const char *txt) {
	if (!mt_filters.debug) return;

	mt_TRACE_FUNK_top(0xFFFF, (strlen(func) + strlen(txt) + 6));

	char		*pt1;
	uint32_t	*pt4;

	pt1 = pt0;

	strcpy(pt1, func); pt1 += strlen(func) + 1;
	pt4 = (uint32_t *) pt1;
	*pt4++ = line;
	pt1 = (char *) pt4;
	strcpy(pt1, txt); pt1 += strlen(txt) + 1;

	pt0 = pt1;

	mt_TRACE_FUNK_bottom;
}
