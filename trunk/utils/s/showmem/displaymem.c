/**
 * This file is responsible for providing the various types of display
 * feedback to the user about the mappings in the system.
 */
 
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#include "showmem.h"

struct io_data {
 	FILE 				*output;
	system_entry_t 		*system;
	struct showmem_opts *opts;
	uint64_t			total_private;
	uint64_t			total_shared;
};

char *formatValue(long value) {
	static char data[20];
	static char endfix;
	
	if(value > (1024 * 1024)) {
		value /= (1024 * 1024);
		endfix = 'M';
	} else if (value > 1024) {
		value /= 1024;
		endfix = 'k';
	} else {
		endfix = 'b';
	}
	
	sprintf(data, "%ld%c", value, endfix); 	

	return data;
}

static char *get_basename(char *name) {
	char *p;

	if((p = strrchr(name, '/')) != NULL) {
		p++;
	} else {
		p = name;
	}
	return p;
}

/**
 * Dump the buffer content in a consistant manner.  Return the total value accumulated
 * by the line. Current formatting is:
 *  Total | Code | Data | Heap | Stack | Other
 *
 * This also has a flag to indicate if the RAM was system ram or not
 */
static uint64_t dump_totals(FILE *fp, uint64_t *totals, int *isram, int len, int accumulate, int pid, char *name, char *subname) {
	uint64_t 	sum = 0LL;
	int 		i;
	char		cs, ce;
		
	for(i = 0; i < len; i++) {
		sum += totals[i];
	}

	if(fp == NULL) {
		return sum;
	}
	
	//Total | Code | Data | Heap | Stack | Other
	cs = ce = ' ';
	fprintf(fp, "%c%8lld%c ", cs, ((accumulate) ? sum : 0LL), ce);
	cs = (totals[TYPE_CODE] && isram && !isram[TYPE_CODE]) ? '(' : ' ';
	fprintf(fp, "%c%8lld%c ", cs, totals[TYPE_CODE], (cs == ' ') ? ' ' : ')');
	cs = (totals[TYPE_DATA] && isram && !isram[TYPE_DATA]) ? '(' : ' ';
	fprintf(fp, "%c%8lld%c ", cs, totals[TYPE_DATA], (cs == ' ') ? ' ' : ')');
	cs = (totals[TYPE_HEAP] && isram && !isram[TYPE_HEAP]) ? '(' : ' ';
	fprintf(fp, "%c%8lld%c ", cs, totals[TYPE_HEAP], (cs == ' ') ? ' ' : ')');
	cs = (totals[TYPE_STACK] && isram && !isram[TYPE_STACK]) ? '(' : ' ';
	fprintf(fp, "%c%8lld%c ", cs, totals[TYPE_STACK], (cs == ' ') ? ' ' : ')');
	cs = (totals[TYPE_UNKNOWN] && isram && !isram[TYPE_UNKNOWN]) ? '(' : ' ';
	fprintf(fp, "%c%8lld%c ", cs, totals[TYPE_UNKNOWN], (cs == ' ') ? ' ' : ')');
	fprintf(fp, "%10d ", pid);

	if(subname != NULL) {
		fprintf(fp, "%s (%s)\n", name, subname);
	} else {
		fprintf(fp, "%s\n", name);
	}
	
	return sum;
}

static shared_memblock_t *get_shared_block(system_entry_t *system, int index) {
	if(index < 0 || index >= system->shared_blocks.block_count) {
		return NULL;
	}
	
	return &system->shared_blocks.memblocks[index];
}

/**
 * This reports the data structure associated with a particular process.  It
 * provides the report in a two fold manner:
 * [Process Summary: Total | Code | Data | Heap | Stack | Other]
 * [Per Library:     ----- | XXXX | YYYY | ---- | ----- | -----]
 * 
 * The Process Summary, for the code/data portion should represent the
 * sum of the library (and executable) components underneath them.  The
 * "billing" of code/data is reported for the first instance of a library
 * or executable but is not billed for future reported processes.
 * 
 * The Total column is the summary of the Code/Data/Heap/Stack/Other
 * entries
 */ 
