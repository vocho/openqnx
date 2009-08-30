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



#include "pidin.h"


/* Remember that the entries in this structure are position dependent.
 * So if new entries are being used or entries are being modified, 
 * always ensure that the position of a format letter in the structure
 * remains the same as it always has been. Code in other places in pidin
 * attempts to validate that the format letter and the index into the 
 * array match are in sync. */
struct format formats[256] = {
/* ^@ */	{ -1, 0, 0, 0},
/* ^a */	{ -1, 0, 0, 0},
/* ^b */	{ -1, 0, 0, 0},
/* ^c */	{ -1, 0, 0, 0},
/* ^d */	{ -1, 0, 0, 0},
/* ^e */	{ -1, 0, 0, 0},
/* ^f */	{ -1, 0, 0, 0},
/* ^g */	{ -1, 0, 0, 0},
/* ^h */	{ -1, 0, 0, 0},
/* ^i */	{ -1, 0, 0, 0},
/* ^j */	{ -1, 0, 0, 0},
/* ^k */	{ -1, 0, 0, 0},
/* ^l */	{ -1, 0, 0, 0},
/* ^m */	{ -1, 0, 0, 0},
/* ^n */	{ -1, 0, 0, 0},
/* ^o */	{ -1, 0, 0, 0},
/* ^p */	{ -1, 0, 0, 0},
/* ^q */	{ -1, 0, 0, 0},
/* ^r */	{ -1, 0, 0, 0},
/* ^s */	{ -1, 0, 0, 0},
/* ^t */	{ -1, 0, 0, 0},
/* ^u */	{ -1, 0, 0, 0},
/* ^z */	{ -1, 0, 0, 0},
/* ^w */	{ -1, 0, 0, 0},
/* ^x */	{ -1, 0, 0, 0},
/* ^y */	{ -1, 0, 0, 0},
/* ^z */	{ -1, 0, 0, 0},
/* ^[ */	{ -1, 0, 0, 0},
/* ^\ */	{ -1, 0, 0, 0},
/* ^] */	{ -1, 0, 0, 0},
/* ^^ */	{ -1, 0, 0, 0},
/* ^_ */	{ -1, 0, 0, 0},
/*   */		{ -1, 0, 0, 0},
/* ! */		{ -1, 0, 0, 0},
/* " */		{ -1, 0, 0, 0},
/* # */		{ -1, 0, 0, 0},
/* $ */		{ -1, 0, 0, 0},
/* % */		{ -1, 0, 0, 0},
/* & */		{ -1, 0, 0, 0},
/* ' */		{ -1, 0, 0, 0},
/* ( */		{ -1, 0, 0, 0},
/* ) */		{ -1, 0, 0, 0},
/* * */		{ -1, 0, 0, 0},
/* + */		{ -1, 0, 0, 0},
/* , */		{ -1, 0, 0, 0},
/* - */		{ -1, 0, 0, 0},
/* . */		{ -1, 0, 0, 0},
/* / */		{ -1, 0, 0, 0},
/* 0 */		{ -1, 0, 0, 0},
/* 1 */		{ -1, 0, 0, 0},
/* 2 */		{ -1, 0, 0, 0},
/* 3 */		{ -1, 0, 0, 0},
/* 4 */		{ -1, 0, 0, 0},
/* 5 */		{ -1, 0, 0, 0},
/* 6 */		{ -1, 0, 0, 0},
/* 7 */		{ -1, 0, 0, 0},
/* 8 */		{ -1, 0, 0, 0},
/* 9 */		{ -1, 0, 0, 0},
/* : */		{ 18, "MemoryPhys", MemoryPhys, PRESERVE_RIGHT, ':'},
/* ; */		{ 10, "OffsetPhys", MemObjectOffsetPhys, 0, ';'},
/* < */		{ 5, "Code", MemObjectCode, 0, '<'},
/* = */		{ 5, "Data", MemObjectData, 0, '='},
/* > */		{ 8, "Mapaddr", MemObjectMapAddr, 0, '>'},
/* ? */		{ 10, "Offset", MemObjectOffset, 0, '?'},
/* @ */		{ 12, "Flags", MemObjectFlags, 0, '@'},
/* A */		{  9, "Arguments", Arguments, TITLE_LEFT_JUSTIFIED|ZI, 'A'},
/* B */		{ 28, "Blocked", WhereBlocked, TITLE_LEFT_JUSTIFIED|DATA_LEFT_JUSTIFIED|ZI|TU, 'B'},
/* C */		{  7, "child", Child, TITLE_RIGHT_JUSTIFIED, 'C'},
/* D */		{ 16, "Debug flags", DebugFlags, TITLE_LEFT_JUSTIFIED|ZI, 'D'},
/* E */		{ 11, "Environment", Environment, TITLE_LEFT_JUSTIFIED|ZI, 'E'},
/* F */		{ 16, "Thread flags", ThreadFlags, TITLE_LEFT_JUSTIFIED|ZI|TU, 'F'},
/* G */		{  7, "sibling", Sibling, TITLE_RIGHT_JUSTIFIED, 'G'},
/* H */		{ 20, "ExtSched", ExtSched, TITLE_LEFT_JUSTIFIED|DATA_LEFT_JUSTIFIED|ZI|TU, 'H'},
/* I */		{ 11, "pid-tid", PidTidField, /*DATA_FILL|*/TITLE_LEFT_JUSTIFIED|ZI|TU, 'I'},
/* J */		{ 11, "STATE", State, TITLE_LEFT_JUSTIFIED|ZI|TU, 'J'},
/* K */		{ 17, "kernel call", KerCall, TITLE_LEFT_JUSTIFIED|ZI|TU, 'K'},
/* L */		{  7, "sid", Sid, TITLE_RIGHT_JUSTIFIED, 'L'},
/* M */		{ 18, "Memory", Memory, PRESERVE_RIGHT, 'M'},
/* N */		{ 18, "name", Name, TITLE_LEFT_JUSTIFIED|PRESERVE_RIGHT|ZI|NA, 'N'},
/* O */		{ -1, 0, 0, 0},
/* P */		{  7, "pgrp", Pgrp, TITLE_RIGHT_JUSTIFIED, 'P'},
/* Q */		{ 40, "", Interrupt, 0, 'Q'}, /* Interrupts */
/* R */		{ 40, "", Timers, MULTI_LINE, 'R'}, /* Timers */
/* S */		{ 16, "signals ignored", SigIgnore, TU, 'S'},
/* T */		{  2, "nT", NumThreads, TITLE_LEFT_JUSTIFIED|ZI, 'T'},
/* U */		{  6, "uid", Uid, TITLE_RIGHT_JUSTIFIED, 'U'},
/* V */		{  6, "gid", Gid, TITLE_RIGHT_JUSTIFIED, 'V'},
/* W */		{  6, "euid", EUid, TITLE_RIGHT_JUSTIFIED, 'W'},
/* X */		{  6, "egid", EGid, TITLE_RIGHT_JUSTIFIED, 'X'},
/* Y */		{  6, "suid", SUid, TITLE_RIGHT_JUSTIFIED, 'Y'},
/* Z */		{  6, "sgid", SGid, TITLE_RIGHT_JUSTIFIED, 'Z'},
/* [ */		{ 40, "", Channels, 0, '['}, /* Channels */
/* \ */		{ -1, 0, 0, 0},
/* ] */		{ -1, 0, 0, 0},
/* ^ */		{ -1, 0, 0, 0},
/* _ */		{ -1, 0, 0, 0},
/* ` */		{ -1, 0, 0, 0},
/* a */		{  8, "pid", pid, TITLE_RIGHT_JUSTIFIED, 'a'},
/* b */		{  3, "tid", tid, TITLE_LEFT_JUSTIFIED|ZI|TU, 'b'},
/* c */		{  5, "code", codesize, ZI|TITLE_RIGHT_JUSTIFIED, 'c'},
/* d */		{  5, "data", datasize, ZI|TITLE_RIGHT_JUSTIFIED, 'd'},
/* e */		{  7, "ppid", parentpid, TITLE_RIGHT_JUSTIFIED, 'e'},
/* f */		{ 16, "Flags", ProcessFlags, TITLE_LEFT_JUSTIFIED, 'f'},
/* g */		{ -1, 0, 0, 0},
/* h */		{ 20, "thread name", ThreadName, TITLE_LEFT_JUSTIFIED|PRESERVE_RIGHT|ZI|NA, 'h'},
/* i */		{ 40, "", Rmasks, MULTI_LINE, 'i'},
/* j */		{ -1, 0, 0, 0},
/* k */		{ -1, 0, 0, 0},
/* l */		{ 3, "cpu", LastCPU, TU, 'l'},
/* m */		{  12, "stack", stacksize, ZI|TU|TITLE_RIGHT_JUSTIFIED, 'm'},
/* n */		{  4, "name", long_name, TITLE_LEFT_JUSTIFIED|ZI, 'n'},
/* o */		{ 40, "", Coids, ZI|MULTI_LINE|PROCESS_ONLY, 'o'},
/* p */		{  4, "prio", priority, ZI|TU, 'p'},
#ifndef NO_BACKTRACE_LIB
/* q */		{ 60, "backtrace", ThreadBacktrace, TITLE_LEFT_JUSTIFIED|DATA_LEFT_JUSTIFIED, 'q'},
#else
/* q */		{ -1, 0, 0, 0},
#endif
/* r */		{ 40, "", Registers, 0, 'r'},
/* s */		{ 16, "signals pending", SigPending, ZI|TU, 's'},
/* t */		{ 12, "start time", ProcessStartTime, 0, 't'},
/* u */		{ 6, "utime", ProcessUtime, TITLE_RIGHT_JUSTIFIED, 'u'},
/* v */		{ 6, "stime", ProcessStime, TITLE_RIGHT_JUSTIFIED, 'v'},
/* w */		{ 6, "cutime", ProcessCutime, TITLE_RIGHT_JUSTIFIED, 'w'},
/* x */		{ 6, "cstime", ProcessCstime, TITLE_RIGHT_JUSTIFIED, 'x'},
/* y */		{ 12, "thread start", ThreadStartTime, TU, 'y'},
/* z */		{ 6, "sutime", ThreadSUtime, TU|TITLE_RIGHT_JUSTIFIED, 'z'},
/* If new Entries are added after 'z', remember to read the comments at the
 * beginning of this format structure, and also appropriately update the
 * validate_format function in pidin.c */
/* { */		{ -1, 0, 0, 0},
/* | */		{ -1, 0, 0, 0},
/* } */		{ -1, 0, 0, 0},
/* ~ */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{  0, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
/* ? */		{ -1, 0, 0, 0},
};

char const		   *const kc_names[] =
{
	"nop",	
	"trace_event",	
	"ring0",	
	"???",
	"???",
	"???",
	"???",
	
	"sys_cpupage_get",
	"sys_cpupage_set",
	"???",
	"???",
	
	"msg_sendv",
	"msg_sendvnc",
	"msg_error",
	"msg_receivev",
	"msg_replyv",
	"msg_readv",
	"msg_writev",
	"msg_readwritev",
	"msg_info",
	"msg_send_pulse",
	"msg_deliver_event",
	"msg_keydata",
	"msg_readiov",
	"msg_receivepulsev",
	"msg_verifyevent",
	
	"signal_kill",
	"signal_return",
	"signal_fault",
	"signal_action",
	"signal_procmask",
	"signal_suspend",
	"signal_waitinfo",
	"???",
	"???",
	
	"channel_create",
	"channel_destroy",
	"???",
	"???",
	
	"connect_attach",
	"connect_detach",
	"connect_server_info",
	"connect_client_info",
	"connect_flags",
	"???",
	"???",
	
	"thread_create",
	"thread_destroy",
	"thread_destroyall",
	"thread_detach",
	"thread_join",
	"thread_cancel",
	"thread_ctl",
	"???",
	"???",
	
	"interrupt_attach",
	"interrupt_detach_func",
	"interrupt_detach",
	"interrupt_wait",
	"interrupt_mask",
	"interrupt_unmask",
	"???",
	"???",
	"???",
	"???",
	
	"clock_time",
	"clock_adjust",
	"clock_period",
	"clocl_id",
	"???",
	
	"timer_create",
	"timer_destroy",
	"timer_settime",
	"timer_info",
	"timer_alarm",
	"timer_timeout",
	"???",
	"???",
	
	"sync_create",
	"sync_destroy",
	"sync_mutex_lock",
	"sync_mutex_unlock",
	"sync_condvar_wait",
	"sync_condvar_signal",
	"sync_sem_post",
	"sync_sem_wait",
	"sync_ctl",
	"sync_mutex_revive",
	
	"sched_get",
	"sched_set",
	"sched_yield",
	"sched_info",
	"???",
	
	"net_cred",
	"net_vtid",
	"net_unblock",
	"net_infoscoid",
	"net_signal_kill",
	"???"
};

const char		   *const thread_state[] =
{
		"DEAD",
		"RUNNING",
		"READY",
		"STOPPED",
		"SEND",
		"RECEIVE",
		"REPLY",
		"STACK",
		"WAITTHREAD",
		"WAITPAGE",
		"SIGSUSPEND",
		"SIGWAITINFO",
		"NANOSLEEP",
		"MUTEX",
		"CONDVAR",
		"JOIN",
		"INTR",
		"SEM",
		"WAITCTX",
		"NET_SEND",
		"NET_REPLY"
};

#ifdef __X86__
#define REGNAME_LIST \
	{	"edi", "esi", "ebp", "exx", "ebx", "edx", "ecx", "eax", \
		"eip", "cs", "efl", "esp", "ss",NULL }
#elif __PPC__
#define REGNAME_LIST \
	{	"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9", \
		"r10","r11","r12","r13","r14","r15","r16","r17","r18", \
		"r19","r20","r21","r22","r23","r24","r25","r26","r27", \
		"r28","r29","r30","r31", \
		"ctr","lr","msr","iar","cr","xer","ear","u","u2", NULL }
#elif __MIPS__
#define REGNAME_LIST \
	{	"r0","at","v0","v1","a0","a1","a2","a3","t0","t1","t2", \
		"t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5", \
		"s6","s7","t8","t9","k0","k1","gp","sp","s8","ra","sreg", \
		"lo","hi","bv","ca","epc", NULL }
/* Mips has 64bit reg contexts, even for 32bit registers */
#elif __SH__
#define REGNAME_LIST \
	{	"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9", \
		"r10","r11","r12","r13","r14","r15","sr","pc","gbr", \
		"mach","macl","pr", NULL }
#elif __ARM__
#define REGNAME_LIST \
	{	"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9", \
		"r10","fp","ip","sp","lr","pc","spsr", NULL}
#else
#define REGNAME_LIST { NULL	} /* empty register name list, display "--" */
#endif

const char* regnames[] = REGNAME_LIST;
const int nregnames = (sizeof(regnames)/sizeof(char*)) -1; /* -1 for NULL */


const int		num_thread_states = sizeof(thread_state) / sizeof(thread_state[0]);

char const	   *const spaces = "                                                                         ";
char const	   *const zeros = "000000000000000000000000000000000000000000000000000000000000000";
char const         *const na = "(n/a)";
procfs_debuginfo	name;
procfs_mapinfo		mem;
procfs_status		status;

__SRCVERSION("pidin_data.c $Rev: 209166 $");
