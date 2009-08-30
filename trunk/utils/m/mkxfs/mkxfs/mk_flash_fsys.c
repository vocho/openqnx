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
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "struct.h"
#ifdef HAVE_F3S
	#include _NTO_HDR_(sys/f3s_spec.h)
	#include _NTO_HDR_(sys/f3s_comp.h)
#endif

#if defined(HAVE_F3S)

#include "mkffs.h"


static block_info_t *init_block(int block_size, uint16_t unit_flags);
static void write_boot_record(block_info_t *block_info, uint16_t spare, ffs_sort_t *sort);
static void write_f3s(ffs_sort_t *sort, int block_size);
static block_info_t *write_dir_entry(ffs_sort_t *sorted_list, block_info_t *block_info);
static void write_extent(block_info_t *block_info, f3s_extptr_t next, uint16_t extent_flags, uint32_t size);
static block_info_t *write_dir_entry(ffs_sort_t *sorted_list, block_info_t *block_info);
static void write_extent(block_info_t *block_info, f3s_extptr_t next, uint16_t extent_flags, uint32_t size);
static void child_extent(ffs_sort_t *entry, uint32_t size_child_entry, block_info_t *block_info);
static void sibling_extent(ffs_sort_t *entry, uint32_t size_sib_entry, block_info_t *block_info);
static block_info_t *write_file(ffs_sort_t *sort, block_info_t *block_info);
static block_info_t *write_lnk(ffs_sort_t *sort, block_info_t *block_info);
static void first_file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info);
static void file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info);
static void spare_block_alloc(block_info_t *block_info);

//
//	Some globals used for the creation of the F3S flash filesystem
//

static f3s_head_t			extent_prev;

static block_info_t *init_block(int block_size, uint16_t unit_flags){
    
	f3s_extptr_t		extptr,
						next;
	f3s_unit_t			unit;
	block_info_t **		binfo;
	uint32_t			position;
	uint16_t			extent_flags;
	uint8_t				*init_blk_ptr;
    
	//
	//	setup the block allocation structure
	//

	binfo = realloc (block_info, (block_index + 1) * sizeof(block_info_t *));
	if (binfo == NULL) return (NULL);
	block_info = binfo;

	block_info[block_index] = (block_info_t *)malloc(sizeof(block_info_t));
	if (block_info[block_index] == NULL)
		mk_flash_exit("malloc failed: %s\n", strerror(errno));	

	block_info[block_index]->block_index = block_index;
	block_info[block_index]->available_space = block_size;
	block_info[block_index]->offset_top = 0;
	block_info[block_index]->block_size = block_size;
	block_info[block_index]->offset_bottom = block_size;
	block_info[block_index]->extent_index = 0;
	block_info[block_index]->next = NULL;

	// 
	//	every block will contain a unit header information  
	//	structure
	//

	unit.status = swap16(target_endian,unit_flags);
	unit.struct_size = swap16(target_endian,sizeof(unit));
	unit.endian = target_endian ? 'B' : 'L';
	unit.age = 0;					//	hard coded, does not apply here
        if (unit_flags&F3S_UNIT_NO_LOGI)
	 unit.logi = ~0;
        else
	 unit.logi = swap16(target_endian,block_index+1);
	unit.unit_pow2 = swap16(target_endian,calc_log2(block_size));
	unit.reserve = 0xffff;
	unit.erase_count = 0;			//	hard coded, does not apply here

	//
	//	setup the f3s_extptr_t to boot structure
	//	these values are hard coded since they should not change
	//

	extptr.logi_unit = swap16(target_endian,F3S_FIRST_LOGI);
	extptr.index = swap16(target_endian,F3S_BOOT_INDEX);

	unit.boot = extptr;

	//
	//	create a memory region to build our block
	//	

	if ((init_blk_ptr = (char *) malloc(block_size)) == NULL)
		mk_flash_exit("memory allocation failed: %s\n",strerror(errno));

	//
	//	initialize block
	//

	memset(init_blk_ptr,0xff,block_size);

	//
	// the unit header info is written at the start of each block
	//

	memcpy(init_blk_ptr,&unit,sizeof(unit));

	position = (block_info[block_index]->block_index) * block_info[block_index]->block_size;

	lseek(flashimage, position,SEEK_SET);

	if (write(flashimage,init_blk_ptr,block_size) != block_size)
		mk_flash_exit("write failed: %s\n",strerror(errno));
	

	//
	//	write extent information
	//
	
	next.logi_unit = FNULL;
	next.index = FNULL;
	
	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_SYS | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_NO_SPLIT | F3S_EXT_ALLOC |F3S_EXT_LAST);

	write_extent(block_info[block_index], next, extent_flags, sizeof(unit));
                                       
	//
	//	update block information structure
	//

	block_info[block_index]->available_space -= sizeof(unit);
	block_info[block_index]->offset_top = sizeof(unit);

	//
	//	free our malloc'd memory
	//

	block_index++;

	free(init_blk_ptr);

	return block_info[block_index-1];
}


