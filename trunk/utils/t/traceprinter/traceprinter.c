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





#ifdef __USAGE

%C - prints events from an event file

%C	[-vn] [-f in_file] [-o out_file]

%C	- prints out trace data stored in the event file
	to stdout/out_file. traceprinter looks in the default
	file /dev/shmem/tracebuffer.kev for the event file.

Options:
	-v            verbose option
	-n            remove new line characters from printed
	              argument lines
	-f <in_file>  name of the file that contains trace info
	              (default is /dev/shmem/tracebuffer.kev)
	-o <out_file> name of the output file where the data
	              should be printed/stored (default is stdout)

Usage notes:
	Must run tracelogger utility first to create trace
	data information (event file). tracelogger outputs
	trace data to /dev/shmem/tracebuffer.kev by default.
	traceprinter will parse and print out this trace data.  
#endif

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h> 
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include _NTO_HDR_(sys/kercalls.h)
#include _NTO_HDR_(sys/trace.h)
#include _NTO_HDR_(sys/traceparser.h)

/*
 *  task states(s) from neutrino.h
 */
const char * const task_state[] = {
	"THDEAD       ",
	"THRUNNING    ",
	"THREADY      ",
	"THSTOPPED    ",
	"THSEND       ",
	"THRECEIVE    ",
	"THREPLY      ",
	"THSTACK      ",
	"THWAITTHREAD ",
	"THWAITPAGE   ",
	"THSIGSUSPEND ",
	"THSIGWAITINFO",
	"THNANOSLEEP  ",
	"THMUTEX      ",
	"THCONDVAR    ",
	"THJOIN       ",
	"THINTR       ",
	"THSEM        ",
	"THWAITCTX    ",
	"THNET_SEND   ",
	"THNET_REPLY  "
};

#define VERSION_MAJOR 1
#define VERSION_MINOR 02

#define _TP_STR_SIZE  (512)

/* globals */
static const char* g_input_file = "/dev/shmem/tracebuffer.kev";
static       char* g_out_file   = NULL;
static const char* g_arg_format = "\n%26s:";
static       FILE* g_out_stream = NULL;
static       FILE* g_err_stream = NULL;
static       int   g_verbose;

/* [conditional] swap */
#define SWAP_32BITS(l) \
        (((uint32_t)(l)&0x000000ff)<<24| \
         ((uint32_t)(l)&0x0000ff00)<<8 | \
         ((uint32_t)(l)&0x00ff0000)>>8 | \
         ((uint32_t)(l)&0xff000000)>>24)

#define SWAP_64BITS(l) \
      	(((uint64_t)(l)&0x00000000000000ffLL)<<56| \
      	 ((uint64_t)(l)&0x000000000000ff00LL)<<40| \
      	 ((uint64_t)(l)&0x0000000000ff0000LL)<<24| \
         ((uint64_t)(l)&0x00000000ff000000LL)<<8 | \
         ((uint64_t)(l)&0x000000ff00000000LL)>>8 | \
         ((uint64_t)(l)&0x0000ff0000000000LL)>>24| \
         ((uint64_t)(l)&0x00ff000000000000LL)>>40| \
         ((uint64_t)(l)&0xff00000000000000LL)>>56) 


#define CS32(l) \
((*(unsigned*)traceparser_get_info(tp_state, _TRACEPARSER_INFO_ENDIAN_CONV, NULL)) \
?SWAP_32BITS(l):(l))

#define CS64(l) \
((*(unsigned*)traceparser_get_info(tp_state, _TRACEPARSER_INFO_ENDIAN_CONV, NULL)) \
?SWAP_64BITS(l):(l))

/* "folded" version of ck_print()  */
#define CK_PRN(s) (ck_print(strlen(s), s))

/*
 *  Checks for printable characters
 */
static char* ck_print(unsigned s, char* c_p)
{
	static char  n_c='\0';
	       char* s_p=c_p;

	while(s--&&*s_p) {
		if (*s_p != '\n' && !isprint((int)(*s_p))) {
			return (&n_c);
		}
		s_p++;
	}

	return (c_p);
}

/* Callback setting macro definition used only inside main() */
#define TP_CS(cs_func) \
if(cs_func) \
{ \
	(void) fprintf \
	( \
	 g_err_stream, \
	 "TRACEPARSER: function call traceparser_cs() failed, internal error code(%d)\n", \
	 *(unsigned*)traceparser_get_info(tp_state, _TRACEPARSER_INFO_ERROR, NULL) \
	); \
	traceparser_destroy(&tp_state); \
	\
	exit (-1); \
}

/* Range callback setting macro definition used only inside main() */
#define TP_CSR(csr_func) \
if(csr_func) \
{ \
	(void) fprintf \
	( \
	 g_err_stream, \
	 "TRACEPARSER: function call traceparser_cs_range() failed, internal error code(%d)\n", \
	 *(unsigned*)traceparser_get_info(tp_state, _TRACEPARSER_INFO_ERROR, NULL) \
	); \
	traceparser_destroy(&tp_state); \
	\
	exit (-1); \
}

/* shortened traceparser state structure type */
typedef struct traceparser_state* tp_state_t;

/*
 * Print formated string to the stream
 */
static int trace_print
(
	tp_state_t  tp_state, /* traceparser state */
	unsigned    t,        /* timestamp         */
	unsigned    h,        /* header            */
	char*       p,        /* buffer pointer    */
	unsigned    l,        /* buffer length     */
	const char* f_c_p,    /* class string      */
	const char* k,        /* event string      */
	const char* a,        /* arguments/format  */
	va_list     c_a       /* variable args     */
)
{
	const char* n_a=a;
	int         c  =0;
	unsigned    c_p=_NTO_TRACE_GETCPU(h);

	if (l==2) {
		(void) fprintf(g_out_stream, "t:0x%08x CPU:%02d %s:%s",
		              t, c_p, f_c_p, k);
		c = 0;
		while(n_a&&(c<(l*sizeof(int)))) {
			if (*(f_c_p=n_a)=='f') {
				++f_c_p;
				(void) fprintf(g_out_stream, " %s:", (f_c_p+1));
				switch(*f_c_p)
				{
					case 'N': /* name */
						(void) fprintf(g_out_stream, "%s\n", CK_PRN(c+p));

						return (0);
					break;
					case 'S': /* str */
						(void) fprintf(g_out_stream, "\"%.4s\" (0x%08x)", ck_print(4, c+p), *((unsigned*) (c+p)));
						c += 4;
					break;
					case 'D': /* dec */
						(void) fprintf(g_out_stream, "%d", CS32(*((unsigned*)(p+c))));
						c += 4;
					break;
					case 'H': /* hex */
						(void) fprintf(g_out_stream, "0x%08x", CS32(*((unsigned*)(p+c))));
						c += 4;
					break;
					case 'P': /* ptr  */
						(void) fprintf(g_out_stream, "%p", (void*) CS32(*((unsigned*)(p+c))));
						c += 4;
					break;
					case 'E':	/* 64 bit decimal */
						(void) fprintf(g_out_stream, "%lld", CS64(*((uint64_t*)(p+c))));
						c += 8;
					break;
					case 'X':	/* 64 bit hex */
						(void) fprintf(g_out_stream, "0x%llx", CS64(*((uint64_t*)(p+c))));
						c += 8;
					break;
					default:
					break;
				}
			}
			n_a = va_arg(c_a, char*);
		}
		(void) fprintf(g_out_stream, "\n");
	} else if (l>2) {
		int  s_c=0;
		int  h_c=0;
		char s[_TP_STR_SIZE];
		char h[4*_TP_STR_SIZE];

		(void) fprintf(g_out_stream, "t:0x%08x CPU:%02d %s:%s",
		              t, c_p, f_c_p, k);
		c = 0;
		while((c<(l*sizeof(int)))&&n_a) {
			const char* f_c_p=n_a;

			if (*f_c_p=='f') ++f_c_p;
			if (*f_c_p!='s') {
				if (s_c) {
					(void) fprintf(g_out_stream, "\"%s\" (0x%s)", CK_PRN(s), h);
					(void) fprintf(g_out_stream, g_arg_format, (f_c_p+1));
					h_c = s_c = 0;
				} else {
					(void) fprintf(g_out_stream, g_arg_format, (f_c_p+1));
				}
			}
			switch(*f_c_p)
			{
				case 'N': /* Name */
					(void) fprintf(g_out_stream, "%s\n", CK_PRN(c+p));

					return (0);
				break;
				case 'S': /* str */
					(void) sprintf(s+s_c, "%.4s", ck_print(4, p+c));
					(void) sprintf(h+h_c, "%08x", *((unsigned*)(p+c)));
					h_c += 8;
					s_c += 4;
					c   += 4;
				break;
				case 's': /* str */
					(void) sprintf(s+s_c, "%.4s", ck_print(4, p+c));
					(void) sprintf(h+h_c, " 0x%08x", *((unsigned*)(p+c)));
					h_c += 11;
					s_c += 4;
					c   += 4;
				break;
				case 'D': /* dec */
					(void) fprintf(g_out_stream, "%d", CS32(*((unsigned*)(p+c))));
					c += 4;
				break;
				case 'H': /* hex */
					(void) fprintf(g_out_stream, "0x%08x", CS32(*((unsigned*)(p+c))));
					c += 4;
				break;
				case 'P': /* ptr  */
					(void) fprintf(g_out_stream, "%p", (void*) CS32(*((unsigned*)(p+c))));
					c += 4;
				break;
				case 'E':	/* 64 bit decimal */
					(void) fprintf(g_out_stream, "%lld", CS64(*((uint64_t*)(p+c))));
					c += 8;
				break;
				case 'X':	/* 64 bit hex */
					(void) fprintf(g_out_stream, "0x%llx", CS64(*((uint64_t*)(p+c))));
					c += 8;
				break;
				default:
				break;
			}
			n_a = va_arg(c_a, char*);
		}
		if (s_c) {
			(void) fprintf(g_out_stream, "\"%s\" (0x%s)\n", CK_PRN(s), h);
		} else {
			(void) fprintf(g_out_stream, "\n");
		}
	} else {
		if (g_verbose) {
			(void) fprintf
			(
			 g_err_stream,
			 "TRACEPRINTER: %s(%s) narrow arguments number error\n",
			 f_c_p,
			 k
			);
		}

		return (-1);
	}

	return (0);
}

/*
 *
 *  Format printing ker-calls:
 *                              H - hexadecimal (32 bit)
 *                              D - decimal (32 bit)
 *                              X - hexadecimal (64 bit)
 *                              E - decimal (64 bit)
 *                              S - begin character string
 *                              s - cont character string
 *                              P - pointer
 *                              N - named string
 *
 *                              f - fast mode prefix
 */