int walk_process_overview_callback(pid_entry_t *pmb, void *data) {
	uint64_t		totals[TYPE_MAX];
	uint64_t		ret;
	int				isram[TYPE_MAX];
	struct io_data 	*io = (struct io_data *)data;
	priv_memblock_t		*mi;
	shared_memblock_t 	*smi;
	char			*name = NULL;
	char 			*unknown = "unknown";
	char 			*procnto = "procnto";
	int 			type, i = 0;
	
	name = pmb->name;
	if(name == NULL) {
		name = (pmb->pid == 1) ? procnto : unknown;
	} else {
		name = get_basename(name);
	}
	
	memset(totals, 0, TYPE_MAX * sizeof(*totals));

	//Add all of the private donations this process has made
	for(i = 0; i < pmb->block_count; i++) {
		mi = &pmb->memblocks[i];

		if(!IS_RAM_CONSUMING(mi->memblock.flags)) {
			continue;
		}

		type = (mi->memblock.type < TYPE_MAX) ? mi->memblock.type : TYPE_UNKNOWN;
		totals[type] += mi->memblock.size;
	}
	
	//Include the unique shared library components (shared but not really)
	for(i = 0; i < io->system->shared_blocks.block_count; i++) {
		smi = &io->system->shared_blocks.memblocks[i];
		if(smi->pid != pmb->pid) {
			continue;
		}

		if(!IS_RAM_CONSUMING(smi->memblock.flags)) {
			continue;
		}

		type = (smi->memblock.type < TYPE_MAX) ? smi->memblock.type : TYPE_UNKNOWN;
		totals[type] += smi->memblock.size;
	}
	
	//Summarize the total process information 
	if(io->opts->show_process_summary) {
		ret = dump_totals(io->output, totals, NULL, TYPE_MAX, 1, pmb->pid, name, NULL);
	} else {
		ret = dump_totals(NULL, totals, NULL, TYPE_MAX, 1, pmb->pid, name, NULL);
	}
	io->total_private += ret;

	//Show the breakdown of where these values come from
	for(i = 0; i < pmb->block_count; i++) {
		mi = &pmb->memblocks[i];
	
		//Only report on the code/data portions and what they contribute
		if(mi->memblock.type != TYPE_CODE && 
		   mi->memblock.type != TYPE_DATA && 
  		   mi->memblock.type != TYPE_STACK) {
			continue;
		}

		//Each iteration of this loop prints only specific detailed types
		memset(totals, 0, TYPE_MAX * sizeof(*totals));
		memset(isram, 0, TYPE_MAX * sizeof(*isram));
		
		//Report both the code and the data sections at the same time
		if(mi->memblock.type == TYPE_CODE || mi->memblock.type == TYPE_DATA) {
			totals[mi->memblock.type] = mi->memblock.size;
			isram[mi->memblock.type] = IS_RAM_CONSUMING(mi->memblock.flags);

			smi = get_shared_block(io->system, mi->shared_index);
			if(smi != NULL && smi->pid == pmb->pid) {
				smi->pid = -1*smi->pid;				//Mark as reported
				totals[smi->memblock.type] = smi->memblock.size;
				isram[smi->memblock.type] = IS_RAM_CONSUMING(smi->memblock.flags);
			} 
			
			if(totals[TYPE_CODE] == 0 && totals[TYPE_DATA] == 0) {
				continue;
			}

			if(io->opts->show_process_details & PROCESS_DETAIL_LIBRARY) {		
				dump_totals(io->output, totals, isram, TYPE_MAX, 0, pmb->pid, name, (mi->memblock.name) ? mi->memblock.name : unknown);
			}		
		} else if(mi->memblock.type == TYPE_STACK) {
			char threadname[20];
			sprintf(threadname, "thread %d", mi->tid);

			totals[mi->memblock.type] = mi->memblock.size;
			//NOTE: This is just a sanity check, should never happen
			isram[mi->memblock.type] = IS_RAM_CONSUMING(mi->memblock.flags);

			if(io->opts->show_process_details & PROCESS_DETAIL_STACK) {		
				dump_totals(io->output, totals, isram, TYPE_MAX, 0, pmb->pid, name, threadname);
			}		
		}
	}		

	//Show the rest of the shared libraries not reported above??
	for(i = 0; i < io->system->shared_blocks.block_count; i++) {
		smi = &io->system->shared_blocks.memblocks[i];
		if(smi->pid != pmb->pid) {
			continue;
		}
	
		memset(totals, 0, TYPE_MAX * sizeof(*totals));	
		memset(isram, 0, TYPE_MAX * sizeof(*isram));	

		type = (smi->memblock.type < TYPE_MAX) ? smi->memblock.type : TYPE_UNKNOWN;
		totals[type] += smi->memblock.size;
		isram[type] = IS_RAM_CONSUMING(smi->memblock.flags);

		smi->pid = -1*smi->pid;									//Mark as reported
		
		if(io->opts->show_process_details & PROCESS_DETAIL_LIBRARY) {		
			dump_totals(io->output, totals, isram, TYPE_MAX, 0, pmb->pid, name, (smi->memblock.name) ? smi->memblock.name : unknown);
		}
	}
		
	return 0;
}

