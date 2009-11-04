/*
 * services/mt_logger/mt_logger.c
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

#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/trace.h>
#include <sys/mman.h>
#include <sys/mt_trace.h>

#include "mt_logger.h"

mt_data_ctrl_t		*mt_buf;
char 				*fname = "kernel_0";
int 				fic = -1;

#if 1	/* stats ! */
unsigned full_notified = 0;
unsigned full_logged = 0;
unsigned other_logged = 0;
unsigned no_log_needed = 0;
unsigned min_interval = 0xFFFFFFFF;
unsigned last_time = 0;
size_t	total_size = 0;
#endif

/* this is a thread that caches signals and handles them */
void * signal_catcher_thread( void *arg ) {
	sigset_t set;
	siginfo_t sinfo;

	sigemptyset( &set );
	sigaddset( &set, SIGINT );
	sigaddset( &set, SIGTERM );
	sigaddset( &set, SIGHUP );
	sigaddset( &set, SIGABRT );
	sigaddset( &set, SIGALRM );
	sigaddset( &set, SIGUSR1 );

	while( 1 )
	{
		if ( sigwaitinfo( &set, &sinfo ) != -1 )
		{
			//info( "Caught signal %d\n", sinfo.si_signo );
			if (sinfo.si_signo == SIGALRM)
			{
				printf("quit\n");
				break;
			}
		}
	}
	return NULL;
}

/* This thread is catching pulses (flush, term) */
void * pulse_catcher_thread( void *arg ) {

	typedef union
	{
		struct _pulse   pulse;
	} my_message_t;

	/* Create the pulse channel */
	mtctl_initflush_data_t pulseData;
	int chid = ChannelCreate(0);
	pulseData.channel = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);

	/* Init the pulse channel */
	MtCtl(_MT_CTL_INIT_FLUSH_PULSE, &pulseData);

	my_message_t msg;
	while (1)
	{
		if (MsgReceivePulse(chid, &msg, sizeof(msg), NULL) == 0)
		{
			if (_MT_FLUSH_PULSE_CODE == msg.pulse.code)
			{
				full_notified++;
				mt_new_trace_to_file(0);
			}
		}
	}

	return NULL;
}

/* old single-traceset function that writes traceset to file */
void * mt_trace_to_file(int sig, siginfo_t *extra, void *cruft) {

	void						*pt0_tsc, *pt0_tsb;
	mt_data_ctrl_t				*ptdc;
	ltt_subbuffer_header_t		*pthd;
	size_t						ts_size;

	ptdc = mt_buf;

	/* get the size of the traceset (= diff(trace_begin, trace_current) ) */
	pt0_tsc = ptdc->data_current;
	pt0_tsb = ptdc->buf_begin;
	ts_size = (pt0_tsc - pt0_tsb);

	pthd = (void *) (ptdc + 1);	/* header is next to data controll */
	pthd->buf_size = ts_size;
	printf("USR: size is '%u'\n", ts_size);
	write (fic, pthd, ts_size);

	return NULL;

}
/* multi-tracesets handeling: finds a full one, copies to file, and resets it */
void * mt_new_trace_to_file(int ts_num) {

	uint8_t						*pt1_tsb, *pt1_tse;
	uint8_t						*pt1;
	mt_data_ctrl_t				*ptdc;
	ltt_subbuffer_header_t		*pthd;
	size_t						ts_size;
	int							i;

	ptdc = mt_buf;

	if (ts_num) {
		/* if ts_num is non 0, we want to collect the specific traceset number 'ts_num'
		 * even if non-full (typically at the end to get last traces),
		 * it must be non-empty though!
		 */
		ptdc += (ts_num - 1);	/* go to the 'ts_num'th traceset (data_ctrl structure) */
		if (ptdc->status == -1)	/* desired traceset is empty, don't log */
			return NULL;
		else
			++other_logged;
	} else {
		/* check for a full traceset */
		/* incrementing controll structues until we find a full traceset */
		for (i = 0; i < _MT_TRACESETS_PER_CPU; ++i) {
			if (ptdc->status > 0)
				break;
			else
				++ptdc;
		}
		if (i == _MT_TRACESETS_PER_CPU) {
			/* no traceset is full */
			++no_log_needed;
			return NULL;
		} else
			++full_logged;
	}

	/* get the size of the traceset (= diff(trace_begin, trace_current) ) */
	pt1_tse = ptdc->data_current;
	pt1_tsb = ptdc->buf_begin;
	ts_size = (pt1_tse - pt1_tsb);

	/* get to the traceset, we are still pointing at the control sturcture */
	pt1 = (uint8_t *) (ptdc + (ptdc->buf_remaining + 1));	/* at the end of all data control structures */
	pt1 += (_MT_TRACESET_SIZE * (_MT_TRACESETS_PER_CPU - 1 - ptdc->buf_remaining));	/* skipping other trace sets */

	pthd = (ltt_subbuffer_header_t*) pt1;
	pthd->buf_size = ts_size;
	write(fic, (void *) pthd, ts_size);

	/* reset traceset */
	ptdc->status = -1;

	total_size += ts_size;

	return NULL;

}

