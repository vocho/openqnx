#include "kdebug.h"

void
cpu_init() {
}


#define MK_MOV_PC_L(disp,rn)	(0xd000 + disp + (rn<<8))
/* used only in four bytes boundary */
void
set_trap(unsigned off, void (*trap_func)(), unsigned extra) {
	uint16_t	*vector;
	uint32_t	*data;
	int32_t		func;

	vector = (uint16_t*) ((unsigned)_syspage_ptr->un.sh.exceptptr  + off) ; /* put in P2 area */
	data = (uint32_t*)(vector + 4);
	func = (uint32_t)trap_func;
	/* mov.l @(1,pc),r0 */
	*vector++ = MK_MOV_PC_L(1, 0);
	/* nop */
	*vector++ = 0x9;
	/* jmp @r0 */
	*vector++ = 0x402b;
	/* nop */
	*vector++ = 0x9;
	/* data */
	*data = func | extra;
	icache_flush((uintptr_t)vector);
}

void
copy_code(uint32_t *exc, void *code_start, void *code_end) {
	int len = (uintptr_t)code_end - (uintptr_t)code_start;

	memcpy(exc, code_start, len);
	while(len > 0) {
		icache_flush((uintptr_t)exc);
		exc += 8;
		len -= 32;
	}
}

extern void exc_unexpected();
extern void exc_general_start();
extern void exc_general_end();

/*
 * init_traps()
 *	Initialize machine-dependent exception stuff
 */
void
init_traps(void) {
	copy_code((void*)((unsigned)_syspage_ptr->un.sh.exceptptr) + SH_EXC_GENERAL, (void*)exc_general_start, (void*)exc_general_end);
//	copy_code((void*)((unsigned)0xa0000000+0x8040000) + SH_EXC_GENERAL, (void*)exc_general_start, (void*)exc_general_end);
/*	
	set_trap(SH_EXC_BASE_RESET, exc_unexpected, 0x00);
	set_trap(SH_EXC_TLBMISS, exc_unexpected, 0x00);
	set_trap(SH_EXC_INTR, exc_unexpected, 0x00);
*/
// enable interrupt
	set_sr(get_sr()&~SH_SR_BL);

}
