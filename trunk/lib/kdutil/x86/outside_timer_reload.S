	.extern async_timer_reload

.global outside_timer_reload
outside_timer_reload:
	jmp		async_timer_reload
