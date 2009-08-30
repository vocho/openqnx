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
 *  sys/mman.h POSIX Memory Management
 *

 */
#ifndef __MMAN_H_INCLUDED
#define __MMAN_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

#if defined(__MODE_T)
typedef __MODE_T	mode_t;
#undef __MODE_T
#endif

_C_STD_END

#if defined(__OFF_T)
typedef __OFF_T		off_t;
#undef __OFF_T
#endif

#if defined(__OFF64_T)
typedef __OFF64_T	off64_t;
#undef __OFF64_T
#endif

#if !defined(__EXT_POSIX1_199309) && (defined(__EXT_POSIX1_198808) || defined(__EXT_POSIX1_199009))
#error POSIX Threads needs P1003.1b-1993 or later
#endif

#include <_pack64.h>

/* Sharing types: Must choose either SHARED, PRIVATE or ANON */
#define MAP_SHARED      0x00000001     /* share changes          */
#define MAP_PRIVATE     0x00000002     /* changes are private    */
#if defined(__EXT_UNIX_MISC) || defined(__EXT_QNX)
#define MAP_FILE        0x00000000
#define MAP_PRIVATEANON 0x00000003     /* anonymous private memory */
#endif
#if defined(__EXT_QNX)
#define MAP_ANONYMOUS   MAP_ANON
#define MAP_TYPE        0x0000000f     /* mask for share type    */
#endif
#if defined(__EXT_UNIX_HIST)
#define MAP_NORESERVE	0x00004000
#define MAP_RENAME		0x00008000
#endif

/* Flags to mmap (or-ed in to MAP_SHARED or MAP_PRIVATE)     */
#define MAP_FIXED       0x00000010     /* user assigns address   */
#if defined(__EXT_QNX)
#define MAP_ELF         0x00000020     /* Patch ELF relocations  */
#define MAP_NOSYNCFILE  0x00000040     /* Memory not synced to file */
#define MAP_LAZY        0x00000080     /* allow lazy mapping     */
#define MAP_STACK       0x00001000     /* memory used for stack  */
#define MAP_BELOW       0x00002000     /* allocate below hint    */
#define MAP_NOINIT		0x00004000     /* don't have to initialize the memory */
#define MAP_PHYS        0x00010000     /* map physical memory    */
#define MAP_NOX64K      0x00020000     /* don't cross 64k bound  */
#define MAP_BELOW16M    0x00040000     /* allocate below 16M     */
#define MAP_ANON        0x00080000     /* anonymous memory       */
#define MAP_SYSRAM      0x01000000     /* system ram             */
#define MAP_SPARE1 		0x10000000	   /* reserved for system use */	
#define MAP_SPARE2 		0x20000000	   /* reserved for system use */	
#define MAP_SPARE3 		0x40000000	   /* reserved for system use */	
#define MAP_SPARE4 		0x80000000	   /* reserved for system use */	
#define MAP_RESERVMASK 	0xf0000000	   /* reserved for system use */	

/* Page mapping flags which may be or-ed together         */
#define PG_MODIFIED		0x00100000	   /* pages modified */
#define PG_REFERENCED	0x00200000	   /* pages referenced */
#define PG_HWMAPPED		0x00400000	   /* pages are present and mapped */
#define PG_MASK			0x00f00000	   /* mask for page bits */
#endif

/* Page protection flags which may be or-ed together         */
#define PROT_NONE       0x00000000
#define PROT_READ       0x00000100     /* pages can be read      */
#define PROT_WRITE      0x00000200     /* pages can be written   */
#define PROT_EXEC       0x00000400     /* pages can be executed  */
#if defined(__EXT_QNX)
#define PROT_NOCACHE    0x00000800     /* pages are not cached   */
#define PROT_MASK       0x00000f00     /* mask for prot bits     */
#endif

/* flags for msync() */
#define MS_ASYNC                0x0000001   /* Flush modified area to storage (Don't wait) */
#define MS_SYNC                 0x0000002   /* Flush modified area to storage (Return when complete) */
#define MS_INVALIDATE           0x0000004   /* Invalidate area (next access will retrieve from storage) */
#if defined(__EXT_QNX)
#define MS_INVALIDATE_ICACHE    0x1000000   /* If PROT_EXEC, force execution from cache or storage */
#define MS_CACHE_ONLY			0x2000000	/* Only operate on CPU cache */
#endif

/* flags for mlockall() */
#define MCL_CURRENT		0x000000001
#define MCL_FUTURE		0x000000002

#define MAP_FAILED      ((void *)-1)

