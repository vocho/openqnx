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



#include <lib/compat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/stat.h>
#include "struct.h"


int					block_size;
int					chain_paddr;
int					compressed;
int 				split_image;
struct addr_space	image;
struct addr_space	ram;
char				*mountpoint;

int					cluster_size;
int					num_blocks;
int					spare_blocks=1;
static int			align_group_id;

enum common_attrs {
	ATTR_BIGENDIAN,
	ATTR_CD,
	ATTR_FILTER,
	ATTR_GID,
	ATTR_MOUNT,
	ATTR_OPTIONAL,
	ATTR_PERMS,
	ATTR_PREFIX,
	ATTR_SEARCH,
	ATTR_TYPE,
	ATTR_UID,
	ATTR_FOLLOWLINK,
	ATTR_DPERMS
};

struct attr_types common_attr_table[] = {
	{ "bigendian",	ATTR_BIGENDIAN },
	{ "cd=",		ATTR_CD },
	{ "filter=",	ATTR_FILTER },
	{ "gid=",		ATTR_GID },
	{ "mount=",		ATTR_MOUNT },
	{ "optional",	ATTR_OPTIONAL },
	{ "perms=",		ATTR_PERMS },
	{ "prefix=",	ATTR_PREFIX },
	{ "search=",	ATTR_SEARCH },
	{ "type=",		ATTR_TYPE },
	{ "uid=",		ATTR_UID },
	{ "followlink",	ATTR_FOLLOWLINK },
	{ "dperms=",	ATTR_DPERMS },
	{ NULL }
};

void
parse_addr_space_spec(struct addr_space *space, char *sval) {
	unsigned long	v;
	char			*next;

	v = getsize(sval, &next);
	if(sval != next) space->addr = v;
	sval = next;
	for( ;; ) {
		switch(*sval) {
		case '-':
			space->endaddr = getsize(sval + 1, &sval);
			break;
		case ',':
			space->maxsize = getsize(sval + 1, &sval);
			break;
		case '=':
			space->totalsize = getsize(sval + 1, &sval);
			break;
		case '%':
			space->align = getsize(sval + 1, &sval);
			break;
		default:
			return;
		}
	}
}


static void
add_symbolic_perms_spec(unsigned *mask, unsigned *set, char *sval) {
	unsigned	who;
	int			op = 0;
	unsigned	perm;

	/*
	 *		mode	::= clause[,clause]
	 *		clause	::= [who] op [perm]
	 *		who		::= [u|g|o|a]...
	 *		op		::= +|-|=
	 *		perm	::= [r|w|x|s|g|t]...
	 */
	#define WHO_USER		S_IRWXU
	#define WHO_GROUP		S_IRWXG
	#define WHO_OTHER		S_IRWXO

	#define PERM_READ		(S_IRUSR|S_IRGRP|S_IROTH)
	#define PERM_WRITE		(S_IWUSR|S_IWGRP|S_IWOTH)
	#define PERM_EXECUTE	(S_IXUSR|S_IXGRP|S_IXOTH)
	#define PERM_BITS		(S_ISUID|S_ISGID|S_ISVTX)

	*mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
	*set = 0;
	for( ;; ) {
		who = 0;
		for( ;; ) {
			if(*sval == 'u') {
				who |= WHO_USER;
			} else if(*sval == 'g') {
				who |= WHO_GROUP;
			} else if(*sval == 'o') {
				who |= WHO_OTHER;
			} else if(*sval == 'a') {
				who |= WHO_USER|WHO_GROUP|WHO_OTHER;
			} else {
				break;
			}
			++sval;
		}
		if(who == 0) who = WHO_USER|WHO_GROUP|WHO_OTHER;
		switch(*sval++) {
		case '-': op = -1; break;
		case '=': op =  0; break;
		case '+': op = +1; break;
		default:
			error_exit("Invalid permissions operator '%c'.\n", sval[-1]);
			break;
		}
		perm = 0;
		for( ;; ) {
			if(*sval == 'r') {
				perm |= PERM_READ;
			} else if(*sval == 'w') {
				perm |= PERM_WRITE;
			} else if(*sval == 'x') {
				perm |= PERM_EXECUTE;
			} else if(*sval == 's') {
				perm |= S_ISUID;
			} else if(*sval == 'g') {
				perm |= S_ISGID;
			} else if(*sval == 't') {
				perm |= S_ISVTX;
			} else {
				break;
			}
			++sval;
		}
		if(perm == 0) {
			error_exit("Invalid permission type '%c'.\n", *sval);
		}
		if((perm & ~PERM_BITS) == 0) who = WHO_USER|WHO_GROUP|WHO_OTHER;
		perm &= who | PERM_BITS;
		if(op == 0) *mask &= ~who;
		if(op <= 0) *mask &= ~perm;
		if(op >= 0) *set |= perm;
		if(*sval != ',') break;
		++sval;
	}
}

