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





/*
 * Utility for putting the machine back into text mode, using the BIOS.
 * Also re-enables console output.
 */

#include <string.h>
#include <fcntl.h>
#ifdef __QNXNTO__
#include <sys/dcmd_chr.h>
#else
#include <sys/console.h>
#endif

#include <graphics/display.h>
#include <graphics/disputil.h>
#include <graphics/vbios.h>

int
main()
{
#ifndef __QNXNTO__
	struct		_console_ctrl *cc;
#endif
	unsigned	state;
	int		fd;
	disp_adapter_t	adapter;
#ifdef __X86__
	vbios_regs_t	regs;
	vbios_context_t	*vbios;
#endif

	memset(&adapter, 0, sizeof adapter);

	/* Register with the display utility lib */
	if (disp_register_adapter(&adapter) == -1) {
		perror("register_adapter");
		return 1;
	}

#ifdef __X86__
	if ((vbios = (vbios_context_t *) vbios_register(&adapter, 0)) == NULL) {
		perror("vbios_register");
		return 1;
	}

	regs.eax = 3;
	if (vbios_int(vbios, 0x10, &regs, 0) < 0)
		perror("vbios_int");
#endif

	/* Now re-enable the console */
#ifdef __QNXNTO__
	if ((fd = open("/dev/con1", O_RDWR)) == -1) {
		perror("open");
		return 1;
	}

	state = _CONCTL_INVISIBLE_CHG;
	devctl(fd, DCMD_CHR_SERCTL, &state, sizeof state, 0);
	close(fd);
#else
	if ((fd = open("/dev/con1", O_RDWR)) == -1) {
		perror("open");
		exit (1);
	}
	cc = console_open(fd, O_RDWR);
	close(fd);
	if (cc == 0) {
		perror("console_open");
		exit (1);
	}

	console_ctrl(cc, -1, 0,
	    CONSOLE_NORESIZE | CONSOLE_NOSWITCH | CONSOLE_INVISIBLE);

	console_close(cc);
#endif

	return 0;
}