static void write_boot_record(block_info_t *block_info, uint16_t spare, ffs_sort_t *sort)
{ 
	f3s_boot_t			boot_record;
	f3s_extptr_t		root,
						next;
	uint32_t			nbytes, 
						namelen,
						size,
						position;
	uint16_t			boot_flags,
						extent_flags;

	//
	//	setup the boot record structure
	//

	boot_flags = ~F3S_BOOT_NO_INDEX;
	boot_record.status = swap16(target_endian,boot_flags);
	boot_record.struct_size = swap16(target_endian,sizeof(boot_record));
	memcpy(&boot_record.sig, F3S_SIG_STRING, F3S_SIG_SIZE);
	boot_record.rev_major = F3S_REV_MAJOR;
	boot_record.rev_minor = F3S_REV_MINOR;

	//	
	//	offset information needed by the filesystem manager
	//

	boot_record.unit_index = 0;

	//
	//	to be determined once the filesystem is completed
	//

	boot_record.unit_total = 0;

	boot_record.unit_spare = swap16(target_endian,spare);
  	
	//
	//	these two entries are hard coded for now
	//

	boot_record.align_pow2 = swap16(target_endian,2);
	boot_record.xip_pow2 = swap16(target_endian,12);

	//
	//	setup root extent pointer
	//

	root.logi_unit = swap16(target_endian,F3S_FIRST_LOGI);
	root.index = swap16(target_endian,F3S_ROOT_INDEX);	

	boot_record.root = root;

	
	memcpy(blk_buffer_ptr,&boot_record,sizeof(boot_record));

	position = block_info->offset_top;
	
	lseek(flashimage,position,SEEK_SET);
	
	if((nbytes=write(flashimage,blk_buffer_ptr,sizeof(boot_record))) != 
			sizeof(boot_record))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	//
	//	call extent header routine for boot structure
	//
	
	next.logi_unit = FNULL;
	next.index = FNULL;

	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_SYS | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_NO_SPLIT | F3S_EXT_ALLOC | F3S_EXT_LAST);
	write_extent(block_info, next,  extent_flags, sizeof(boot_record));

	//
	//	update block_info structures
	//

	block_info->available_space -= sizeof(boot_record);
	block_info->offset_top +=  sizeof(boot_record);

	//
	//	call extent header routine for root structure (dirent)	
	//

	extent_flags = (extent_flags & ~F3S_EXT_SYS) | F3S_EXT_DIR; 
	
	//
	//	calculate the size of the root name (can't assume /)
	//

	namelen = strlen(sort->name) + 1;
	size = sizeof(f3s_dirent_t) + F3S_NAME_ALIGN(namelen) + sizeof(f3s_stat_t);
	write_extent(block_info, next, extent_flags, size);
}

