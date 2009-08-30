#include "kdebug.h"

unsigned short	ker_cs;
unsigned short	ker_ds;

extern void dbg_init(void);

void
cpu_init() {
	ker_cs = _cs();
	ker_ds = _ds();

	dbg_init();
}
