#
#	_start.s
#	Startup entry of the kdumper image.
#

	.extern _main
	.extern _stack_top
	
	.include	"ppc/util.ah"
	.include	"asmoff.def"

#
# _start()
# 
	.global	_start

# Must be just in front of "_start" label	
	.extern	bootstrap
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
_start:
	#
	# init kdumper stack
	#
	lis		%r9,_stack_top@ha
	lwz		%r1,_stack_top@l(%r9)
	subi	%r1,%r1,16
	stw		%r1,_stack_top@l(%r9)
	
	#
	# init small data area ptrs
	#
	loada	%r13,_SDA_BASE_
	loada	%r2,_SDA2_BASE_
	
	# syspage pointer pointer is in R3 - pass it to _main.
	
	b		_main					# let's get the show on the road!