static void write_f3s(ffs_sort_t *sort, int block_size){

	block_info_t		*block_info;
	uint32_t			size_of_entry;
	ffs_sort_t			*sort_temp_ptr;
	uint16_t			unit_flags;

	//
	//	setup generic buffer
	//

	if ((blk_buffer_ptr = (char *)malloc(0x4000)) == NULL)
		mk_flash_exit("malloc failed: %s\n",strerror(errno));	

	//
	//	create a temporary file
	//

	tmp_name = mk_tmpfile();
	
	if ((flashimage = open(tmp_name,O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
				S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH)) == -1)
			mk_flash_exit("create of %s failed: %s\n",tmp_name,strerror(errno));

	unit_flags = ((~F3S_UNIT_MASK | F3S_UNIT_READY) & ~F3S_UNIT_NO_LOGI);
	block_info = init_block(block_size, unit_flags);

	//
	// first block requires a boot record
	//

	write_boot_record (block_info, spare_blocks,sort);

	//
	// write the initialized block to the file.  each entry will be written  
	// to disk individually.
	//
	// check if this entry has been written to the file yet
	// run through the list until we get to the end
	//

	while(sort){
		if (!(sort->status & FFS_SORT_ENTRY_SET)){

			switch(sort->mode & S_IFMT){

			case S_IFDIR:
				if(verbose)
					fprintf(debug_fp,"writing directory entry -> %s\n",sort->name);
				block_info = write_dir_entry(sort, block_info);
				break;
			case S_IFREG:
				if(verbose)
					fprintf(debug_fp,"writing file entry      -> %s ",sort->name);
				block_info = write_file(sort,block_info);
				break;	        
			case S_IFLNK:
				if(verbose)
					fprintf(debug_fp,"writing link entry      -> %s ",sort->name);
				block_info = write_lnk(sort,block_info);
				break;	        
			case S_IFIFO:
				if(verbose)
					fprintf(debug_fp,"writing fifo entry      -> %s\n",sort->name);
				block_info = write_dir_entry(sort, block_info);
				break;
			} 
		}

		//
		// check for a child, only a directory can have a child  
		//

		if (!sort->child && (sort->mode & (S_IFDIR|S_IFIFO))){


		//
		//	directory entry under f3s can not have a NULL first extent.  The extent pointer must
		//  refer to an extent that in turn points to 0 sized data.
		//

			child_extent(sort,0,block_info);
		
		}      

		if (sort->child && (sort->mode & S_IFDIR)){

			// 
			//child			
			//	

			if ((sort_temp_ptr = find_child(sort)) == NULL)
				mk_flash_exit("find_child failed\n");
		

			size_of_entry = sizeof(f3s_dirent_t) + F3S_NAME_ALIGN(strlen(sort_temp_ptr->name)+1) + sizeof(f3s_stat_t);
			sort_temp_ptr->size_of_entry = size_of_entry;

			if (block_info->available_space < (size_of_entry + sizeof(f3s_head_t)))
		        block_info = init_block(block_size, unit_flags);

			child_extent(sort,sort_temp_ptr->size_of_entry,block_info);

			
			//
			//check the mode of our child
			//

			switch(sort_temp_ptr->mode & S_IFMT){
			
			case S_IFDIR:
				if(verbose)
					fprintf(debug_fp,"writing directory entry -> %s\n",sort_temp_ptr->name);

				block_info = write_dir_entry(sort_temp_ptr, block_info);
				break;

			case S_IFREG:
				if(verbose)
					fprintf(debug_fp,"writing file entry      -> %s ",sort_temp_ptr->name);

				block_info = write_file(sort_temp_ptr,block_info);
			    break;
				
			case S_IFLNK:
				if(verbose)
					fprintf(debug_fp,"writing link entry      -> %s ",sort_temp_ptr->name);

				block_info = write_lnk(sort_temp_ptr,block_info);
				break;
			
			case S_IFIFO:
				if(verbose)
					fprintf(debug_fp,"writing FIFO entry      -> %s\n",sort_temp_ptr->name);

				block_info = write_dir_entry(sort_temp_ptr, block_info);
				break;

			}			
       	}

		//
		//check for a sibling
		//

		if (sort->sibling){
	     	if ((sort_temp_ptr = find_sibling(sort)) == NULL)
				mk_flash_exit("find_sibling failed\n");
				
		
			size_of_entry = sizeof(f3s_dirent_t) + F3S_NAME_ALIGN(strlen(sort_temp_ptr->name)+1) + sizeof(f3s_stat_t);
			sort_temp_ptr->size_of_entry = size_of_entry;

			if (block_info->available_space < (size_of_entry + sizeof(f3s_head_t)))
			       block_info = init_block(block_size, unit_flags);
			sibling_extent(sort,sort_temp_ptr->size_of_entry,block_info);
          		

			switch(sort_temp_ptr->mode & S_IFMT){
				
			case S_IFDIR:
				if(verbose)
					fprintf(debug_fp,"writing directory entry -> %s\n",sort_temp_ptr->name);

				block_info = write_dir_entry(sort_temp_ptr, block_info);
				break;
						
			case S_IFREG:
				if(verbose)
					fprintf(debug_fp,"writing file entry      -> %s ",sort_temp_ptr->name);

				block_info = write_file(sort_temp_ptr,block_info);
				break;

			case S_IFLNK:
				if(verbose)
					fprintf(debug_fp,"writing link entry      -> %s ",sort_temp_ptr->name);

				block_info = write_lnk(sort_temp_ptr,block_info);
				break;

			case S_IFIFO:
				if(verbose)
					fprintf(debug_fp,"writing fifo entry      -> %s\n",sort_temp_ptr->name);

				block_info = write_dir_entry(sort_temp_ptr, block_info);
				break;
						
			}
		}
		sort=sort->next;
	}	

	//
	//	write out the rest of the filesystem.  This includes the spare block(s) and formatting
	//	additional blocks
	//

	spare_block_alloc(block_info);
	
	//
	//	free our malloc buffer
	//

	free(blk_buffer_ptr);
}


