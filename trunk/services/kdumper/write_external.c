#include "kdumper.h"


void
write_external(void *p, unsigned len) {
	// Send data to external writer
	dip->writer(p, len);
}
