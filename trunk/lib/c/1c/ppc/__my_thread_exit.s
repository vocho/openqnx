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



#  

#
#
#  Trampoline code - this function is actually 'returned' to when the
#  main thread function falls off the end of it's code. The 'value_ptr'
#  parameter will be in the CPU's return register (R3 for PPC).
#

	.extern pthread_exit

	.global	__my_thread_exit
	
	.section ".text"
__my_thread_exit:
	# R3 is both the return register and first parameter register
.ifdef __PIC__
	b	pthread_exit@plt
.else
	b	pthread_exit
.endif
	.type __my_thread_exit,@function
	.size __my_thread_exit,.-__my_thread_exit