static block_info_t *write_dir_entry(ffs_sort_t *sorted_list, block_info_t *block_info){

	f3s_dirent_t			dir;
	f3s_extptr_t			first,
							next;
	f3s_stat_t				stat;
	uint32_t				pos,
							nbytes;
	uint32_t				entry_pos;
	uint16_t					dir_size;
	uint8_t					*name,
							namelen;

	first.logi_unit = FNULL;
	first.index = FNULL;
	next.logi_unit = FNULL;
	next.index = FNULL;
		
	//
	//	setup directory entry
	//

	dir.status = swap16(target_endian,~F3S_DIRENT_NO_STAT);
	dir.struct_size = swap16(target_endian,sizeof(dir));
	dir.first = first;
	namelen = strlen(sorted_list->name) + sizeof(char);
	dir.namelen = namelen;
	name = strdup(sorted_list->name);

	//
	//	is this entry a compressed file?
	//

	if (sorted_list->status  & COMPRESSED)
		stat.status = swap16(target_endian, ~F3S_STAT_NO_COMP);
	else
		stat.status = FNULL;

	stat.struct_size = swap16(target_endian,sizeof(stat));
	stat.uid = swap32(target_endian,sorted_list->uid);
	stat.gid = swap32(target_endian,sorted_list-> gid);
	stat.mtime = swap32(target_endian,sorted_list->ffs_ftime);
	stat.ctime = swap32(target_endian,sorted_list->ffs_ftime);
	stat.mode = swap32(target_endian,sorted_list->mode);

	dir_size = sizeof(dir) + F3S_NAME_ALIGN(namelen) + sizeof(stat);
	dir.moves = 0;
	dir.reserve = 0xffff;

	memcpy(blk_buffer_ptr, &dir, sizeof(dir));
	memcpy(blk_buffer_ptr + sizeof(dir), name, namelen);
	memcpy(blk_buffer_ptr +(sizeof(dir)+F3S_NAME_ALIGN(namelen)), &stat, sizeof(stat));

	//
	// find position within the file
	//

	pos = ((block_info->block_index) * (block_info->block_size))
			+ block_info->offset_top;

	//
	//	make sure entry is DWORD aligned
	//

	entry_pos = lseek(flashimage,pos,SEEK_SET);
	if ((nbytes = write(flashimage,blk_buffer_ptr,dir_size)) !=dir_size)
		mk_flash_exit("write failed: %s\n",strerror(errno));
    
	//
	// updated sorted_list
    //
	
	sorted_list->status = FFS_SORT_ENTRY_SET;
	sorted_list->entry_pos = entry_pos; 
	sorted_list->size_of_entry = dir_size;
	sorted_list->extent_pos = (block_info->block_index * block_info->block_size) + block_info->offset_bottom;

	//
	// update block_info
	//

	block_info->offset_top += dir_size;
	block_info->available_space -= dir_size;

	return block_info;
}