int main(int argc, char *argv[]) /*int main()*/
{
	pthread_t sig_thr;
	unsigned runtime = 8;
	int hPulseThread;

	printf("\nUSR Filename is : '%s'\n", fname);

	/* file open */
	if ((fic = open(fname, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR
			| S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1)
	{
		printf("USR Cannot open file '%s'.\n", fname);
		exit(1);
	}

	int c = getopt(argc, argv, "ds:");
	int filter = 1;
	switch (c)
	{
	case 'd':
		filter &= 0x1;
		break;
	case 's':
		runtime = atoi(optarg);
		printf("USR: setting run time to %i\n", runtime);
		break;
	}

	/* Create and start the pulse listener thread */
	if (pthread_create(&hPulseThread, NULL, pulse_catcher_thread, NULL))
	{
		perror("Creating pulse catcher thread");
		close(fic);
		return -1;
	}
	pthread_setschedprio(hPulseThread, 50);

	paddr_t shared;
	mtctl_inittracelogger_data_t traceData;
	traceData.filter = filter;
	traceData.shared_memory = &shared;

	/* Allocate memory and start tracing */
	MtCtl(_MT_CTL_INIT_TRACELOGGER, &traceData);

	mt_buf = mmap((void *) (unsigned) 0, _MT_ALLOC_SIZE, PROT_READ
			| PROT_WRITE, (MAP_SHARED | MAP_PHYS), NOFD, shared);
	if (mt_buf == MAP_FAILED)
	{
		perror("Memory mapping failed");
		MtCtl(_MT_CTL_TERMINATE_TRACELOGGER, NULL);
		close(fic);
		return -1;
	}

	if (pthread_create(&sig_thr, NULL, signal_catcher_thread, NULL) == -1)
	{
		perror("Creating signal catcher thread");
		MtCtl(_MT_CTL_TERMINATE_TRACELOGGER, NULL);
		munmap(mt_buf, _MT_ALLOC_SIZE);
		close(fic);
		return -1;
	} /* end of QNX code */
	pthread_setschedprio(sig_thr, 50);

	/* set program end delay */
	alarm(runtime); // Warning: this should be moved before pthread_create()

	/* Boost the current thread priority */
	pthread_setschedprio(pthread_self(), 50);

	pthread_join(sig_thr, NULL);
	printf(" going to quit\n");

	//mt_trace_to_file(0, NULL, NULL);
	mt_new_trace_to_file(1);
	mt_new_trace_to_file(2);

	/* unmap / deallocate memory */
	munmap(mt_buf, _MT_ALLOC_SIZE);
	MtCtl(_MT_CTL_TERMINATE_TRACELOGGER, NULL);

	/* file close */
	close(fic);

	printf("*** stats : ***\n");
	if (full_notified)
		printf("\tmin_interval was  %12u [ns]\n", (unsigned) (min_interval / 3));
	printf("\ttotal_size            = %6u [bytes]\n", total_size);
	printf("tracesets:\n");
	printf("\tmt_TRACESET_SIZE      = %6u [bytes]\n", _MT_TRACESET_SIZE);
	printf("\ttraceset writing size = %6u [bytes]\n", (unsigned) (_MT_TRACESET_SIZE * _MT_BUFFER_FULL));
	printf("\tfull_notified = %3u\n", full_notified);
	printf("\tfull_logged   = %3u\n", full_logged);
	printf("\tother_logged  = %3u\n", other_logged);
	printf("\tno_log_needed = %3u\n", no_log_needed);

	printf("USR end\n\n");
	return 0;
}



