	.extern async_timer_reload

	.globl	outside_timer_reload
outside_timer_reload:
	b		async_timer_reload