static int
parse_common_attr(char *token, struct attr_file_entry *attrp) {
	int					ival;
	char				*sval;

	switch(decode_attr(0, common_attr_table, token, &ival, &sval)) {
	case ATTR_BIGENDIAN:
		target_endian = ival;
		break;
	case ATTR_CD:
		if(chdir(attrp->cd = strdup(sval)) == -1)
			error_exit("Unable to cd to %s : %s\n", sval, strerror(errno));
		break;
	case ATTR_FILTER:
		if(strstr(sval, "none") && strlen(sval) < 7) {
			attrp->filter = NULL;
			break;
		}
		attrp->filter = strdup(sval);
		break;
	case ATTR_GID:
		if(strcmp(sval, "*") == 0) {
			attrp->inherit_gid = 1;
		} else {
			attrp->inherit_gid = 0;
			attrp->gid = ival;
		}
		break;
	case ATTR_MOUNT:
		mountpoint = strdup(sval);
		break;
	case ATTR_OPTIONAL:
		attrp->optional = ival;
		break;
	case ATTR_PERMS:
		if(strcmp(sval, "*") == 0) {
			attrp->perms_mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
			attrp->perms_set = 0;
		} else if(sval[0] >= '0' && sval[0] <= '7') {
			attrp->perms_mask = 0;
			attrp->perms_set = strtoul (sval, NULL, 8);
		} else {
			add_symbolic_perms_spec(&attrp->perms_mask, &attrp->perms_set, sval);
		}
		break;
	case ATTR_PREFIX:
		attrp->prefix = strdup(sval);
		break;
	case ATTR_SEARCH:
		attrp->search_path = strdup(sval);
		break;
	case ATTR_TYPE:
		/* Always reset newdir so that any other 
		 * "type=" attribute will clear it */
		attrp->newdir = 0;
		if(strcmp(sval, "fifo") == 0) {
			attrp->mode = S_IFIFO;
		} else if(strcmp(sval, "link") == 0) {
			attrp->mode = S_IFLNK;
		} else if(strcmp(sval, "file") == 0) {
			attrp->mode = S_IFREG;
		} else if(strcmp(sval, "dir") == 0) {
			attrp->mode = S_IFDIR;
			/* specify that this is a standalone dir with
			 * newdir flag */
			attrp->newdir = 1;
		} else {
			error_exit("Invalid file type '%s'.\n", sval);
		}
		break;
	case ATTR_UID:
		if(strcmp(sval, "*") == 0) {
			attrp->inherit_uid = 1;
		} else {
			attrp->inherit_uid = 0;
			attrp->uid = ival;
		}
		break;
	case ATTR_FOLLOWLINK:
		attrp->follow_sym_link = ival;
		break;
	case ATTR_DPERMS:
		if(strcmp(sval, "*") == 0) {
			attrp->dperms_mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
			attrp->dperms_set = 0;
		} else if(sval[0] >= '0' && sval[0] <= '7') {
			attrp->dperms_mask = 0;
			attrp->dperms_set = strtoul (sval, NULL, 8);
		} else {
			add_symbolic_perms_spec(&attrp->dperms_mask, &attrp->dperms_set, sval);
		}
		break;
	case -2: /* attribute was conditional - we're ignoring it - see decode_attr*/
		return -1;
	default:
		return 0;
	}
	return 1;
}

