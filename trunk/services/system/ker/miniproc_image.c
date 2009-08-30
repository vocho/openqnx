/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */

#include "externs.h"
#include <sys/iomsg.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <spawn.h>
#include <sys/dcmd_chr.h>
#include <sys/image.h>
#include <sys/elf.h>


#if defined(__X86__)

#if defined(__WATCOMC__)

void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry * sysp );
#pragma aux enter_func = \
	"mov ebp,ebx" \
	"jmp eax" \
	parm [eax] [edx] [ecx] [ebx] modify exact [eax];

#elif defined(__GNUC__)

static inline void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry *sysp ) {
	asm("	movl %3,%%ebp	;"
		"	jmp	*%0 "
		:
		: "a" (start), "d" (argv), "c" (envp), "b" (sysp));
}

#else

#error compiler not supported

#endif

#elif defined(__PPC__)

static inline void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry *sysp ) {
	asm(
		"subi 	%%r1,%%r1, 4;"
		"li		%%r11,-16;"
		"and	%%r1,%%r1,%%r11;"
		"li		%%r11,0;"
		"stw	%%r11,0(%%r1);"
		"mtlr	%0;"
		"lwz	%%r3,-4(%1);"
		"mr		%%r4,%1;"
		"mr		%%r5,%2;"
		"li		%%r6,0;"
		"li		%%r7,0;"
		"blr"
		:
		: "r" (start), "b" (argv), "r" (envp)
		: "3", "4", "5", "6", "7"
	);
}

#elif defined(__MIPS__)

static inline void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry *sysp ) {
	asm("	j	%0 ;"
		"	 move $4,%1 ;"
		:
		: "r" (start), "r" (sysp));
}

#elif defined(__SH__)

static inline void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry *sysp ) {
	asm("	jsr	@%0 ;"
		"	mov %1,r4 ;"
		:
		: "r" (start), "r" (sysp));
}

#elif defined(__ARM__)

static inline void 
enter_func(Elf32_Addr start, char **argv, char **envp, volatile struct syspage_entry *sysp ) {
	asm("	mov	r0, %1\n"
		"	b	%0"
		:
		: "r" (start), "r" (sysp));
}

#else
   #error enter_func not configured for system
#endif

static void *
find_file( char *cmd, char *path ) {
	//NYI: How to match up file name with dir entry?
	return NULL;
}

static void *
startup(void * in) {
	union script_cmd	*scp = in;
	Elf32_Ehdr			*ehdr;
	int					i; 
	char				**args;
	char				**argv;
	char				**envp;
	char				*cp;
	char				*path = NULL;

	args = alloca((scp->external.argc + scp->external.envc + 4) * sizeof(void *));

	if (args == NULL)
		exit (1);

	*args++ = (void *)(uintptr_t)scp->external.argc;

    argv = args;
	for(cp = scp->external.args, i = 0; i < scp->external.argc; ++i) {
		*args++ = cp;
		while(*cp++) {
			/* nothing to do */
		}
	}
	*args++ = NULL;

    envp = args;
	for(i = 0; i < scp->external.envc; ++i) {
		if(memcmp(cp, "PATH=", 5) == 0) {
			path = cp + 5;
		}
		*args++ = cp;
		while(*cp++) {}
	}
	*args++ = NULL;

	*args = NULL;

    ehdr = find_file( argv[0], path );
	if(ehdr == NULL)
		exit(1);
	
	enter_func(ehdr->e_entry, argv, envp, privateptr->user_syspageptr);
	return NULL;
}

static void
wait_a_tenth()
{
	struct sigevent notify;
	uint64_t		tim;

	notify.sigev_notify = SIGEV_UNBLOCK;
	tim = 100000000;
	TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_NANOSLEEP, &notify, &tim, NULL);
}

static int
map_ifs(struct asinfo_entry *as, char *name, void *data) {
	struct image_header		**ihpp = data;
	
	*ihpp = mmap(0, (as->end - as->start) + 1, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_PHYS, NOFD, as->start);
	return 1;
}

void
miniproc_image_start(void) {
	union script_cmd		*scp;
	int						 tid = 0;
	struct image_header		*ihp;
	union  image_dirent		*dir;
	void					*end;

	if(walk_asinfo("imagefs", map_ifs, &ihp)) crash();

	dir = (void *)((char *)ihp + ihp->dir_offset);
	for( ;; ) {
		if(dir->attr.size == 0) return;
		if(dir->attr.ino == ihp->script_ino) break;
		dir = (void *)((char *)dir + dir->attr.size);
	}
	scp = (void *)((char *)ihp + dir->file.offset);
	end = (char *)scp + dir->file.size;
	while(scp < (union script_cmd *)end) {
		char *cp = scp->external.args;

//NYI: check scp->hdr.type
		if(strcmp(cp, "reopen") == 0) {
			if( scp->external.argc == 2 ) {
				cp += strlen(cp) + 1;
			} else {
				cp = "/dev/console";
			}

			wait_a_tenth();

			ConnectDetach(0);
			ConnectDetach(1);
			ConnectDetach(2);
			(void)open(cp, O_RDWR);
			(void)dup(0);
			(void)dup(0);
		} else if(strcmp(cp, "waitfor") == 0) {
			int fd;

			cp += strlen(cp) + 1;

			while((fd = open(cp, O_RDONLY)) == -1)
				wait_a_tenth();
			close(fd);
		} else {
			if(tid) (void)ThreadJoin(tid, NULL);
			tid = ThreadCreate(0, startup, scp, 0);
			if(scp->external.flags & SCRIPT_FLAGS_BACKGROUND) tid = 0;
		}
		scp = (void *)((char *)scp + scp->hdr.size_lo + (scp->hdr.size_hi << 8));
	}
}

__SRCVERSION("miniproc_image.c $Rev: 160064 $");
