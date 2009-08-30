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


	.text

	.extern main
	.extern _init_libc
	.extern _preinit_array
	.weak _init
	.extern _init_array
	.weak _fini
	.extern _fini_array
	.extern	exit 
	.extern	errno 

	.globl	_start
_start:	
# init fpscr_value for gcc
# r4 -- atexit; r15 -- args	
# r0 is used to pass back pid in fork case
	mov.l	start_2,r1
	mov.l	start_3,r4
	jsr	@r1
	mov.l	r0,@-r15
# call _init_libc
	mov		r15,r5
	# argc
	mov.l	@(4,r15),r4
	# argv
	add		#8,r5
	mov.l	start_4,r0
	mov		r4,r1
	mov		#4,r6
	shll2	r1
	mov		r4,r10	
	add		r1,r6
	mov		r5,r11
	# envp
	add		r5,r6
	# auxp
	mov		r6,r7
1:	
	mov.l	@r7,r1
	tst		r1,r1
	bf.s	1b
	add		#4,r7

	jsr		@r0
	mov		r6,r12


# call _preinit_array
	mov.l   start_17,r4
	mov.l	start_18,r5
	mov.l   start_14,r8
tst           r8,r8
bt	t0
	jsr             @r8
	nop
t0:


# call _init
	mov.l	start_5,r8
tst		r8,r8
bt	t1
	jsr		@r8
	nop
t1:	

# call _init_array
	mov.l   start_19,r4
	mov.l	start_20,r5
	mov.l	start_15,r8
tst		r8,r8
bt	t2
	jsr		@r8
	nop
t2:

# register _fini
	mov.l	start_6,r9
	mov.l	start_7,r4
	mov.l	start_9,r13
tst		r4,r4
bt	t3
	jsr		@r9
	nop	
t3:

# call _fini_array
	mov.l   start_21,r4
	mov.l	start_22,r5
	mov.l	start_16,r9
tst		r9,r9
bt	t4	
 	jsr	@r9
	nop	
t4:

#	clear errno
	sub		r0,r0

#ifdef MCRT1
# register _mcleanup
	mov.l	start_6,r9
	mov.l	start_10,r4
	jsr		@r9
	nop
# call monstartup
	mov.l	start_11,r4
	mov.l	start_12,r5
	mov.l	start_13,r0
	jsr		@r0
	nop
#endif

#	exit(main)	
	mov.l	start_1,r1
	mov		r10,r4
	mov		r11,r5
	mov.l	r0,@r13
	jsr		@r1
	mov		r12,r6
	mov.l	start_8,r1
	jmp		@r1
	mov		r0,r4
	.align 2
start_2:
	.long		__set_fpscr
start_3:
	.long		0xc0400
start_4:
	.long		_init_libc
start_5:
	.long		_init
start_6:
	.long		atexit
start_7:
	.long		_fini
start_9:
	.long		errno
start_1:
	.long		main
start_8:
	.long		exit
#ifdef MCRT1
start_10:
	.long		_mcleanup	
start_11:
	.long		.text	
start_12:
	.long		_etext
start_13:
	.long		monstartup
#endif
start_14:
	.long		_preinit_array
start_15:
	.long		_init_array
start_16:
	.long		_fini_array
start_17:
	.long		__preinit_array_start
start_18:
	.long		__preinit_array_end
start_19:
	.long		__init_array_start
start_20:
	.long		__init_array_end
start_21:
	.long		__fini_array_start
start_22:
	.long		__fini_array_end
	
	.size _start,.-_start
	.type _start,@function