static void write_extent(block_info_t *block_info, f3s_extptr_t next, uint16_t extent_flags, uint32_t size){

	f3s_head_t			extent;
	f3s_extptr_t		super;
	uint32_t			position;

	//
	//	setup the extent header information
	//

	extent.status = swap16(target_endian,extent_flags);

	//
	//	must make sure that the offset is dword aligned
	//

	extent.reserve = 0xff;
	extent.text_offset_hi = block_info->offset_top>> (2 /* debug: hardcoded */ +F3S_OFFSET_HI_POW2);
	extent.text_offset_lo = swap16(target_endian,block_info->offset_top>> 2 /* debug: hardcoded */);
	extent.text_size      = swap16(target_endian,size);

	super.logi_unit = FNULL;  //this utility will never have to deal with supersede extents.
	super.index     = FNULL;
	extent.super    = super;

	next.logi_unit  = FNULL;
	next.index      = FNULL;
	extent.next     = next;

	memcpy(blk_buffer_ptr, &extent, sizeof(extent));
	
	position = (block_info->block_size * block_info->block_index) + ((block_info->offset_bottom) - sizeof(extent));

	if (lseek(flashimage,position,SEEK_SET) == -1)
		mk_flash_exit("seek failed: %s\n",strerror(errno));

	if (write(flashimage,blk_buffer_ptr,sizeof(extent)) != sizeof(extent))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	//
	// update block information
	//

	block_info->available_space -= sizeof(extent);
	block_info->offset_bottom   -= sizeof(extent);
	block_info->extent_index++;

	//
	//	update previous extent to not be "last"
	//

	if (block_info->extent_index >1){

		if (lseek(flashimage,position+sizeof(extent),SEEK_SET) == -1)
			mk_flash_exit("lseek failed: %s\n",strerror(errno));

		if (read(flashimage,&extent_prev,sizeof(extent_prev)) != sizeof(extent_prev))
			mk_flash_exit("read failed: %s\n",strerror(errno));

		extent_prev.status &= swap16(target_endian,~F3S_EXT_LAST);
		memcpy(blk_buffer_ptr,&extent_prev, sizeof(extent_prev));

		if (lseek(flashimage,position+sizeof(extent),SEEK_SET) == -1)
			mk_flash_exit("lseek failed: %s\n",strerror(errno));

		if (write(flashimage,blk_buffer_ptr,sizeof(extent)) != sizeof(extent))
			mk_flash_exit("write failed: %s\n",strerror(errno));
	}
}


static void child_extent(ffs_sort_t *entry, uint32_t size_child_entry, block_info_t *block_info){

	f3s_extptr_t		first,
						next;
	uint32_t			pos,
						nbytes;
	uint16_t			extent_flags;

	//
	// Setup 'first' pointer to new extent
	//
	
	first.logi_unit = swap16(target_endian,block_info->block_index + 1);	
	first.index =  swap16(target_endian,block_info->extent_index);

	//
	// now write the entry to the correct place 
    // 'first' entry is 4 bytes offset into the f3s_dirent_t structure
	//
	
	pos = entry->entry_pos + sizeof(int);


	memcpy(blk_buffer_ptr,&first, sizeof(first));

	lseek(flashimage,pos,SEEK_SET);

	if ((nbytes = write(flashimage,blk_buffer_ptr,sizeof(first))) !=sizeof(first))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	entry->status |=FFS_SORT_ENTRY_SET;

	//
	//	write extent for the new entry
	//

	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_DIR | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_ALLOC | F3S_EXT_LAST | F3S_EXT_NO_SPLIT);

	next.logi_unit = FNULL;
	next.index = FNULL;

	write_extent(block_info, next, extent_flags, size_child_entry);
}


static void sibling_extent(ffs_sort_t *entry, uint32_t size_sib_entry, block_info_t *block_info){

	f3s_extptr_t		next,
						dir_next;
	f3s_head_t			dir_extent;
	uint32_t			pos,
						nbytes;
	uint16_t			extent_flags;

	//
	// Setup 'next' pointer to new extent
	//
	
	dir_next.logi_unit = swap16(target_endian,block_info->block_index + 1);			
	dir_next.index =  swap16(target_endian,block_info->extent_index);

	//
	// now write the entry to the correct place 
	// 'next' entry is 
	//

	pos = entry->extent_pos; 

	lseek(flashimage, pos, SEEK_SET);

	if (read(flashimage,&dir_extent,sizeof(dir_extent)) != sizeof(dir_extent))
		mk_flash_exit("read failed: %s\n",strerror(errno));

	dir_extent.status &= swap16(target_endian,~F3S_EXT_NO_NEXT);
	dir_extent.next = dir_next;
	
	lseek(flashimage, pos, SEEK_SET);

	memcpy(blk_buffer_ptr,&dir_extent, sizeof(dir_extent));

	if ((nbytes = write(flashimage,blk_buffer_ptr,sizeof(dir_extent))) !=sizeof(dir_extent))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	entry->status |=FFS_SORT_ENTRY_SET;

	//
	//	write extent for the new entry
	//

	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_DIR | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_ALLOC | F3S_EXT_LAST | F3S_EXT_NO_SPLIT);

	next.logi_unit = FNULL;
	next.index = FNULL;
	write_extent(block_info, next, extent_flags, size_sib_entry);
}


