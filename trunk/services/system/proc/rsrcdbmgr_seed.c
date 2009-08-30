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

#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <hw/sysinfo.h>
#include <sys/rsrcdbmgr.h>
#include "rsrcdbmgr.h"

/* When we are ready to seed, then turn this on */
#define PERFORM_SEEDING 1

/* Old style seeding means we build a big in memory array and do
   all of the magic splitting and sorting ourselves, this is 
   unfortuneate because it is now handled by the resource database
   and this is a wack of code.
#define OLD_STYLE_SEEDING
*/

struct _alloc_array {
	int 		index, count;
	rsrc_alloc_t 	*array;
};

/*************************
 This function is used to seed the resource database based on the
 values which are found in the system page.

 The logic behind this is that we will add the global ranges
 of values using a _create_ call and then we will remove data
 from those ranges using a _detach_ call, which means that 
 proc can return the missing data at a later date if it so
 chooses.
***************************/

#define BETWEEN(s1, e1, n)      ((s1) <= (n) && (n) <= (e1))
#define OVERLAP(s1, e1, s2, e2) (((s1) <= (s2) && (s2) <= (e1)) || \
                                ((s2) <= (s1) && (s1) <= (e2))) 
#define CONTAINS(s1, e1, s2, e2) (((s1) <= (s2) && (s2) <= (e1)) && \
                                 ((s1) <= (e2) && (e2) <= (e1))) 
#define CONTAINSNE(s1, e1, s2, e2) (((s1) < (s2) && (s2) < (e1)) && \
                                 ((s1) < (e2) && (e2) < (e1))) 

#if defined(PERFORM_SEEDING)

#if defined(OLD_STYLE_SEEDING)
static void add_element(struct _alloc_array *array, rsrc_alloc_t *alloc) {
	if (array->index >= array->count) {
		array->array = (rsrc_alloc_t *)_srealloc(array->array, 
		  		 array->count * sizeof(rsrc_alloc_t),
		  		 (array->count + 10) * sizeof(rsrc_alloc_t));
		array->count += 10;
	}

	memcpy(&array->array[array->index++], alloc, sizeof(*alloc));
}
#endif

static int add_range(struct _alloc_array *array, rsrc_alloc_t *alloc) {
#if defined(OLD_STYLE_SEEDING)
	int i, j, lastindex, insstart, insend;
	rsrc_alloc_t extra;

	insstart = array->index;
	add_element(array, alloc);
	insend = array->index;

	//Go through the list pruning the entry 
	for (i=0; i<insstart; i++) {
		for (j=insstart; j<array->index; j++) {
		
			//Must be the same types ...
			if ((array->array[i].flags & RSRCDBMGR_TYPE_MASK) !=
			    (array->array[j].flags & RSRCDBMGR_TYPE_MASK)) {
				continue;
			}

			//Check for overlaping start/end
			if (!(OVERLAP(array->array[i].start, 	
						array->array[i].end,
						array->array[j].start, 
						array->array[j].end))) {
				continue;
			}

			//If the new block is completely contained, nuke it
			if (CONTAINS(array->array[i].start, 
						 array->array[i].end,
						 array->array[j].start,
						 array->array[j].end)) {
				array->index--;
				memmove(&array->array[j], 
				        &array->array[j+1], 
					(array->index - j) * sizeof(extra));
				j--;	//Backup the counter to re-do this element
				continue;
			}

			//If the new entry contains the old one, then create a 
			//new block and add it in.
			if (CONTAINSNE(array->array[j].start, 
						   array->array[j].end,
						   array->array[i].start,
						   array->array[i].end)) {
				extra.start = array->array[j].start;
				extra.end = array->array[i].start -1;
				extra.flags = array->array[j].flags;
				add_element(array, &extra);
				insend = array->index;
				array->array[j].start = array->array[i].end+1;
				continue;
			}

			//If the start borders, shrink it
			if (BETWEEN(array->array[j].start, 
						array->array[j].end, 
						array->array[i].end)) {
				array->array[j].start = array->array[i].end+1;
				continue;
			}

			//If the end borders, shrink it
			if (BETWEEN(array->array[j].start, 
						array->array[j].end, 
						array->array[i].start)) {
				array->array[j].end = array->array[i].start-1;
				continue;
			}
		}
	}

	//Go through and take all the new blocks and put them 
	//in in the right spot (ie sorted by start).
	insend = array->index;
	for (i=insstart; i<insend; i++) {
		memcpy(&extra, &array->array[i], sizeof(extra));
		for (lastindex=-1, j= 0; j<i; j++) {
			if ((array->array[i].flags & RSRCDBMGR_TYPE_MASK) !=
			    (array->array[j].flags & RSRCDBMGR_TYPE_MASK)) {
				continue;
			}

			if (extra.start < array->array[j].start) {
				lastindex = j;
				break;
			}		
		}
		if (lastindex >= 0)	{
			memmove(&array->array[lastindex+1], 
					&array->array[lastindex],
					(i - lastindex) * sizeof(extra));
			memcpy(&array->array[lastindex], &extra, sizeof(extra));
		}
	}		
#else
	if (rsrcdbmgr_proc_interface(alloc, 1, RSRCDBMGR_REQ_CREATE) != EOK) {
		return -1;
	}
#endif

	return 0;
}

