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



/*
 *  sys/kdebug.h
 *

 */
#ifndef __KDEBUG_H_INCLUDED
#define __KDEBUG_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#define KDEBUG_PROC_CURRENT			2
#define KDEBUG_PROC_HAS_REQUEST		2
#define KDEBUG_DEBUG_CURRENT		1
#define KDEBUG_CURRENT				1

enum {
	KDEBUG_TYPE_PROCESS,
	KDEBUG_TYPE_OBJECT
};

struct kdebug_entry {
	struct kdebug_entry				*next;
	struct kdebug_entry				**prev;
	unsigned						text_addr;	
	unsigned						text_size;
	int								text_reloc;
	unsigned						data_addr;	
	unsigned						data_size;
	int								data_reloc;
	signed char						handle;
	char							type;
	unsigned short					flags;
	void							*ptr;
};

#define KDEBUG_FLAG_DEBUGGED		0x0001		/* set by kernel debugger when it assigns a handle */
#define KDEBUG_FLAG_UNLOADED		0x0002		/* set by proc when process image is removed */
#define KDEBUG_FLAG_STOP_ON_ENTRY	0x0004		/* set be kdebug when stopping on first instr */

enum {
	KDREQ_VADDRINFO,
	KDREQ_PATH,
	KDREQ_PARKIT,
	KDREQ_INVOKE,
};

union kd_request {
	struct kdr_hdr {
		unsigned	req;
	}							hdr;	/*in*/

	struct kdr_vaddrinfo {
		struct kdr_hdr		hdr; 		/*in*/
		struct kdebug_entry	*entry;		/*in*/
		_Uintptrt			vaddr;		/*in*/
		_Paddr64t			paddr;		/*out*/
		_Paddr64t			size;		/*out*/
		unsigned			prot;		/*out*/
	}							vaddrinfo;

	struct kdr_path {
		struct kdr_hdr		hdr; 		/*in*/
		struct kdebug_entry	*entry;		/*in*/
		char				*buff;		/*in*/
		unsigned			len;		/*in/out*/
	}							path;

	struct kdr_parkit {
		struct kdr_hdr	hdr;			/*in*/
	}							parkit;

	struct kdr_invoke {
		struct kdr_hdr	hdr;			/*in*/
		void			*ptr;			/*in/out*/
		void			(*output)(char *p, unsigned len);	/*in*/
	}							invoke;
};


struct kdebug_info {
	unsigned char					proc_version;
	unsigned char					debug_version;
	unsigned short					flags;
	struct kdebug_entry				*process_list;
	struct kdebug_entry				*resident_list;
	/* This next one we can mark as unused1 after a while: 2008/04/24 */
	int								(*debug_path)(struct kdebug_entry *entry, char *path, int pathbuffsize);
	int								(*request)(union kd_request *);
	const struct kdebug_private		*kdbg_private;
	unsigned						(*unused2)(struct kdebug_entry *entry, _Uintptrt *vaddr, unsigned *size);
	const char						*timestamp;
	/* This next one we can mark as unused3 after a while: 2008/04/24 */
	unsigned						(*vaddr_to_paddr2)(struct kdebug_entry *entry, _Uintptrt vaddr, _Paddr64t *paddr, _Paddr64t *size);
	const struct kdump_private		*kdump_private;
	void							*reserved[5];
};

struct kdebug_callback {
	unsigned short					kdebug_version;
	unsigned short					callback_len;
	int								(*watch_entry)(struct kdebug_entry *entry, unsigned entry_vaddr);
	unsigned						(*fault_entry)(struct kdebug_entry *entry, unsigned sigcode, void *regs);
	void							(*msg_entry)(const char *msg, unsigned len);
	void							(*update_plist)(struct kdebug_entry *entry);
	struct cpu_extra_state			*extra;
	void							*reserved[7];
};

	
#define KDEBUG_FLAG_DIRTY			0x0001

struct kdebug_vector_entry {
	unsigned short	nentries;
	unsigned short	nfree;
	void			*free;
	void			**vector;
};
#define KDEBUG_VECTOR_VER			1

struct kdebug_private {
	unsigned						flags;
	void							*actives;			/* pointer to the active array pointer */
	void							*aspaces;			/* pointer to the aspaces_prp array pointer */
	void							**irqstack;			/* pointer to irq stack */
	struct kdebug_vector_entry		*process_vector;	/* pointer to process vector */
	unsigned short					vector_ver;			/* version of vector type */
	unsigned short					th_tid_off;			/* offset within active to tid-1 */
	unsigned short					th_reg_off;			/* offset within active to registers */
	unsigned short					th_process_off;		/* offset within active to process entry */
	unsigned short					pr_pid_off;			/* offset within process to pid */
	unsigned short					pr_debug_name_off;	/* offset within process to name pointer */
	unsigned short					irq_reg_off;		/* offset within irq stack to registers */
	unsigned short					pr_threads_off;		/* offset within process to threads vector */
	unsigned short					pr_kdebug_off;		/* offset within process to kdebug_entry */
	unsigned short					th_pri_off;			/* offset within thread entry of priority */
	unsigned short					pr_memory_off;		/* offset within process entry of memory */
};


struct kdump_private {
	void						*pmem_root;				/* root of phys mem structures */
	unsigned					*colour_mask;			/* pointer to colour mask */
	unsigned short				as_pgdir_off;			/* offset within aspace entry of pgdir */
	unsigned short				paddr_size;				/* bytes in a paddr_t */
};

#define KDEBUG_PRIVATE_SEGS			0x00000001			/* private flags */

#define KDEBUG_PRIVATE_INDIRECT		0x8000				/* If set in offset, target is a pointer */

#endif

/* __SRCVERSION("kdebug.h $Rev: 193061 $"); */
