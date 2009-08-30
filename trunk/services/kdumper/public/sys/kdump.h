/*
 *  sys/kdump.h
 *
 */
#ifndef __KDUMP_H_INCLUDED
#define __KDUMP_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


#define WRITE_CMD_ADDR	((void *)-(uintptr_t)1)
#define WRITE_INIT		0
#define WRITE_FINI		1

#define KDUMP_SYSTEM			0x00000000
#define KDUMP_ACTIVE			0x00000001
#define KDUMP_ALL				0x00000002

#define KDUMP_IFS_BOOTSTRAP		0x00000000
#define KDUMP_IFS_FULL			0x00000010

#define KDUMP_MEM_MASK			0x0000000F
#define KDUMP_IFS_MASK			0x000000F0

struct kdump {
	int             dump_type;
	int				kdebug_wanted;
	int				compress;
	int				big;
	void			(*writer)(void *, unsigned);
	unsigned		spare[3];
	unsigned		kp_size;
	unsigned		kp_idx;
	char			kp_buff[1]; /* variable sized, determined by 'size' field */
};

#define NOTE_VERSION_CURRENT	0

struct kdump_note {
	_Uint8t			note_version;
	_Uint8t			sig_num;
	_Uint8t			sig_code;
	_Uint8t			fault_num;
	_Uint16t		num_cpus; 
	_Uint16t		faulting_cpu;
	_Uint32t		regset_size;
	_Uint32t		cpu_info;
	_Uint32t		procnto_segnum;
	_Uint32t		syspage_segnum;
	_Uint32t		dumpinfo_segnum;
	_Uint32t		spare[3];
	/* Followed by array of 'num_cpus' struct kdump_note_cpu's */
};

struct kdump_note_cpu {
	_Paddr64t		pgtbl;
	_Uint32t		cpunum;
	_Uint32t		spare[4];
	char			process_name[100];
	/* Followed by register set */
};

#endif

/* __SRCVERSION("kdump.h $Rev: 168306 $"); */