#if defined(__EXT_POSIX1_200112)	/* Approved 1003.1d D14 */
#define POSIX_MADV_NORMAL		0	/* No advice to give */
#define POSIX_MADV_SEQUENTIAL	1	/* Sequentialy from lower to higher addresses */
#define POSIX_MADV_RANDOM		2	/* Random order */
#define POSIX_MADV_WILLNEED		3	/* Expects to access specified range */
#define POSIX_MADV_DONTNEED		4	/* Will not access specified range */
#endif

#if defined(__EXT_QNX)
#define NOFD            	(-1)
#define MAP_DEVICE_FAILED	((_Uintptrt)MAP_FAILED)

#if defined(__X86__) \
 || defined(__PPC__) \
 || defined(__MIPS__) \
 || defined(__SH__) \
 || defined(__ARM__)
	#define __PAGESIZE 0x1000
#else
    #error PAGESIZE not configured for system
#endif
#endif

__BEGIN_DECLS

#if defined(__EXT_QNX)
#define UNMAP_INIT_REQUIRED	0x00000001
#define UNMAP_INIT_OPTIONAL	0x00000002

extern void *_mmap(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off64_t __off, unsigned __align, void **__base, _CSTD size_t *__size);
extern void *_mmap2(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off64_t __off, unsigned __align, unsigned __preload, void **__base, _CSTD size_t *__size);
extern _Uintptrt mmap_device_io(_CSTD size_t __len, _Uint64t __io);
extern void *mmap_device_memory(void *__addr, _CSTD size_t __len, int __prot, int __flags, _Uint64t __physical);
extern int munmap_device_io(_Uintptrt __io, _CSTD size_t __len);
extern int munmap_device_memory(void *__addr, _CSTD size_t __len);
extern int munmap_flags(void *__addr, _CSTD size_t __len, unsigned __flags);
#endif

#if defined(__EXT_POSIX1_200112)	/* Approved 1003.1d D14 */
extern int posix_madvise(void *__addr, _CSTD size_t __len, int __advice);
#endif

#if _LARGEFILE64_SOURCE - 0 > 0 || _FILE_OFFSET_BITS - 0 == 64
extern void *mmap64(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off64_t __off);
#endif
#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__WATCOMC__)
extern void *mmap(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off_t __off);
#pragma aux mmap "mmap64";
#else
static __inline void * __attribute__((__unused__)) mmap(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off_t __off) {
    return mmap64(__addr, __len, __prot, __flags, __fd, __off);
}
#endif
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
extern void *mmap(void *__addr, _CSTD size_t __len, int __prot, int __flags, int __fd, off_t __off);
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
extern int mprotect(void *__addr, _CSTD size_t __len, int __prot);
extern int mlock(__const void * __addr, _CSTD size_t __len);
extern int mlockall(int __flags);
extern int msync(void *__addr, _CSTD size_t __len, int __flags);
extern int munmap(void *__addr, _CSTD size_t __len);
extern int munlock(__const void *__addr, _CSTD size_t __len);
extern int munlockall(void);

extern int shm_open(__const char *__path, int __oflag, _CSTD mode_t __mode);
extern int shm_unlink(__const char *__path);

#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1j D10 */

#define POSIX_TYPED_MEM_ALLOCATE			0x0001
#define POSIX_TYPED_MEM_ALLOCATE_CONTIG		0x0002
#define POSIX_TYPED_MEM_MAP_ALLOCATABLE		0x0004
extern int posix_typed_mem_open(__const char *__name, int __oflag, int __tflag);

struct __posix_typed_mem_info64 {
    _Uint64t            posix_tmi_length;
    _Uint32t            __reserved[4];
};

struct posix_typed_mem_info {
#if __INT_BITS__ == 32
#if defined(__LITTLEENDIAN__)
    _CSTD size_t        posix_tmi_length;
    _Uint32t            __posix_tmi_length_hi;
#elif defined(__BIGENDIAN__)
    _Uint32t            __posix_tmi_length_hi;
    _CSTD size_t        posix_tmi_length;
#else
 #error endian not configured for system
#endif
#if defined(__EXT_QNX)
/*
 * POSIX defines only the posix_tmi_length field and explicitly does not allow
 * posix_tmi_length_hi to be visible, so only define it as a QNX extension.
 */
 #define posix_tmi_length_hi	__posix_tmi_length_hi
#endif
#elif __INT_BITS__ == 64
    _CSTD size_t        posix_tmi_length;
#else
 #error Not configured for number of INT bits
#endif
    _Uint32t            __reserved[4];
};
extern int posix_typed_mem_get_info(int __fd, struct posix_typed_mem_info *__info);