/* Add the interrupts from the IRQ list */
static void add_irq(struct _alloc_array *array) {
	struct intrinfo_entry *intrinfo;
	rsrc_alloc_t	alloc;
	int num;

	num = _syspage_ptr->intrinfo.entry_size / sizeof(*intrinfo);
	intrinfo = SYSPAGE_ENTRY(intrinfo);

	while (num-- > 0) {
		memset(&alloc, 0, sizeof(alloc));
		alloc.start = intrinfo->vector_base;
		alloc.end = alloc.start + intrinfo->num_vectors -1;
		alloc.flags = RSRCDBMGR_IRQ;
		if (add_range(array, &alloc) == -1) {
			if(ker_verbose >= 2) {
				kprintf("Add_range failed. Add_irq %d\n", num);
			}
		}
		intrinfo++;
	}
}

/* Return the type of this ASINFO offset item (based on aspace) */
static int get_as_type(int addrspace) {
	struct asinfo_entry *asinfo;
	char				*name;
	int					type;

	type = -1;
	while (addrspace != HWI_NULL_OFF) {
		asinfo = SYSPAGE_ENTRY(asinfo);
		asinfo = (struct asinfo_entry *)((char *)asinfo + addrspace);

		name = __hwi_find_string(asinfo->name);
		if (strcmp(name, "io") == 0) {
			type = RSRCDBMGR_IO_PORT;
			break;
		}
		else if (strcmp(name, "memory") == 0) {
			type = RSRCDBMGR_MEMORY;
			break;
		}
		addrspace = asinfo->owner;
	}
	return type;
}

/* Add the IO and MEMORY from the ASINFO section */
static void add_io_and_memory(struct _alloc_array *array) {
	struct asinfo_entry  *asinfo;
	rsrc_alloc_t alloc;
	int			 num;

	num = _syspage_ptr->asinfo.entry_size / sizeof(*asinfo);
	asinfo = SYSPAGE_ENTRY(asinfo);

	while (num-- > 0) {
		char *name;
		name = __hwi_find_string(asinfo->name);

		/*
 		Here we check to see if the item is IO or
		MEMORY memory. Note that we need to check for 
		io that lives in both worlds. 
		*/
		memset(&alloc, 0, sizeof(alloc));
		if (name && strcmp(name, "io") == 0) {
			alloc.start = asinfo->start;
			alloc.end = asinfo->end;
			alloc.flags = RSRCDBMGR_IO_PORT;
			/* We only add io that is not memory io */
			switch(get_as_type(asinfo->owner)) {
			case RSRCDBMGR_MEMORY:
				break;
			case RSRCDBMGR_IO_PORT:
			default:
				if (add_range(array, &alloc) == -1) {
					if(ker_verbose >= 2) {
						kprintf("Add range failed. add_io %d\n", num);
					}
				}
				break;
			}
		}
		else if (name && strcmp(name, "memory") == 0) {
			alloc.start = asinfo->start;
			alloc.end = asinfo->end;
			alloc.flags = RSRCDBMGR_MEMORY;
			if (add_range(array, &alloc) == -1) {
				if(ker_verbose >= 2) {
					kprintf("Add range failed. add_mem %d\n", num);
				}
			}
		}
		asinfo++;
	}
}


static void reserve_ranges(struct _alloc_array *array) {
	hwi_tag                         *tag;
	void                            *next;
	char                            *name;
	rsrc_alloc_t					alloc;
	int								type;

	//Walk the HWINFO list first
	tag = (hwi_tag *)SYSPAGE_ENTRY(hwinfo);
	while(tag->prefix.size != 0) {

        next = (hwi_tag *)((uint32_t *)tag + tag->prefix.size);
        name = __hwi_find_string(tag->prefix.name);

		//Really I need to look at the AS of this entry to 
		//determine if this is io memory or regular memory.
		if (name && strcmp(name, HWI_TAG_NAME_location) == 0) {
			if ((type = get_as_type(tag->location.addrspace)) == -1) {
				tag = next;
				continue;
			}
			alloc.start = tag->location.base;
			alloc.end = alloc.start + tag->location.len -1;
			alloc.flags = type | RSRCDBMGR_FLAG_RSVP;
			if (add_range(array, &alloc) == -1) {
				if(ker_verbose >= 2) {
					kprintf("Add range failed. rsvp %x\n", alloc.flags);
				}
			}
		}
		else if (name && strcmp(name, HWI_TAG_NAME_irq) == 0) {
			alloc.start = 
			alloc.end = tag->irq.vector;
			alloc.flags = RSRCDBMGR_IRQ | RSRCDBMGR_FLAG_RSVP;
			if (add_range(array, &alloc) == -1) {
				if(ker_verbose >= 2) {
					kprintf("Add range failed. rsvp %x\n", alloc.flags);
				}
			}
		}
        tag = next;
	}
}
#endif

void rsrcdbmgr_seed() {
#if defined(PERFORM_SEEDING)	
	struct _alloc_array aa;
	memset(&aa, 0, sizeof(aa));

//*** Mark these areas as reserved
	reserve_ranges(&aa);

//*** Make the rest of these items as being free
	//IRQ -- Taken from the intrinfo section (vector_base, num_vectors)
	add_irq(&aa);

	//IO_PORTS & MEMORY -- Taken from the ASINFO section
	add_io_and_memory(&aa);

	//DMA -- Not yet available on the system page

#if defined(OLD_STYLE_SEEDING)
	//If this fails, we should try and do it piecewise
	if (rsrcdbmgr_proc_interface(aa.array, aa.index, RSRCDBMGR_REQ_CREATE) != EOK) {
		if(ker_verbose >= 2) {
			kprintf("Problem seeding rsrc database \n");
		}
	}

	_sfree(aa.array, aa.count * sizeof(aa.array));
#endif

#endif
}

__SRCVERSION("rsrcdbmgr_seed.c $Rev: 153052 $");
