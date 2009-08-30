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


#include "kdebug.h"
#include <unistd.h>
#include <process.h>
#include <sys/image.h>
#include <sys/startup.h>

int 			debug_flag = 0;
char 			progname[16] = "kdebug";
char 			*debug_arg;
int 			force_use_image = 0;
int				async_check = 1;
int				interesting_faults = SIGCODE_FATAL | SIGCODE_INTR | SIGCODE_KERNEL | SIGCODE_PROC;
int				all_sigtraps = 0;
char 			mountpt[100] = "/usr/nto";
int				protocol = 0;
int 			(*old_timer_reload)(struct syspage_entry *, struct qtime_entry *);
struct debug_callout *channel0;
struct debug_callout *hlink;

CPU_REGISTERS			kerdebug_reg;
struct cpu_extra_state	extra_state;

unsigned	ser_clk;
unsigned	ser_div;

struct kdebug_callback kdebug_callbacks = {
	KDEBUG_CURRENT,
	sizeof( struct kdebug_callback ),
	outside_watch_entry,
	outside_fault_entry,
	outside_msg_entry,
	outside_update_plist,
	&extra_state
};

static int
dummy_timer_reload(struct syspage_entry *sysp, struct qtime_entry *qtime) {
	return 1;
}

static void
dummy_display_char(struct syspage_entry *sysp, char c) {
}

static int
dummy_poll_key(struct syspage_entry *sysp) {
	return -1;
}

static int
dummy_break_detect(struct syspage_entry *sysp) {
	return 0;
}

int
main(int argc, char **argv) {
	int 					t;
	uintptr_t  				start_vaddr;
	int						kerdbg = 0;
	struct callout_entry	*callout;

	cpu_init();
	if(strlen(argv[0]) < sizeof progname - 1) {
		strcpy(progname, argv[0]);
	}

	cpu_init_map();
	cpu_init_extra(&extra_state);

	while((t = getopt(argc, argv, "ac:IKvD:m:rP:TU")) != -1) {
		switch(t) {
		case 'a':
			async_check = 0;
			break;
		case 'c':
			ser_clk = strtoul(optarg, &optarg, 10);
			if(*optarg == '/') {
				ser_div = strtoul(optarg + 1, NULL, 10);
			}
			break;
		case 'I':
			force_use_image = !force_use_image;
			break;
		case 'm':
			strncpy(mountpt, optarg, sizeof mountpt);
			break;
		case 'v':
			++debug_flag;
			break;
		case 'K':
			++kerdbg;
			break;
		case 'D':
			if(*optarg)
				debug_arg = optarg;
			break;
		case 'U':
			interesting_faults |= SIGCODE_USER;
			break;
		case 'T':
			all_sigtraps = 1;
			break;
		case 'P':
			protocol = strtoul(optarg, &optarg, 10);
			break;
		}
	}

	callout = SYSPAGE_ENTRY(callout);
	hlink = channel0 = &callout->debug[0];
	if(debug_arg == NULL) ++hlink;
	kprintf_init();

	init_traps();

	start_vaddr = next_bootstrap();
	if(start_vaddr == 0) {
		kprintf("%s: No next program to start.\n", progname);
		_exit(1);
	}

	if(hlink->display_char == NULL || hlink->poll_key == NULL) {
//		kprintf("\nRemote protocol channel missing required routines\n");
//		_exit(1);
		hlink->display_char = dummy_display_char;
		hlink->poll_key = dummy_poll_key;
	}
	if(channel0->break_detect == NULL) {
		channel0->break_detect = dummy_break_detect;
	}
	if(hlink->break_detect == NULL) {
		hlink->break_detect = dummy_break_detect;
		async_check = 0;
	}
	if(async_check) {
		old_timer_reload = callout->timer_reload;
		if(old_timer_reload == NULL) old_timer_reload = dummy_timer_reload;
		callout->timer_reload = outside_timer_reload;
	}

	init_kerdebug(debug_arg);
	
	if(debug_flag) kprintf("%s: starting next program at 0x%x\n", progname, start_vaddr);

	private->kdebug_call = &kdebug_callbacks;

	go(start_vaddr, kerdbg);
	return 0;
}

char *
find_typed_string(int index) {
	unsigned 			i;
	unsigned			type;
	char				*names;

	names = SYSPAGE_ENTRY(typed_strings)->data;
	i = 0;
	for( ;; ) {
		type = *(uint32_t *)&names[i];
		if(type == index) return(&names[i+sizeof(uint32_t)]);
		if(type == _CS_NONE) return(NULL);
		i += sizeof(uint32_t);
		i += strlen(&names[i]) + 1;
		i = ROUND(i, sizeof(uint32_t));
	}
}

void
dbg_putc(char c) {
	hlink->display_char(_syspage_ptr, c);
}

int
dbg_getc(void) {
	int		c;

	do {
		c = hlink->poll_key(_syspage_ptr);
	} while(c < 0);
	return(c & 0xff);
}

int
dbg_getc_connect_check(void) {
	int		c;

	do {
		if(channel0 != hlink) {
			c = channel0->poll_key(_syspage_ptr);
			if(IS_ABORT_CHAR(c)) return(-1);
		}
		c = hlink->poll_key(_syspage_ptr);
		if(IS_ABORT_CHAR(c)) return(-1);
	} while(c < 0);
	return(c & 0xff);
}


int
dbg_break_detect(void) {
	int	b;

	b = hlink->break_detect(_syspage_ptr);
//	if(!b && (hlink != channel0)) {
//		b = channel0->break_detect(_syspage_ptr);
//	}
	return(b);
}

int
kdebug_timer_reload(struct syspage_entry *sysp, struct qtime_entry *qtime) {
	int	ret;

	ret = old_timer_reload(sysp, qtime);
	if(dbg_break_detect()) {
		DebugKDBreak();
	}
	return(ret);
}