static int tp(tp_state_t tp_state, unsigned t,
              unsigned h, char* p, unsigned l, const char* e, const char *a, ...)
{
	va_list ap;
	int     r_v;
	char    e_s[_TP_STR_SIZE];
	unsigned *pu = (unsigned *)p;

	va_start(ap, a);
	if (_NTO_TRACE_GETEVENT(h)>=_TRACE_MAX_KER_CALL_NUM) {
		(void) sprintf(e_s, "%s/%02d", e, _NTO_TRACE_GETEVENT(h)-_TRACE_MAX_KER_CALL_NUM);
		if ( pu[0] == -1 ) {
			unsigned    c_p=_NTO_TRACE_GETCPU(h);
			r_v = (fprintf(g_out_stream, "t:0x%08x CPU:%02d %s:%s ret_val:-1, errno:%d", t, c_p, "KER_EXIT", e_s, pu[1] ) > 0);
		} else {
			r_v = trace_print(tp_state, t, h, p, l, "KER_EXIT", e_s, a, ap);
		}
	} else {
		(void) sprintf(e_s, "%s/%02d", e, _NTO_TRACE_GETEVENT(h));
		r_v = trace_print(tp_state, t, h, p, l, "KER_CALL", e_s, a, ap);
	}
	va_end(ap);

	return (r_v);
}

/*
 *
 *  Format printing other classes:
 *                                 H - hexadecimal (32 bit)
 *                                 D - decimal (32 bit)
 *                                 X - hexadecimal (64 bit)
 *                                 E - decimal (64 bit)
 *                                 S - begin character string
 *                                 s - cont character string
 *                                 P - pointer
 *                                 N - named string
 *
 *                                 f - fast mode prefix
 */
static int to(tp_state_t tp_state, unsigned t, unsigned h, char* p,
              unsigned l, const char* c, const char* e, const char *a, ...)
{
	va_list ap;
	int     r_v;

	va_start(ap, a);
	r_v = trace_print(tp_state, t, h, p, l, c, e, a, ap);
	va_end(ap);

	return (r_v);
}

/*
 *  Pseudo macro-function definintions. Used inside
 *  _NTO_TRACE_KERCALL class/subclasses callback functions.
 */
#define _TP2(k,a,b)            {return (tp(s,t,h,(char*)p,l,#k,#a,#b,NULL));}
#define _TP3(k,a,b,c)          {return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,NULL));}
#define _TP4(k,a,b,c,d)        {return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,#d,NULL));}
#define _TP5(k,a,b,c,d,e)      {return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,#d,#e,NULL));}
#define _TP6(k,a,b,c,d,e,f)    {return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,#d,#e,#f,NULL));}
#define _TP7(k,a,b,c,d,e,f,g)  {return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,#d,#e,#f,#g,NULL));}
#define _TP8(k,a,b,c,d,e,f,g,i){return (tp(s,t,h,(char*)p,l,#k,#a,#b,#c,#d,#e,#f,#g,#i,NULL));}
#define _TP(k) \
{ \
	(void) fprintf \
	( \
	 g_out_stream, \
	 "t:0x%08x CPU:%02d INT_CALL:%s/%d\n", \
	 t, \
	 _NTO_TRACE_GETCPU(h), \
	 #k, \
	 __ ## k \
	); \
	return (0); \
}

/*
 *  Pseudo macro-function definintions. Used inside
 *  other class/subclasses callback functions.
 */
#define _TO(j,k,a)              	{return (to(s,t,h,(char*)p,l,j,k,a,NULL));}
#define _TO2(j,k,a,b)              {return (to(s,t,h,(char*)p,l,j,k,a,b,NULL));}
#define _TO3(j,k,a,b,c)            {return (to(s,t,h,(char*)p,l,j,k,a,b,c,NULL));}
#define _TO4(j,k,a,b,c,d)          {return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,NULL));}
#define _TO5(j,k,a,b,c,d,e)        {return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,e,NULL));}
#define _TO6(j,k,a,b,c,d,e,f)      {return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,e,f,NULL));}
#define _TO7(j,k,a,b,c,d,e,f,g)    {return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,e,f,g,NULL));}
#define _TO8(j,k,a,b,c,d,e,f,g,i)  {return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,e,f,g,i,NULL));}
#define _TO9(j,k,a,b,c,d,e,f,g,i,m){return (to(s,t,h,(char*)p,l,j,k,a,b,c,d,e,f,g,i,m,NULL));}

/*
 *  Kernel call entry callback functions
 *  (class - _NTO_TRACE_KERCALL, subclass - _NTO_TRACE_KERCALLENTER)
 */
static int c_ker_nop_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NOP, fHdummy, fHempty);
}
static int c_ker_trace_event_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5(TRACE_EVENT, fHmode, fHclass[header], Hevent[time_off], Hdata_1, Hdata_2);
}
static int c_ker_ring0_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(RING0, fPfunc_p, fParg_p);
}
static int c_ker_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE1, fHempty, fHempty);
}
static int c_ker_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE2, fHempty, fHempty);
}
static int c_ker_spare3_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE3, fHempty, fHempty);
}
static int c_ker_spare4_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE4, fHempty, fHempty);
}
static int c_ker_sys_cpupage_get_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_CPUPAGE_GET, fDindex, fHempty);
}
static int c_ker_sys_cpupage_set_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_CPUPAGE_SET, fDindex, fHvalue);
}
static int c_ker_sys_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_SPARE1, fHempty, fHempty);
}
static int c_ker_sys_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_SPARE2, fHempty, fHempty);
}
static int c_ker_msg_sendv_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(MSG_SENDV, fHcoid, Dsparts, Drparts, fSmsg, s, s);
}
static int c_ker_msg_sendvnc_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(MSG_SENDVNC, fHcoid, Dsparts, Drparts, fSmsg, s, s);
}
static int c_ker_msg_error_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_ERROR, fHrcvid, fDerr);
}
static int c_ker_msg_receivev_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_RECEIVEV, fHchid, fDrparts);
}
static int c_ker_msg_replyv_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(MSG_REPLYV, fHrcvid, Dsparts, fHstatus, Ssmsg, s, s);
}
static int c_ker_msg_readv_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_READV, fHrcvid, Prmsg_p, Drparts, fHoffset);
}
static int c_ker_msg_writev_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(MSG_WRITEV, fHrcvid, Dsparts, fHoffset, Smsg, s, s);
}
static int c_ker_msg_readwritev_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_READWRITEV, fHsrc_rcvid, fHdst_rcvid);
}
static int c_ker_msg_info_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_INFO, fHrcvid, fPinfo_p);
}
static int c_ker_msg_send_pulse_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_SEND_PULSE, fHcoid, Dpriority, fHcode, Hvalue);
}
static int c_ker_msg_deliver_event_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 MSG_DELIVER_EVENT,
	 fHrcvid,
	 fDevent->sigev_notify,
	 Pevent->sigev_notify_function_p,
	 Hevent->sigev_value,
	 Pevent->sigev_notify_attributes_p
	);
}
static int c_ker_msg_keydata_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_KEYDATA, fHrcvid, fHop);
}
static int c_ker_msg_readiov_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_READIOV, fHrcvid, Dparts, fHoffset, Hflags);
}
static int c_ker_msg_receivepulsev_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_RECEIVEPULSEV, fHchid, fDrparts);
}
static int c_ker_msg_verify_event_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 MSG_VERIFY_EVENT,
	 fHrcvid,
	 fDevent->sigev_notify,
	 Pevent->sigev_notify_function_p,
	 Hevent->sigev_value,
	 Pevent->sigev_notify_attributes_p
	);
}
static int c_ker_signal_kill_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(SIGNAL_KILL, Hnd, fDpid, Dtid, fDsigno, Hcode, Hvalue);
}
static int c_ker_signal_return_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_RETURN, fPs_p, fHempty);
}
static int c_ker_signal_fault_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_FAULT, fDsigcode, fPaddr);
}
static int c_ker_signal_action_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP7
	(
	 SIGNAL_ACTION,
	 Dpid,
	 Psigstub_p,
	 fDsigno,
	 fPact->sa_handler_p,
	 Hact->sa_flags,
	 Hact->sa_mask.bits[0],
	 Hact->sa_mask.bits[1]
	);
}
static int c_ker_signal_procmask_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 SIGNAL_PROCMASK,
	 fDpid,
	 fDtid,
	 Hhow,
	 Hsig_blocked->bits[0],
	 Hsig_blocked->bits[1]
	);
}
static int c_ker_signal_suspend_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SUSPEND, fHsig_blocked->bits[0], fHsig_blocked->bits[1]);
}
static int c_ker_signal_waitinfo_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_WAITINFO, fHsig_wait->bits[0], fHsig_wait->bits[1]);
}
static int c_ker_signal_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SPARE1, fHempty, fHempty);
}
static int c_ker_signal_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SPARE2, fHempty, fHempty);
}
static int c_ker_channel_create_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_CREATE, fHflags, fHempty);
}
static int c_ker_channel_destroy_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_DESTROY, fHchid, fHempty);
}
static int c_ker_channel_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_SPARE1, fHempty, fHempty);
}
static int c_ker_chancon_attr_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANCON_ATTR, fDchid, fHflags);
}
static int c_ker_connect_attach_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5(CONNECT_ATTACH, fHnd, fDpid, Hchid, Dindex, Hflags);
}
static int c_ker_connect_detach_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_DETACH, fHcoid, fHempty);
}
static int c_ker_connect_server_info_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_SERVER_INFO, fDpid, fHcoid);
}
static int c_ker_connect_client_info_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_CLIENT_INFO, fHscoid, fDngroups);
}
static int c_ker_connect_flags_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(CONNECT_FLAGS, Dpid, fHcoid, Hmasks, fHbits);
}
static int c_ker_connect_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_SPARE1, fHempty, fHempty);
}
static int c_ker_connect_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_SPARE2, fHempty, fHempty);
}
static int c_ker_thread_create_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "THREAD_CREATE",
	 "Dpid",
	 "fPfunc_p",
	 "fParg_p",
	 "Hflags",
	 "Dstacksize",
	 "Pstackaddr_p",
	 "Pexitfunc_p",
	 "Dpolicy",
#if defined(__EXT_QNX)
	 "Dsched_priority",
	 "Dsched_curpriority",
	 "Dparam.__ss_low_priority",
	 "Dparam.__ss_max_repl",
	 "Dparam.__ss_repl_period.tv_sec",
	 "Dparam.__ss_repl_period.tv_nsec",
	 "Dparam.__ss_init_budget.tv_sec",
	 "Dparam.__ss_init_budget.tv_nsec",
	 "Dparam.empty",
	 "Dparam.empty",
#else
	 "Dsched_priority",
	 "Dsched_curpriority",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
