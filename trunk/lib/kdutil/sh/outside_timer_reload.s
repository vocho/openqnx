	.extern async_timer_reload

#/*
# * int outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
# *  Check for an async stop. Run on kernel stack.
# */
	.global outside_timer_reload
outside_timer_reload:
	sts.l	pr,@-r15
	mov.l	outside_timer_reload_0,r0
	jsr		@r0
	nop
	lds.l	@r15+,pr
	rts
	nop

	.align 2
outside_timer_reload_0:
	.long	async_timer_reload
