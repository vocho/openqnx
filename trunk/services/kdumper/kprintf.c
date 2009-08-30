#include "kdumper.h"
#include <stdarg.h>
#include <string.h>

extern void		outside_display_char(struct syspage_entry *, char);

static void		(*orig_display_char)(struct syspage_entry *, char);


void
display_char(struct syspage_entry *sp, char c) {
	dip->kp_buff[dip->kp_idx] = c;
	if(++dip->kp_idx >= dip->kp_size) {
		dip->kp_idx = 0;
	}
	orig_display_char(sp, c);
}


void
kprintf_init(void) {
	struct debug_callout	*channel;

	channel = &SYSPAGE_ENTRY(callout)->debug[0];

	orig_display_char = channel->display_char;
	kprintf_setup(orig_display_char, sizeof(paddr_t));
	channel->display_char = outside_display_char;
}
