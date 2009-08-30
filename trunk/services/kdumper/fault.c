#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fault.h>
#include "kdumper.h"


extern unsigned 	outside_fault_entry(struct kdebug_entry *entry, unsigned code, void *regs);

static volatile	int dumping;
static unsigned		(*old_fault_entry)(struct kdebug_entry *entry, unsigned sigcode, void *regs);

static struct cpu_extra_state	extra_state;

static void
parkit(void) {
	struct kdebug_info	*kinfo;

	kinfo = SYSPAGE_ENTRY(system_private)->kdebug_info;

	// We can remove the proc_version check after a while and
	// just assume kinfo->request is present: 2008/05/05
	if((kinfo != NULL) && (kinfo->proc_version >= KDEBUG_PROC_HAS_REQUEST)) {
		union kd_request	r;

		r.hdr.req = KDREQ_PARKIT;
		kinfo->request(&r);
	}

}
 

unsigned long 
fault_entry(struct kdebug_entry *entry, unsigned long sigcode, void *ctx) {
    if(!(sigcode & interesting_faults)) {
		return old_fault_entry(entry, sigcode, ctx);
	}
	if(dip->kdebug_wanted) {
		sigcode = old_fault_entry(entry, sigcode, ctx);
	}
	if(sigcode != 0) {
		do {
		} while(dumping);
		dumping = 1;

		InterruptDisable();
		cpu_save_extra(private->kdebug_call->extra);
		if(debug_flag > 0) {
			/*
			 * Make sure that the leading tag starts on a line of its
			 * own.
			 *
			 * Without the leading newline, the tag could start in the
			 * middle of a line (e.g. on a crash in the middle of a
			 * printf), which would confuse the regression framework
			 * whose parsing is line based.
			 */
			kprintf("\nKERNEL DUMP START\n");
		}
		parkit();
		dump_system(sigcode, ctx, writer->func);
		if(debug_flag > 0) {
			kprintf("KERNEL DUMP STOP\n");
		}

		//Force the system to do an 'abnormal' reboot
		SYSPAGE_ENTRY(callout)->reboot(_syspage_ptr, 1);	
    }
	return sigcode;
}


static int
dummy_watch_entry(struct kdebug_entry *entry, unsigned entry_vaddr) {
	return 0;
}

static unsigned
dummy_fault_entry(struct kdebug_entry *entry, unsigned sigcode, void *regs) {
	return sigcode;
}

static void
dummy_msg_entry(const char *msg, unsigned len) {
}

static void
dummy_update_plist(struct kdebug_entry *entry) {
}

static struct kdebug_callback kdumper_callbacks = {
	KDEBUG_CURRENT,
	sizeof(struct kdebug_callback),
	dummy_watch_entry,
	dummy_fault_entry,
	dummy_msg_entry,
	dummy_update_plist,
	&extra_state,
};


void
fault_init(void) {
	if(private->kdebug_call == NULL) {
		private->kdebug_call = &kdumper_callbacks;
	}
	old_fault_entry = private->kdebug_call->fault_entry;
	private->kdebug_call->fault_entry = outside_fault_entry;

	cpu_init_extra(private->kdebug_call->extra);
}