#if _LARGEFILE64_SOURCE - 0 > 0 || _FILE_OFFSET_BITS - 0 == 64
extern int posix_mem_offset64(__const void *__addr, _CSTD size_t __len, off64_t *__off, _CSTD size_t *__contig_len, int *__fd);
#endif
#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__WATCOMC__)
extern int posix_mem_offset(__const void *__addr, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len, int *__fd);
#pragma aux posix_mem_offset "posix_mem_offset64";
#else
static __inline int __attribute__((__unused__)) posix_mem_offset(__const void *__addr, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len, int *__fd) {
    return posix_mem_offset64(__addr, __len, __off, __contig_len, __fd);
}
#endif
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
extern int posix_mem_offset(__const void *__addr, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len, int *__fd);
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
#endif


#if defined(__EXT_QNX)		/* 1003.1j D5, used only for getting physical addresses */
#if _LARGEFILE64_SOURCE - 0 > 0 || _FILE_OFFSET_BITS - 0 == 64
extern int mem_offset64(__const void *__addr, int __fd, _CSTD size_t __len, off64_t *__off, _CSTD size_t *__contig_len);
#endif
#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__WATCOMC__)
extern int mem_offset(__const void *__addr, int __fd, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len);
#pragma aux mem_offset "mem_offset64";
#else
static __inline int __attribute__((__unused__)) mem_offset(__const void *__addr, int __fd, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len) {
    return mem_offset64(__addr, __fd, __len, __off, __contig_len);
}
#endif
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
extern int mem_offset(__const void *__addr, int __fd, _CSTD size_t __len, off_t *__off, _CSTD size_t *__contig_len);
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
#endif

#if defined(__EXT_QNX)		/* From posix 1003.1j D5 */
/* From posix 1003.1j D5 !!!!!!!!!!!!! USE posix_typed_* calls instead !!!!!!!!! */

struct mem_info {
#if __INT_BITS__ == 32
#if defined(__LITTLEENDIAN__)
    _CSTD size_t        mi_length;
    _Uint32t            mi_length_hi;
#elif defined(__BIGENDIAN__)
    _Uint32t            mi_length_hi;
    _CSTD size_t        mi_length;
#else
 #error endian not configured for system
#endif
#elif __INT_BITS__ == 64
    _CSTD size_t        mi_length;
#else
 #error Not configured for number of INT bits
#endif
    _Uint32t            mi_reserved[4];
};

#define mem_get_info(__fd, __flags, __info) \
	posix_typed_mem_get_info((__fd), (struct posix_typed_mem_info *)(__info))
#endif

#if defined(__EXT_QNX)

#define TYPEMEM_BACKSTORE   "/backstore"        /* Memory that can be swapped out to slower storage */
#define TYPEMEM_SYSRAM      "/sysram"           /* System memory pool */
#define TYPEMEM_DEVICE      "/device"           /* Devices mapping areas (e.g. video buffers) */

#define TYPEMEM_SYSRAM_ISA  "/sysram.isa"       /* X86 system memory below 16M (for DMA) */
#define TYPEMEM_SYSRAM_DMA  "/sysram.dma"       /* X86 memory below 16M not crossing 64K boundary. */
#define TYPEMEM_DEVICE_ISA  "/device.isa"       /* X86 device memory below 16M (Usually 640K-1M) */

extern int shm_ctl(int __fd, int __flags, _Uint64t __physical, _Uint64t __size);
extern int shm_ctl_special(int __fd, int __flags, _Uint64t __physical, _Uint64t __size, unsigned __special);

#define SHMCTL_ANON			0x00000001			/* Allocate anon memory for object */
#define SHMCTL_PHYS			0x00000002			/* Providing physical memory for object */
#define SHMCTL_ISADMA		0x00000004			/* Memory should be suitable for ISA DMA (e.g. < 16M, NOX64) */
#define SHMCTL_GLOBAL		0x00000010			/* Hint that object is globally visible */
#define SHMCTL_PRIV			0x00000020			/* Hint that accessing object requires privilege */
#define SHMCTL_LOWERPROT	0x00000040			/* Hint that access to this object can use lower protection in exchange for better performance */
#define SHMCTL_LAZYWRITE	0x00000080			/* Hint that mapping of this object could use lazy-writing mechanisms */
#define SHMCTL_NOX64K		0x00000100			/* Memory should not cross 64K boundry */
#define SHMCTL_HIGHUSAGE	0x00000200			/* Hint that the object is high usage and the system should use limited resources for better performance */
#define SHMCTL_LAZY			0x00000400			/* Delay actually allocating memory until first reference */

#define SHMCTL_HAS_SPECIAL	0x00008000
#define SHMCTL_FLAG_MASK	0x0000ffff
#endif

#include <_packpop.h>

__END_DECLS

#endif

/* __SRCVERSION("mman.h $Rev: 201352 $"); */
