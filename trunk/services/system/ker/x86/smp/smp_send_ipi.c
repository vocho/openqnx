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

#include "externs.h"

// SMP interprocessor interrupt vectors
extern void __ipi1(), __ipi2();

unsigned dummy_lapicid = 0;
unsigned *lapicid_addr = &dummy_lapicid; // so we work with just 1 cpu.

#define LAPIC_ID	((uintptr_t)lapicid_addr)
#define LAPIC_VER	((uintptr_t)lapicid_addr + (0x030 - 0x20))
#define LAPIC_SPIV	((uintptr_t)lapicid_addr + (0x0F0 - 0x20))
#define LAPIC_ICRL	((uintptr_t)lapicid_addr + (0x300 - 0x20))
#define LAPIC_ICRH	((uintptr_t)lapicid_addr + (0x310 - 0x20))
#define LAPIC_ESR	((uintptr_t)lapicid_addr + (0x370 - 0x20))
#define LAPIC_TPR       ((uintptr_t)lapicid_addr + (0x080 - 0x20))

#define ICR_FIXED	0x00004000

#define apic_read(reg)			(*(volatile unsigned *)(reg))
#define apic_write(reg, val)	(*(volatile unsigned *)(reg) = (val))

#define get_apic_id()			((apic_read(LAPIC_ID)) >> 24 & 0xff)

static unsigned	cpu_remap[PROCESSORS_MAX];
static unsigned	reservAPICICRH[PROCESSORS_MAX];
static unsigned	reservAPICICRL[PROCESSORS_MAX];

static void 
set_cpunum(unsigned new) {
	apic_write(LAPIC_ID, (apic_read(LAPIC_ID) & 0x00ffffff) | (new << 24));
}


// Try to use interrupts at the end of the IDT for IPI's, since the local APIC
// gives them higher priority. Older startups didn't leave room for this,
// so default back to the old values if there's no space.
static unsigned
find_smp_intr() {
	unsigned				i;
	struct segment_info		idt;
	struct intrinfo_entry	*iip;
	unsigned				num_intrs;
	unsigned				smp_intr;
	unsigned				cpu_intr;
	unsigned				max_intr;

	//Find the highest hardware interrupt being used
	max_intr = 0;
	for(i = 0, iip = intrinfoptr; i < intrinfo_num; ++i, ++iip) {
		cpu_intr = iip->cpu_intr_base;
		if(iip->cpu_intr_stride != 0) {
			cpu_intr += (iip->num_vectors-1)*iip->cpu_intr_stride;
		}
		if(cpu_intr > max_intr) max_intr = cpu_intr;
	}	

	sidt(&idt);
	/*lint -e{530} */
	num_intrs = (idt.limit+1) / sizeof(struct x86_gate_descriptor_entry);
	if((max_intr+2) > num_intrs) {
		smp_intr = 0x26;
	} else {
		smp_intr = num_intrs - 2;
	}
//kprintf("Using %x for IPI's (num_intrs=%d)\n", smp_intr, num_intrs);	

	set_trap(smp_intr+0, __ipi1);
	set_trap(smp_intr+1, __ipi2);

	return smp_intr;
}

// Enable the local APIC
unsigned 
init_send_ipi() {
	unsigned		apic_id;
	unsigned		cpu;
	static unsigned	smp_intr;

	if(NUM_PROCESSORS == 1) {
		return 0;
	}
	if(smp_intr == 0) {
		smp_intr = find_smp_intr();
	}	
	if(_syspage_ptr->smp.entry_size > 0) {
		lapicid_addr = (void *)(0x20 + SYSPAGE_ENTRY(smp)->cpu);
	} else {
		//
		// This code can be removed later, after all the startup's have
		// been updated to use the new CPU independent smp section.
		//
		struct x86_smpinfo_entry *smp;

		smp = SYSPAGE_CPU_ENTRY(x86,smpinfo);
		lapicid_addr = (void *)(0x20 + (uintptr_t)smp->lapic_addr);
		//
		// We reprogram all the lapic id's to be consecutive starting
		// at 0. The boot processor which gets all intr's must be 0.
		//
		set_cpunum(smp->lapicid_to_index[get_apic_id()]);
	}

	apic_id = get_apic_id();
	if((startup_cpunum == 0) && alives[0]) {
		// old startup, can be removed later (added 2004/01/07)
		cpu = apic_id;
	} else {
		cpu = startup_cpunum;
	}

	// Upshift APICID by 24 so we don't have to do it all
	// the time in the send_ipi[2] routines.
	cpu_remap[cpu] = apic_id << 24;

   	apic_write(LAPIC_TPR, 0);
	apic_write(LAPIC_SPIV, apic_read(LAPIC_SPIV) | 0x100);
	
	// save the reserved bits in APIC cmd reg
	reservAPICICRH[cpu] = apic_read(LAPIC_ICRH) & 0x00ffffff;
	reservAPICICRL[cpu] = (apic_read(LAPIC_ICRL) & 0xfff33000)
							| smp_intr | ICR_FIXED;
	return(cpu);
}

void rdecl
send_ipi(int cpu, int ipi_cmd) {
	unsigned			dst;
	unsigned			cmd;
	unsigned			prev_flags;
	unsigned			my_cpu;

	if(NUM_PROCESSORS == 1) return;

//kprintf("IPI(%d,%x)", cpu, ipi_cmd);

	if(ipicmds[cpu] & ipi_cmd) return;

	// Has to be atomic
	if(atomic_set_value((void *)&ipicmds[cpu], ipi_cmd) == 0) {
		my_cpu = RUNCPU;
		// Set target APIC ID
		dst = reservAPICICRH[my_cpu] | cpu_remap[cpu];
		cmd = reservAPICICRL[my_cpu];

		// Wait for the "Send Pending" bit to clear
		for( ;; ) {
			prev_flags = disable();
			if(!(apic_read(LAPIC_ICRL) & (1 << 12))) break;
			restore(prev_flags);
		} 

		apic_write(LAPIC_ICRH, dst);
		apic_write(LAPIC_ICRL, cmd); // Send command
		restore(prev_flags);
	}
}

// Send a check interrupts IPI to the CPU in the kernel.
void
send_ipi2() {
	unsigned			dst;
	unsigned			cmd;
	unsigned			my_cpu;

   	my_cpu = RUNCPU;
	// Set target APIC ID
	dst = reservAPICICRH[my_cpu] | cpu_remap[cpunum];
	cmd = reservAPICICRL[my_cpu] + 1;

	// Wait for the "Send Pending" bit to clear
	do {
	} while(apic_read(LAPIC_ICRL) & (1 << 12));

	apic_write(LAPIC_ICRH, dst);
	apic_write(LAPIC_ICRL, cmd); // Send command
}

__SRCVERSION("smp_send_ipi.c $Rev: 153052 $");
