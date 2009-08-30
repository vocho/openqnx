/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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

/*
 * qnx_osinfo.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <devctl.h>
#include <errno.h>
#include <sys/syspage.h>
#include <sys/procfs.h>
#include <sys/resource.h>
#include <hw/pci.h>
#include <mig4nto.h>

#define BUFF_SIZE 		1024	/* confstr buffer size.                     */
#define NSEC_TO_USEC    1000    /* Convert nsecs to usecs.                  */
#define VERSION_MULT    100     /* Multiply major number by this value.     */

static int get_total_mem(void);
static int get_free_mem(void);
static short unsigned get_tick_size(void);

/*
 *  The fields in the osdata structure are set to the following values:
 *                      
 *  info->tick_size     The current ticksize or resolution of the 
 *                      realtime clock in usecs.
 *  info->version       QNX 6.2.0 reports a version of 602, where QNX 4.25 
 *                      reported 425.
 *  info->sflags        Bitfield containing:        
 *                      _PSF_PROTECTED      Running in protected mode.
 *                      _PSF_NDP_INSTALLED  An fpu is installed.
 *                      _PSF_EMULATOR_INSTALLED An fpu emulator is installed
 *                      _PSF_PCI_BIOS       A PCI BIOS is present
 *                      _PSF_32BIT_KERNEL   32-bit kernel is being used.
 *
 *                      These flags are not supported:
 *                      _PSF_EMU16_INSTALLED    16-bit 80x87 emulator running
 *                      _PSF_EMU32_INSTALLED    32-bit 80x87 emulator running
 *                      _PSF_32BIT          Proc32 is running.
 *                      _PSF_APM_INSTALLED  Advanced Power Management
 *                      _PSF_RESERVE_DOS    
 *                      _PSF_RESERVE_DOS32  Lower 640K reserved for DOS.
 *  info->nodename      QNX 4 nid retrieved from the mig4nto Name Resource
 *                      Manager.
 *  info->cpu           Processor type 486,586,...
 *  info->machine       Name of this machine on the network.
 *  info->totpmem       Total physical memory.
 *  info->freepmem      Free physical memory.
 *  info->totmemk       Total memory in Kb, up to USHORT_MAX (65535).
 *  info->freememk      Free memory in Kb, up to USHORT_MAX (65535).
 *  info->cpu_features  Contains CPU Speed in Mhz.
 *
 *  The remaining fields are set to MIG4NTO_UNSUPP.
 */