static block_info_t *write_file(ffs_sort_t *sort, block_info_t *block_info){

	FILE			*read_file_fp;
	uint8_t			done=0,
					firstpass=1,
					*file_buffer;
	uint16_t		unit_flags;
	uint32_t		update_dot=0,
					pos,
					remain,
					remain_dword,
					prev_extent;
	int32_t			read_size;

	//
	//	allocate file buffer
	//

	if ((file_buffer = (char *)malloc(0x4000)) == NULL)
		mk_flash_exit("malloc failed: %s\n",strerror(errno));

	//
	// open the file for read
	//

	if ((read_file_fp = fopen(sort->host_fullpath,"rb")) == NULL)
		mk_flash_exit("fopen of %s failed: %s\n",sort->host_fullpath,strerror(errno));

	//
	//	check for compressed file.  We check for the "FZIP" signature which is offset 1 long into 
	//	the compressed file
	//

	fseek(read_file_fp, sizeof(f3s_comp_t), SEEK_SET);

	fread(file_buffer, F3S_COMP_SIG_SIZE, 1, read_file_fp);

    /* updated by christian to support lzo */

	if (memcmp(file_buffer, F3S_FZIP_SIG, F3S_COMP_SIG_SIZE) &&
         memcmp(file_buffer, F3S_FLZO_SIG, F3S_COMP_SIG_SIZE) &&
         memcmp(file_buffer, F3S_FUCL_SIG, F3S_COMP_SIG_SIZE))
		sort->status &=~COMPRESSED;
	else 
		sort->status |=COMPRESSED;
		
	//
	//	seek back to the beginning of the file
	//

	fseek(read_file_fp, 0L, SEEK_SET);

	//
	//	write the entry.  f3s_dirent_t is used for both files and directory entries.
	//

	block_info = write_dir_entry(sort, block_info);


    if(verbose){
		putc(42,debug_fp);
		fflush(debug_fp);
	}

	prev_extent = 0;

	
	while(!done){
		read_size = block_info->available_space - sizeof(f3s_head_t);
		if (read_size > MAX_WRITE)
			read_size = MAX_WRITE;
		if (read_size <= 0){
			unit_flags = ((~F3S_UNIT_MASK | F3S_UNIT_READY) & ~F3S_UNIT_NO_LOGI);
			block_info = init_block(block_info->block_size, unit_flags);
		}
		read_size = block_info->available_space - sizeof(f3s_head_t);

		if (read_size > MAX_WRITE)
			read_size = MAX_WRITE;
		

		//
		// a regular file   	
		//

		remain = fread(file_buffer,1,read_size,read_file_fp);


		//
		//	update 'first' or 'next' extent pointer.  First pass requires the 'first' pointer
		//	updated, subsequent extents are 'next' extent pointers.
		//

		if (firstpass){
			first_file_extent(sort->entry_pos+4, remain, block_info);
			firstpass = 0;
		}
    	else {
			file_extent(prev_extent, remain, block_info);
			
		}

		//
		//	keep a copy of the position of the previous extent
		//

        prev_extent = (block_info->block_index * block_info->block_size) + block_info->offset_bottom;

		//
		//	check whether we've read all that we can read
		//

		if (remain < read_size)
			done = 1;
		
		//
		//	verbose display of file being written
		//

		if(verbose){
			update_dot +=remain;
			if (update_dot >=MAX_WRITE){
				putc(42,debug_fp);
				fflush(debug_fp);
				update_dot = 0;
			}
		}

        //
		//	position and write the content of the file to the image file
		//
		
		pos = (block_info->block_index * block_info->block_size) + block_info->offset_top;

		lseek(flashimage,pos,SEEK_SET);

		if (write(flashimage,file_buffer,remain) != remain)
			mk_flash_exit("write failed: %s\n",strerror(errno));

		//
		//	must make sure that the offset_top is DWORD aligned
		//

		remain_dword = (((remain) + (sizeof(uint32_t)-1))&~(sizeof(uint32_t)-1));

		block_info->offset_top += remain_dword;
		block_info->available_space -= remain_dword;

	}

	//
	//close the file
	//

	fclose(read_file_fp);

	if (verbose)
		fprintf(debug_fp,"\n");
	
	free(file_buffer);
	return block_info;
}


