#include "kdebug.h"

void
cpu_init() {
	unsigned				status;

	status = getcp0_sreg();
	status &= ~MIPS_SREG_IMASK;
	setcp0_sreg(status);
	status |= MIPS_SREG_CU0 | MIPS_SREG_CU1;
	setcp0_sreg(status);
}


/*
 * init_traps()
 *	Initialize machine-dependent exception stuff
 */
void
init_traps(void) {
	extern uchar_t exception_code_start[],
		exception_code_end[];

	/*
	 * Install exception handling
	 */
	memcpy((void *)MIPS_R4K_K0BASE, exception_code_start,
		exception_code_end - exception_code_start);

	/*
	 * Must flush the caches
	 */
	cache_flush(MIPS_R4K_K0BASE, exception_code_end - exception_code_start);

	/*
	 * use lower vector table
	 */
	setcp0_sreg(getcp0_sreg() & ~MIPS_SREG_BEV);
}
