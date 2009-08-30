#
#	_start.s
#	Startup entry of the gdb image.
#

	.extern _main
	.extern _stack_top
	
	.include	"sh/util.ah"
	.include	"asmoff.def"

#
# _start()
# The very first code of GDB
# 
	.global	_start

# Must be just in front of "_start" label	
	.extern	bootstrap
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
_start:
	mov.l	_start_1,r0		
	# get syspage ptr	
	stc		r0_bank,r4
	#
	# init kdebug stack
	#
	mov.l	_start_2,r15

	# get the show on the road!
	jmp		@r0		
	nop	
	.align 2
_start_1:
	.long	_main
_start_2:
	.long	_stack_top
