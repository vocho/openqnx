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

#if defined(HAVE_F3S) || defined(HAVE_F3S_V3)

#include "mkffs.h"


#define FNULL                       0xffff
#define MAX_WRITE                   0x4000
#define FFS_SORT_ENTRY_SET			0x0001
#define COMPRESSED					0x0002

//
//	Some globals used for the creation of the F3S flash filesystem
//

int					flashimage;
uint32_t			block_index=0;
block_info_t		**block_info;
uint8_t 			*blk_buffer_ptr;
uint8_t				*tmp_name;

//
//	this routine fills the ffs_sort_t structures necessary to build a flash filesystem.
//	The routine walks the 'tree_entry' double linked list creating an ffs_sort single linked
//	list
//

void mk_flash_exit(char *format, ...) {
	va_list arglist;

	va_start(arglist, format);
	vfprintf(stderr, format, arglist);
	va_end(arglist);

	close(flashimage);

	exit(1);
}

ffs_sort_t *ffs_entries(struct tree_entry *trp, uint8_t *mountpoint)
{
	ffs_sort_t		*sort,
					*sort_hold,
					*sort_add;
	uint16_t		level=1;

	sort_add = (ffs_sort_t *)malloc(sizeof(ffs_sort_t));
	if (sort_add == NULL) {
		mk_flash_exit("%s\n",strerror(ENOMEM));
	}
	sort_hold = (ffs_sort_t *)malloc(sizeof(ffs_sort_t));
	if (sort_hold == NULL) {
		mk_flash_exit("%s\n",strerror(ENOMEM));
	}

	//
	//	special case root
	//

	sort_hold->next = sort_add;
	sort_add->name = mountpoint;

	//
	//	special case for empty filesystem
	//

	if (!trp)
		sort_add->child = NULL;
	else
		sort_add->child = trp->name;

	sort_add->status = 0;
	sort_add->level = 0;
	sort_add->sibling = NULL;
	sort_add->parent = NULL;
	sort_add->ffs_mtime = no_time ? 0 : time(NULL);
	sort_add->ffs_ftime = no_time ? 0 : time(NULL);

	// 
	//	mode, uid and gid are hard coded for root entry
	//

	sort_add->perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	sort_add->mode =  S_IFDIR | sort_add->perms;
	sort_add->gid = 0;
	sort_add->uid = 0;

	sort_hold = sort_add;
	sort_hold->prev = NULL;
	sort = sort_hold;

	while(trp){
		sort_add = (ffs_sort_t *)malloc(sizeof(ffs_sort_t));
		if (sort_add == NULL) {
			mk_flash_exit("%s\n",strerror(ENOMEM));
		}
		sort_hold->next = sort_add;
		sort_add->name = trp->name;
		sort_add->fip = trp->fip;
		if (level > 1)
			sort_add->parent = trp->parent->name;
		else 
			sort_add->parent = NULL;
		
		if (!(trp->fip->attr->mode & S_IFDIR)){
			sort_add->host_fullpath = trp->fip->hostpath;
			sort_add->perms = trp->fip->host_perms;
			sort_add->perms &= trp->fip->attr->perms_mask;
			sort_add->perms |= trp->fip->attr->perms_set;
		}
		else {
			sort_add->perms = trp->fip->host_perms;
			sort_add->perms &= trp->fip->attr->dperms_mask;
			sort_add->perms |= trp->fip->attr->dperms_set;
		}

		sort_add->status = 0;

		sort_add->mode = trp->fip->attr->mode | sort_add->perms;

		if(trp->fip->flags & FILE_FLAGS_EXEC) 
			sort_add->perms &= ~S_ISVTX;

		sort_add->level = level;

		sort_add->ffs_mtime = trp->fip->host_mtime;
		sort_add->ffs_ftime = trp->fip->host_mtime;

		sort_add->gid = (trp->fip->attr->inherit_gid) ? trp->fip->host_gid : trp->fip->attr->gid;
		sort_add->uid = (trp->fip->attr->inherit_uid) ? trp->fip->host_uid : trp->fip->attr->uid;

		if (trp->sibling)
			sort_add->sibling = trp->sibling->name;
		else
			sort_add->sibling = NULL;
		if (trp->child)		
			sort_add->child = trp->child->name;
		else
			sort_add->child = NULL;
		sort_add->prev=sort_hold;
		sort_hold = sort_add;
		

		if (trp->child){
			trp = trp->child;
			level++;
		}
		else{
			if (trp->sibling)
				trp = trp->sibling;
			else{
				while(!trp->sibling){
					if(level > 1){
						trp = trp->parent;
						level--;
					}
					else
						break;
				}
				trp = trp->sibling;
			 }
		}
	}
	sort_hold->next = NULL;
	return(sort);
}


void write_image(FILE *fp)
{
	ssize_t			remain;
	uint8_t			buf[0x1000],
					done=0;

	lseek(flashimage, 0L, SEEK_SET);

	while(!done){
		remain = read(flashimage,&buf,sizeof(buf));
		if (-1 == remain){
			mk_flash_exit("reading flashimage failed:%s\n", strerror(errno));
		}

		if (fwrite(&buf,1,remain,fp) != remain)
			mk_flash_exit("writing to output file failed: %s\n",strerror(errno));

		if (remain < sizeof(buf))
			done = 1;
	}
	close(flashimage);
}


//
// function that will scan the linked list for a child belonging
// to the ptr passed
//

ffs_sort_t *find_child(ffs_sort_t *sort){

	ffs_sort_t			*child;

	child = sort->next;

	//
	// check for first entry, level 0 (root)
	//

	if (sort->level != 0){

	//
	// not root
	//

		for(;;){
	        if (!(strcmp(child->parent, sort->name))  &&  ((sort->level+1) == child->level)) 
				break;
			else{
				if (child->next)
					child=child->next;
				else
					return NULL;
			}
		}
	}
	else{			
		if (!child)
			return NULL;
	}

	return child;
}


//
// this routine will scan the sort list for a sibling (brother/sister)
//

ffs_sort_t *find_sibling(ffs_sort_t *sort){

	ffs_sort_t			*sibling;

	sibling = sort->next;

	if (sort->level > 1)

		while((strcmp(sort->parent, sibling->parent)) || 
				(sort->level != sibling->level) || (strcmp(sort->sibling, sibling->name))){

			if (sibling->next)
				sibling=sibling->next;	
			else
				return NULL;
	}
	else 
		while((strcmp(sort->sibling, sibling->name)) || (sort->level != sibling->level)){
			if (sibling->next)
				sibling=sibling->next;
			else
				return NULL;
		}

	return sibling;
};


unsigned calc_log2(unsigned blksize) {
	unsigned	log2;

	log2 = 0;
	while(blksize != 1){
		++log2;
		blksize >>= 1;
	}
	return log2;
}

#endif

int
ffs_need_seekable(struct file_entry *list) {
	//Dummy implementation - should return true
	return 0;
}


__SRCVERSION("mkffs.c $Rev: 153052 $");
