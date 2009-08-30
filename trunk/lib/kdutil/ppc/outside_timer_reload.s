	.extern async_timer_reload


#/*
# * int outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
# *  Check for an async stop. Run on kernel stack.
# */
	.global outside_timer_reload
outside_timer_reload:
	stwu 	%r1,-16(%r1)
	mflr 	%r0
	stw 	%r2,8(%r1)
	stw 	%r13,12(%r1)
	stw 	%r0,20(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bl 		async_timer_reload
	lwz 	%r0,20(%r1)
	mtlr 	%r0
	lwz 	%r2,8(%r1)
	lwz 	%r13,12(%r1)
	addi	%r1,%r1,16
	blr
