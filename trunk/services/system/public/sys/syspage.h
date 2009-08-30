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
 *  sys/syspage.h
 *

 */

#ifndef __SYSPAGE_H_INCLUDED
#define __SYSPAGE_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

/* Leave deprecated sections enabled for right now */
#if !defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
    #define ENABLE_DEPRECATED_SYSPAGE_SECTIONS
#elif ENABLE_DEPRECATED_SYSPAGE_SECTIONS == 0
	#undef ENABLE_DEPRECATED_SYSPAGE_SECTIONS
#endif

#if defined(ENABLE_DEPRECATED_SYSPAGE_SECTIONS)
	#define DEPRECATED_SECTION_NAME(name)	name
#else
	#define DEPRECATED_SECTION_NAME(name)	__mangle_name_to_cause_compilation_errs##name
#endif


#undef _SPPTR
#undef _SPFPTR

#if __PTR_BITS__ != 32
    #define _SPPTR(_t)				_Uint32t
    #define _SPFPTR(_r, _n, _p)		_Uint32t _n
#else
    #define _SPPTR(_t)				_t *
    #define _SPFPTR(_r, _n, _p)		_r (*_n) _p
#endif

__BEGIN_DECLS

#if defined(__CLOCKADJUST)
struct _clockadjust __CLOCKADJUST;
#undef __CLOCKADJUST
#endif

struct _thread_local_storage;
struct _process_local_storage;
struct syspage_entry;
struct tracebuf;

#if defined(__SLIB_DATA_INDIRECT) && !defined(_syspage_ptr) && !defined(__SLIB)
  struct syspage_entry *__get_syspage_ptr(void);
  #define _syspage_ptr (__get_syspage_ptr())
#else
  extern struct syspage_entry				*_syspage_ptr;
#endif

typedef struct {
	_Uint16t		entry_off;
	_Uint16t		entry_size;
} syspage_entry_info;

/*
 * Include CPU specific definition for network queries.
 */
#if	!defined(SYSPAGE_TARGET_ALL) \
 && !defined(SYSPAGE_TARGET_X86) \
 && !defined(SYSPAGE_TARGET_PPC) \
 && !defined(SYSPAGE_TARGET_MIPS) \
 && !defined(SYSPAGE_TARGET_ARM) \
 && !defined(SYSPAGE_TARGET_SH)
	#if defined(__X86__)
		#define SYSPAGE_TARGET_X86
	#elif defined(__PPC__)
		#define SYSPAGE_TARGET_PPC
	#elif defined(__MIPS__)
		#define SYSPAGE_TARGET_MIPS
	#elif defined(__ARM__)
		#define SYSPAGE_TARGET_ARM
	#elif defined(__SH__)
		#define SYSPAGE_TARGET_SH
	#else
		#error not configured for system
	#endif
#endif

#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_X86)
#include _NTO_HDR_(x86/syspage.h)
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_PPC)
#include _NTO_HDR_(ppc/syspage.h)
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_MIPS)
#include _NTO_HDR_(mips/syspage.h)
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_ARM)
#include _NTO_HDR_(arm/syspage.h)
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_SH)
#include _NTO_HDR_(sh/syspage.h)
#endif


/*
 * The CPUPAGE_ENTRY structure and the INDEXES must be matched.
 */
struct cpupage_entry {
	_SPPTR(struct _thread_local_storage)	tls;
	_SPPTR(struct _process_local_storage)	pls;
	_SPPTR(struct syspage_entry)			syspage;
	unsigned long							cpu;
	unsigned long							state;
	union {
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_SH)
		struct sh_cpupage_entry				sh;
#endif
		struct {
			long	dummy[8];
		}									dummy;
	} un;
	unsigned long							spare[8];
};

enum {
	CPUPAGE_ADDR = -1,	/* R */
	CPUPAGE_TLS = 0,	/* R */
	CPUPAGE_PLS,		/* RW */
	CPUPAGE_SYSPAGE,	/* R */
	CPUPAGE_CPU,		/* R */
	CPUPAGE_STATE,		/* R */
	CPUPAGE_MAX
};

/*
 *  Architecture independent system page definitions
 */

/*
 *	System page types
 */
enum {
	SYSPAGE_X86,
	SYSPAGE_PPC,
	SYSPAGE_MIPS,
	SYSPAGE_SPARE,
	SYSPAGE_ARM,
	SYSPAGE_SH
};

#define QTIME_FLAG_TIMER_ON_CPU0	0x00000001
#define QTIME_FLAG_CHECK_STABLE		0x00000002

