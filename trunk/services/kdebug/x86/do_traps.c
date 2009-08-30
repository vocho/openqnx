#include "kdebug.h"

unsigned
set_trap(int trapnum, void *func)
{
	struct x86_gate_descriptor_entry *gdp;
	uint32_t	addr, old;

	gdp = &_syspage_ptr->un.x86.idt[trapnum];

	old = gdp->offset_lo | (gdp->offset_hi << 16);
	addr = (unsigned)func;
	gdp->offset_lo = addr & 0xffff;
	gdp->offset_hi = (addr >> 16) & 0xffff;
	gdp->selector = ker_cs;
	gdp->intel_reserved = 0;
	gdp->flags = X86_PRESENT | (X86_INT_GATE32 >> 1) | X86_DPL3;
	return old;
}

/*
 * init_traps()
 *	Initialize machine-dependent exception stuff
 */
void
init_traps()
{
	extern void __trp_entry();
	extern void __msg_entry();
	extern void __dbg_entry();
	extern void __brk_entry();
	extern void __exc_entry();

	set_trap(0x01,	&__dbg_entry);
	set_trap(0x03,	&__brk_entry);
	set_trap(0x0b,	&__exc_entry);
	set_trap(0x0c,	&__exc_entry);
	set_trap(0x0d,	&__exc_entry);
	set_trap(0x0e,	&__exc_entry);
	set_trap(0x20,	&__trp_entry);
	set_trap(0x21,	&__msg_entry);
}
