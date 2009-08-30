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

	.file	"cpuid.c"
	.version	"01.01"
gcc2_compiled.:
.text
	.align 4
.globl hw_cpuid
	.type	 hw_cpuid,@function
hw_cpuid:
	pushl %ebp
	movl %esp,%ebp
	subl $12,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 12(%ebp),%esi
	movl 16(%ebp),%edi
	movl 8(%ebp),%eax
/APP
	pushfl
	popl   %ebx
	movl   %ebx, %ecx
	xorl   $0x00200000,%ecx
	pushl  %ecx
	popfl
	pushfl
	popl   %ebx
	cmpl   %ebx, %ecx
	je     1f
	xorl   %eax, %eax
	jmp    2f
	1:
	cpuid
	mov	%edx, -4(%ebp)
	orl    %esi, %esi
	jz     2f
	movl   %ebx, 0(%esi)
	movl   %edx, 4(%esi)
	movl   %ecx, 8(%esi)
	2:
/NO_APP
	movl %eax,%esi
	testl %edi,%edi
	je .L3
	movl -4(%ebp),%eax
	movl %eax,(%edi)
.L3:
	movl %esi,%eax
	popl %ebx
	popl %esi
	popl %edi
	leave
	ret
.Lfe1:
	.size	 hw_cpuid,.Lfe1-hw_cpuid
	.ident	"GCC: (GNU) 2.95.3 20010315 (release)"
