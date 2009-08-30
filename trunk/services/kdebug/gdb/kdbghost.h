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
 * This file holds all the host-debugger specific, cpu independent
 * stuff
*/

#ifndef __KDBGHOST_H__
#define __KDBGHOST_H__

#include <sys/mman.h>
#include "kdebug.h"

typedef int	boolean;
#define TRUE 1
#define FALSE 0

#define INVALID_ADDR	0xFFFFFFFF

/*
 * Data structures
 */
extern boolean gdb_debug;
extern char inbuf[];
extern char outbuf[];
extern char scratch[];

extern break_opcode bp_opcode;
extern uintptr_t bp_addr;

extern boolean gdb_interface(struct kdebug_entry *, CPU_REGISTERS *, ulong_t);
extern boolean gethexnum(char *, char **, int *);
extern boolean getpacket(void);
extern boolean parse2hexnum(char *, int *, int *);
extern boolean parsehexnum(char *, int *);
extern char *hex2mem(char *, char *, int);
extern char *mem2hex(char *, char *, int);
extern void putpacket(void);
extern void gdb_set_reloc_sem(void);
extern boolean gdb_test_reloc_sem(void);
extern void gdb_clear_reloc_sem(void);
extern void gdb_putstr(const char *, int);
extern int gdb_printf(const char *, ...);
extern int set_break(uintptr_t addr);
extern boolean restore_break(void);
extern void gdb_read_membytes(CPU_REGISTERS *);
extern void gdb_write_membytes(CPU_REGISTERS *);
extern void msg_entry(const char *msg, unsigned len);
extern int watch_entry(struct kdebug_entry *, uintptr_t);
extern int is_watch_entry(struct kdebug_entry *, uintptr_t);
extern unsigned safe_read(uintptr_t addr);


/*
 * processor dependent routines
 */
extern void gdb_mark_frame_unused(CPU_REGISTERS *);
extern boolean gdb_frame_unused(CPU_REGISTERS *);
extern void gdb_show_exception_info(ulong_t, CPU_REGISTERS *);
extern void gdb_get_cpuregs(CPU_REGISTERS *, FPU_REGISTERS *);
extern void gdb_set_cpuregs(CPU_REGISTERS *, FPU_REGISTERS *);
extern void gdb_proc_continue(CPU_REGISTERS *, int);
extern paddr_t gdb_image_base(paddr_t);
extern uintptr_t	gdb_location(CPU_REGISTERS *);
extern void gdb_prep_reboot(void);
extern int cpu_handle_alignment(void *, unsigned);

extern void	*mapping_add(uintptr_t, size_t, unsigned, size_t *);
extern void	mapping_del(void *, size_t);

#if !defined(CPU_CACHE_FLUSH)
#define CPU_CACHE_FLUSH(m, v, l)	cache_flush((uintptr_t)(m), (l))
#endif

#endif
