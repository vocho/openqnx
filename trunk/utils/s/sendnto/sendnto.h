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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

struct output {
	unsigned	check_rate;
	void		(*init)(void);
	int			(*open)(const char *name, int baud);
	void		(*flush)(int fd);
	int			(*write)(int fd, const void *buff, int len);
	int			(*get)(int fd, int timeout);
	void		(*close)(int fd);
};

#ifndef DEFN
#define DEFN	extern
#endif

DEFN int	echo;			// Request an echo every data record
DEFN int	quiet;			// Don't print percentage of image dowloaded
DEFN int	verbose;		// Tell me exactly what it is doing
DEFN int	laplink;
DEFN int	force_error;
DEFN int	host_endian;
DEFN int	target_endian;
DEFN int	gdb_pc_regnum;
DEFN int	gdb_pc_regsize;
DEFN int	gdb_cputype;
DEFN struct output	output;
	
#define NUM_ELTS( array )	(sizeof(array) / sizeof( array[0] ))

extern void		outputdevice(void);
extern void		outputinet(void);

extern int 		opendevice(const char *devicename, int baud);
extern void		flushdevice(int devicefd);
extern int		writedevice(int devicefd, const void *buff, int len);
extern int		getdevice(int fd, int timeout);
extern int 		checklapdevice(int devicefd);

extern int 		swap16(int val);
extern long		swap32(long val);

extern int	xfer_start_nto(int devicefd);
extern int	xfer_data_nto(int devicefd, int seq, unsigned long addr, const void *data, int nbytes);
extern int 	xfer_done_nto(int devicefd, int seq, unsigned long start_addr);

extern int	xfer_start_gdb(int devicefd);
extern int	xfer_data_gdb(int devicefd, int seq, unsigned long addr, const void *data, int nbytes);
extern int 	xfer_done_gdb(int devicefd, int seq, unsigned long start_addr);