static block_info_t *write_lnk(ffs_sort_t *sort, block_info_t *block_info){

	uint8_t			lnk_buffer[F3S_PATH_MAX+1];
	uint16_t		unit_flags;
	uint32_t		pos,
					lnk_dword;
	int32_t			lnk_size;
	struct file_entry *fip = sort->fip;

	//
	//	write the entry.  f3s_dirent_t is used for both files and directory entries.
	//

	block_info = write_dir_entry(sort, block_info);

	//
	//	get the content of the symlink
	//

	if(!fip->hostpath || *fip->hostpath == '\0'){
		if ((lnk_size = readlink(sort->host_fullpath, lnk_buffer, F3S_PATH_MAX)) == -1)
			mk_flash_exit("readlink failed: %s\n",strerror(errno));
	}
	else{
		strcpy(lnk_buffer, fip->hostpath);
		lnk_size = strlen(lnk_buffer);
	}

    if(verbose){
		putc(42,debug_fp);
		fflush(debug_fp);
	}


	if (block_info->available_space < lnk_size + sizeof(f3s_head_t)){

		unit_flags = ((~F3S_UNIT_MASK | F3S_UNIT_READY) & ~F3S_UNIT_NO_LOGI);
		block_info = init_block(block_info->block_size, unit_flags);
	}
		
	//
	//	update 'first' or 'next' extent pointer.  First pass requires the 'first' pointer
	//	updated, subsequent extents are 'next' extent pointers.
	//

	first_file_extent(sort->entry_pos+4, lnk_size, block_info);

	//
	//	position and write the content of the file to the image file
	//
		
	pos = (block_info->block_index * block_info->block_size) + block_info->offset_top;

	lseek(flashimage,pos,SEEK_SET);

	if (write(flashimage,lnk_buffer,lnk_size) != lnk_size)
		mk_flash_exit("write failed: %s\n",strerror(errno));

	//
	//	must make sure that the offset_top is DWORD aligned
	//

	lnk_dword = (((lnk_size) + (sizeof(uint32_t)-1))&~(sizeof(uint32_t)-1));

	if (verbose)
		fprintf(debug_fp,"\n");

	block_info->offset_top += lnk_dword;
	block_info->available_space -= lnk_dword;

	return block_info;
}


static void first_file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info){

	f3s_extptr_t		next,
						first;
	uint32_t			nbytes;
	uint16_t			extent_flags;
	
	

	//
	// Setup extent pointer to new extent
	//
	
	first.logi_unit = swap16(target_endian,block_info->block_index + 1);			
	first.index = swap16(target_endian,block_info->extent_index);

	//
	// write the update to the correct place 
	//
	

	lseek(flashimage, position, SEEK_SET);

	memcpy(blk_buffer_ptr, &first, sizeof(first)); 

	if ((nbytes = write(flashimage,blk_buffer_ptr,sizeof(first))) !=sizeof(first))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	//
	//	write extent for the new entry
	//

	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_FILE | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_ALLOC | F3S_EXT_LAST | F3S_EXT_NO_SPLIT);

	next.logi_unit = FNULL;
	next.index = FNULL;
	write_extent(block_info, next, extent_flags, size_file_entry);
}


static void file_extent(uint32_t position, uint32_t size_file_entry, block_info_t *block_info){

	f3s_extptr_t		next,
						dir_next;
	f3s_head_t			dir_extent;
	uint32_t			nbytes;
	uint16_t			extent_flags;
	
	

	//
	// Setup 'next' pointer to new extent
	//
	
	dir_next.logi_unit = swap16(target_endian,block_info->block_index + 1);			
	dir_next.index =  swap16(target_endian,block_info->extent_index);

	//
	// now write the entry to the correct place 
    // 'next' entry is 
	//
	
	lseek(flashimage, position, SEEK_SET);

	if (read(flashimage,&dir_extent,sizeof(dir_extent)) != sizeof(dir_extent))
		mk_flash_exit("read failed: %s\n", strerror(errno));

	dir_extent.status &= swap16(target_endian,~F3S_EXT_NO_NEXT);
	dir_extent.next = dir_next;
	
	lseek(flashimage, position, SEEK_SET);

	memcpy(blk_buffer_ptr,&dir_extent, sizeof(dir_extent));

	if ((nbytes = write(flashimage,blk_buffer_ptr,sizeof(dir_extent))) !=sizeof(dir_extent))
		mk_flash_exit("write failed: %s\n",strerror(errno));

	//
	//	write extent for the new entry
	//

	extent_flags = ~F3S_EXT_TYPE & 0xff00;
	extent_flags |=	(F3S_EXT_FILE | F3S_EXT_NO_NEXT | F3S_EXT_NO_SUPER | F3S_EXT_ALLOC | F3S_EXT_LAST | F3S_EXT_NO_SPLIT);

	next.logi_unit = FNULL;
	next.index = FNULL;
	write_extent(block_info, next, extent_flags, size_file_entry);

}


