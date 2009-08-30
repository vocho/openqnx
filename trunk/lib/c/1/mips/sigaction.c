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




#include <signal.h>
#include <inttypes.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>

extern uint32_t	__cpu_flags;

extern void __signalstub32(void);
extern void __signalstub64(void);

int
sigaction(int signo, const struct sigaction *act, struct sigaction *oact) {
	void	(*stub)(void);

	//
	// Optimize signal delivery by going directly to the proper stub
	// routine.
	//
	if(__cpu_flags & MIPS_CPU_FLAG_64BIT) {
		stub = &__signalstub64;
	} else {
		stub = &__signalstub32;
	}
	return(SignalAction(0, stub, signo, act, oact));
}

__SRCVERSION("sigaction.c $Rev: 153052 $");
