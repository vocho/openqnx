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
 * Tracelogger
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <sys/trace.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/kercalls.h>
#include <sys/siginfo.h>
#include <time.h>
#include <unistd.h>

#include "kevfile.h"
#include "utils.h"
#include "kernel_if.h"

#define DEFAULT_ITERATIONS	32
#define DEFAULT_KERNEL_BUFS	32
#define DEFAULT_USER_BUFS 	64
#define DEFAULT_PATH		"/dev/shmem/tracebuffer.kev"

/* ugh, globals */
static int daemon_mode = 0;
static int ring_mode = 0;
static int persist_kbuffers = 0;
static int reuse_kbuffers = 0;
static paddr_t kernel_buffers_paddr = 0;
static int max_iterations = -1; /* starts off at invalid value */
static int duration = -1;
static int timer_started = 0;
static struct sigevent finished_ev;
static struct sigevent filled_ev;
static int coid;
static kevfile_t *kev;
unsigned	verify = 0;	/* PDB */
pid_t		pdb_pid = 3;	/* PDB */

extern int verbosity;

#define TRACE_MK_INDEX(i) ((i) << 13)

#define TRACE_PULSE_FINISHED _PULSE_CODE_MINAVAIL
#define TRACE_PULSE_FILLED _PULSE_CODE_MINAVAIL+1

static inline void *tracecpy(void *dst, const void *src, size_t nbytes) {
	void			*ret = dst;
	unsigned		n;

	/* Both addresses must be aligned to stuff in int size chunks */
	if(		nbytes >= sizeof(unsigned) &&
			((uintptr_t)src & (sizeof(unsigned) - 1)) == 0 &&
			((uintptr_t)dst & (sizeof(unsigned) - 1)) == 0) {
		unsigned			*d = (unsigned *)dst;
		unsigned			*s = (unsigned *)src;


		n = ((nbytes>>2) + 15)/16;

		switch ((nbytes>>2)%16){
		case 0:        do {  *d++ = *s++;
		case 15:             *d++ = *s++;
		case 14:             *d++ = *s++;
		case 13:             *d++ = *s++;
		case 12:             *d++ = *s++;
		case 11:             *d++ = *s++;
		case 10:             *d++ = *s++;
		case 9:              *d++ = *s++;
		case 8:              *d++ = *s++;
		case 7:              *d++ = *s++;
		case 6:              *d++ = *s++;
		case 5:              *d++ = *s++;
		case 4:              *d++ = *s++;
		case 3:              *d++ = *s++;
		case 2:              *d++ = *s++;
		case 1:              *d++ = *s++;
   	                   } while (--n > 0);
		}

		// remainder
		nbytes = nbytes & 0x3;


		if(nbytes) {
			dst = (unsigned char *)d;
			src = (const unsigned char *)s;
		}
	}

	/* Get the unaligned bytes, or the remaining bytes */
	while(nbytes) {
		*(unsigned char *)dst = *(const unsigned char *)src;
		dst = (char *)dst + 1;
		src = (const char *)src + 1;
		--nbytes;
	}

	return ret;
}

