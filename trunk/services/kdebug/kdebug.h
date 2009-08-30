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
	Includes
*/

#ifndef __KDEBUG_H__
#define __KDEBUG_H__

#include <spawn.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syspage.h>
#include <confname.h>
#include <kernel/nto.h>
#include <kernel/kdutil.h>
#include <sys/image.h>
#include "kdbgcpu.h"

struct handle_info {
	int				id;
	unsigned		flags;
	char			*code;
	int				code_reloc;
	unsigned		code_size;
	char			*data;
	int				data_reloc;
	unsigned		data_size;
	char			*name;
	};

#include "kdbghost.h"

#define INFO_FLAG_PROCESS		0x00000001

// Proto's
//extern int get_time(void);
extern void out_text(const char *, unsigned);
char * find_typed_string(int index);

#define IS_ABORT_CHAR(c)	(((c) >= '\0') && (((c) == '~') || ((c) & 0x100)))

#define TRUNC(_x,_a)	((unsigned)(_x)&((_a)-1))
#define ROUND(_x,_a)	(((unsigned)(_x)+((_a)-1)) & ~((_a)-1))
#define ALIGN(_x) 		ROUND(_x, sizeof(unsigned))

#define WANT_FAULT(sigcode) (((sigcode) & interesting_faults) \
					|| (((SIGCODE_SIGNO(sigcode) == SIGTRAP)) && all_sigtraps))
	
/*
	Prototypes
*/

//void 	set_clock_trap(int mode);
void 	init_traps(void);
void	cpu_init(void);
void 	go(uintptr_t ip, int kerdbg);

//uintptr_t kerdbg_mapmem_push(paddr_t paddr);
//int kerdbg_mapmem_pop(int flush);

//extern void clear_handle(void);
//extern int next_handle(int handle, struct handle_info *info, int prev);
//struct kdebug_entry *find_handle(int id);
//unsigned get_handle(struct kdebug_entry *);
int	kdebug_timer_reload(struct syspage_entry *, struct qtime_entry *);

void dbg_putc(char c);
int dbg_getc(void);
int dbg_getc_connect_check(void);
int dbg_break_detect(void);

void		kprintf_init(void);

int 		init_kerdebug(const char *parm);

int			debugpath(struct kdebug_entry *p, char *buf, unsigned bufsize);
unsigned	vaddrinfo(struct kdebug_entry *p, uintptr_t vaddr, paddr_t *addr, size_t *len);
int 		cache_control(uintptr_t base, size_t len, int flags);
void		cache_flush(uintptr_t base, size_t len);

unsigned 	outside_fault_entry(struct kdebug_entry *entry, unsigned code, void *regs);
int 		outside_watch_entry(struct kdebug_entry *entry, unsigned vaddr);
void		outside_msg_entry(const char *msg, unsigned len);
void		outside_update_plist(struct kdebug_entry *entry);
int			outside_timer_reload(struct syspage_entry *, struct qtime_entry *);

/*
	Globals
*/

extern int debug_flag;
extern int map_phys;
extern char	mountpt[];
extern int async_check;
extern int force_use_image;
extern int handle_user_faults;
extern char *optarg;
extern int optind;
extern int interesting_faults;
extern unsigned ser_clk;
extern unsigned ser_div;
extern int protocol;
extern struct cpu_extra_state		extra_state;
extern int (*old_timer_reload)(struct syspage_entry *, struct qtime_entry *);
extern struct debug_callout *channel0;
extern struct debug_callout *hlink;
extern int all_sigtraps;

//NYI: need to figure out CPU we're running on in an SMP system
#define CURRCPU		0

#endif