#endif
	 "Dguardsize",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 NULL
	));
}
static int c_ker_thread_destroy_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(THREAD_DESTROY, fDtid, Dpriority, fPstatus_p);
}
static int c_ker_thread_destroyall_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_DESTROYALL, fHempty, fHempty);
}
static int c_ker_thread_detach_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_DETACH, fDtid, fHempty);
}
static int c_ker_thread_join_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_JOIN, fDtid, fPstatus_p);
}
static int c_ker_thread_cancel_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_CANCEL, fDtid, fPcanstub_p);
}
static int c_ker_thread_ctl_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_CTL, fHcmd, fPdata_p);
}
static int c_ker_thread_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_SPARE1, fHempty, fHempty);
}
static int c_ker_thread_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_SPARE2, fHempty, fHempty);
}
static int c_ker_interrupt_attach_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5(INTERRUPT_ATTACH, fDintr, Phandler_p, Parea_p, Dareasize, fHflags);
}
static int c_ker_interrupt_detach_func_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_DETACH_FUNC, fDintr, fPhandler_p);
}
static int c_ker_interrupt_detach_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_DETACH, fDid, fHempty);
}
static int c_ker_interrupt_wait_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(INTERRUPT_WAIT, fHflags, fDtimeout_tv_sec, Dtimeout_tv_nsec);
}
static int c_ker_interrupt_mask_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_MASK, fDintr, fDid);
}
static int c_ker_interrupt_unmask_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_UNMASK, fDintr, fDid);
}
static int c_ker_interrupt_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE1, fHempty, fHempty);
}
static int c_ker_interrupt_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE2, fHempty, fHempty);
}
static int c_ker_interrupt_spare3_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE3, fHempty, fHempty);
}
static int c_ker_interrupt_spare4_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE4, fHempty, fHempty);
}
static int c_ker_clock_time_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_TIME, fDid, fDnew(sec), Dnew(nsec));
}
static int c_ker_clock_adjust_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_ADJUST, fDid, fDnew->tick_count, Dnew->tick_nsec_inc);
}
static int c_ker_clock_period_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_PERIOD, fDid, fDnew->nsec, Dnew->fract);
}
static int c_ker_clock_id_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CLOCK_ID, fDpid, fDtid);
}
static int c_ker_clock_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CLOCK_SPARE2, fHempty, fHempty);
}
static int c_ker_timer_create_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 TIMER_CREATE,
	 fHid,
	 fDevent->sigev_notify,
	 Pevent->sigev_notify_function_p,
	 Hevent->sigev_value,
	 Pevent->sigev_notify_attributes_p
	);
}
static int c_ker_timer_destroy_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_DESTROY, fHid, fHempty);
}
static int c_ker_timer_settime_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6
	(
	 TIMER_SETTIME,
	 fHid,
	 Hflags,
	 fDitime->nsec(sec),
	 Ditime->nsec(nsec),
	 Ditime->interval_nsec(sec),
	 Ditime->interval_nsec(nsec)
	);
}
static int c_ker_timer_info_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(TIMER_INFO, fDpid, fHid, Hflags, Pinfo_p);
}
static int c_ker_timer_alarm_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 TIMER_ALARM,
	 fHid,
	 fDitime->nsec(sec),
	 Ditime->nsec(nsec),
	 Ditime->interval_nsec(sec),
	 Ditime->interval_nsec(nsec)
	);
}
static int c_ker_timer_timeout_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP8
	(
	 TIMER_TIMEOUT,
	 Did,
	 fHtimeout_flags,
	 fDntime(sec),
	 Dntime(nsec),
	 Devent->sigev_notify,
	 Pevent->sigev_notify_function_p,
	 Hevent->sigev_value,
	 Pevent->sigev_notify_attributes_p
	);
}
static int c_ker_timer_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_SPARE1, fHempty, fHempty);
}
static int c_ker_timer_spare2_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_SPARE2, fHempty, fHempty);
}
static int c_ker_sync_create_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP8
	(
	 SYNC_CREATE,
	 fDtype,
	 fPsync_p,
	 Dcount,
	 Downer,
	 Dprotocol,
	 Hflags,
	 Dprioceiling,
	 Dclockid
	);
}
static int c_ker_sync_destroy_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(SYNC_DESTROY, fPsync_p, Dcount, fDowner);
}
static int c_ker_sync_mutex_lock_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(SYNC_MUTEX_LOCK, fPsync_p, Dcount, fDowner);
}
static int c_ker_sync_mutex_unlock_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(SYNC_MUTEX_UNLOCK, fPsync_p, Dcount, fDowner);
}
static int c_ker_sync_condvar_wait_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6
	(
	 SYNC_CONDVAR_WAIT,
	 fPsync_p,
	 fPmutex_p,
	 Dsync->count,
	 Dsync->owner,
	 Dmutex->count,
	 Dmutex->owner
	);
}
static int c_ker_sync_condvar_signal_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(SYNC_CONDVAR_SIGNAL, fPsync_p, fDall, Dsync->count, Dsync->owner);
}
static int c_ker_sync_sem_post_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(SYNC_SEM_POST, fPsync_p, fDcount, Downer);
}
static int c_ker_sync_sem_wait_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(SYNC_SEM_WAIT, fPsync_p, Dtry, fDcount, Downer);
}
static int c_ker_sync_ctl_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5(SYNC_CTL, fDcmd, fPsync_p, Pdata_p, Dcount, Downer);
}
static int c_ker_sync_mutex_revive_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(SYNC_MUTEX_REVIVE, fPsync_p, Dcount, fDowner);
}
static int c_ker_sched_get_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_GET, fDpid, fDtid);
}
static int c_ker_sched_set_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "SCHED_SET",
	 "fDpid",
	 "Dtid",
	 "Dpolicy",
#if defined(__EXT_QNX)
	 "fDsched_priority",
	 "Dsched_curpriority",
	 "Dparam.__ss_low_priority",
	 "Dparam.__ss_max_repl",
	 "Dparam.__ss_repl_period.tv_sec",
	 "Dparam.__ss_repl_period.tv_nsec",
	 "Dparam.__ss_init_budget.tv_sec",
	 "Dparam.__ss_init_budget.tv_nsec",
	 "Dparam.empty",
	 "Dparam.empty",
#else
	 "fDsched_priority",
	 "Dsched_curpriority",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
#endif
	 NULL
	));
}
static int c_ker_sched_yield_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_YIELD, fHempty, fHempty);
}
static int c_ker_sched_info_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_INFO, fDpid, fDpolicy);
}
static int c_ker_sched_ctl_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_CTL, fHcmd, fHempty);
}
static int c_ker_net_cred_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_CRED, fHcoid, fPinfo_p);
}
static int c_ker_net_vtid_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "NET_VTID",
	 "fDvtid",
	 "fPinfo_p",
	 "Dtid",
	 "Hcoid",
	 "Dpriority",
	 "Dsrcmsglen",
	 "Hkeydata",
	 "Dsrcnd",
	 "Ddstmsglen",
	 "Dzero",
	 NULL
	));
}
static int c_ker_net_unblock_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_UNBLOCK, fHvtid, fHempty);
}
static int c_ker_net_infoscoid_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_INFOSCOID, fHscoid, fHinfoscoid);
}
static int c_ker_net_signal_kill_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP8
	(
	 NET_SIGNAL_KILL,
	 Dcred->ruid,
	 Dcred->euid,
	 Hnd,
	 fDpid,
	 Dtid,
	 fDsigno,
	 Hcode,
	 Hvalue
	);
}
static int c_ker_msg_current_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_CURRENT, fHrcvid, fHempty);
}
static int c_ker_net_spare1_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_SPARE1, fHempty, fHempty);
}
static int c_ker_bad_en(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(BAD, fHempty, fHempty);
}

/*
 *  Kernel call exit callback functions
 *  (class - _NTO_TRACE_KERCALL, subclass - _NTO_TRACE_KERCALLEXIT)
 */
