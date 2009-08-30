//	Definitions for pager code

/* This is used when PAE is disabled */
/* To make these work the last entry of the page directory points to itself */

#define PG1_REMAP        0x3ff
#define PG1_PDIRADDR     (PG1_REMAP << 22 | PG1_REMAP << 12)	// Pointer to start of page directory
#define PG1_PTEADDR      (PG1_REMAP << 22)						// Pointer to first page table

#define V1TOPDIRP(v)    (uint32_t *)(PG1_PDIRADDR | (((uint32_t)(v))>>20&~3))		// Pointer to page directory entry
#define V1TOPTEP(v)     (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&~3))		// Pointer to page table entry
#define V1TOPTP(v)      (uint32_t *)(PG1_PTEADDR  | (((uint32_t)(v))>>10&0x3ff000))	// Pointer to start of page table
#define V1TOPADDR(v)	((*V1TOPTEP(v)&~(PAGESIZE-1))|((uint32_t)(v)&(PAGESIZE-1)))	// Physical address


/* This is used when PAE is enabled */
/* To make these work the last 4 entries of the PDPT point to 4 pages within a 16k page directory */

#define PG2_PDIRADDR    0xffffc000		// Pointer to start of page directory
#define PG2_PTEADDR     0xff800000		// Pointer to first page table

#define V2TOPDIRP(v)    (uint64_t *)(PG2_PDIRADDR | (((uint32_t)(v))>>18&~7))		// Pointer to page directory entry
#define V2TOPTEP(v)     (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&~7))		// Pointer to page table entry
#define V2TOPTP(v)      (uint64_t *)(PG2_PTEADDR  | (((uint32_t)(v))>>9&0x7ff000))	// Pointer to start of page table
#define V2TOPADDR(v)    ((*V2TOPTEP(v)&~(__PAGESIZE-1))|((uint32_t)(v)&(__PAGESIZE-1)))	// Physical address
