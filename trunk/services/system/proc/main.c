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


int main(void) {

#ifdef VARIANT_gcov
    extern void gcov_init(void);
	// Call gcov system constructors
	gcov_init();
#endif

	if ( elf_load_hook == NULL ) {
		elf_load_hook = elf_load;
	}

	// Initialize message subsystem. This should be the first thing done.
	message_init();

	// Start up the rsrc database manager.  It is used for resource allocation by drivers
	rsrcdbmgr_init();

	// Initialize the system manager
	sysmgr_init();

	// Start path manager. It will handle all pathname resolutions.
	pathmgr_init();

	// Start /dev/null manager.
	devnull_init();

	// Start /dev/text manager. It will be the default console.
	devtext_init();

	// Start the tty manager.  It redirects /dev/tty to controlling terminal.
	devtty_init();

	// Start the std manager.  It handles /dev/std[in|out|err].
	devstd_init();

	// Start memory manager. It will install /dev/mem, /dev/zero and /dev/shmem/.
	memmgr_init();

	// Start process manager. It will handle process command (e.g. spawning, sessions, terminating)
	procmgr_init();

	// Initialize cpu specific specific functions. (e.g. v86 calls on x86)
	special_init();

	// Start /proc/ manager. It is used for process level debugging.
	procfs_init();

	// Start image filesystem manager. It's mountpoint will be /proc/boot, the startup files will be spawned.
	bootimage_init();

	// Start named semaphore manager.  It will operate at /dev/sem/.
	namedsem_init();

	// do any remaining module specific initializations
	module_init(10);

	// Start handling messages. This does not return....
	return message_start();
}

__SRCVERSION("main.c $Rev: 211988 $");