struct qtime_entry {
	_Uint64t					cycles_per_sec;	/* for ClockCycles */
	_Uint64t volatile			nsec_tod_adjust;
	_Uint64t volatile			nsec;
	unsigned long				nsec_inc;
	unsigned long				boot_time; /* UTC seconds when machine booted */

	struct _clockadjust			adjust;
	unsigned long				timer_rate;	/* times 10^timer_scale */
			 long				timer_scale;
	unsigned long				timer_load;
	         long				intr;
	unsigned long				epoch;
	unsigned long				flags;
	unsigned int				rr_interval_mul;
	unsigned long				spare0;
	_Uint64t volatile			nsec_stable;
	unsigned long				spare[4];
};

struct intrspin;

struct debug_callout {
	_SPFPTR(void, display_char, (struct syspage_entry *, char));
	_SPFPTR(int, poll_key, (struct syspage_entry *));
	_SPFPTR(int, break_detect, (struct syspage_entry *));
	_SPFPTR(void, spare[1],);
};

typedef enum {
	DEBUG_WATCHDOG_STOP = 0,
	DEBUG_WATCHDOG_CONTINUE,
	DEBUG_WATCHDOG_FEED
} debug_watchdog_cmd;

struct callout_entry {
	_SPFPTR(void, reboot, (struct syspage_entry *, int));
	_SPFPTR(int, power, (struct syspage_entry *, unsigned, _Uint64t *));

	_SPFPTR(void, timer_load, (struct syspage_entry *, struct qtime_entry *));
	_SPFPTR(int, timer_reload, (struct syspage_entry *, struct qtime_entry *));
	_SPFPTR(unsigned, timer_value, (struct syspage_entry *, struct qtime_entry *));

	struct debug_callout	debug[2];

	_SPFPTR(void, debug_watchdog, (struct syspage_entry *, debug_watchdog_cmd));
	_SPFPTR(void, spare[3],);
};

struct callin_entry {
	_SPFPTR(void, spare0,);
	_SPFPTR(int, interrupt_mask, (unsigned, int));
	_SPFPTR(int, interrupt_unmask, (unsigned, int));
	_SPFPTR(int, trace_event, (int *));
	_SPFPTR(void, spare[11],);
};

/* 'meminfo' is deprecated - use the 'asinfo' section */
enum {
	MEMTYPE_NONE,
	MEMTYPE_RAM,
	MEMTYPE_IMAGEFSYS,
	MEMTYPE_BOOTRAM,
	MEMTYPE_RESERVED,
	MEMTYPE_IOMEM,
	MEMTYPE_FLASHFSYS
};

struct meminfo_entry {
	_Paddr32t		addr;
	_Uint32t		size;
	_Uint32t		type;
	_Uint32t		spare;
};

#define AS_NULL_OFF			((_Uint16t)-1)

#define AS_ATTR_READABLE	0x0001
#define AS_ATTR_WRITABLE	0x0002
#define AS_ATTR_CACHABLE	0x0004
#define AS_ATTR_VISIBLE		0x0008
#define AS_ATTR_KIDS		0x0010
#define AS_ATTR_CONTINUED	0x0020

#define AS_PRIORITY_DEFAULT	100

struct asinfo_entry {
	_Uint64t			start;
	_Uint64t			end;
	_Uint16t			owner;
	_Uint16t			name;
	_Uint16t			attr;
	_Uint16t			priority;
	int					(*alloc_checker)(struct syspage_entry *__sp,
									 _Uint64t	*__base,
									 _Uint64t	*__len,
									 _Sizet		__size,
									 _Sizet		__align);
	_Uint32t			spare;
};

struct hwinfo_entry {
	unsigned long	data[1]; /* variable sized, see <hw/sysinfo.h> */
};

#define SYSTEM_PRIVATE_FLAG_ABNORMAL_REBOOT	0x00000001
#define	SYSTEM_PRIVATE_FLAG_EALREADY_NEW	0x00000002

struct system_private_entry {
	_SPPTR(struct cpupage_entry)	user_cpupageptr;
	_SPPTR(struct syspage_entry)	user_syspageptr;
	_SPPTR(struct cpupage_entry)	kern_cpupageptr;
	_SPPTR(struct syspage_entry)	kern_syspageptr;
	_SPPTR(struct kdebug_info)		kdebug_info;
	_SPPTR(struct kdebug_callback)	kdebug_call;
	struct {
		_Paddr32t						base;
		_Uint32t						entry;
	}								boot_pgm[4];
	unsigned long					boot_idx;
	unsigned long					cpupage_spacing;
	unsigned long					private_flags;
	_Uint32t						pagesize;
	/* 'ramsize' is deprecated - use the 'asinfo' section */
	_Uint32t						ramsize;
	_SPPTR(struct tracebuf)			tracebuf;