static void
parse_common_init(struct attr_file_entry *attrp) {
	attrp->cd = getcwd(NULL, 1024);
	if(attrp->cd == NULL) {
		error_exit("Current working directory does not exist.\n");
	}
	attrp->prefix = NULL;
	attrp->uip_flags = PF_X | PF_W;
	attrp->perms_mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
	attrp->dperms_mask = PERM_READ|PERM_WRITE|PERM_EXECUTE|PERM_BITS;
	attrp->dperms_set = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
	attrp->mode = S_IFREG;
	attrp->inherit_uid = 1;
	attrp->inherit_gid = 1;
	attrp->follow_sym_link = 1;
	attrp->optional = 1;
	attrp->autolink = 1;
}


enum ifs_attrs {
	ATTR_AUTOLINK,
	ATTR_CHAIN,
	ATTR_CODE,
	ATTR_DATA,
	ATTR_IMAGE,
	ATTR_KEEPLINKED,
	ATTR_LINKER,
	ATTR_PHYSICAL,
	ATTR_RAM,
	ATTR_RAW,
	ATTR_RELOCS,
	ATTR_SCRIPT,
	ATTR_VIRTUAL,
	ATTR_COMPRESS,
	ATTR_COMPRESS2,
	ATTR_PAGE_ALIGN,
	ATTR_KEEPSECTION,
	ATTR_MODULE,
	ATTR_PHYS_ALIGN,
};

struct attr_types ifs_attr_table[] = {
	{ "autolink",	ATTR_AUTOLINK },
	{ "chain=",		ATTR_CHAIN },
	{ "code=",		ATTR_CODE },
	{ "data=",		ATTR_DATA },
	{ "image=",		ATTR_IMAGE },
	{ "keeplinked",	ATTR_KEEPLINKED },
	{ "linker=",	ATTR_LINKER },
	{ "physical=",	ATTR_PHYSICAL },
	{ "ram=",		ATTR_RAM },
	{ "raw",		ATTR_RAW },
	{ "relocs",		ATTR_RELOCS },
	{ "script",		ATTR_SCRIPT },
	{ "virtual=",	ATTR_VIRTUAL },
	{ "compress",	ATTR_COMPRESS },
	{ "compress=",	ATTR_COMPRESS2 },
	{ "page_align",	ATTR_PAGE_ALIGN },
	{ "keepsection=",ATTR_KEEPSECTION },
	{ "module=",	ATTR_MODULE },
	{ "phys_align=",	ATTR_PHYS_ALIGN },
	{ NULL }
};


static void
add_linker_spec(char *sval) {
	struct linker_entry	*new;
	struct linker_entry	tmp_linker;

	memset(&tmp_linker, 0, sizeof(tmp_linker));
	if(*sval == '(') {
		unsigned	i;

		++sval;
		i = 0;
		for( ;; ) {
			if(*sval == ')') break;
			if(*sval == ';') {
				if(++i >= CLASS_LAST) {
					error_exit("Too many elf class types.\n");
				}
				++sval;
			}
			if(*sval == ',') ++sval;
			tmp_linker.class[i].list[tmp_linker.class[i].num]
				= strtoul(sval, &sval, 0);
			if(++(tmp_linker.class[i].num) > MAX_CLASS_ELTS) {
				error_exit("Too many elf class elements.\n");
			}
		}
		++sval;
	}
	if(tmp_linker.class[CLASS_EHDR].num == 0) {
		//No Elf header types specifed, assume ET_REL
		tmp_linker.class[CLASS_EHDR].num = 1;
		tmp_linker.class[CLASS_EHDR].list[0] = ET_REL;
	}
	new = malloc(sizeof(*new) + strlen(sval));
	if(new == NULL) {
		error_exit("No memory for linker spec.\n");
	}
	*new = tmp_linker;
	new->next = booter.linker;
	booter.linker = new;
	strcpy(new->spec, sval);
}

