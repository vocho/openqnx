/*
 * services/system/public/sys/mt_trace.h

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

#ifndef __mt_trace_h__
#define __mt_trace_h__

#include <sys/types.h>
#include <stdint.h>

#define _MT_FLUSH_PULSE_CODE				97

#define _MT_CTL_INIT_FLUSH_PULSE			0x01
#define _MT_CTL_INIT_TRACELOGGER			0x02
#define _MT_CTL_TERMINATE_TRACELOGGER		0x03
#define _MT_CTL_DUMMY						0xFF

#define _MT_BUFFER_FULL						0.9	/* relative value, ex: 0.7 --> stop filling at 70 % capacity*/
#define _MT_TRACESETS_PER_CPU				2 /* !!! ALERT: only 2 are supported yet !!! */
#define _MT_TRACESET_SIZE					(256 * 1024)
#define _MT_ALLOC_SIZE						(sizeof(mt_data_ctrl_t) + _MT_TRACESET_SIZE) * _MT_TRACESETS_PER_CPU

/* mt_TRACESET_SIZE is (obviously) the size of ONE traceset.
 * There are as many tracesets as mt_TRACESETS_PER_CPU number of CUPs.
 * A traceset contains a "ltt_subbuffer_header" and traces (first one with id 29).
 * We allocate for each traceset a data_ctrl structure to manage data inside the traceset.
 *
 * A ltt_subbuffer_header is 68 B.
 * A trace is 4 + trace_size (0 to 16 till now) B
 * Header26 adds 16 B
 * Header31 adds  4 B
 *
 * num_traces = ( mt_TRACESET_SIZE - (ltt_subbuffer_header = 68) - (header16 = 16) ) / trace_average
 * mt_TRACESET_SIZE = (ltt_subbuffer_header = 68) + (header16 = 16) + num_traces * trace_average
 *
 * The trace buffer containes first all the data Scontrol structures, and next the tracesets.
 * Tracesets are in the same order as their corresponding data control structures.
 */

typedef struct mtctl_initflush_data
{
    int channel;
} mtctl_initflush_data_t;

typedef struct mtctl_inittracelogger_data
{
    int filter;
    paddr_t* shared_memory;
} mtctl_inittracelogger_data_t;

typedef struct mt_data_ctrl
{
#if 0
	uint8_t			cpu_id;
	uint8_t			block_num;		/* typically 0 or 1, if we have 2 buffers, one filling, one logging */
#endif
	int				status;		/* -1 = empty, 0 = filling, 1 = full, 2 = overfull (last byte reached) */
	unsigned		buf_remaining;	/* how many structured are following right next before raw data */
	void			*buf_begin;
	void			*data_current;
	void			*buf_lim;
} mt_data_ctrl_t;

// from LTTng v0.82: include/linux/ltt-tracer.h :270 (see LTTng licence agreement)
/*
 * We use asm/timex.h : cpu_khz/HZ variable in here : we might have to deal
 * specifically with CPU frequency scaling someday, so using an interpolation
 * between the start and end of buffer values is not flexible enough. Using an
 * immediate frequency value permits to calculate directly the times for parts
 * of a buffer that would be before a frequency change.
 *
 * Keep the natural field alignment for _each field_ within this structure if
 * you ever add/remove a field from this header. Packed attribute is not used
 * because gcc generates poor code on at least powerpc and mips. Don't ever
 * let gcc add padding between the structure elements.
 */
typedef struct ltt_subbuffer_header
{
        uint64_t cycle_count_begin;     /* Cycle count at subbuffer start */
        uint64_t cycle_count_end;       /* Cycle count at subbuffer end */
        uint32_t magic_number;          /*
                                         * Trace magic number.
                                         * contains endianness information.
                                         */
        uint8_t major_version;
        uint8_t minor_version;
        uint8_t arch_size;              /* Architecture pointer size */
        uint8_t alignment;              /* LTT data alignment */
        uint64_t start_time_sec;        /* NTP-corrected start time */
        uint64_t start_time_usec;
        uint64_t start_freq;            /*
                                         * Frequency at trace start,
                                         * used all along the trace.
                                         */
        uint32_t freq_scale;            /* Frequency scaling (divisor) */
        uint32_t lost_size;             /* Size unused at end of subbuffer */
        uint32_t buf_size;              /* Size of this subbuffer */
        uint32_t events_lost;           /*
                                         * Events lost in this subbuffer since
                                         * the beginning of the trace.
                                         * (may overflow)
                                         */
        uint32_t subbuf_corrupt;        /*
                                         * Corrupted (lost) subbuffers since
                                         * the begginig of the trace.
                                         * (may overflow)
                                         */
        uint8_t header_end[0];          /* End of header */
} ltt_subbuffer_header_t;

#endif /* __mt_trace_h__ */