	_Paddr32t						kdump_info;
	_Uint32t						spare[3];
	union kernel_entry {
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_X86)
		struct x86_kernel_entry			x86;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_PPC)
		struct ppc_kernel_entry			ppc;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_MIPS)
		struct mips_kernel_entry		mips;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_ARM)
		struct arm_kernel_entry			arm;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_SH)
		struct sh_kernel_entry			sh;
#endif
	}								kercall;
};

/*
 * System independent CPU flags (system dependent grow from bottom up)
 */
#define CPU_FLAG_FPU		(1UL <<  31)  /* CPU has floating point support */
#define CPU_FLAG_MMU		(1UL <<  30)  /* MMU is active */

struct cpuinfo_entry {
	_Uint32t		cpu;
	_Uint32t		speed;
	_Uint32t		flags;
	_Uint32t		spare[4];
	_Uint16t		name;
	_Uint8t			ins_cache;
	_Uint8t			data_cache;
};

#define CACHE_LIST_END		0xff	/* terminate a cache hierarchy list */

/*
 * Cache flags
 */
#define CACHE_FLAG_INSTR	0x0001	/* cache holds instructions */
#define CACHE_FLAG_DATA		0x0002	/* cache holds data */
#define CACHE_FLAG_UNIFIED	0x0003	/* cache holds both ins & data */
#define CACHE_FLAG_SHARED	0x0004	/* cache is shared between multiple  */
									/* processors in an SMP system */
#define CACHE_FLAG_SNOOPED	0x0008	/* cache implements a bus snooping */
									/* protocol */
#define CACHE_FLAG_VIRT_TAG	0x0010	/* cache is virtually tagged */
#define CACHE_FLAG_VIRTUAL	0x0010	/* backwards compatability flag for above */
#define CACHE_FLAG_WRITEBACK 0x0020	/* cache does writeback, not writethru */
#define CACHE_FLAG_CTRL_PHYS 0x0040	/* control function takes 32-bit paddrs */
#define CACHE_FLAG_SUBSET	0x0080	/* this cache obeys the 'subset' property */
#define CACHE_FLAG_NONCOHERENT 0x0100 /* cache is non-coherent on SMP */
#define CACHE_FLAG_NONISA	0x0200 /* cache doesn't obey ISA cache instr */
#define CACHE_FLAG_NOBROADCAST 0x0400 /* cache ops aren't broadcast on bus for SMP */
#define CACHE_FLAG_VIRT_IDX	0x0800	/* cache is virtually indexed */
#define CACHE_FLAG_CTRL_PHYS64 0x1000 /* control function takes 64-bit paddrs */

struct cacheattr_entry {
	_Uint32t	next;
	_Uint32t	line_size;
	_Uint32t	num_lines;
	_Uint32t	flags;
	_SPFPTR(unsigned, control, (_Paddr32t, unsigned, int, struct cacheattr_entry *, volatile struct syspage_entry *));
	_Uint16t	ways;
	_Uint16t	spare0[1];
	_Uint32t	spare1[2];
};

struct typed_strings_entry {
	char				data[4]; /* variable size */
};

struct strings_entry {
	char				data[4]; /* variable size */
};

/*
 * System independent interrupt flags
 * (system dependent grow from bottom up).
 */
#define INTR_FLAG_NMI					0x8000U
#define INTR_FLAG_CASCADE_IMPLICIT_EOI	0x4000U
#define INTR_FLAG_CPU_FAULT				0x2000U
#define INTR_FLAG_SMP_BROADCAST_MASK	0x1000U
#define INTR_FLAG_SMP_BROADCAST_UNMASK	0x0800U

/*
 * System independent interrupt code gen flags
 * (system dependent grow from bottom up).
 */
#define INTR_GENFLAG_LOAD_SYSPAGE		0x8000U
#define INTR_GENFLAG_LOAD_INTRINFO		0x4000U
#define INTR_GENFLAG_LOAD_LEVEL			0x2000U
#define INTR_GENFLAG_LOAD_INTRMASK		0x1000U
#define INTR_GENFLAG_NOGLITCH			0x0800U
#define INTR_GENFLAG_LOAD_CPUNUM		0x0400U
#define INTR_GENFLAG_ID_LOOP			0x0200U

