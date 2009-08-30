	
.text

.global	cpu_init_extra
cpu_init_extra:
	ret

.global	cpu_save_extra
cpu_save_extra:
	mov		4(%esp),%eax
	mov		%cr0,%edx
	mov		%edx,0*4(%eax)
	mov		%cr2,%edx
	mov		%edx,2*4(%eax)
	mov		%cr3,%edx
	mov		%edx,3*4(%eax)
//	mov		%cr4,%edx				//What happens with this on a 386?
//	mov		%edx,4*4(%eax)
	ret


.global	cpu_restore_extra
cpu_restore_extra:
	ret