void
parse_keepsection(struct name_list **owner, char *sval) {
	char	*start;
	char	chr;

	for( ;; ) {
		start = sval;
		for( ;; ) {
			chr = *sval;
			if(chr == '\0') break;
			if(chr == ',') break;
			++sval;
		}
		if(sval > start) {
			*sval = '\0';
			ifs_section(owner, start);
		}
		if(chr == '\0') break;
		++sval;
	}
}


//                               System Runs
// Code   Data   Image In     Virtual  Physical   Comments
//  uip    uip     ram     .    yes      yes      run once
//  uip    cpy     ram     .    yes       no      run multiple
//  cpy    uip     ram     .    ---      ---      non-sensible
//  cpy    cpy     ram     .    yes      yes      run multiple but wasteful
//                         .
//  uip    uip     rom     .     no       no      not possible (1)
//  uip    cpy     rom     .    yes      yes      normal xip case
//  cpy    uip     rom     .    ---      ---      not possible (1)
//  cpy    cpy     rom     .    yes      yes      normal filesystem case
//
// (1) - Make the data cpy
//

void
ifs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp) {
	int					i;
	int					ival;
	char				*sval;
	struct name_list	**owner;
	struct name_list	*mod;
	
	attrp->module_list = NULL;
	owner = &attrp->module_list;

	for(i = 0; i < tokenc; ++i) {
		if(!parse_common_attr(tokenv[i], attrp) ) {
			switch(decode_attr(1, ifs_attr_table, tokenv[i], &ival, &sval)) {
			case ATTR_AUTOLINK:
				attrp->autolink = ival;
				break;
			case ATTR_CHAIN:
				chain_paddr = ival;
				break;
			case ATTR_CODE:
				switch(*sval) {
				case 'c':
				case 'C':
					attrp->uip_flags &= ~PF_X;
					break;
				case 'u':
				case 'U':
					attrp->uip_flags |= PF_X;
					break;
				default:
					fprintf(stderr,"warning: invalid code directive %s\n",sval);
				}
				break;
			case ATTR_DATA:
				switch(*sval) {
				case 'C':
				case 'c':
					attrp->uip_flags &= ~PF_W;
					break;
				case 'U':
				case 'u':
					attrp->uip_flags |= PF_W;
					break;
				default:
					fprintf(stderr,"warning: invalid data directive %s\n",sval);
				}
				break;
			case ATTR_IMAGE:
				parse_addr_space_spec(&image, sval);
				break;
			case ATTR_KEEPLINKED:
				attrp->keep_linked = ival;
				break;
			case ATTR_LINKER:
				add_linker_spec(sval);
				break;
			case ATTR_PHYSICAL:
				attrp->bootfile = 1;
				proc_booter_data(sval, 0);
				break;
			case ATTR_RAM:
				split_image = 1;
				attrp->uip_flags &= ~PF_W;
				parse_addr_space_spec(&ram, sval);
				break;
			case ATTR_RAW:
				attrp->raw = ival;
				break;
			case ATTR_RELOCS:
				//
				// Sets both so that we can handle default case of having
				// both of them off -> if virtual, strip, if physical, don't.
				//
				attrp->keep_relocs = ival;
				attrp->strip_relocs = ival ^ 1;
				break;
			case ATTR_SCRIPT:
				attrp->scriptfile = ival;
				break;
			case ATTR_VIRTUAL:
				attrp->bootfile = 1;
				proc_booter_data(sval, 1);
				break;
			case ATTR_COMPRESS:
				if(ival) ival = 3; //Use UCL compression as the default
				//fall through
			case ATTR_COMPRESS2:
				compressed = ival;
				break;
			case ATTR_PAGE_ALIGN:
				attrp->page_align = ival;
				break;
			case ATTR_KEEPSECTION:
				parse_keepsection(&attrp->keepsection, sval);
				break;
			case ATTR_MODULE:
				mod = malloc(sizeof(*mod) + strlen(sval));
				if(mod == NULL) {
					error_exit("No memory for module name.\n");
				}
				strcpy(mod->name, sval);
				mod->next = NULL;
				*owner = mod;
				owner = &mod->next;
				break;
			case ATTR_PHYS_ALIGN:
				attrp->phys_align = ival;
				if ( attrp->phys_align && strstr( sval, ",group" ) ) {
					attrp->phys_align_group = ++align_group_id;
				} else {
					attrp->phys_align_group = 0;
				}
				break;
			}
		}
	}
}

