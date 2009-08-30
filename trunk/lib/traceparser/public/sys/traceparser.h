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
 *  sys/traceparser.h     Public definitions
 *

 */

#ifndef __TRACEPARSER_H_INCLUDED
#define __TRACEPARSER_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif

#ifndef __TRACE_H_INCLUDED
#include _NTO_HDR_(sys/trace.h)
#endif

#undef _TRACE_MK_HK
#define _TRACE_MK_HK(k)  _TRACEPARSER_INFO_##k

/* User info modes */
typedef enum {
	_TRACE_HEADER_KEYWORDS(),
	_TRACEPARSER_INFO_SYSPAGE = -1,              /* ret. type (syspage_entry*) casted to (void*) */
	_TRACEPARSER_INFO_ENDIAN_CONV = -2,          /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_NOW_CALLBACK_CLASS = -3,   /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_NOW_CALLBACK_EVENT = -4,   /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_PREV_CALLBACK_CLASS = -5,  /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_PREV_CALLBACK_EVENT = -6,  /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_PREV_CALLBACK_RETURN = -7, /* ret. type (int*)      casted to (void*)      */
	_TRACEPARSER_INFO_DEBUG = -8,                /* ret. type (unsigned*) casted to (void*)      */
	_TRACEPARSER_INFO_ERROR = -9                 /* ret. type (unsigned*) casted to (void*)      */
} info_modes_t;

/* Internal error codes */
typedef enum {
	_TRACEPARSER_UNKNOWN_CLASS,           /* 0  - class not found                         */
	_TRACEPARSER_UNKNOWN_PR_TH_EVENT,     /* 1  - event of process/thread class not found */
	_TRACEPARSER_UNKNOWN_CONTROL_EVENT,   /* 2  - control class event not found           */
	_TRACEPARSER_UNKNOWN_KER_CALL_EVENT,  /* 3  - kernel call class event not found       */
	_TRACEPARSER_UNKNOWN_INT_EVENT,       /* 4  - interrupt class event not found         */
	_TRACEPARSER_UNKNOWN_CONTAINER_EVENT, /* 5  - container class event not found         */
	_TRACEPARSER_UNKNOWN_USR_EVENT,       /* 6  - user class event not found              */
	_TRACEPARSER_UNKNOWN_COMM_EVENT,      /* 7  - comm class event not found              */
	_TRACEPARSER_NO_MORE_MEMORY,          /* 8  - no more memory                          */
	_TRACEPARSER_WRONG_H_KEYWORD,         /* 9  - invalid header keyword                  */
	_TRACEPARSER_NULL_STATE_PTR,          /* 10 - null pointer of traceparser state struct*/
	_TRACEPARSER_WRONG_ENDIAN_TYPE,       /* 11 - unknown endian type                     */
	_TRACEPARSER_CANNOT_READ_IN_FILE,     /* 12 - input tracebuffer file cannot be read   */
	_TRACEPARSER_WRONG_FILE_VERSION,      /* 13 - wrong version                           */
	_TRACEPARSER_MISSING_FIELD,           /* 14 - header field not found                  */
	_TRACEPARSER_UNKNOWN_ERROR            /* 15 - unknown error                           */
} traceparser_error_t;

/* Debug modes */
#define _TRACEPARSER_DEBUG_ALL     (0xffffffff)    /* everything is debugged             */
#define _TRACEPARSER_DEBUG_NONE    (0x00000000)    /* nothing is debugged                */
#define _TRACEPARSER_DEBUG_ERRORS  (0x00000001<<0) /* only critical errors are debugged  */
#define _TRACEPARSER_DEBUG_HEADER  (0x00000001<<1) /* only header information is debugged*/
#define _TRACEPARSER_DEBUG_SYSPAGE (0x00000001<<2) /* only syspage data is debugged      */
#define _TRACEPARSER_DEBUG_EVENTS  (0x00000001<<3) /* only row input events are debugged */

__BEGIN_DECLS

struct traceparser_state;
typedef int (*tracep_callb_func_t)(struct traceparser_state* __state_ptr, void* __user_data,
                                   unsigned __header, unsigned __time, unsigned* __buffer, unsigned __buffer_len);

/* User interface to the traceparser library */
extern struct traceparser_state* traceparser_init(struct traceparser_state* __state_ptr);
extern void  traceparser_destroy(struct traceparser_state** __state_ptr_ptr);
extern void* traceparser_get_info(struct traceparser_state* __state_ptr,
                                  info_modes_t __info_modes, unsigned* __len);
extern int   traceparser_debug(struct traceparser_state* __state_ptr,
                               FILE* __stream_ptr, unsigned __flags);
extern int   traceparser_cs(struct traceparser_state* __state_ptr, void* __user_data,
                            tracep_callb_func_t __func_ptr, unsigned __class, unsigned __event);
extern int   traceparser_cs_range(struct traceparser_state* __state_ptr, void* __user_data,
                                  tracep_callb_func_t __func_ptr, unsigned __class,
                                  unsigned __event1, unsigned __event2);
extern int   traceparser(struct traceparser_state* __state_ptr, void* __user_data, const char* __file_name);

__END_DECLS

#endif

/* __SRCVERSION("traceparser.h $Rev: 153052 $"); */