const struct sigevent *dump_buf( int off, tracebuf_t *bufp, int force )
{
	kevfile_buf_t	*kbuf;
	unsigned		index, nbytes, len, iterations_left;

	InterruptLock( &kev->buf_spin );
	nbytes = kevfile_space_left(kev);
	iterations_left = max_iterations - kev->buffers_logged;
	InterruptUnlock( &kev->buf_spin );

	index = _TRACE_GET_BUFFNUM(off);

	bufp = &kernel_buffers_vaddr[index];
	len = bufp->h.num_events * sizeof(struct traceevent);

	nbytes = min(nbytes,len);
	/* try to log a buffer IFF
	 * 	we have space left in the output file
	 * 	we have no limit on iterations OR we have iterations left OR we are being forced to log despite iteration limits
	 * */
	if ( (nbytes >= sizeof(traceevent_t)) && (max_iterations == 0 || iterations_left > 0 || force ) ) {

		kbuf = kevfile_buffer_get( kev );
		if ( kbuf == NULL ) {
			/* yes, we have no bananas, we have no bananas, today! :v( */
			InterruptLock( &bufp->h.spin );
			bufp->h.tail_ptr = bufp->h.begin_ptr;
			bufp->h.num_events = 0;
			bufp->h.flags = 0;
			InterruptUnlock( &bufp->h.spin );

			kev->buffers_dropped++;
			return NULL;
		}

		InterruptLock( &bufp->h.spin );
		bufp->h.flags |= _TRACE_FLAGS_WRITING;
		InterruptUnlock( &bufp->h.spin );

		tracecpy( &kbuf->data[0], bufp->data, nbytes );

		kbuf->nbytes = nbytes;
		kbuf->off = off;
		kev->buffers_logged++;

		InterruptLock( &bufp->h.spin );
		bufp->h.tail_ptr = bufp->h.begin_ptr;
		bufp->h.num_events = 0;
		bufp->h.flags = 0;
		InterruptUnlock( &bufp->h.spin );

		kevfile_buffer_put( kev, kbuf );

		/* if we logged a buffer then we might now be done */
		if ( kevfile_space_left( kev ) < sizeof(traceevent_t) || (!force && max_iterations != 0 && iterations_left <= 1) ) {
			finished_ev.sigev_value.sival_int = off;
			return &finished_ev;
		}
	}
	else {
		finished_ev.sigev_value.sival_int = off;
		return &finished_ev;
	}
	if ( !(kev->flags & KEVFILE_MAP) || (duration != -1 && !timer_started)) {
		filled_ev.sigev_value.sival_int = off;
		return &filled_ev;
	}
	return NULL;
}

const struct sigevent *hookfunc( int off )
{
	tracebuf_t		*bufp;
	unsigned		index;
	++verify;	/* PDB */
	pdb_pid = getpid();	/* PDB */
	index = _TRACE_GET_BUFFNUM(off);
	bufp = &kernel_buffers_vaddr[index];
	if ( bufp->h.flags & _TRACE_FLAGS_RING ) {
		ring_mode = 1;
		filled_ev.sigev_value.sival_int = off;
		return &filled_ev;
	} else {
		ring_mode = 0;
	}
	return dump_buf( off, bufp, 0 );
}

void *signal_catcher_thread( void *arg )
{
	sigset_t set;
	siginfo_t sinfo;
	unsigned priority = (unsigned)arg;
	//printf("PDBug: in signal_catcher_thread\n");	/* PDB */

	sigemptyset( &set );
	sigaddset( &set, SIGINT );
	sigaddset( &set, SIGTERM );
	sigaddset( &set, SIGHUP );
	sigaddset( &set, SIGABRT );

	while( 1 ) {
		if ( sigwaitinfo( &set, &sinfo ) != -1 ) {
			info( "Caught signal %d\n", sinfo.si_signo );
			TraceEvent( _NTO_TRACE_STOP );
			MsgSendPulse( coid, priority, TRACE_PULSE_FINISHED, 0 );
		}
	}
	return NULL;
}

static void add_argument_attribute(int argc, char **argv) {
	char *key = "TRACELOGGER_ARGS=";
	char *buffer, *insert;
	int len, i;

	len = strlen(key);

	for(i = 0; i < argc; i++) {
		// +1 for the space or for the trailing null
		len += strlen(argv[i]) + 1;
	}

	buffer = alloca(len);
	if(buffer == NULL) {
		return;
	}

	insert = buffer;
	strcpy(insert, key);
	insert += strlen(key);

	for(i = 0; i < argc; i++) {
		int c;
		for(c = 0; argv[i][c] != '\0'; c++) {
			*insert++ = argv[i][c];
		}
		if(i < argc - 1 ) {
			*insert++ = ' ';
		} else {
			*insert++ = '\0';
		}
	}

	add_user_attribute(buffer);
}

