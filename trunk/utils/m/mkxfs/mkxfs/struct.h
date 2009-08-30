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
#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/elf_notes.h)
#include _NTO_HDR_(sys/elf_dyn.h)

#include _NTO_HDR_(sys/startup.h)
#include _NTO_HDR_(sys/image.h)

#include <sys/types.h>

#if defined(__QNX__)     ||	\
	defined(__SOLARIS__) ||	\
	defined(linux)

	#define HOST_HAS_EXECUTE_PERM	1

#elif defined(__NT__) || defined(__CYGWIN__) || defined (__MINGW32__)

	#undef HOST_HAS_EXECUTE_PERM

#else

	#error HOST_HAS_EXECUTE_PERM not set

#endif
	

#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

#define SWAP16( val ) ( (((val) >> 8) & 0xff) | (((val) << 8) & 0xff00) )


#define TREE_FLAGS_PROCESSED	0x0001
struct tree_entry {
	struct tree_entry		*sibling;	// This MUST be the first member
	struct tree_entry		*parent;
	struct tree_entry		*child;
	struct file_entry		*fip;		// Will be NULL if there is no info
	unsigned				 flags;		// For the exclusive use of the filesystem module
	char					 name[1];	// Variable length field
} ;

#define STACK_GROW  10
struct inode_stack {
	int					count;
	int					size;
	ino_t			  	*inodes;
};

struct name_list {
	struct name_list	*next;
	char				name[1]; // varible sized
};

struct keep_section;

#define FILE_FLAGS_BOOT			0x0001
#define FILE_FLAGS_SCRIPT		0x0002
#define FILE_FLAGS_STARTUP		0x0004
#define FILE_FLAGS_MUST_RELOC	0x0008
#define FILE_FLAGS_EXEC			0x0010
#define FILE_FLAGS_RELOCATED	0x0020
#define FILE_FLAGS_RUNONCE		0x0040
#define FILE_FLAGS_STRIP_RELOCS	0x0080
#define FILE_FLAGS_SO			0x0100
#define FILE_FLAGS_DATA_IN_RAM	0x0200
#define FILE_FLAGS_CODE_IN_RAM	0x0400
#define FILE_FLAGS_CRC_VALID	0x0800

struct file_entry {
	struct file_entry		*next;		// This must be the first member
	char					*hostpath;
	char					*targpath;
	int						 host_perms;
	int						 host_uid;
	int						 host_gid;
	time_t					 host_mtime;
	unsigned				 size;
	unsigned				 ram_offset;	// Needed for physical system only
	unsigned				 file_offset;
	unsigned				 run_offset;
	unsigned				 entry;			// Needed for startup only
	int						 big_endian;
	int						 machine;
	int						 flags;
	unsigned				 inode;
	struct attr_file_entry	*attr;
	struct bootargs_entry	*bootargs;
	char					*linker;
	struct keep_section		*sect;
	uint32_t				host_file_crc;
};

struct tmpfile_entry {
	struct tmpfile_entry	*next;
	char					*name;
};

struct attr_file_entry {
	char						*cd;
	char						*search_path;
	char						*prefix;
	char						*filter;
	struct name_list			*keepsection;
	struct name_list			*module_list;
	int							 mode;
	int							 gid;
	int							 uid;
	unsigned					 perms_mask;
	unsigned					 perms_set;
	unsigned					 dperms_mask;
	unsigned					 dperms_set;
	unsigned					 uip_flags;
	unsigned					 raw : 1;
	unsigned					 inherit_uid : 1;
	unsigned					 inherit_gid : 1;
	unsigned					 compress : 1;
	unsigned					 bootfile : 1;
	unsigned					 scriptfile : 1;
	unsigned					 keep_linked : 1;
	unsigned					 optional : 1;
	unsigned					 keep_relocs : 1;
	unsigned					 strip_relocs : 1;
	unsigned					 follow_sym_link : 1;
	unsigned					 autolink : 1;
	unsigned					 newdir : 1;
	unsigned					 page_align : 1;
	unsigned					 phys_align;
	int					         phys_align_group;
};

struct attr_file_list {
	struct attr_file_list	*next;
	struct attr_file_entry	attr;
};


struct attr_script_entry {
	char							*argv0;
	int								 priority;
	int								 policy;
	unsigned						 flags;
	unsigned						 cpu;
	unsigned						 external : 1;
	union script_external_extsched	 extsched;
	} ;

struct attr_script_list {
	struct attr_script_list		*next;
	struct attr_script_entry	attr;
};


struct attr_types {
	char	*name;
	int		 code;
};

#define MAX_CLASS_ELTS	16
enum {
	CLASS_MACHINE,
	CLASS_EHDR,
	CLASS_PHDR,
	CLASS_LAST
};

struct linker_entry {
	struct linker_entry	*next;
	struct elf_class {
		unsigned	num;
		unsigned	list[MAX_CLASS_ELTS];
	}				class[CLASS_LAST];
	char			spec[1];
};