static int c_ker_nop_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NOP, fHempty, fHempty);
}
static int c_ker_trace_event_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TRACE_EVENT, fHret_val, fHempty);
}
static int c_ker_ring0_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(RING0, fHret_val, fHempty);
}
static int c_ker_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE1, fHempty, fHempty);
}
static int c_ker_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE2, fHempty, fHempty);
}
static int c_ker_spare3_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE3, fHempty, fHempty);
}
static int c_ker_spare4_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SPARE4, fHempty, fHempty);
}
static int c_ker_sys_cpupage_get_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_CPUPAGE_GET, fHret_val, fHempty);
}
static int c_ker_sys_cpupage_set_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_CPUPAGE_SET, fHret_val, fHempty);
}
static int c_ker_sys_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYS_SPARE1, fHempty, fHempty);
}
static int c_ker_sys_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_SPARE2, fHempty, fHempty);
}
static int c_ker_msg_sendv_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_SENDV, fDstatus, fSrmsg, s, s);
}
static int c_ker_msg_sendvnc_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_SENDVNC, fHret_val, fSrmsg, s, s);
}
static int c_ker_msg_error_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_ERROR, fDret_val, fHempty);
}
static int c_ker_msg_receivev_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "MSG_RECEIVEV",
	 "fHrcvid",
	 "fSrmsg",
	 "s",
	 "s",
	 "Dinfo->nd",
	 "Dinfo->srcnd",
	 "Dinfo->pid",
	 "Dinfo->tid",
	 "Dinfo->chid",
	 "Dinfo->scoid",
	 "Dinfo->coid",
	 "Dinfo->msglen",
	 "Dinfo->srcmsglen",
	 "Dinfo->dstmsglen",
	 "Dinfo->priority",
	 "Dinfo->flags",
	 "Dinfo->reserved",
	 NULL
	));
}
static int c_ker_msg_replyv_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_REPLYV, fDret_val, fHempty);
}
static int c_ker_msg_readv_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_READV, fDrbytes, fSrmsg, s, s);
}
static int c_ker_msg_writev_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_WRITEV, fDwbytes, fHempty);
}
static int c_ker_msg_readwritev_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_READWRITEV, fDmsglen, fHempty);
}
static int c_ker_msg_info_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "MSG_INFO",
	 "fHret_val",
	 "fDinfo->nd",
	 "Dinfo->srcnd",
	 "Dinfo->pid",
	 "Dinfo->tid",
	 "Dinfo->chid",
	 "Dinfo->scoid",
	 "Dinfo->coid",
	 "Dinfo->msglen",
	 "Dinfo->srcmsglen",
	 "Dinfo->dstmsglen",
	 "Dinfo->priority",
	 "Dinfo->flags",
	 "Hempty",
	 NULL
	));
}
static int c_ker_msg_send_pulse_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_SEND_PULSE, fDstatus, fHempty);
}
static int c_ker_msg_deliver_event_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_DELIVER_EVENT, fHret_val, fHempty);
}
static int c_ker_msg_keydata_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_KEYDATA, fHret_val, fDnewkey);
}
static int c_ker_msg_readiov_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP4(MSG_READIOV, fDrbytes, fSrmsg, s, s);
}
static int c_ker_msg_receivepulsev_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "MSG_RECEIVEPULSEV",
	 "fHrcvid",
	 "fSrmsg",
	 "s",
	 "s",
	 "Dinfo->nd",
	 "Dinfo->srcnd",
	 "Dinfo->pid",
	 "Dinfo->tid",
	 "Dinfo->chid",
	 "Dinfo->scoid",
	 "Dinfo->coid",
	 "Dinfo->msglen",
	 "Dinfo->srcmsglen",
	 "Dinfo->dstmsglen",
	 "Dinfo->priority",
	 "Dinfo->flags",
	 "Dinfo->reserved",
	 NULL
	));
}
static int c_ker_msg_verify_event_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_VERIFY_EVENT, fDstatus, fHempty);
}
static int c_ker_signal_kill_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_KILL, fDret_val, fHempty);
}
static int c_ker_signal_return_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_RETURN, fHret_val, fHempty);
}
static int c_ker_signal_fault_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6(SIGNAL_FAULT, fHret_val, fHreg_1, Hreg_2, Hreg_3, Hreg_4, Hreg_5);
}
static int c_ker_signal_action_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 SIGNAL_ACTION,
	 fHret_val,
	 fPoact->sa_handler_p,
	 Hoact->sa_flags,
	 Hoact->sa_mask.bits[0],
	 Hoact->sa_mask.bits[1]
	);
}
static int c_ker_signal_procmask_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3
	(
	 SIGNAL_PROCMASK,
	 fDret_val,
	 fHold_sig_blocked->bits[0],
	 Hold_sig_blocked->bits[1]
	);
}
static int c_ker_signal_suspend_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SUSPEND, fHret_val, fPsig_blocked_p);
}
static int c_ker_signal_waitinfo_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "SIGNAL_WAITINFO",
	 "fDsig_num",
	 "Dsi_signo",
	 "fDsi_code",
	 "Dsi_errno",
	 "Dp[0]",
	 "Dp[1]",
	 "Dp[2]",
	 "Dp[3]",
	 "Dp[4]",
	 "Dp[5]",
	 "Dp[6]",
	 NULL
	));
}
static int c_ker_signal_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SPARE1, fHempty, fHempty);
}
static int c_ker_signal_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SIGNAL_SPARE2, fHempty, fHempty);
}
static int c_ker_channel_create_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_CREATE, fHchid, fHempty);
}
static int c_ker_channel_destroy_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_DESTROY, fHret_val, fHempty);
}
static int c_ker_channel_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANNEL_SPARE1, fHempty, fHempty);
}
static int c_ker_chancon_attr_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CHANCON_ATTR, fDchid, fHflags);
}
static int c_ker_connect_attach_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_ATTACH, fHcoid, fHempty);
}
static int c_ker_connect_detach_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_DETACH, fHret_val, fHempty);
}
static int c_ker_connect_server_info_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "CONNECT_SERVER_INFO",
	 "fHcoid",
	 "fDinfo->nd",
	 "Dinfo->srcnd",
	 "Dinfo->pid",
	 "Dinfo->tid",
	 "Dinfo->chid",
	 "Dinfo->scoid",
	 "Dinfo->coid",
	 "Dinfo->msglen",
	 "Dinfo->srcmsglen",
	 "Dinfo->dstmsglen",
	 "Dinfo->priority",
	 "Dinfo->flags",
	 "Dinfo->reserved",
	 NULL
	));
}
static int c_ker_connect_client_info_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "CONNECT_CLIENT_INFO",
	 "fHret_val",
	 "fDinfo->nd",
	 "Dinfo->pid",
	 "Dinfo->sid",
	 "Hflags",
	 "Dinfo->ruid",
	 "Dinfo->euid",
	 "Dinfo->suid",
	 "Dinfo->rgid",
	 "Dinfo->egid",
	 "Dinfo->sgid",
	 "Dinfo->ngroups",
	 "Dinfo->grouplist[0]",
	 "Dinfo->grouplist[1]",
	 "Dinfo->grouplist[2]",
	 "Dinfo->grouplist[3]",
	 "Dinfo->grouplist[4]",
	 "Dinfo->grouplist[5]",
	 "Dinfo->grouplist[6]",
	 "Dinfo->grouplist[7]",
	 NULL
	));
}
static int c_ker_connect_flags_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_FLAGS, fHold_flags, fHempty);
}
static int c_ker_connect_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_SPARE1, fHempty, fHempty);
}
static int c_ker_connect_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CONNECT_SPARE2, fHempty, fHempty);
}
static int c_ker_thread_create_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_CREATE, fHthread_id, fHowner);
}
static int c_ker_thread_destroy_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_DESTROY, fHret_val, fHempty);
}
static int c_ker_thread_destroyall_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_DESTROYALL, fHret_val, fHempty);
}
static int c_ker_thread_detach_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_DETACH, fHret_val, fHempty);
}
static int c_ker_thread_join_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_JOIN, fHret_val, fPstatus_p);
}
static int c_ker_thread_cancel_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_CANCEL, fHret_val, fHempty);
}
static int c_ker_thread_ctl_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_CTL, fHret_val, fHempty);
}
static int c_ker_thread_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_SPARE1, fHempty, fHempty);
}
static int c_ker_thread_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(THREAD_SPARE2, fHempty, fHempty);
}
static int c_ker_interrupt_attach_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_ATTACH, fHint_fun_id, fHempty);
}
static int c_ker_interrupt_detach_func_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_DETACH_FUNC, fDret_val, fHempty);
}
static int c_ker_interrupt_detach_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_DETACH, fDret_val, fHempty);
}
static int c_ker_interrupt_wait_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_WAIT, fDret_val, fHempty);
}
static int c_ker_interrupt_mask_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_MASK, fHmask_level, fHempty);
}
static int c_ker_interrupt_unmask_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_UNMASK, fHmask_level, fHempty);
}
static int c_ker_interrupt_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE1, fHempty, fHempty);
}
static int c_ker_interrupt_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE2, fHempty, fHempty);
}
static int c_ker_interrupt_spare3_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE3, fHempty, fHempty);
}
static int c_ker_interrupt_spare4_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(INTERRUPT_SPARE4, fHempty, fHempty);
}
static int c_ker_clock_time_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_TIME, fDret_val, fDold(sec), Dold(nsec));
}
static int c_ker_clock_adjust_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_ADJUST, fDret_val, fDold->tick_count, Dold->tick_nsec_inc);
}
static int c_ker_clock_period_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3(CLOCK_PERIOD, fDret_val, fDold->nsec, Dold->fract);
}
static int c_ker_clock_id_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CLOCK_ID, fHret_val, fHempty);
}
static int c_ker_clock_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(CLOCK_SPARE2, fHempty, fHempty);
}
static int c_ker_timer_create_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_CREATE, fHtimer_id, fHempty);
}
static int c_ker_timer_destroy_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_DESTROY, fHret_val, fHempty);
}
static int c_ker_timer_settime_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 TIMER_SETTIME,
	 fHret_val,
	 fDoitime->nsec(sec),
	 Doitime->nsec(nsec),
	 Doitime->interval_nsec(sec),
	 Doitime->interval_nsec(nsec)
	);
}
static int c_ker_timer_info_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "TIMER_INFO",
	 "fDprev_id",
	 "fDinfo->itime.nsec(sec)",
	 "Dinfo->itime.nsec(nsec)",
	 "Dinfo->itime.interval_nsec(sec)",
	 "Dinfo->itime.interval_nsec(nsec)",
	 "Dinfo->otime.nsec(sec)",
	 "Dinfo->otime.nsec(nsec)",
	 "Dinfo->otime.interval_nsec(sec)",
	 "Dinfo->otime.interval_nsec(nsec)",
	 "Dinfo->flags",
	 "Dinfo->tid",
	 "Dinfo->notify",
	 "Dinfo->clockid",
	 "Dinfo->overruns",
	 "Dinfo->event.sigev_notify",
	 "Pinfo->event.sigev_notify_function_p",
	 "Hinfo->event.sigev_value",
	 "Pinfo->event.sigev_notify_attributes_p",
	 NULL
	));
}
static int c_ker_timer_alarm_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP5
	(
	 TIMER_ALARM,
	 fHret_val,
	 fDoitime->nsec(sec),
	 Doitime->nsec(nsec),
	 Doitime->interval_nsec(sec),
	 Doitime->interval_nsec(nsec)
	);
}
static int c_ker_timer_timeout_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP3
	(
	 TIMER_TIMEOUT,
	 fHprev_timeout_flags,
	 fDotime(sec),
	 Dotime(nsec)
	);
}
static int c_ker_timer_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_SPARE1, fHempty, fHempty);
}
static int c_ker_timer_spare2_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(TIMER_SPARE2, fHempty, fHempty);
}
static int c_ker_sync_create_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_CREATE, fDret_val, fHempty);
}
static int c_ker_sync_destroy_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_DESTROY, fDret_val, fHempty);
}
static int c_ker_sync_mutex_lock_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_MUTEX_LOCK, fDret_val, fHempty);
}
static int c_ker_sync_mutex_unlock_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_MUTEX_UNLOCK, fDret_val, fHempty);
}
static int c_ker_sync_condvar_wait_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_CONDVAR_WAIT, fDret_val, fHempty);
}
static int c_ker_sync_condvar_signal_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_CONDVAR_SIG, fDret_val, fHempty);
}
static int c_ker_sync_sem_post_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_SEM_POST, fHret_val, fHempty);
}
static int c_ker_sync_sem_wait_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_SEM_WAIT, fDret_val, fHempty);
}
static int c_ker_sync_ctl_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_CTL, fDret_val, fHempty);
}
static int c_ker_sync_mutex_revive_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SYNC_MUTEX_REVIVE, fDret_val, fHempty);
}
static int c_ker_sched_get_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "SCHED_GET",
	 "fDret_val",
#if defined(__EXT_QNX)
	 "fDsched_priority",
	 "Dsched_curpriority",
	 "Dparam.__ss_low_priority",
	 "Dparam.__ss_max_repl",
	 "Dparam.__ss_repl_period.tv_sec",
	 "Dparam.__ss_repl_period.tv_nsec",
	 "Dparam.__ss_init_budget.tv_sec",
	 "Dparam.__ss_init_budget.tv_nsec",
	 "Dparam.empty",
	 "Dparam.empty",
#else
	 "fDsched_priority",
	 "Dsched_curpriority",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
	 "Dempty",
