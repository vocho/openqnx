
	
#/*
# * ulong_t outside_fault_entry (struct kdebug *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
# *
# */
	.global	outside_fault_entry
outside_fault_entry:
	mov.l	r13,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	stc		sr,r13
	mov		r15,r14
	mov		r13,r0
	or		#0xf0,r0
	ldc		r0,sr
	mov.l	fault_entry_addr,r0
	jsr		@r0
	nop
	mov		r14,r15
	ldc		r13,sr
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	mov.l	@r15+,r13
	rts
	nop

	
#/*
# * void outside_display_char(struct syspage_entry *syspage, char ch)
# *  Display the given character
# */
	.global outside_display_char
outside_display_char:
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov		r15,r14
	mov.l	display_char_addr,r0
	jsr		@r0
	nop
	mov		r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts
	nop

	.align 2	
fault_entry_addr:
	.long	fault_entry
display_char_addr:
	.long	display_char
