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
 * dev_info.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <devctl.h>
#include <errno.h>
#include <libgen.h>
#include <sys/dcmd_chr.h>
#include <mig4nto.h>

static unsigned short compute_unit(char *basetty);
static void get_driver_type(char *driver_type, char *basetty);

/*
 *	If the call is successful, the info structure contains:
 * 
 *	int unit            Unit number of this device.     
 *						(for example, /dev/con2 would have a unit of 2).
 *	nid_t nid           The node-id where this device exists.  Note that
 *                      this will contain a QNX Neutrino node descriptor
 *                      (nd) and not a QNX 4 network id (nid).
 *	pid_t driver_pid    Process ID of the driver task that controls 
 *						this device.
 *	char driver_type[16] A symbolic name describing the device nature.
 *	char tty_name[MAX_TTY_NAME]     A complete pathname that may be used 
 *									to open this device.
 *
 *	The next items are not filled in by this version of dev_info.
 *	int tty             TTY number of this device.
 *	unsigned nature     The nature or type of this device.
 *	unsigned attributes The character attributes supported by this device.
 *	unsigned capabilities   The capabilities of this device.
 *	short unsigned  flags   Flag bits.
 */
int
dev_info(int __fd, struct _dev_info_entry *__info)
{
	struct stat my_stat;
	struct _ttyinfo itty;
	struct _server_info srv_info;
	int     rval = 0;
	char    ttybuf[TTY_NAME_MAX];
	char    *basetty;

	if ((__fd < 0) || (fstat(__fd, &my_stat) != 0)) {
		errno = EBADF;
		rval = -1;
		return rval;
	}

	if ((!S_ISCHR(my_stat.st_mode)) && (!S_ISFIFO(my_stat.st_mode))) {
		errno = ENOSYS;
		rval = -1;
		return rval;
	}

	memset(__info, 0, sizeof(struct _dev_info_entry));
	if (ConnectServerInfo(getpid(), __fd, &srv_info) == -1) {
		rval = -1;
		return rval;
	}
	__info->nid = srv_info.nd;        	/* The node-id where device exists. */
	__info->driver_pid = srv_info.pid;  /* pid of task that controls device */

	if (devctl(__fd, DCMD_CHR_TTYINFO, &itty, sizeof(itty), NULL) == EOK) {
		memcpy(__info->tty_name, itty.ttyname, sizeof(__info->tty_name));
		__info->tty_name[sizeof(__info->tty_name) - 1] = '\0'; 
		memcpy(ttybuf, itty.ttyname, sizeof(ttybuf));
		ttybuf[sizeof(ttybuf) - 1] = '\0'; 
		__info->open_count = itty.opencount;
		basetty = basename(ttybuf);

		__info->unit = compute_unit(basetty);  
		get_driver_type(__info->driver_type, basetty);
	}
	__info->pgrp    = getpgid(__info->driver_pid);
	__info->session = getsid(__info->driver_pid);
	__info->major   = major(my_stat.st_rdev);     /* major number.    */

	return rval;
}

static unsigned short
compute_unit(char *basetty)
{
	unsigned short rval = -1;   
	char *ptr = basetty;

	if (ptr) {
		ptr += strlen(basetty) - 1;
		while (ptr - 1 > basetty && isdigit(*(ptr-1)))
			ptr--;
		rval = atoi(ptr);
	}
	return rval;
}

static void
get_driver_type(char *driver_type, char *basetty)
{
	char DriverArray[6][16] = {
		{"console"},    /* Console device               */
		{"serial"},     /* Asynchronous Serial device   */
		{"parallel"},   /* Parallel (printer) device    */
		{"netlink"},    /* Network link                 */
		{"pseudo"},     /* Pseudo terminal device       */
		{"unknown"}     /* Unknown terminal device      */
	};

	int index = -1;

	if (driver_type) {
		strcpy(driver_type, DriverArray[5]);
		if (basetty) {
			if (strstr(basetty, "con"))
				index = 0;
			else if (strstr(basetty, "ser"))
				index = 1;
			else if (strstr(basetty, "par"))
				index = 2;
			else if (strstr(basetty, "net"))
				index = 3;
			else if (strstr(basetty, "pty")) 
				index = 4;
			if (index != -1) 
				strcpy(driver_type, DriverArray[index]);
		}
	}
}