#endif
	 NULL
	));
}
static int c_ker_sched_set_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_SET, fDret_val, fHempty);
}
static int c_ker_sched_yield_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_YIELD, fHret_val, fHempty);
}
static int c_ker_sched_info_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP6
	(
	 SCHED_INFO,
	 fDret_val,
	 Dpriority_min,
	 fDpriority_max,
	 Dinterval_sec,
	 Dinterval_nsec,
	 Dpriority_priv
	);
}
static int c_ker_sched_ctl_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(SCHED_CTL, fHempty, fHempty);
}
static int c_ker_net_cred_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	return (tp
	(
	 s,
	 t,
	 h,
	 (char*)p,
	 l,
	 "NET_CRED",
	 "fDret_val",
	 "fDinfo->nd",
	 "Dinfo->pid",
	 "Dinfo->sid",
	 "Hinfo->flags",
	 "Dinfo->ruid",
	 "Dinfo->euid",
	 "Dinfo->suid",
	 "Dinfo->rgid",
	 "Dinfo->egid",
	 "Dinfo->sgid",
	 "Dinfo->ngroups",
	 "Dinfo->grouplist[0]",
	 "Dinfo->grouplist[1]",
	 "Dinfo->grouplist[2]",
	 "Dinfo->grouplist[3]",
	 "Dinfo->grouplist[4]",
	 "Dinfo->grouplist[5]",
	 "Dinfo->grouplist[6]",
	 "Dinfo->grouplist[7]",
	 NULL
	));
}
static int c_ker_net_vtid_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_VTID, fHret_val, fHempty);
}
static int c_ker_net_unblock_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_UNBLOCK, fHret_val, fHempty);
}
static int c_ker_net_infoscoid_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_INFOSCOID, fHret_val, fHempty);
}
static int c_ker_net_signal_kill_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_SIGNAL_KILL, fDstatus, fHempty);
}
static int c_ker_msg_current_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(MSG_CURRENT, fHempty, fHempty);
}
static int c_ker_net_spare1_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(NET_SPARE1, fHempty, fHempty);
}
static int c_ker_bad_ex(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP2(BAD, fHret_val, fHempty);
}

/*
 *  "Interrupted" kernel call callback functions
 */
static int c_ker_nop_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NOP);
}
static int c_ker_trace_event_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TRACE_EVENT);
}
static int c_ker_ring0_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_RING0);
}
static int c_ker_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SPARE1);
}
static int c_ker_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SPARE2);
}
static int c_ker_spare3_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SPARE3);
}
static int c_ker_spare4_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SPARE4);
}

static int c_ker_sys_cpupage_get_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYS_CPUPAGE_GET);
}
static int c_ker_sys_cpupage_set_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYS_CPUPAGE_SET);
}
static int c_ker_sys_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYS_SPARE1);
}
static int c_ker_sys_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_SPARE2);
}

static int c_ker_msg_sendv_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_SENDV);
}
static int c_ker_msg_sendvnc_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_SENDVNC);
}
static int c_ker_msg_error_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_ERROR);
}
static int c_ker_msg_receivev_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_RECEIVEV);
}
static int c_ker_msg_replyv_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_REPLYV);
}
static int c_ker_msg_readv_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_READV);
}
static int c_ker_msg_writev_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_WRITEV);
}
static int c_ker_msg_readwritev_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_READWRITEV);
}
static int c_ker_msg_info_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_INFO);
}
static int c_ker_msg_send_pulse_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_SEND_PULSE);
}
static int c_ker_msg_deliver_event_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_DELIVER_EVENT);
}
static int c_ker_msg_keydata_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_KEYDATA);
}
static int c_ker_msg_readiov_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_READIOV);
}
static int c_ker_msg_receivepulsev_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_RECEIVEPULSEV);
}
static int c_ker_msg_verify_event_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_VERIFY_EVENT);
}

static int c_ker_signal_kill_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_KILL);
}
static int c_ker_signal_return_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_RETURN);
}
static int c_ker_signal_fault_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_FAULT);
}
static int c_ker_signal_action_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_ACTION);
}
static int c_ker_signal_procmask_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_PROCMASK);
}
static int c_ker_signal_suspend_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_SUSPEND);
}
static int c_ker_signal_waitinfo_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_WAITINFO);
}
static int c_ker_signal_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_SPARE1);
}
static int c_ker_signal_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SIGNAL_SPARE2);
}

static int c_ker_channel_create_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CHANNEL_CREATE);
}
static int c_ker_channel_destroy_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CHANNEL_DESTROY);
}
static int c_ker_channel_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CHANNEL_SPARE1);
}
static int c_ker_chancon_attr_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CHANCON_ATTR);
}

static int c_ker_connect_attach_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_ATTACH);
}
static int c_ker_connect_detach_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_DETACH);
}
static int c_ker_connect_server_info_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_SERVER_INFO);
}
static int c_ker_connect_client_info_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_CLIENT_INFO);
}
static int c_ker_connect_flags_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_FLAGS);
}
static int c_ker_connect_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_SPARE1);
}
static int c_ker_connect_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CONNECT_SPARE2);
}

static int c_ker_thread_create_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_CREATE);
}
static int c_ker_thread_destroy_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_DESTROY);
}
static int c_ker_thread_destroyall_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_DESTROYALL);
}
static int c_ker_thread_detach_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_DETACH);
}
static int c_ker_thread_join_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_JOIN);
}
static int c_ker_thread_cancel_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_CANCEL);
}
static int c_ker_thread_ctl_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_CTL);
}
static int c_ker_thread_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_SPARE1);
}
static int c_ker_thread_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_THREAD_SPARE2);
}

static int c_ker_interrupt_attach_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_ATTACH);
}
static int c_ker_interrupt_detach_func_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_DETACH_FUNC);
}
static int c_ker_interrupt_detach_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_DETACH);
}
static int c_ker_interrupt_wait_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_WAIT);
}
static int c_ker_interrupt_mask_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_MASK);
}
static int c_ker_interrupt_unmask_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_UNMASK);
}
static int c_ker_interrupt_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_SPARE1);
}
static int c_ker_interrupt_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_SPARE2);
}
static int c_ker_interrupt_spare3_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_SPARE3);
}
static int c_ker_interrupt_spare4_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_INTERRUPT_SPARE4);
}

static int c_ker_clock_time_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CLOCK_TIME);
}
static int c_ker_clock_adjust_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CLOCK_ADJUST);
}
static int c_ker_clock_period_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CLOCK_PERIOD);
}
static int c_ker_clock_id_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CLOCK_ID);
}
static int c_ker_clock_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_CLOCK_SPARE2);
}

static int c_ker_timer_create_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_CREATE);
}
static int c_ker_timer_destroy_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_DESTROY);
}
static int c_ker_timer_settime_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_SETTIME);
}
static int c_ker_timer_info_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_INFO);
}
static int c_ker_timer_alarm_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_ALARM);
}
static int c_ker_timer_timeout_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_TIMEOUT);
}
static int c_ker_timer_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_SPARE1);
}
static int c_ker_timer_spare2_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_TIMER_SPARE2);
}

static int c_ker_sync_create_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_CREATE);
}
static int c_ker_sync_destroy_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_DESTROY);
}
static int c_ker_sync_mutex_lock_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_MUTEX_LOCK);
}
static int c_ker_sync_mutex_unlock_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_MUTEX_UNLOCK);
}
static int c_ker_sync_condvar_wait_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_CONDVAR_WAIT);
}
static int c_ker_sync_condvar_signal_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_CONDVAR_SIGNAL);
}
static int c_ker_sync_sem_post_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_SEM_POST);
}
static int c_ker_sync_sem_wait_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_SEM_WAIT);
}
static int c_ker_sync_ctl_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_CTL);
}
static int c_ker_sync_mutex_revive_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SYNC_MUTEX_REVIVE);
}

static int c_ker_sched_get_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SCHED_GET);
}
static int c_ker_sched_set_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SCHED_SET);
}
static int c_ker_sched_yield_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SCHED_YIELD);
}
static int c_ker_sched_info_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SCHED_INFO);
}
static int c_ker_sched_ctl_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_SCHED_CTL);
}

static int c_ker_net_cred_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_CRED);
}
static int c_ker_net_vtid_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_VTID);
}
static int c_ker_net_unblock_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_UNBLOCK);
}
static int c_ker_net_infoscoid_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_INFOSCOID);
}
static int c_ker_net_signal_kill_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_SIGNAL_KILL);
}
static int c_ker_msg_current_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_MSG_CURRENT);
}
static int c_ker_net_spare1_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_NET_SPARE1);
}

static int c_ker_bad_ix(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TP(KER_BAD);
}

/*
 * Control events
 */
static int p_time_set(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("CONTROL ", "TIME", "fHmsb", "fHlsb(offset)");
}

static int p_buffer(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
	(
	 g_out_stream,
	 "t:0x%08x CPU:%02d CONTROL: BUFFER sequence = %d, num_events = %d\n",
	 t,
	 _NTO_TRACE_GETCPU(h),
	 CS32(b_p[0]) & 0x7ff,
	 CS32(b_p[1])
	);

	return 0;
}

/*
 *  Interrupt entry callback function
 *  (class - _NTO_TRACE_INT, subclass - _NTO_TRACE_INTENTER)
 */
static int p_int_en(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
	(
	 g_out_stream,
	 "t:0x%08x CPU:%02d INT_ENTR:0x%08x (%d)       IP:0x%08x\n",
	 t,
	 _NTO_TRACE_GETCPU(h),
	 CS32(b_p[0]),
	 CS32(b_p[0]),
	 CS32(b_p[1])
	);

	return (0);
}

/*
 *  Interrupt handler entry callback function
 *  (class - _NTO_TRACE_INT, subclass - _NTO_TRACE_INT_HANDLER_ENTER)
 */
static int p_int_handler_en(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
	(
	 g_out_stream,
	 "t:0x%08x CPU:%02d INT_HANDLER_ENTR:0x%08x (%d)       PID:%d IP:0x%08x AREA:0x%08x\n",
	 t,
	 _NTO_TRACE_GETCPU(h),
	 CS32(b_p[1]),
	 CS32(b_p[1]),
	 CS32(b_p[0]),
	 CS32(b_p[2]),
	 CS32(b_p[3])
	);

	return (0);
}

/*
 *  Interrupt exit callback function
 *  (class - _NTO_TRACE_INT, subclass - _NTO_TRACE_INTEXIT)
 */
static int p_int_ex(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
	(
	 g_out_stream,
	 "t:0x%08x CPU:%02d INT_EXIT:0x%08x (%d) inkernel:0x%08x\n",
	 t,
	 _NTO_TRACE_GETCPU(h),
	 CS32(b_p[0]),
	 CS32(b_p[0]),
	 CS32(b_p[1])
	);

	return (0);
}

