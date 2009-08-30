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




const char * const sys_siglist[] = {
    "empty",
    "hangup",
    "interrupt",
    "quit",
    "illegal instruction",
    "trace trap",
    "abort",
    "mutex deadlock",
    "floating point exception",
    "kill",
    "bus error",
    "segmentation violation",
    "bad argument to system call",
    "write on pipe with no reader",
    "real-time alarm clock",
    "software termination signal",
    "user defined signal 1",
    "user defined signal 2",
    "death of child",
    "power-fail restart",
    "window change",
    "urgent condition on I/O channel",
    "asynchronus I/O",
    "sendable stop signal not from tty",
    "stop signal from tty",
    "continue a stopped process",
    "attempted background tty read",
    "attempted background tty write",
    "virtual timer expired",
    "profiling timer expired",
	"exceeded cpu limit",
	"exceeded file size limit",
	"signal 32",
	"signal 33",
	"signal 34",
	"signal 35",
	"signal 36",
	"signal 37",
	"signal 38",
	"signal 39",
	"signal 40",
	"rtsig 41",
	"rtsig 42",
	"rtsig 43",
	"rtsig 44",
	"rtsig 45",
	"rtsig 46",
	"rtsig 47",
	"rtsig 48",
	"rtsig 49",
	"rtsig 50",
	"rtsig 51",
	"rtsig 52",
	"rtsig 53",
	"rtsig 54",
	"rtsig 55",
	"rtsig 56"
};

const int sys_nsig = sizeof sys_siglist / sizeof sys_siglist[0];