struct __intrgen_data {
	_Uint16t	genflags;
	_Uint16t	size;
	_SPFPTR(void, rtn, (void));
};

#define INTR_CONFIG_FLAG_PREATTACH	0x0001
#define INTR_CONFIG_FLAG_DISALLOWED	0x0002
#define INTR_CONFIG_FLAG_IPI		0x0004

struct intrinfo_entry {
	_Uint32t				vector_base;
	_Uint32t				num_vectors;
	_Uint32t				cascade_vector;
	_Uint32t				cpu_intr_base;
	_Uint16t				cpu_intr_stride;
	_Uint16t				flags;
	struct __intrgen_data	id;
	struct __intrgen_data	eoi;
	_SPFPTR(int, mask, (struct syspage_entry *, int));
	_SPFPTR(int, unmask, (struct syspage_entry *, int));
	_SPFPTR(unsigned, config, (struct syspage_entry *, struct intrinfo_entry *, int));
	_Uint32t				spare[4];
};

struct smp_entry {
	_SPFPTR 			(void, send_ipi, (struct syspage_entry *, unsigned, int, unsigned *));
	_SPPTR(void)		start_addr;
	_Uint32t			pending;
	_Uint32t			cpu;
};

struct pminfo_entry {
	_Uint32t			wakeup_pending;
	_Uint32t			wakeup_condition;
	_Uint32t			spare[4];
	_Uint32t			managed_storage[1]; /* Variable sized */
};

enum mdriver_state {
	MDRIVER_INIT,
	MDRIVER_STARTUP,
	MDRIVER_STARTUP_FINI,
	MDRIVER_KERNEL,
	MDRIVER_PROCESS,
	MDRIVER_INTR_ATTACH,
	MDRIVER_STARTUP_PREPARE,
};

struct mdriver_entry {
	_Uint32t	intr;
	_SPFPTR		(int, handler, (int state, void *data));
	_SPPTR(void) data;
	_Paddr32t	data_paddr;
	_Uint32t	data_size;
	_Uint32t	name;
	_Uint32t	internal;
	_Uint32t	spare[1];
};


struct syspage_entry {
	_Uint16t			size;		/* size of syspage_entry */
	_Uint16t			total_size;	/* size of system page */
	_Uint16t			type;
	_Uint16t			num_cpu;
	syspage_entry_info	system_private;
	syspage_entry_info	asinfo;
	syspage_entry_info	meminfo;
	syspage_entry_info	hwinfo;
	syspage_entry_info	cpuinfo;
	syspage_entry_info	cacheattr;
	syspage_entry_info	qtime;
	syspage_entry_info	callout;
	syspage_entry_info	callin;
	syspage_entry_info	typed_strings;
	syspage_entry_info	strings;
	syspage_entry_info	intrinfo;
	syspage_entry_info	smp;
	syspage_entry_info	pminfo;
	syspage_entry_info	mdriver;
	long				spare[2];
	union {
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_X86)
		struct x86_syspage_entry 	x86;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_PPC)
		struct ppc_syspage_entry 	ppc;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_MIPS)
		struct mips_syspage_entry	mips;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_ARM)
		struct arm_syspage_entry	arm;
#endif
#if defined(SYSPAGE_TARGET_ALL) || defined(SYSPAGE_TARGET_SH)
		struct sh_syspage_entry	sh;
#endif
		struct {
			long						filler[20];
		} filler;
	} un;
};

#define _SYSPAGE_ENTRY_SIZE( __base, __field ) \
	    ((__base)->__field.entry_size)

#define SYSPAGE_ENTRY_SIZE( __field )           _SYSPAGE_ENTRY_SIZE( _syspage_ptr, __field )

#define _SYSPAGE_ENTRY( __base, __field ) \
	((struct __field##_entry *)(void *)((char *)(__base) + (__base)->__field.entry_off))

#define _SYSPAGE_CPU_ENTRY( __base, __cpu, __field ) \
	((struct __cpu##_##__field##_entry *)(void *)((char *)(__base) + (__base)->un.__cpu.__field.entry_off))

#define SYSPAGE_ENTRY( __field )			_SYSPAGE_ENTRY( _syspage_ptr, __field )
#define SYSPAGE_CPU_ENTRY( __cpu, __field )	_SYSPAGE_CPU_ENTRY( _syspage_ptr, __cpu, __field )

__END_DECLS

#undef _SPPTR
#undef _SPFPTR

#endif /* __SYSPAGE_H_INCLUDED */

/* __SRCVERSION("syspage.h $Rev: 206732 $"); */