static char *print_sigevent( tp_state_t tp_state, void *sigevent_arg )
{
#ifdef __QNXNTO__
static char buf[1024];
char *p = buf;
struct sigevent *info = (struct sigevent *)sigevent_arg;
int	notify = CS32(info->sigev_notify );
#ifdef SIGEV_FLAG_CRITICAL
int critical = (notify & SIGEV_FLAG_CRITICAL);

	notify &= SIGEV_TYPE_MASK;
#endif

	switch ( notify ) {
	case SIGEV_SIGNAL:
		p += sprintf( p, "SIGNAL %u", CS32((int)info->sigev_signo) );
		break;
	case SIGEV_SIGNAL_CODE:
		p += sprintf( p, "%s %lu %#x:%#lx", "SIGNAL_CODE", CS32((long)info->sigev_signo), CS32(info->sigev_code), CS32((long)info->sigev_value.sival_int) );
		break;
	case SIGEV_SIGNAL_THREAD:
		p += sprintf( p, "%s %lu %#x:%#lx", "SIGNAL_THREAD", CS32((long)info->sigev_signo), CS32(info->sigev_code), CS32((long)info->sigev_value.sival_int) );
		break;
	case SIGEV_PULSE:
		p += sprintf( p, "PULSE %#lx:%ld %#x:%#lx", CS32((long)info->sigev_coid), CS32((long)info->sigev_priority), CS32((int)info->sigev_code), CS32((long)info->sigev_value.sival_int) );
		break;
	case SIGEV_UNBLOCK:
		p += sprintf( p, "UNBLOCK" );
		break;
	case SIGEV_INTR:
		p += sprintf( p, "INTR" );
		break;
	case SIGEV_THREAD:
		p += sprintf( p, "THREAD %lx:%lx", CS32((long)info->sigev_code), CS32((long)info->sigev_value.sival_int) );
		break;
	case 0:
		p += sprintf( p, "NONE" );
		break;
	default:
		p += sprintf( p, "UNKNOWN(%#x)", notify );
		break;
	}
#ifdef SIGEV_FLAG_CRITICAL
	if ( critical ) {
		p += sprintf( p, " CRITICAL" );
	}
#endif
	return buf;
#else
	return "INTR EVENT";
#endif /* ! __QNXNTO__ */
}

/*
 *  Interrupt exit callback function
 *  (class - _NTO_TRACE_INT, subclass - _NTO_TRACE_INT_HANDLER_EXIT)
 */
static int p_int_handler_ex(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
	(
	 g_out_stream,
	 "t:0x%08x CPU:%02d INT_HANDLER_EXIT:0x%08x (%d) SIGEVENT:%s\n",
	 t,
	 _NTO_TRACE_GETCPU(h),
	 CS32(b_p[0]),
	 CS32(b_p[0]),
//	 CS32(b_p[1]) == 0 ? "":print_sigevent( (struct sigevent *)&b_p[1] )
	 print_sigevent( tp_state, &b_p[1] )
	);

	return (0);
}

/*
 *  Thread state callback function
 *  (class - _NTO_TRACE_THREAD)
 */
static int p_th_st(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	(void) fprintf
		(
		 g_out_stream,
		 "t:0x%08x CPU:%02d THREAD  :%s pid:%d tid:%d ",
		 t,
		 _NTO_TRACE_GETCPU(h),
		 task_state[_NTO_TRACE_GETEVENT(h)], 
		 CS32(b_p[0]),
		 CS32(b_p[1])
		);
	// check for wide event 
	if ( b_s > 2 ) {
		(void) fprintf
		(
		 g_out_stream,
		 "priority:%d policy:%d",
		 CS32(b_p[2]),
		 CS32(b_p[3])
		);
		//following fields are not always present. check for each separately
		if ( b_s > 4 ) (void) fprintf( g_out_stream, " partition:%d", CS32(b_p[4])); 
		if ( b_s > 5 ) (void) fprintf( g_out_stream, " sched_flags:0x%08x", CS32(b_p[5])); 
	}
	(void) fprintf ( g_out_stream, "\n" );
	return (0);
}

/*
 *  Vthread state callback function
 *  (class - _NTO_TRACE_VTHREAD)
 */
static int p_vth_st(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("VTHREAD ", task_state[_NTO_TRACE_GETEVENT(h)-_TRACE_MAX_TH_STATE_NUM], "fDpid", "fDtid");
}

/*
 *  Process create callback function
 *  (class - _NTO_TRACE_PROCESS)
 */
static int p_pr_cr(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("PROCESS ", "PROCCREATE   ", "fDppid", "fDpid" );
}

/*
 *  Process create name callback function
 *  (class - _NTO_TRACE_PROCESS)
 */
static int p_pr_cr_n(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("PROCESS ", "PROCCREATE_NAME", "fDppid", "fDpid", "Nname");
}

/*
 *  Thread create callback function
 *  (class - _NTO_TRACE_THREAD)
 */
static int p_th_cr(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("THREAD  ", "THCREATE     ", "fDpid", "fDtid", "Dpolicy" );
}

/*
 *  Vthread create callback function
 *  (class - _NTO_TRACE_VTHREAD)
 */
static int p_vth_cr(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("VTHREAD ", "VTHCREATE    ", "fDpid", "fDtid");
}

/*
 *  Process destroy callback function
 *  (class - _NTO_TRACE_PROCESS)
 */
static int p_pr_des(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("PROCESS ", "PROCDESTROY  ", "fDppid", "fDpid");
}

/*
 *  Process destroy name callback function
 *  (class - _NTO_TRACE_PROCESS)
 */
static int p_pr_des_n(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("PROCESS ", "PROCDESTROY_NAME", "fDppid", "fDpid", "Nname");
}

/*
 *  Thread destroy callback function
 *  (class - _NTO_TRACE_THREAD)
 */
static int p_th_des(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("THREAD  ", "THDESTROY    ", "fDpid", "fDtid");
}

/*
 *  Vthread destroy callback function
 *  (class - _NTO_TRACE_VTHREAD)
 */
static int p_vth_des(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("VTHREAD ", "VTHDESTROY   ", "fDpid", "fDtid");
}

/*
 *  Process create name callback function
 *  (class - _NTO_TRACE_PROCESS)
 */
static int p_th_name(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("PROCESS ", "PROCTHREAD_NAME", "fDpid", "fDtid", "Nname");
}

/*
 *  Open pathname
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_pathmgr(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("SYSTEM  ", "PATHMGR_OPEN   ", "fDpid", "fDtid", "Npath");
}

static int s_address(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO("SYSTEM  ", "ADDRESS", "fHaddr");
}

/*
 *  MMAP message
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_mmap(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO9("SYSTEM  ", "MMAP           ", "fDpid", "fXvaddr", "fXlen", "fHflags", "Hprot", "Dfd", "Xalign", "Xoffset", "Npath");
}

/*
 *  MUNMAP message
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_munmap(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("SYSTEM  ", "MUNMAP         ", "fDpid", "fXvaddr", "fXlen");
}

/*
 *  Object name message
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_map_name(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO4("SYSTEM  ", "MAPNAME        ", "fDpid", "fHvaddr", "fHlen", "Nname");
}

/*
 *  APS partition name defined 
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_aps_name(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	if ( l == 2 ) l++;
	_TO2("SYSTEM  ", "APS_NAME       ", "fDpartition_id", "fNname");
}
/*
 *  Change is APS partition budget
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_aps_budgets(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("SYSTEM  ", "APS_NEW_BUDGET ", "fDpartition_id", "fDpercentage_budget", "Dcritical_budget");
}

/*
 *  APS partition exceeds critcal budget (bankruptcy)
 *  (class - _NTO_TRACE_SYSTEM)
 */
static int s_aps_bankruptcy(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("SYSTEM  ", "APS_BANKRUPTCY ", "fDpid", "fDtid", "fDpartition_id");
}

static int s_function_enter(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("SYSTEM  ", "FUNC_ENTER", "fHthisfn", "fHcall_site");
}
static int s_function_exit(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("SYSTEM  ", "FUNC_EXIT", "fHthisfn", "fHcall_site");
}
static int s_slog(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO3("SYSTEM  ", "SLOG", "fDopcode", "fDseverity", "Nmessage");
}

/*
 *  User class callback function
 *  (class - _NTO_TRACE_USER)
 */
static int usr_ev(tp_state_t tp_state, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	if(l<=2) {
		(void) fprintf
		(
		 g_out_stream,
		 "t:0x%08x CPU:%02d USREVENT:EVENT:%d, d0:0x%08x d1:0x%08x\n",
		 t,
		 _NTO_TRACE_GETCPU(h),
		 _NTO_TRACE_GETEVENT(h),
		 CS32(p[0]),
		 CS32(p[1])
		);
	} else {
		(void) fprintf
		(
		 g_out_stream,
		 "t:0x%08x CPU:%02d USREVENT:EVENT:%d STR:\"%s\"\n",
		 t,
		 _NTO_TRACE_GETCPU(h),
		 _NTO_TRACE_GETEVENT(h),
		 CK_PRN((l?(((char*)p)[(4*l)-1]='\0'),(char*)p:""))
		);
	}

	return (0);
}

/*
 *  Communication class callback function
 *  (class - _NTO_TRACE_COMM)
 */
static int comm_smsg(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_MESSAGE  ", "fHrcvid", "fDpid");
}

static int comm_spulse(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE    ", "fHscoid", "fDpid");
}

static int comm_rmsg(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "REC_MESSAGE  ", "fHrcvid", "fDpid");
}

static int comm_rpulse(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "REC_PULSE    ", "fHscoid", "fDpid");
}

static int comm_snd_pulse_exe(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE_EXE", "fHscoid", "fDpid");
}

static int comm_snd_pulse_dis(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE_DIS", "fHscoid", "fDpid");
}

static int comm_snd_pulse_dea(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE_DEA", "fHscoid", "fDpid");
}

static int comm_snd_pulse_un(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE_UN ", "fHscoid", "fDpid");
}

static int comm_snd_pulse_qun(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "SND_PULSE_QUN", "fHscoid", "fDpid");
}

static int comm_signal(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO9
	(
	 "COMM    ",
	 "SIGNAL       ",
	 "fDsi_signo",
	 "fDsi_code",
	 "Dsi_errno",
	 "H__pad[0]",
	 "H__pad[1]",
	 "H__pad[2]",
	 "H__pad[3]",
	 "H__pad[4]",
	 "H__pad[5]"
	);
}

static int comm_reply(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "REPLY_MESSAGE", "fDtid", "fDpid");
}

static int comm_msgerror(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	_TO2("COMM    ", "MSG_ERROR", "fDtid", "fDpid");
}

/*
 *  empty "NULL" callback function
 *  (class - _NTO_TRACE_EMPTY)
 */
static int null_callback(tp_state_t s, void* d, unsigned h, unsigned t, unsigned* p, unsigned l)
{
	if (g_verbose) {
		(void) fprintf(g_out_stream, "TRACEPRINTER: header file and syspage have been processed\n");
	}

	return (0);
}

