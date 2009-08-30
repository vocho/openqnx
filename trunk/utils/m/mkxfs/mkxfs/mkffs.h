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





#include <inttypes.h>

#define FNULL                       0xffff
#define MAX_WRITE                   0x4000
#define MAX_BLOCKS					0x0200
#define FFS_SORT_ENTRY_SET			0x0001
#define COMPRESSED					0x0002

typedef enum {
	MAP_NEVER,
	MAP_ALWAYS,
	MAP_UPPER
} mapping_t;

typedef struct ffs_sort{
	short int status;
	unsigned int dir_pos;
	unsigned int extent_pos;
	char *name;
	char *sibling;
	char *parent;
	char *child;
	char *host_fullpath;
 	unsigned int size_of_entry;
	unsigned int entry_pos;
	uint16_t level;
	uint32_t perms;
    uid_t  uid;
    uid_t  gid;
    time_t ffs_mtime;   	
    time_t ffs_ftime;   	
    mode_t mode;
	struct ffs_sort *next;
	struct ffs_sort *prev;
	struct file_entry *fip;
} ffs_sort_t;

typedef struct block_info_s{
 	unsigned short int block_index;
	unsigned int block_size;
	unsigned int available_space;
	unsigned int offset_top;
	unsigned int offset_bottom;
	unsigned int pos;
	unsigned int extent_index;
	struct block_info_s *next;
	struct block_info_s *prev;
} block_info_t;

extern int				flashimage;
extern uint32_t			block_index;
extern block_info_t **	block_info;
extern uint8_t *		blk_buffer_ptr;
extern uint8_t *		tmp_name;

extern void			mk_flash_exit(char *format, ...);
extern ffs_sort_t *	ffs_entries(struct tree_entry *trp, uint8_t *mountpoint);
extern void			write_image(FILE *fp);
extern ffs_sort_t *	find_child(ffs_sort_t *sort);
extern ffs_sort_t *	find_sibling(ffs_sort_t *sort);
extern unsigned		calc_log2(unsigned blksize);
extern int			ffs_need_seekable(struct file_entry *list);

#if 0
extern void spare_block_alloc(block_info_t *block_info);
extern void write_extent(block_info_t *block_info, f3s_extptr_t next, uint16_t extent_flags, uint32_t size);
extern block_info_t *write_dir_entry(ffs_sort_t *sorted_list, block_info_t *block_info);
extern void	child_extent(ffs_sort_t *sort, uint32_t size_child_entry, block_info_t *block_info);
extern void	sibling_extent(ffs_sort_t *sort, uint32_t size_sib_entry, block_info_t *block_info);
extern void	file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info);
extern void	first_file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info);
extern block_info_t *write_file(ffs_sort_t *sorted_list, block_info_t *block_info);
extern block_info_t *write_lnk(ffs_sort_t *sorted_list, block_info_t *block_info);
extern void write_f3s(ffs_sort_t *sort, int block_size);
extern void write_boot_record(block_info_t *block_info, uint16_t spare, ffs_sort_t *sort);
extern block_info_t *init_block(int block_size, uint16_t unit_flags);
extern void write_image(FILE *fp);
extern ffs_sort_t *ffs_entries(struct tree_entry *trp, uint8_t *mountpoint);
#endif