static char default_linker[] = {
#ifdef OLD_GCC_DRIVER
	"gcc --no-keep-memory -nostdlib -nostartfiles -nodefaultlibs -bnto"
#else
	"qcc"
#ifdef DEBUG
	" -vv"
#endif
	" -bootstrap -nostdlib -Wl,--no-keep-memory -Vgcc_nto"
#endif
	"%(m==3,x86%)%(m==6,x86%)"
	"%(m==8,mips%)"
	"%(m==20,ppc%)"
	"%(m==40,arm%)"
	"%(m==42,sh%)"
	"%(m!=3,%(m!=6,%(e==0, -EL%)%(e==1, -EB%)%)%)"
	"%(h!=0, -Wl,-Ttext -Wl,0x%t%)%(d!=0, -Wl,-Tdata -Wl,0x%d%)"
	" -o%o %i"
	"%[M -L%^i -Wl,-uinit_%^n -lmod_%n%]"
};


void
ifs_parse_init(struct attr_file_entry *attrp) {
	parse_common_init(attrp);
	attrp->search_path = "${MKIFS_PATH}";
	add_linker_spec(default_linker);
}



enum ffs_attrs {
	ATTR_BLOCK_SIZE,
	ATTR_MAX_SIZE,
	ATTR_SPARE_BLOCKS,
	ATTR_MIN_SIZE
};

struct attr_types ffs_attr_table[] = {
	{ "block_size=",	ATTR_BLOCK_SIZE},
	{ "max_size=",		ATTR_MAX_SIZE },
	{ "spare_blocks=",	ATTR_SPARE_BLOCKS },
	{ "min_size=",	ATTR_MIN_SIZE },
	{ NULL }
};

void
ffs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp) {
	int					i;
	int					ival;
	char				*sval;

	for(i = 0; i < tokenc; ++i) {
		if(!parse_common_attr(tokenv[i], attrp) ) {
			switch(decode_attr(1, ffs_attr_table, tokenv[i], &ival, &sval)) {
			case ATTR_BLOCK_SIZE:
				block_size = getsize(sval, &sval);
				break;
			case ATTR_SPARE_BLOCKS:
				spare_blocks = ival;
				break;
			case ATTR_MAX_SIZE:
				image.maxsize = getsize(sval, &sval);
				break;
			case ATTR_MIN_SIZE:
				image.minsize = getsize(sval, &sval);
				break;
			}
		}
	}
}

void
ffs_parse_init(struct attr_file_entry *attrp) {
	parse_common_init(attrp);
	attrp->search_path = "";
}



enum etfs_attrs {
	ATTR_ETFS_NUM_BLOCKS,
	ATTR_ETFS_BLOCK_SIZE,
	ATTR_ETFS_CLUSTER_SIZE,
};

struct attr_types etfs_attr_table[] = {
	{ "num_blocks=",		ATTR_ETFS_NUM_BLOCKS },
	{ "block_size=",		ATTR_ETFS_BLOCK_SIZE },
	{ "cluster_size=",		ATTR_ETFS_CLUSTER_SIZE },
	{ NULL }
};



void
etfs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp) {
	int					i;
	int					ival;
	char				*sval;

	for(i = 0; i < tokenc; ++i) {
		if(!parse_common_attr(tokenv[i], attrp) ) {
			switch(decode_attr(1, etfs_attr_table, tokenv[i], &ival, &sval)) {
			case ATTR_ETFS_NUM_BLOCKS:
				num_blocks = getsize(sval, &sval);
				break;
			case ATTR_ETFS_BLOCK_SIZE:
				block_size = getsize(sval, &sval);
				break;
			case ATTR_ETFS_CLUSTER_SIZE:
				cluster_size = getsize(sval, &sval);
				break;
			}
		}
	}
}

void
etfs_parse_init(struct attr_file_entry *attrp) {
	parse_common_init(attrp);
	attrp->search_path = "";
}

__SRCVERSION("parse_file_attr.c $Rev: 203655 $");