int main(int argc, char **argv)
{
	int      opt     =0;
	unsigned tp_flags=_TRACEPARSER_DEBUG_HEADER|_TRACEPARSER_DEBUG_ERRORS;
	struct traceparser_state* tp_state;

	/* Initialize some static values */
	g_out_stream = stdout;
	g_err_stream = stderr;

	/* command line arguments */
	while((opt=getopt(argc, argv, "f:o:nv"))!=(-1)) {
		switch(opt)
		{
			case 'f': /* input trace file */
				g_input_file = optarg;
				break;
			case 'o': /* output file */
				g_out_file = optarg;
				break;
			case 'n': /* remove new line characters  */
				g_arg_format = " %s:";
				break;
			case 'v': /* verbose */
				g_verbose = 1;
				tp_flags  = _TRACEPARSER_DEBUG_ALL;
				break;
			default:  /* unknown */
				(void) fprintf
				(
				 g_err_stream,
				 "TRACEPRINTER: error parsing command-line arguments - exitting\n"
				);

				return (-1);
		}
	}

	/* Conditionally, open output stream and check if OK */
	if (g_out_file&&(g_out_stream=fopen(g_out_file, "w"))==NULL) {
		(void) fprintf
		(
		 g_err_stream,
		 "TRACEPRINTER: cannot open output file %s - exiting!\n",
		 g_out_file
		);

		exit (-1);
	}

	/* Printing version information */
	(void) fprintf(g_out_stream, "TRACEPRINTER version %d.%02d\n", VERSION_MAJOR, VERSION_MINOR);
	(void) fflush(g_out_stream);

	/* Initialize the traceparser state structure */
	if ((tp_state=traceparser_init(NULL))==NULL) {
		(void) fprintf
		(
		 g_err_stream,
		 "TRACEPRINTER: cannot initialize traceparser state structure - exiting!\n"
		);

		exit (-1);
	}

	/* Print header and errors to the stream */
	if (traceparser_debug(tp_state, g_out_stream, tp_flags)) {
		(void) fprintf
		(
		 g_err_stream,
		 "TRACEPRINTER: cannot set debug mode - exiting!\n"
		);

		exit (-1);
	}

	/* Setting "NULL" callback function */
	TP_CS(traceparser_cs(tp_state, NULL, null_callback, _NTO_TRACE_EMPTY, _NTO_TRACE_EMPTYEVENT));

	/* Setting kernel entry event callbacs */
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_nop_en, _NTO_TRACE_KERCALLENTER, __KER_NOP));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_trace_event_en, _NTO_TRACE_KERCALLENTER, __KER_TRACE_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_ring0_en, _NTO_TRACE_KERCALLENTER, __KER_RING0));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare3_en, _NTO_TRACE_KERCALLENTER, __KER_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare4_en, _NTO_TRACE_KERCALLENTER, __KER_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_get_en, _NTO_TRACE_KERCALLENTER, __KER_SYS_CPUPAGE_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_set_en, _NTO_TRACE_KERCALLENTER, __KER_SYS_CPUPAGE_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_SYS_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_NET_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendv_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_SENDV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendvnc_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_SENDVNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_error_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_ERROR));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivev_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_RECEIVEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_replyv_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_REPLYV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readv_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_READV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_writev_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_WRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readwritev_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_READWRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_info_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_send_pulse_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_SEND_PULSE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_deliver_event_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_DELIVER_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_keydata_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_KEYDATA));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readiov_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_READIOV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivepulsev_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_RECEIVEPULSEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_verify_event_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_VERIFY_EVENT));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_kill_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_return_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_RETURN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_fault_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_FAULT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_action_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_ACTION));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_procmask_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_PROCMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_suspend_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_SUSPEND));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_waitinfo_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_WAITINFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_SIGNAL_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_create_en, _NTO_TRACE_KERCALLENTER, __KER_CHANNEL_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_destroy_en, _NTO_TRACE_KERCALLENTER, __KER_CHANNEL_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_CHANNEL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_chancon_attr_en, _NTO_TRACE_KERCALLENTER, __KER_CHANCON_ATTR));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_attach_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_detach_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_server_info_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_SERVER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_client_info_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_CLIENT_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_flags_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_FLAGS));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_CONNECT_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_create_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroy_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroyall_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_DESTROYALL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_detach_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_join_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_JOIN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_cancel_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_CANCEL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_ctl_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_THREAD_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_attach_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_func_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_DETACH_FUNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_wait_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_mask_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_MASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_unmask_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_UNMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare3_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare4_en, _NTO_TRACE_KERCALLENTER, __KER_INTERRUPT_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_time_en, _NTO_TRACE_KERCALLENTER, __KER_CLOCK_TIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_adjust_en, _NTO_TRACE_KERCALLENTER, __KER_CLOCK_ADJUST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_period_en, _NTO_TRACE_KERCALLENTER, __KER_CLOCK_PERIOD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_id_en, _NTO_TRACE_KERCALLENTER, __KER_CLOCK_ID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_CLOCK_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_create_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_destroy_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_settime_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_SETTIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_info_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_alarm_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_ALARM));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_timeout_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_TIMEOUT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare2_en, _NTO_TRACE_KERCALLENTER, __KER_TIMER_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_create_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_destroy_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_lock_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_MUTEX_LOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_unlock_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_MUTEX_UNLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_wait_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_CONDVAR_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_signal_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_CONDVAR_SIGNAL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_post_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_SEM_POST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_wait_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_SEM_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_ctl_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_revive_en, _NTO_TRACE_KERCALLENTER, __KER_SYNC_MUTEX_REVIVE));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_get_en, _NTO_TRACE_KERCALLENTER, __KER_SCHED_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_set_en, _NTO_TRACE_KERCALLENTER, __KER_SCHED_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_yield_en, _NTO_TRACE_KERCALLENTER, __KER_SCHED_YIELD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_info_en, _NTO_TRACE_KERCALLENTER, __KER_SCHED_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_ctl_en, _NTO_TRACE_KERCALLENTER, __KER_SCHED_CTL));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_cred_en, _NTO_TRACE_KERCALLENTER, __KER_NET_CRED));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_vtid_en, _NTO_TRACE_KERCALLENTER, __KER_NET_VTID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_unblock_en, _NTO_TRACE_KERCALLENTER, __KER_NET_UNBLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_infoscoid_en, _NTO_TRACE_KERCALLENTER, __KER_NET_INFOSCOID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_signal_kill_en, _NTO_TRACE_KERCALLENTER, __KER_NET_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_current_en, _NTO_TRACE_KERCALLENTER, __KER_MSG_CURRENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_spare1_en, _NTO_TRACE_KERCALLENTER, __KER_NET_SPARE1));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_bad_en, _NTO_TRACE_KERCALLENTER, __KER_BAD));

	/* Setting kernel exit event callbacs */
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_nop_ex, _NTO_TRACE_KERCALLEXIT, __KER_NOP));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_trace_event_ex, _NTO_TRACE_KERCALLEXIT, __KER_TRACE_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_ring0_ex, _NTO_TRACE_KERCALLEXIT, __KER_RING0));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare3_ex, _NTO_TRACE_KERCALLEXIT, __KER_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare4_ex, _NTO_TRACE_KERCALLEXIT, __KER_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_get_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYS_CPUPAGE_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_set_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYS_CPUPAGE_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYS_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendv_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_SENDV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendvnc_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_SENDVNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_error_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_ERROR));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivev_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_RECEIVEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_replyv_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_REPLYV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readv_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_READV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_writev_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_WRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readwritev_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_READWRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_info_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_send_pulse_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_SEND_PULSE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_deliver_event_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_DELIVER_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_keydata_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_KEYDATA));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readiov_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_READIOV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivepulsev_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_RECEIVEPULSEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_verify_event_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_VERIFY_EVENT));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_kill_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_return_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_RETURN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_fault_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_FAULT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_action_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_ACTION));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_procmask_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_PROCMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_suspend_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_SUSPEND));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_waitinfo_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_WAITINFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_SIGNAL_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_create_ex, _NTO_TRACE_KERCALLEXIT, __KER_CHANNEL_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_destroy_ex, _NTO_TRACE_KERCALLEXIT, __KER_CHANNEL_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_CHANNEL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_chancon_attr_ex, _NTO_TRACE_KERCALLEXIT, __KER_CHANCON_ATTR));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_attach_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_detach_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_server_info_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_SERVER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_client_info_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_CLIENT_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_flags_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_FLAGS));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_CONNECT_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_create_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroy_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroyall_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_DESTROYALL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_detach_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_join_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_JOIN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_cancel_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_CANCEL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_ctl_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_THREAD_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_attach_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_func_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_DETACH_FUNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_wait_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_mask_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_MASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_unmask_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_UNMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare3_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare4_ex, _NTO_TRACE_KERCALLEXIT, __KER_INTERRUPT_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_time_ex, _NTO_TRACE_KERCALLEXIT, __KER_CLOCK_TIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_adjust_ex, _NTO_TRACE_KERCALLEXIT, __KER_CLOCK_ADJUST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_period_ex, _NTO_TRACE_KERCALLEXIT, __KER_CLOCK_PERIOD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_id_ex, _NTO_TRACE_KERCALLEXIT, __KER_CLOCK_ID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_CLOCK_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_create_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_destroy_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_settime_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_SETTIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_info_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_alarm_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_ALARM));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_timeout_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_TIMEOUT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare2_ex, _NTO_TRACE_KERCALLEXIT, __KER_TIMER_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_create_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_destroy_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_lock_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_MUTEX_LOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_unlock_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_MUTEX_UNLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_wait_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_CONDVAR_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_signal_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_CONDVAR_SIGNAL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_post_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_SEM_POST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_wait_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_SEM_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_ctl_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_revive_ex, _NTO_TRACE_KERCALLEXIT, __KER_SYNC_MUTEX_REVIVE));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_get_ex, _NTO_TRACE_KERCALLEXIT, __KER_SCHED_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_set_ex, _NTO_TRACE_KERCALLEXIT, __KER_SCHED_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_yield_ex, _NTO_TRACE_KERCALLEXIT, __KER_SCHED_YIELD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_info_ex, _NTO_TRACE_KERCALLEXIT, __KER_SCHED_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_ctl_ex, _NTO_TRACE_KERCALLEXIT, __KER_SCHED_CTL));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_cred_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_CRED));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_vtid_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_VTID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_unblock_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_UNBLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_infoscoid_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_INFOSCOID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_signal_kill_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_current_ex, _NTO_TRACE_KERCALLEXIT, __KER_MSG_CURRENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_spare1_ex, _NTO_TRACE_KERCALLEXIT, __KER_NET_SPARE1));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_bad_ex, _NTO_TRACE_KERCALLEXIT, __KER_BAD));

	/* Setting interrupted kernel exit event callbacs  */
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_nop_ix, _NTO_TRACE_KERCALLINT, __KER_NOP));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_trace_event_ix, _NTO_TRACE_KERCALLINT, __KER_TRACE_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_ring0_ix, _NTO_TRACE_KERCALLINT, __KER_RING0));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare3_ix, _NTO_TRACE_KERCALLINT, __KER_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_spare4_ix, _NTO_TRACE_KERCALLINT, __KER_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_get_ix, _NTO_TRACE_KERCALLINT, __KER_SYS_CPUPAGE_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_cpupage_set_ix, _NTO_TRACE_KERCALLINT, __KER_SYS_CPUPAGE_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_SYS_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sys_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_NET_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendv_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_SENDV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_sendvnc_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_SENDVNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_error_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_ERROR));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivev_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_RECEIVEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_replyv_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_REPLYV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readv_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_READV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_writev_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_WRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readwritev_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_READWRITEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_info_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_send_pulse_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_SEND_PULSE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_deliver_event_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_DELIVER_EVENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_keydata_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_KEYDATA));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_readiov_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_READIOV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_receivepulsev_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_RECEIVEPULSEV));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_verify_event_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_VERIFY_EVENT));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_kill_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_return_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_RETURN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_fault_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_FAULT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_action_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_ACTION));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_procmask_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_PROCMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_suspend_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_SUSPEND));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_waitinfo_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_WAITINFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_signal_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_SIGNAL_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_create_ix, _NTO_TRACE_KERCALLINT, __KER_CHANNEL_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_destroy_ix, _NTO_TRACE_KERCALLINT, __KER_CHANNEL_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_channel_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_CHANNEL_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_chancon_attr_ix, _NTO_TRACE_KERCALLINT, __KER_CHANCON_ATTR));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_attach_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_detach_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_server_info_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_SERVER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_client_info_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_CLIENT_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_flags_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_FLAGS));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_connect_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_CONNECT_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_create_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroy_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_destroyall_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_DESTROYALL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_detach_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_join_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_JOIN));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_cancel_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_CANCEL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_ctl_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_thread_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_THREAD_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_attach_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_ATTACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_func_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_DETACH_FUNC));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_detach_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_DETACH));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_wait_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_mask_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_MASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_unmask_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_UNMASK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_SPARE2));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare3_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_SPARE3));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_interrupt_spare4_ix, _NTO_TRACE_KERCALLINT, __KER_INTERRUPT_SPARE4));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_time_ix, _NTO_TRACE_KERCALLINT, __KER_CLOCK_TIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_adjust_ix, _NTO_TRACE_KERCALLINT, __KER_CLOCK_ADJUST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_period_ix, _NTO_TRACE_KERCALLINT, __KER_CLOCK_PERIOD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_id_ix, _NTO_TRACE_KERCALLINT, __KER_CLOCK_ID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_clock_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_CLOCK_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_create_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_destroy_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_settime_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_SETTIME));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_info_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_alarm_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_ALARM));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_timeout_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_TIMEOUT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_SPARE1));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_timer_spare2_ix, _NTO_TRACE_KERCALLINT, __KER_TIMER_SPARE2));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_create_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_CREATE));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_destroy_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_DESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_lock_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_MUTEX_LOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_unlock_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_MUTEX_UNLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_wait_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_CONDVAR_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_condvar_signal_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_CONDVAR_SIGNAL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_post_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_SEM_POST));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_sem_wait_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_SEM_WAIT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_ctl_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_CTL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sync_mutex_revive_ix, _NTO_TRACE_KERCALLINT, __KER_SYNC_MUTEX_REVIVE));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_get_ix, _NTO_TRACE_KERCALLINT, __KER_SCHED_GET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_set_ix, _NTO_TRACE_KERCALLINT, __KER_SCHED_SET));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_yield_ix, _NTO_TRACE_KERCALLINT, __KER_SCHED_YIELD));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_info_ix, _NTO_TRACE_KERCALLINT, __KER_SCHED_INFO));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_sched_ctl_ix, _NTO_TRACE_KERCALLINT, __KER_SCHED_CTL));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_cred_ix, _NTO_TRACE_KERCALLINT, __KER_NET_CRED));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_vtid_ix, _NTO_TRACE_KERCALLINT, __KER_NET_VTID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_unblock_ix, _NTO_TRACE_KERCALLINT, __KER_NET_UNBLOCK));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_infoscoid_ix, _NTO_TRACE_KERCALLINT, __KER_NET_INFOSCOID));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_signal_kill_ix, _NTO_TRACE_KERCALLINT, __KER_NET_SIGNAL_KILL));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_msg_current_ix, _NTO_TRACE_KERCALLINT, __KER_MSG_CURRENT));
	TP_CS(traceparser_cs(tp_state, NULL, c_ker_net_spare1_ix, _NTO_TRACE_KERCALLINT, __KER_NET_SPARE1));

	TP_CS(traceparser_cs(tp_state, NULL, c_ker_bad_ix, _NTO_TRACE_KERCALLINT, __KER_BAD));

	/* Setting control event class callbacs */
	TP_CS(traceparser_cs(tp_state, NULL, p_time_set, _NTO_TRACE_CONTROL, _NTO_TRACE_CONTROLTIME));
	TP_CS(traceparser_cs(tp_state, NULL, p_buffer, _NTO_TRACE_CONTROL, _NTO_TRACE_CONTROLBUFFER));

	/* Setting interrupt event class callbacs */
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_int_en, _NTO_TRACE_INTENTER, _NTO_TRACE_INTFIRST, _NTO_TRACE_INTLAST));
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_int_ex, _NTO_TRACE_INTEXIT, _NTO_TRACE_INTFIRST, _NTO_TRACE_INTLAST));
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_int_handler_en, _NTO_TRACE_INT_HANDLER_ENTER, _NTO_TRACE_INTFIRST, _NTO_TRACE_INTLAST));
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_int_handler_ex, _NTO_TRACE_INT_HANDLER_EXIT, _NTO_TRACE_INTFIRST, _NTO_TRACE_INTLAST));

	/* Setting process/thread event class callbacs */
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_th_st, _NTO_TRACE_THREAD, _NTO_TRACE_THDEAD, _NTO_TRACE_THNET_REPLY));
	TP_CSR(traceparser_cs_range(tp_state, NULL, p_vth_st, _NTO_TRACE_VTHREAD, _NTO_TRACE_VTHDEAD, _NTO_TRACE_VTHNET_REPLY));
	TP_CS(traceparser_cs(tp_state, NULL, p_pr_cr, _NTO_TRACE_PROCESS, _NTO_TRACE_PROCCREATE));
	TP_CS(traceparser_cs(tp_state, NULL, p_pr_cr_n, _NTO_TRACE_PROCESS, _NTO_TRACE_PROCCREATE_NAME));
	TP_CS(traceparser_cs(tp_state, NULL, p_th_cr, _NTO_TRACE_THREAD, _NTO_TRACE_THCREATE));
	TP_CS(traceparser_cs(tp_state, NULL, p_vth_cr, _NTO_TRACE_VTHREAD, _NTO_TRACE_VTHCREATE));
	TP_CS(traceparser_cs(tp_state, NULL, p_pr_des, _NTO_TRACE_PROCESS, _NTO_TRACE_PROCDESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, p_pr_des_n, _NTO_TRACE_PROCESS, _NTO_TRACE_PROCDESTROY_NAME));
	TP_CS(traceparser_cs(tp_state, NULL, p_th_des, _NTO_TRACE_THREAD, _NTO_TRACE_THDESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, p_vth_des, _NTO_TRACE_VTHREAD, _NTO_TRACE_VTHDESTROY));
	TP_CS(traceparser_cs(tp_state, NULL, p_th_name, _NTO_TRACE_PROCESS, _NTO_TRACE_PROCTHREAD_NAME));
	TP_CSR(traceparser_cs_range(tp_state, NULL, usr_ev, _NTO_TRACE_USER, _NTO_TRACE_USERFIRST, _NTO_TRACE_USERLAST));

	/* System level events */
	TP_CS(traceparser_cs(tp_state, NULL, s_pathmgr, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_PATHMGR));
	TP_CS(traceparser_cs(tp_state, NULL, s_aps_name, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_APS_NAME));
	TP_CS(traceparser_cs(tp_state, NULL, s_aps_budgets, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_APS_BUDGETS));
	TP_CS(traceparser_cs(tp_state, NULL, s_aps_bankruptcy, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_APS_BNKR));
	TP_CS(traceparser_cs(tp_state, NULL, s_mmap, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_MMAP));
	TP_CS(traceparser_cs(tp_state, NULL, s_munmap, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_MUNMAP));
	TP_CS(traceparser_cs(tp_state, NULL, s_map_name, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_MAPNAME));
	TP_CS(traceparser_cs(tp_state, NULL, s_address, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_ADDRESS));
	TP_CS(traceparser_cs(tp_state, NULL, s_function_enter, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_FUNC_ENTER));
	TP_CS(traceparser_cs(tp_state, NULL, s_function_exit, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_FUNC_EXIT));
	TP_CS(traceparser_cs(tp_state, NULL, s_slog, _NTO_TRACE_SYSTEM, _NTO_TRACE_SYS_SLOG));

	/* Setting communication event class callbacs */
	TP_CS(traceparser_cs(tp_state, NULL, comm_smsg, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SMSG));
	TP_CS(traceparser_cs(tp_state, NULL, comm_spulse, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE));
	TP_CS(traceparser_cs(tp_state, NULL, comm_rmsg, _NTO_TRACE_COMM, _NTO_TRACE_COMM_RMSG));
	TP_CS(traceparser_cs(tp_state, NULL, comm_rpulse, _NTO_TRACE_COMM, _NTO_TRACE_COMM_RPULSE));
	TP_CS(traceparser_cs(tp_state, NULL, comm_snd_pulse_exe, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE_EXE));
	TP_CS(traceparser_cs(tp_state, NULL, comm_snd_pulse_dis, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE_DIS));
	TP_CS(traceparser_cs(tp_state, NULL, comm_snd_pulse_dea, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE_DEA));
	TP_CS(traceparser_cs(tp_state, NULL, comm_snd_pulse_un, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE_UN));
	TP_CS(traceparser_cs(tp_state, NULL, comm_snd_pulse_qun, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SPULSE_QUN));
	TP_CS(traceparser_cs(tp_state, NULL, comm_signal, _NTO_TRACE_COMM, _NTO_TRACE_COMM_SIGNAL));
	TP_CS(traceparser_cs(tp_state, NULL, comm_reply, _NTO_TRACE_COMM, _NTO_TRACE_COMM_REPLY));
	TP_CS(traceparser_cs(tp_state, NULL, comm_msgerror, _NTO_TRACE_COMM, _NTO_TRACE_COMM_ERROR));


	if(opt=traceparser(tp_state, NULL, g_input_file))
	{
		(void) fprintf
		(
		 g_err_stream,
		 "TRACEPARSER: function call traceparser() failed, internal error code(%d)\n",
		 *(unsigned*)traceparser_get_info(tp_state, _TRACEPARSER_INFO_ERROR, NULL)
		);
		traceparser_destroy(&tp_state);

		exit (-1);
	}
	traceparser_destroy(&tp_state);
	if (g_out_file) {
		return (fclose(g_out_stream));
	} else {
		return (0);
	}
}

#ifdef __QNXNTO__
__SRCVERSION("traceprinter.c $Rev: 173790 $");
#endif
