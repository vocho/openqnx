#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#

.include "asmoff.def"
.include "sh/util.ah"

	.text

#
# void
# send_ipi(int cpu, int cmd)
# {
# 	send_ipi_rtn(_syspage_ptr, cpu, cmd, (unsigned *)&ipicmds[cpu]);
# }
# 
routine_start	send_ipi, 1

	mov.l	.L_syspage_ptr, r0
	mov.l	.Lsend_ipi_rtn, r1
	mov.l	.Lipicmds, r7
	mov.l	@r1, r1						! send_ipi_rtn
	mov		r4, r2
	shll2	r2
	add		r2, r7						! &ipicmds[cpu]
	mov		r5, r6						! cmd
	mov		r4, r5						! cpu
	mov.l	@r0, r4						! _syspage_ptr
	jmp		@r1							! tail call to send_ipi_rtn
	nop

	.align 2

.L_syspage_ptr:	.long	_syspage_ptr
.Lsend_ipi_rtn:	.long	send_ipi_rtn
.Lipicmds:		.long	ipicmds

routine_end		send_ipi
