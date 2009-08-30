/**
 * This header contains the data structures and definitions related to tracking
 * process memory mapping information.
 */

#include <inttypes.h>
#include <sys/mman.h>
#include <sys/procfs.h>
#include <sys/stat.h>

#define IS_SHARED(f)   (((f) & MAP_TYPE) == MAP_SHARED)

#define IS_ELF_CODE(f) (((f) & (MAP_ELF | PROT_EXEC | PROT_WRITE | PROT_READ)) == (MAP_ELF | PROT_EXEC | PROT_READ))
#define IS_ELF_DATA(f) (((f) & (MAP_ELF | PROT_WRITE)) == (MAP_ELF | PROT_WRITE))
#define IS_ELF_SHARED_DATA(f)	((((f) & (MAP_SYSRAM|MAP_ANON)) == MAP_SYSRAM) && IS_ELF_DATA((f)))

#define IS_STACK(f) 			(((f) & MAP_STACK) == MAP_STACK)
#define IS_STACK_ALLOCATED(f) 	(((f) & (MAP_STACK | PG_HWMAPPED)) == (MAP_STACK | PG_HWMAPPED))

#define IS_GENERIC_HEAP(f) ((((f) & MAP_PRIVATEANON) == MAP_PRIVATEANON) || \
                            (((f) & (MAP_PRIVATE | MAP_ANON)) == (MAP_PRIVATE | MAP_ANON))) 

#define IS_FROM_IFS(f) ((!((f) & MAP_SYSRAM)) && ((f) & PG_HWMAPPED) && ((f) & MAP_PHYS))

//IFS files can't consume ram, private files need sys ram to consume ram
#define IS_RAM_CONSUMING(f) (!IS_FROM_IFS((f)) && (IS_SHARED((f)) || ((f) & MAP_SYSRAM)))

enum mem_types {
	TYPE_UNKNOWN = 0,
	TYPE_CODE,
	TYPE_DATA,
	TYPE_HEAP,
	TYPE_STACK,
	TYPE_FILE,
	TYPE_ELF_SHARED_DATA,
	TYPE_MAX,
};

struct slib_mapinfo_block;

/**
 * This is a basic memory map block structure, in the future we may use the 
 * straight mapinfo structure, but for now not all of its fields are required.
 */
typedef struct memblock {
	uint64_t	size;
	paddr_t		paddr;				//FUTURE:
	int			type;				//TYPE_* (derivable)
	int 		flags;				//From mapinfo
	int 		zero;				//Empty field
	char		*name;		
} memblock_t;

/**
 * Shared memory map extension of the memory map
 */
typedef struct shared_memblock {
	memblock_t 	memblock;
	dev_t		dev;
	off_t		offset;
	ino_t		ino;
	pid_t		pid;				//If != 0 then only a single "owner"
} shared_memblock_t;

/**
 * Private memory map extension of the memory map
 */
typedef struct priv_memblock {
	memblock_t			memblock;
	uint64_t			vaddr;				//Handy information
	pthread_t			tid;				//If TYPE_STACK
	int					shared_index;		//If TYPE_DATA|TYPE_CODE
} priv_memblock_t;	