int main(int argc, char *argv[])
{
	int timer_id, interrupt_id;
	int wide_mode = 0;
	struct itimerspec timeout;
	int chid;
	int finished = 0;
	int disable_kercalls = 0 ; /* consider flags for this */
	int disable_interrupt = 0 ;
	int disable_process = 0 ;
	int disable_thread = 0 ;
	int disable_vthread = 0 ;
	int disable_comm = 0 ;
	int disable_ring0 = 0 ;
	int disable_system = 0;
	int	add_events = 0;
	char *output_filename = DEFAULT_PATH;
	unsigned max_filesize = 0;
	unsigned num_buffers = DEFAULT_USER_BUFS;
	unsigned num_kbuffers = DEFAULT_KERNEL_BUFS;
	int direct_map = 0;
	int c;
	pthread_attr_t pattr;
	struct sched_param param;

	printf("PDB: verify begin = %u\n", verify);	/* PDB */

	add_argument_attribute(argc, argv);

	/* option processing */
	while ( (c = getopt(argc, argv, "wd:rvk:b:f:S:F:s:n:McA:EPR")) != -1) {
		switch(c) {
		case 'E':
			add_events = 1;
			break;
		case 'A':
			add_user_attribute(optarg);
			break;
		case 'M':
			direct_map = 1;
			break;
		case 'P':
			persist_kbuffers = 1;
			break;
		case 'R':
			reuse_kbuffers = 1;
			break;
		case 'b':
			num_buffers = atoi(optarg);
			break;
		case 'w':
			wide_mode = 1;
			break;
		case 'd':
			daemon_mode = 1;
			break;
		case 'c':
			max_iterations = 0;
			break;
		case 'n':
			max_iterations = atoi(optarg);
			break;
		case 'r':
			ring_mode = 1;
			break;
		case 'v':
			verbosity++;
			break;
		case 'k':
			num_kbuffers = atoi(optarg);
			break;
		case 's':
			duration = atoi(optarg);
			if ( max_iterations < 0 ) { /* still at invalid value? */
				max_iterations = 0; /* then select unlimited iterations */
			}
			break;
		case 'S':
			max_filesize = atoi(optarg); /* rush - should handle k and M suffix */
			if ( strchr( optarg, 'k' ) || strchr( optarg, 'K' )) {
				max_filesize *= 1024;
			} else if ( strchr( optarg, 'm' ) || strchr( optarg, 'M' )) {
				max_filesize *= 1024 * 1024;
			}
			break;
		case 'f':
			output_filename = strdup(optarg);
			break;
		case 'F':
		{
			int type = atoi( optarg ) ;

			switch( type )
			{
				case 1 :
					disable_kercalls = 1 ;
					break ;
				case 2 :
					disable_interrupt = 1 ;
					break ;
				case 3 :
					disable_process = 1 ;
					break ;
				case 4 :
					disable_thread = 1 ;
					break ;
				case 5 :
					disable_vthread = 1 ;
					break ;
				case 6 :
					disable_comm = 1 ;
					break ;
				case 7 :
					disable_system = 1;
					break;
				case 11 :
					disable_ring0 = 1 ;
					break ;
			}
		}
		break ;
		case '?':
		default:
			return EXIT_FAILURE;
		}
	}

	if ( max_iterations < 0 ) {
		max_iterations = DEFAULT_ITERATIONS;
	}

	hack("Cool V6 Technology Inside!\n");

	if ( direct_map && max_filesize == 0 ) {
		fprintf( stderr, "%s: direct_mapping needs max size specified\n", argv[0] );
		return -1;
	}

	/* request I/O privity  */
	if ( ThreadCtl( _NTO_TCTL_IO, 0 ) == -1 ) {
		perror("Requesting I/O Privity");
		return -1;
	}

	/* create logfile */
	/* QUESTION - this will currently happily create a shmem file anywhere.  This could be confusing! */
	kev = kevfile_open( output_filename, direct_map ? KEVFILE_MAP:0, max_filesize, num_buffers );
	if ( kev == NULL ) {
		perror( output_filename );
		return -1;
	}

	/* create channel, connection */
	chid = ChannelCreate(0);
	if ( chid == -1 ) {
		perror("Creating Channel");
		kevfile_close( kev );
		return -1;
	}
	coid = ConnectAttach( ND_LOCAL_NODE, getpid(), chid, _NTO_SIDE_CHANNEL, 0);
	if ( coid == - 1 ) {
		perror("Creating Channel");
		kevfile_close( kev );
		return -1;
	}

	/* create events */
	SIGEV_PULSE_INIT(&finished_ev, coid, -1, TRACE_PULSE_FINISHED, 0 );
	SIGEV_PULSE_INIT(&filled_ev, coid, -1, TRACE_PULSE_FILLED, 0 );

	/* maybe create this guy at higher priority to make sure he is intialized before we continue */
	pthread_attr_init( &pattr );
	pthread_attr_setinheritsched( &pattr, PTHREAD_EXPLICIT_SCHED );
	sched_getparam( 0, &param );
	param.sched_priority++;
	pthread_attr_setschedparam( &pattr, &param );
	param.sched_priority--;
	if ( pthread_create( NULL, NULL, signal_catcher_thread, (void *)param.sched_priority ) == -1 ) {
		perror("Creating signal catcher thread");
		kevfile_close( kev );
		return -1;
	}

	/* allocate kernel buffers */
	if ( kernel_attach( num_kbuffers, reuse_kbuffers, &kernel_buffers_paddr ) == -1 ) {
		perror("Attaching to Kernel Trace Interface");
		kevfile_close( kev );
		return -1;
	}

	/* attach appropriate interrupt handler */
	if ( (interrupt_id = InterruptHookTrace( hookfunc, 0 )) == -1 ) {
		perror("Attach Interrupt Handler");
		kernel_detach( kernel_buffers_paddr, persist_kbuffers );
		kevfile_close( kev );
		return -1;
	}
	/* configure filters */
	if ( !daemon_mode || add_events ) {
		info("Enabling events\n");

		TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_KERCALL );
		TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_KERCALL );

		TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_THREAD );
		TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_THREAD );

		TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_VTHREAD );
		TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_VTHREAD );

		TraceEvent( _NTO_TRACE_CLRCLASSPID, _NTO_TRACE_COMM );
		TraceEvent( _NTO_TRACE_CLRCLASSTID, _NTO_TRACE_COMM );

		TraceEvent( _NTO_TRACE_ADDALLCLASSES ); // PDB: this is responsible for enabling all traces including IRQ

		if( disable_kercalls )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_KERCALL ) ;
		if( disable_interrupt )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_INT ) ;
		if( disable_process )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_PROCESS ) ;
		if( disable_thread )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_THREAD ) ;
		if( disable_vthread )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_VTHREAD ) ;
		if( disable_comm )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_COMM ) ;
		if( disable_ring0 )
			TraceEvent( _NTO_TRACE_DELEVENT, _NTO_TRACE_KERCALL, __KER_RING0 ) ;
		if( disable_system )
			TraceEvent( _NTO_TRACE_DELCLASS, _NTO_TRACE_SYSTEM ) ;

		if ( wide_mode ) {
			TraceEvent( _NTO_TRACE_SETALLCLASSESWIDE );
		} else {
			TraceEvent( _NTO_TRACE_SETALLCLASSESFAST );
		}
	}
	if ( ring_mode ) {
		TraceEvent( _NTO_TRACE_SETRINGMODE );
	} else {
		TraceEvent( _NTO_TRACE_SETLINEARMODE );
	}

	/* setup timer */
	if ( duration != -1 ) {
		timeout.it_value.tv_sec = duration;
		timeout.it_value.tv_nsec = 0;
		timeout.it_interval.tv_sec = 0;
		timeout.it_interval.tv_nsec = 0;

		if ( timer_create( CLOCK_REALTIME, &finished_ev, &timer_id ) == -1 ) {
			perror("Creating Timer");
			kernel_detach(kernel_buffers_paddr, persist_kbuffers);
			kevfile_close( kev );
			return -1;
		}
		info("Created timer for %d seconds\n", duration );
		if ( !daemon_mode ) {
			if ( timer_settime( timer_id, 0, &timeout, NULL ) == -1 ) {
				perror("Starting Timer");
				kernel_detach(kernel_buffers_paddr, persist_kbuffers);
				kevfile_close( kev );
				return -1;
			}
			timer_started = 1;
			info("Set timer for %d seconds\n", duration );
		}
	}
	/* optionally start logging */
	if ( !daemon_mode ) {
		TraceEvent( _NTO_TRACE_START );
	}

	/* wait for events, respond appropriately */
	while(!finished) {
		struct _pulse pulse;
		int ret;
		ret = MsgReceivePulse( chid, &pulse, sizeof(pulse), NULL );
		if ( ret == -1 ) {
			if ( errno == EINTR ) {
				continue;
			}
			finished = 1;
			break;
		}
		switch ( pulse.code ) {
		case TRACE_PULSE_FINISHED:
			TraceEvent( _NTO_TRACE_STOP );
			debug("finished\n");
			if ( !ring_mode ) {
				finished = 1;
			}
			break;
		case TRACE_PULSE_FILLED:
			if ( ring_mode ) {
				int i;
				for ( i = (_TRACE_GET_BUFFNUM(pulse.value.sival_int) + 1) % num_kernel_buffers; ; i = ((i+1)%num_kernel_buffers) ) {
					if ( kernel_buffers_vaddr[i].h.num_events > 0 ) {
						hack("buffer %d: %d events\n", i, kernel_buffers_vaddr[i].h.num_events );
						dump_buf( TRACE_MK_INDEX(i), &kernel_buffers_vaddr[i], 1 );
						if ( kevfile_needs_flush( kev ) ) {
							if ( kevfile_flush(kev) == -1 ) {
								finished = 1;
								perror("writing");
								break;
							}
						}
					}
					/* we've wrapped around to the beginning, time to bail out */
					if ( i == _TRACE_GET_BUFFNUM(pulse.value.sival_int) ) {
						break;
					}
				}
				finished = 1;
			} else {
				if ( duration != -1 && !timer_started ) {
					timer_settime( timer_id, 0, &timeout, NULL );
					timer_started = 1;
					info("Set timer for %d seconds\n", duration );
				}
				while ( kevfile_needs_flush( kev ) ) {
					if ( kevfile_flush( kev ) == -1 ) {
						finished = 1;
						break;
					}
				}
			}
			break;
		default:
			debug("unknown trace pulse code %x\n", pulse.code );
			break;
		}
	}
	TraceEvent( _NTO_TRACE_STOP );
	InterruptDetach( interrupt_id );

	kevfile_flush( kev );
	info("logged %d bytes\n", kev->current_offset );
	info("logged %d buffers\n", kev->buffers_logged );
	if ( kev->buffers_dropped ) {
		info("warning, dropped %d kernel buffers, -b %d wasn't enough\n", kev->buffers_dropped, kev->num_buffers );
	}
	else if ( kev->max_write_depth > 0 ) {
		info("maximum write depth of tracelogger buffers was %d (ie minimum -b option value)\n", kev->max_write_depth );
	}
	kevfile_close( kev );

	/* optionally reset filters */
	if ( !daemon_mode || add_events ) {
		info("Disabling events\n");
		TraceEvent( _NTO_TRACE_DELALLCLASSES );
	}
	/* release kernel buffers */
	kernel_detach( kernel_buffers_paddr, persist_kbuffers );
	printf("PDB: pid = %u\n", pdb_pid);	/* PDB */
	printf("PDB: verify end = %u\n", verify);	/* PDB */
	return 0;
}

__SRCVERSION("tracelogger.c $Rev: 157117 $");