struct attr_booter_entry {
	char				*name;
	struct linker_entry	*linker;
	char				*filter_spec;
	char				*filter_args;
	char				*data;
	unsigned			data_len;
	unsigned			boot_len;
	unsigned			notloaded_len;
	unsigned			vboot_addr;
	unsigned			pagesize;
	unsigned			paddr_bias;
	unsigned			virtual : 1;
	unsigned			processed : 1;
	unsigned			copy_filter : 1;
	unsigned			rsvd_vaddr : 1;
};

struct addr_space {
	unsigned	addr;
	unsigned	maxsize;
	unsigned	totalsize;
	unsigned	minsize;
	unsigned	align;
	unsigned	endaddr;
};

#define TOKENLEN	4096
#define TOKENC		100
struct token_state {
	struct token_state	*prev;
	char				*v[TOKENC];
	unsigned			c;
	char				buf[TOKENLEN];
};

//
// Proto's
//
void ifs_parse_init(struct attr_file_entry *attrp);
void ifs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp);
int ifs_need_seekable(struct file_entry *list);
unsigned ifs_make_fsys(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname);
void ifs_section(struct name_list **owner, const char *name);

void ffs_parse_init(struct attr_file_entry *attrp);
void ffs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp);
int ffs_need_seekable(struct file_entry *list);
unsigned ffs_make_fsys_2(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname);
unsigned ffs_make_fsys_3(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname);

void etfs_parse_init(struct attr_file_entry *attrp);
void etfs_parse_attr(int tokenc, char *tokenv[], struct attr_file_entry *attrp);
int etfs_need_seekable(struct file_entry *list);
unsigned etfs_make_fsys(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname);

int aps_lookup(char *aps);

void parse_addr_space_spec(struct addr_space *, char *);
void parse_file(FILE *src_fp);
void parse_file_name(int tokenc, char *tokenv[], struct attr_file_entry *attrp);
struct file_entry *add_file(struct file_entry **list, char *host, char *target, struct attr_file_entry *attrp, struct stat *);
void collect_dir(char *host, char *target, struct attr_file_entry *attrp, int callindex);
char *collect_inline_file(char *input, char *tmpname);

struct token_state *push_token_state();
void pop_token_state();

void parse_script_init(struct attr_script_entry *attrp);
void parse_script(FILE *src_fp, FILE *dst_fp);
void parse_script_attr(int tokenc, char *tokenv[], struct attr_script_entry *attrp);
void parse_script_cmd(FILE *dst_fp, int tokenc, char *tokenv[], struct attr_script_entry *attrp);

void proc_booter_data(char *name, int virtual);
void proc_booter_filter(unsigned startup_offset, char *intermediate_path, char *output_path);

void mk_script(FILE *dst_fp, struct file_entry *script);

struct attr_file_list * attr_in_attr_file_list(struct attr_file_entry *attrp);
struct attr_file_list * copy_attr_to_attr_file_list(struct attr_file_entry *attrp);
struct attr_file_list * add_attr(struct attr_file_entry *attrp);
struct file_entry * dir_in_file_list(char *hostpath, char *targpath);

void	push_stack(struct inode_stack *is, ino_t inode);
int		inode_in_stack(struct inode_stack *is, ino_t inode);
void	pop_stack (struct inode_stack *is);
void	destroy_stack(struct inode_stack *is);


void strip_pathname(char *path);
char *tokenize(char *input, char *space, char term);
void error_exit(char *format, ...);
int  decode_attr(int report_err, struct attr_types *atp, char *name, int *ivalp, char **svalp);
char *mk_tmpfile();
unsigned long getsize(char *str, char **dst);

short int swap16(int target_endian, int val);
long  int swap32(int target_endian, int val);

char *find_file(char *search, char *hbuf, struct stat *sbuf, char *host, int optional);
void set_cpu(const char *name, int overwrite);

struct tree_entry *make_tree(struct file_entry *list);
void print_tree(struct tree_entry *trp, int level);
int crc32_fn(char* filename, uint32_t *crc32val);
int crc32_fd(int fd, uint32_t *crc32val);

#if defined (__WIN32__) || defined(__NT__)
void fixenviron(char *line, int size);
#endif

//
// Some global vars shared between files.
//
extern int	 target_endian;
extern int	 host_endian;
extern struct addr_space image;
extern struct addr_space ram;
extern struct addr_space default_image;
extern struct addr_space default_ram;
extern int   block_size;
extern int   cluster_size;
extern int   num_blocks;
extern int   spare_blocks;
extern int	 chain_paddr;
extern int	 compressed;
extern int	 verbose;
extern int	 split_image;
extern FILE	*debug_fp;
extern struct attr_booter_entry booter;
extern struct token_state *token;
extern char	*boot_attr_buf;
extern char *mountpoint;
extern int ext_sched;
extern int no_time; /* no timestamps - declared in mkxfs.c */
extern int new_style_bootstrap;
extern char *symfile_suffix;

#define RUP(n, pagesize)	(((n) + ((pagesize)-1)) & ~((pagesize)-1))
#define RDN(n, pagesize)	((n) & ~((pagesize)-1))

#define ATTRIBUTE_SET		(~0u ^ (~0u >> 1))
