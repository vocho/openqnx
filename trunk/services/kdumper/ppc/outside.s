#
# traps.s - GDB exception handling code
#

	.include "asmoff.def"
	.include "ppc/util.ah"



#/*
# * unsigned outside_fault_entry(struct kdebug *entry, unsigned sigcode, void *ctx)
# *
# */
	.global	outside_fault_entry
outside_fault_entry:
	stwu 	%r1,-32(%r1)
	mflr 	%r0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r31,20(%r1)
	mfmsr	%r31
	stw		%r31,24(%r1)
#	lis		%r2,msr_bits_off@ha
#	lwz		%r2,msr_bits_off@l(%r2)
#	andc	%r31,%r31,%r2
#	mtmsr	%r31
#	isync
	stw 	%r0,36(%r1)
	mr		%r31,%r1
	lis		%r1,_stack_top@ha
	lwz		%r1,_stack_top@l(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bl 		fault_entry
	lwz 	%r11,0(%r31)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	lwz 	%r31,-12(%r11)
	lwz 	%r0,-8(%r11)
	mtmsr	%r0
	isync
	mr 		%r1,%r11
	blr


#/*
# * void outside_display_char(struct syspage_entry *syp, char ch)
# */
	.global outside_display_char
outside_display_char:
	stwu 	%r1,-32(%r1)
	mflr 	%r0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r31,20(%r1)
	stw 	%r0,36(%r1)
	mr		%r31,%r1
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bl 		display_char
	lwz 	%r11,0(%r31)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	lwz 	%r31,-12(%r11)
	mr 		%r1,%r11
	blr