int walk_slib_overview_callback(shared_memblock_t *smb, void *data) {
	struct io_data *io = (struct io_data *)data;
	uint64_t		totals[TYPE_MAX];
	int				isram[TYPE_MAX];
	int				type, accumulate;
	
	//Only report those entries not claimed by others
	if(smb->pid == 0) {
		memset(totals, 0, TYPE_MAX * sizeof(*totals));	
		memset(isram, 0, TYPE_MAX * sizeof(*isram));	

		type = (smb->memblock.type < TYPE_MAX) ? smb->memblock.type : TYPE_UNKNOWN;

		totals[type] += smb->memblock.size;
		isram[type] = IS_RAM_CONSUMING(smb->memblock.flags);
		if(isram[type]) {
			io->total_shared += smb->memblock.size;
			accumulate = 1;
		} else {
			accumulate = 0;
		}

		if(io->opts->show_process_summary) {
			dump_totals(io->output, totals, isram, TYPE_MAX, accumulate, -1, (smb->memblock.name) ? smb->memblock.name : "unknown", NULL);
		} 
	}	
	
	return 0;
}

uint64_t normalize_data_size(uint64_t size, char **symp) {
    static char *syms[] = { "", "K", "M", "G", "T", NULL };
    char        **sym;

    sym = syms;
    while((size > 8192) && (*sym != NULL)) {
        size /= 1024;
        ++sym;
    }
    *symp = *sym;
    return size;
}

static uint64_t get_total_mem(void) {
    char                    *str = SYSPAGE_ENTRY(strings)->data;
    struct asinfo_entry     *as = SYSPAGE_ENTRY(asinfo);
    uint64_t                total = 0;
    unsigned                num;

    for(num = _syspage_ptr->asinfo.entry_size / sizeof(*as); num > 0; --num) {
        if(strcmp(&str[as->name], "ram") == 0) {
            total += as->end - as->start + 1;
        }
        ++as;
    }
    return total;
}

void display_overview(system_entry_t *root, struct showmem_opts *opts) {
	struct io_data io;
	
	memset(&io, 0, sizeof(io));
	io.output = stdout;
	io.system = root;
	io.opts = opts;
	
	if(opts->show_process_summary) {
		fprintf(stdout, "Process listing (Total, Code, Data, Heap, Stack, Other)\n");
	}
	iterate_processes(root, walk_process_overview_callback, &io);	
	
	if(opts->show_process_summary) {
		fprintf(stdout, "Shared shared objects (Total, Code, Data, Heap, Stack, Other)\n");
	}
	iterate_sharedlibs(root, walk_slib_overview_callback, &io);	
	
	if(opts->show_system_summary) {
		char 		*c;
		uint64_t 	norm, total;

		total = get_total_mem();
		norm = normalize_data_size(total, &c);
		fprintf(stdout, "System RAM:   %6lld%s (%14lld) \n", norm, c, total);

		total = io.total_private + io.total_shared;
		norm = normalize_data_size(total, &c);
		fprintf(stdout, "Total Used:   %6lld%s (%14lld) \n", norm, c, total);

		norm = normalize_data_size(io.total_private, &c);
		fprintf(stdout, " Used Private:%6lld%s (%14lld) \n", norm, c, io.total_private);
		norm = normalize_data_size(io.total_shared, &c);
		fprintf(stdout, " Used Shared: %6lld%s (%14lld) \n", norm, c, io.total_shared);
	}
}

void iterate_processes(system_entry_t *root, 
                       int (* func)(pid_entry_t *block, void *data), void *data) {
	int ret;
	pid_entry_t *item;
	
	for(item = root->pid_list; item != NULL; item = item->next) {
		ret = func(item, data);
		if(ret < 0) {
			break;
		}
	}
}

void iterate_sharedlibs(system_entry_t *root, 
                       int (* func)(shared_memblock_t *block, void *data), void *data) {
	int i, ret;
	
	for(i = 0; i < root->shared_blocks.block_count; i++) {
		ret = func(&root->shared_blocks.memblocks[i], data);
		if(ret < 0) {
			break;
		}
	}
}                 
