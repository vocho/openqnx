/*
 * services/system/public/sys/montt_trace.h
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syspage.h>
#include <sys/trace.h>
#include <sys/kercalls.h>
#include <sys/neutrino.h>
#include <sys/montt_trace.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define _WRITE_STR(fileid, arg)	write(fileid, arg, sizeof(arg))
#define _WRITE_VAR(fileid, arg)	write(fileid, &(arg), sizeof(arg))

#define montt_SHMEM_SIZE \
	sizeof(montt_shared_info_st) \
	+ montt_TRACESETS_PER_CPU \
		* (sizeof(montt_data_ctrl_st) + montt_TACESET_SIZE) \
		*cpus

int 					fic[8] = -1;
unsigned char			cpus;
montt_data_ctrl_st		*datctl;
void					*delta;

extern void montt_init_thread_stuff();


void montt_trace_set_to_file(montt_data_ctrl_st *dc) {

	montt_data_ctrl_st	*ptdc;
	ltt_subbuffer_header_t	*pthd;
	void				*pt0;
	uint8_t				*pt1;
	uint16_t			*pt2;
	uint32_t			*pt4;
	uint64_t			*pt8;
	int					i;

	ptdc = dc;

	pt0 = (void *) (ptdc + dc->buf_remaining + 1);
	/* we are now supposed to point at the end of the structures, begin of binary tracesets */
	/* we still need to jump to the correct set */
	pt0 += montt_TRACESET_SIZE * dc->buf_remaining;	/* increment of ... bytes */
	/* here we are :-) */

	/* get the size between dc->data_current and dc->buf_begin */
	//...

	/* set the size of che buf_header */
	pthd = pt0;
	//pthd->buf_size = size;

	/* write the data to file */
	//write(fic[dc->cpu_id], pt0, size);

	/* reset the traceset */
	dc->data_current = dc->buf_begin;
	dc->status = -1;

	return;
}

void * montt_trace_to_file(int sig, siginfo_t *extra, void *cruft) {

	montt_data_ctrl_st	*dc;
	dc= datctl;
	int					i;

	/* start checking */
	if (dc->buf_remaining + 1 != cpus * montt_TRACESETS_PER_CPU)
		printf("error unmatching number of data_ctrl structure\n");
	for (i = dc->buf_remaining; i >= 0; --i) {
		if (dc->buf_remaining != i)
			printf("error unmatching id of data_ctrl structure\n");
		/* other checks to do
		 * ptr_beg++ == ptr_end
		 * ptr_end - prt_beg == cte
		 */


		if (dc->status > 0) {
			/* we must log this ! */

			montt_traceset_to_file(dc);
		}
		++dc;
	}

	return;
}


int _main(void) {

	paddr_t 				montt_paddr = 0;
	montt_shared_info_st	*info;
	int						i = 0;
	char					c[2];
	char					name[8] = "kernel_n";

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = montt_trace_to_file;
	if (sigaction(SIGRTMIN, &sa, NULL) < 0)
	    perror("sigaction");




	/* allocate buffers and get adress */
	TraceEvent(_montt_TRACE_ALLOCBUFFER, &montt_paddr, getpid());
	//printf("PDBug USR: pid = %u\n", getpid());

	info = mmap(
		(void *)(unsigned)0, sizeof(montt_shared_info_st),
		PROT_READ|PROT_WRITE,
		(MAP_SHARED|MAP_PHYS),
		NOFD,
		montt_paddr
	);
	if(montt_buf == MAP_FAILED) {
		return -1;
	}

	cpus = info->num_cpus;
	if (cpus < 1 || cpus > 8)
		printf("error: bad number of cpus (max 8) : %u\n", cpus);
	if (info->usr_pid != getpid())
		printf("ERROR: wrong pid!\n");

	/* file(s) open */
	for (i = 0; i < cpus; ++i) {
		sprintf(c, "%d", i);
		name[7]= c[0];
		if((fic[i] = open(name, O_WRONLY | O_APPEND | O_CREAT,
				S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH)) == -1) {
			printf("USR Cannot open file '%s'.\n", fname);
			exit(1);
		}
	}

	munmap(info, sizeof(montt_shared_info_st));
	info = mmap(
			(void *)(unsigned)0, montt_SHMEM_SIZE,
			PROT_READ|PROT_WRITE,
			(MAP_SHARED|MAP_PHYS),
			NOFD,
			montt_paddr
		);
		if(montt_buf == MAP_FAILED) {
			return -1;
		}

	datctl = info + 1;
	dalta = dc->buf_begin - (dc + cpus * montt_TRACESETS_PER_CPU);	/* */

	// register handler and wait...


	/* unmap / deallocate memory */
	munmap( montt_buf, montt_SHMEM_SIZE );

	TraceEvent( _montt_TRACE_DEALLOCBUFFER );

	/* file close */
	//_WRITE_STR(fic, "End of file\n");
	close(fic);

	return 0;
}

#endif