static void spare_block_alloc(block_info_t *block_info){

	uint16_t		block_count,
					boot_block_count,	
					max_block_count,
					min_block_count,
					i,
					unit_flags;
	uint32_t		nbytes;
					
	//
	//	calculate minimum number of blocks for the specified filesystem
	//

	min_block_count = image.minsize/block_size;

	//
	//	calculate maximum number of blocks for the specified filesystem
	//

	if(image.maxsize == 0)
		max_block_count = 0xffff;
	else
		max_block_count = image.maxsize/block_size;


	//
	// forward the list to the last entry
	//

	while (block_info->next)
		block_info = block_info->next;

	//
	// the number of blocks must include the spare block(s) 
	// the 1 is because the the block_info->block_index is indexed from 0
	//
										
	block_count = block_info->block_index + 1 + spare_blocks;	

	//
	// if filesystem is smaller than minsize, pad the filesyste
	//
	
	if (min_block_count > block_count){
		unit_flags = ((~F3S_UNIT_MASK | F3S_UNIT_READY) & ~F3S_UNIT_NO_LOGI);
		for(i=block_count;i<min_block_count;i++)
			block_info = init_block(block_info->block_size, unit_flags);
		block_count = min_block_count;
	}

	if (max_block_count < block_count){
		fprintf(stderr,"\n");
		fprintf(stderr,"WARNING -- filesystem size exceeds %dK (max_size).\n",image.maxsize/1024);
	}
	

	boot_block_count = swap16(target_endian,block_count);
		
	memcpy(blk_buffer_ptr,&boot_block_count,sizeof(boot_block_count));

	//
	// position write to modify f3s_boot_t structure
	// this can be 'hard coded', it will not change.
	//

	if (lseek(flashimage,sizeof(f3s_unit_t)+offsetof(f3s_boot_t,unit_total),SEEK_SET) == -1)
		mk_flash_exit("lseek failed: %s\n",strerror(errno));
	
	if((nbytes=write(flashimage,blk_buffer_ptr,sizeof(boot_block_count))) != sizeof(boot_block_count))
		mk_flash_exit("write failed: %s\n",strerror(errno));
	
	//
	// initialize unit flags for spare block and write them out
	//

	unit_flags = (~F3S_UNIT_MASK | F3S_UNIT_SPARE);

	for(i=0;i<spare_blocks;i++)
		init_block(block_info->block_size, unit_flags);

	if(verbose){
		fprintf(stderr,"Filesystem size = %dK\n",(block_info->block_size*block_count)
			/1024);
		fprintf(stderr, "block size = %dK\n",block_info->block_size/1024);
		fprintf(stderr,"%d spare block(s)\n", spare_blocks);
	}
};

unsigned
ffs_make_fsys_2(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname) {
	struct tree_entry	*trp;
	ffs_sort_t			*sort;

	if(mountpoint == NULL) mountpoint = "";

	//
	//	If target endian is unknown, set to little endian
	//

	if (target_endian == -1)
		target_endian = 0;
	
	//
	//	Make sure block size is defined and reasonable (at least 1K)
	//

	if (block_size < 1024)
		mk_flash_exit("Error: block_size not defined or incorrectly defined (>1K), exiting ...\n");

	trp = make_tree(list);

	sort = ffs_entries(trp, mountpoint);

	write_f3s(sort, block_size);

	write_image(dst_fp);

	return 0;
}

#else

unsigned
ffs_make_fsys_2(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname) {
	error_exit("Embedded file system generation not supported\n");
	return 0;
}

#endif


__SRCVERSION("mk_flash_fsys.c $Rev: 153052 $");
