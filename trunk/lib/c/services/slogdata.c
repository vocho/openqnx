/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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




#include <errno.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/slog.h>


// Moved the slog*() data variables into their own file so that
// we only get one copy of them, no matter how many versions of 
// the slog functions are in various shared objects. The slog functions
// are only in the libc.a file to keep the size of libc.so down.
// This slogdata.c file however, _is_ included in libc.so.

//
// This is almost magic. Every process has a connection open to
// ProcNto which "just exists". It does not need to be opened.
// It is used for a variety of special purposes. If you send
// a xxx message to Proc it will attempt to open "/dev/slog"
// and send that message on as a simple _IO_WRITE. In effect
// it acts as a relay. Why is this good? It means every server
// which uses slogger (and most will) need not keep a separate fd
// open to the slogger manager. It also allows you to stop and
// start a new slogger manager and all clients will be redirected
// to the new one automatically.
// If an app wants to go direct it can do a
//
//   _slogfd = open("/dev/slog", O_WRONLY);
//
// which will override.
//
//
int _slogfd = SYSMGR_COID;
pthread_mutex_t	__slog_mux = PTHREAD_MUTEX_INITIALIZER;
