#include "kdebug.h"

ptp_t	*L1_table;

void
cpu_init() {
#ifdef	DEBUG_GDB
	gdb_debug = 1;
#endif

	L1_table = (ptp_t *)_syspage_ptr->un.arm.L1_vaddr;
}


void
__signalstub()
{
}
