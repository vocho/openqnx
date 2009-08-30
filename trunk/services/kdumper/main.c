#include <stdlib.h>
#include <unistd.h>
#include <sys/image.h>
#include <sys/startup.h>
#include "kdumper.h"


int				interesting_faults = SIGCODE_FATAL | SIGCODE_INTR | SIGCODE_KERNEL | SIGCODE_PROC;
int				debug_flag;
struct kdump	*dip;
struct writer	*writer;

int
main(int argc, char **argv) {
	int 				t;
	uintptr_t  			start_vaddr;
	unsigned			alloc_size;
	struct kdump		temp;
	int					async_channel = -1;

	cpu_init_map();
	kprintf_init();
	fault_init();

	writer = &available_writers[0];

	memset(&temp, 0, sizeof(temp));
	temp.kp_size = 512;
	while((t = getopt(argc, argv, "aAb:BcCIkKl:Uvw:")) != -1) {
		switch(t) {
        case 'a':
            /* KDUMP_ALL takes precedence over KDUMP_ACTIVE */
            if ((temp.dump_type & KDUMP_MEM_MASK) == KDUMP_SYSTEM) {
                temp.dump_type = (temp.dump_type & (~KDUMP_MEM_MASK)) | KDUMP_ACTIVE;
			}
			break;
		case 'A':	
            temp.dump_type = (temp.dump_type & (~KDUMP_MEM_MASK)) | KDUMP_ALL;
			break;
		case 'b':	
			async_channel = strtoul(optarg, NULL, 0);
			break;
		case 'B':
			temp.big = 1;
			break;
		case 'c':
			temp.compress = -1;
			break;
		case 'C':
			temp.compress = 1;
			break;
		case 'I':
			temp.dump_type = (temp.dump_type & (~KDUMP_IFS_MASK)) | KDUMP_IFS_FULL;
			break;
		case 'k':
			temp.kdebug_wanted = 0;
			break;
		case 'K':
			temp.kdebug_wanted = 1;
			break;
		case 'l':
			temp.kp_size = strtoul(optarg, NULL, 0);
			if(temp.kp_size < 100) temp.kp_size = 100;
			break;
		case 'U':
			interesting_faults |= SIGCODE_USER;
			break;
		case 'v':	
			++debug_flag;
			break;
		case 'w':
			writer = &available_writers[0];
			for( ;; ) {
				if(writer->name == NULL) {
					kprintf("'%s' writer unrecognized - using default\n", optarg);
					writer = &available_writers[0];
					break;
				}
				if(strcmp(writer->name, optarg) == 0) break;
				++writer;
			}
			break;
		}
	}

	alloc_size = sizeof(*dip) - 1 + temp.kp_size;
	private->kdump_info = alloc_pmem(alloc_size, 0);
	if(debug_flag) {
		kprintf("Kernel Dumper information structure at paddr %P\n", 
					(paddr_t)private->kdump_info);
	}
	dip = cpu_map(private->kdump_info, alloc_size, PROT_READ|PROT_WRITE, -1, NULL);
	if(dip != NULL) {
		memset(dip, 0, alloc_size);
		*dip = temp;
	} else {
		kprintf("Could not map kdump stucture!\n");
	}
	if(dip->compress >= 0) {
		// If "-c" is used, we don't initialize for compression, which
		// may allow us to save some memory
		compress_init();
	}

	if(async_channel >= 0) {
		async_check_init(async_channel);
	}

	start_vaddr = next_bootstrap();
	if(start_vaddr == 0) {
		kprintf("kdumper: No next program to start.\n");
		_exit(1);
	}

	if(debug_flag) {
		kprintf("kdumper: starting next program at 0x%x\n", start_vaddr);
	}

	startnext(start_vaddr);
	return 0;
}