int
qnx_osinfo(nid_t nid, struct _osinfo *info)
{
	uint32_t    fpu;
	int         totalk;
	int         freek;
	unsigned    lastbus;
	unsigned    version;
	unsigned    hardware;
	char        buf[BUFF_SIZE];

	if (nid != 0 && nid != getnid()) {
		errno = ENOSYS;
		return -1;
	}
		
	memset(info, 0, sizeof(struct _osinfo));
	info->cpu_speed = MIG4NTO_UNSUPP; /* Different in QNX 4 and QNX Neutrino */
	info->num_procs = MIG4NTO_UNSUPP; /* creates until it's out of memory */
	info->tick_size = get_tick_size();

	if (confstr(_CS_RELEASE, buf, sizeof(buf)) != 0) {
		char *c;

		info->version = atoi(buf) * VERSION_MULT;
		if ((c = strchr(buf, '.')) && *(c++))
			info->version += atoi(c);
	}

	info->timesel = MIG4NTO_UNSUPP;	/* Time Segment */
	totalk = get_total_mem() / 1024;
	if (totalk > USHRT_MAX)
		info->totmemk = USHRT_MAX;
	else
		info->totmemk = totalk;

	freek = get_free_mem() / 1024;
	if ( USHRT_MAX < freek )    /* free mem Kb will fit */
		info->freememk = USHRT_MAX;
	else
		info->freememk = freek;

	info->primary_monitor   = MIG4NTO_UNSUPP;
	info->secondary_monitor = MIG4NTO_UNSUPP;
	info->machinesel        = MIG4NTO_UNSUPP;
	info->disksel           = MIG4NTO_UNSUPP;
	info->diskoff           = MIG4NTO_UNSUPP;
	info->ssinfo_offset     = MIG4NTO_UNSUPP;
	info->ssinfo_sel        = MIG4NTO_UNSUPP;
	info->microkernel_size  = MIG4NTO_UNSUPP; 
	info->release           = MIG4NTO_UNSUPP;

	info->sflags = _PSF_PROTECTED;	/* Running in protected mode    */
	/* Check for FPU support. */
	fpu = (CPU_FLAG_FPU & SYSPAGE_ENTRY(cpuinfo)->flags) ? 1 : 0;
	if (fpu)
		info->sflags |= _PSF_NDP_INSTALLED;		/* An fpu is installed  */
	else /* An fpu emulator is running */
		info->sflags |= _PSF_EMULATOR_INSTALLED;

	if (PCI_SUCCESS == pci_present(&lastbus, &version, &hardware))
		info->sflags |= _PSF_PCI_BIOS;			/* A PCI BIOS is present  */

	if (INT_MAX == LONG_MAX)					/* 32-bit detection     */
		info->sflags |= _PSF_32BIT_KERNEL;		/* 32-bit kernel  */

	info->nodename = getnid();
	info->cpu = SYSPAGE_ENTRY(cpuinfo)->cpu;	/* Proc type 486,586,... */

	if (confstr(_CS_HOSTNAME, buf, sizeof(buf)) != 0) {
		memcpy(info->machine, buf, sizeof(info->machine));
		info->machine[sizeof(info->machine) - 1] = '\0';
	}
	info->fpu    		= MIG4NTO_UNSUPP;
	info->bootsrc    	= MIG4NTO_UNSUPP;	/* 'F'loppy 'H'ard disk 'N'etwork */
	info->num_names  	= MIG4NTO_UNSUPP;	/* Max names      */
	info->num_timers 	= MIG4NTO_UNSUPP;	/* Maximum number of timers     */
	info->num_sessions	= MIG4NTO_UNSUPP;	/* Maximum number of sessions   */
	info->num_handlers	= MIG4NTO_UNSUPP;	/* Maximum interrupt handlers   */
	info->reserve64k	= MIG4NTO_UNSUPP;	/* Relocation offset 			*/
	info->num_semaphores    = MIG4NTO_UNSUPP;
	info->prefix_len        = MIG4NTO_UNSUPP;
	info->max_nodes         = MIG4NTO_UNSUPP;	/* Num licensed nodes   */
	info->proc_freemem      = MIG4NTO_UNSUPP;
	info->cpu_loadmask      = MIG4NTO_UNSUPP;
	info->fd_freemem        = MIG4NTO_UNSUPP;
	info->ldt_freemem       = MIG4NTO_UNSUPP;

	info->num_fds[0]        = MIG4NTO_UNSUPP; /* Min fd's             */
	info->num_fds[1]        = MIG4NTO_UNSUPP; /* Max fd's             */
	info->num_fds[2]        = MIG4NTO_UNSUPP; /* Total fd's           */

	info->pidmask           = MIG4NTO_UNSUPP; /* Process ID bit mask 
											     (to determine cyclic and 
												 indexing portions of pid) */
	info->name_freemem      = MIG4NTO_UNSUPP;
	info->top_of_mem        = MIG4NTO_UNSUPP;
	info->freevmem          = MIG4NTO_UNSUPP;
	info->freepmem          = get_free_mem();
	info->totpmem           = get_total_mem();
	info->totvmem           = MIG4NTO_UNSUPP;
	info->cpu_features      = SYSPAGE_ENTRY(cpuinfo)->speed;

	return 0;
}

static int
get_total_mem(void)
{
	char					*str = SYSPAGE_ENTRY(strings)->data;
	struct asinfo_entry		*as = SYSPAGE_ENTRY(asinfo);
	int                     total = 0;
	unsigned				num;

	for(num = _syspage_ptr->asinfo.entry_size / sizeof(*as); num > 0; --num) {
		if(strcmp(&str[as->name], "ram") == 0) {
			total += as->end - as->start + 1;
		}
		++as;
	}
	return total;
}

static int
get_free_mem(void)
{
	struct stat st;
	int    rval = 0;

	if (stat( "/proc", &st) == 0)
		rval = st.st_size;
	
	return rval;
}

static short unsigned
get_tick_size(void)
{
	struct timespec res;

	if (clock_getres(CLOCK_REALTIME, &res) == -1)
		return -1;
	return res.tv_nsec / NSEC_TO_USEC;
}
