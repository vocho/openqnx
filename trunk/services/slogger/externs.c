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



 
#ifdef HDR
	#define EXT extern
#else
	#define EXT
#endif


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/dispatch.h>
#include <sys/procmgr.h>
#include <sys/pathmgr.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/dcmd_chr.h>
#include <sys/slog.h>
#include "struct.h"
#include "proto.h"


EXT int						 Verbose;		// Be noisy
EXT int						 NumInts;		// Number ints in a buffer
EXT int                      LogFflags;     // Logfile flags
EXT int						 LogFsize;		// Maxsize of logfile
EXT char					*LogFname;		// Name of logfile
EXT int						 FilterLog;		// Severity filter for logging
struct slogdev				 SlogDev;		// The slog dev data

__SRCVERSION("externs.c $Rev: 157840 $");
