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




#include <sys/auxv.h>

extern int main(int argc, char *argv[], char *arge[]) __attribute__((__weak__));
extern void *__SysCpupageGet(int index);
extern void *__SysCpupageSet(int index, int value);
extern void _math_emu_stub(unsigned sigcode, void **pdata, void *regs);
extern void *_dll_list(void);
extern void _init_libc(int argc, char *argv[], char *arge[], auxv_t auxv[], void (*exit_func)(void));

/* __SRCVERSION("init.h $Rev: 158864 $"); */
