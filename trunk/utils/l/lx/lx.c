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






#ifdef __USAGE
%C - show extents used by a file

%C	file

Note:

 Block addresses are shown in spatch-style format.

 Requires read access to the block special file (usually a
 disk partition) that the file resides on. Normally this
 would mean running as root.
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#ifndef __QNXNTO__
#include <sys/fsys.h>
#endif
#include <util/fs_qnx4_util.h>
#include <util/util_limits.h>
#include <stdlib.h>

struct _xlat_entry {
	int off;
	int blk;
	int size;
};

char *Prog;
char Source_disk[UTIL_PATH_MAX], *Source_file;
int  Source_fd;

struct _xlat_entry *inode_xlat_table;
int inode_xlat_table_size;

/* misc local-use buffer areas of various types */
char misc_path[UTIL_PATH_MAX];
struct _qnx4fs_fsys_stat misc_fsysstat;
struct stat misc_stat;

void show_extents();
void f_fsys_stat(const char *path, struct _qnx4fs_fsys_stat *buf);
void f_block_read(int fildes, long block, unsigned nblock, void *buf);
void f_lstat(const char *path, struct stat *buf);
void *f_calloc(size_t n, size_t size);

int main(int argc, char **argv) 
{
	int i;

	Prog=argv[0];

	while(( i= getopt( argc, argv, "")) != EOF) {
		switch ( i ){
		default:
			exit( EXIT_FAILURE );
		}
	}

	if ((argc-optind)<1) {
		fprintf(stderr,"%s: Must specify a filename\n",Prog);
		exit(EXIT_FAILURE);
	}

	Source_file = argv[optind];

	if (-1==fsys_get_mount_dev(Source_file, Source_disk)) {
		fprintf(stderr,"%s: Cannot find mount point of '%s' (%s)\n",Prog,Source_file, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (-1==(Source_fd = open(Source_disk,O_RDONLY))) {
		fprintf(stderr,"%s: Cannot open '%s' (%s).\n",Prog,Source_disk, strerror(errno));
		exit(EXIT_FAILURE);
	}

	show_extents();
	return 0;
}

void show_extents()
{
	int curxtnt;             /* current extent # of file we are looking at */
	int cur_xblk=0;          /* disk offset of current extent block */
	struct qnx4fs_xblk cur_xblk_d; /* contents of current extent block */
	int xtnt_idx;            /* index into cur_xblk_d.xblk_xtnts[] */
	int num_xblks=0;

    // get the size of the inodes file
    strcpy(misc_path,Source_file);
	f_lstat(misc_path, &misc_stat);

	// load the inode extent reference table
    //      file offset; disk offset; size
    f_fsys_stat(misc_path, &misc_fsysstat);

	fprintf(stdout,"%s: %s exists on %s\n",Prog,Source_file, Source_disk);

    if (0==(inode_xlat_table_size=misc_fsysstat.st_num_xtnts)) {
		fprintf(stdout,"XTNT %04d[FRST]: %6d blocks @-------:--- [seek@0]\n",
			0,
			0);
		exit(0);
	}

    inode_xlat_table=f_calloc(inode_xlat_table_size, sizeof(*inode_xlat_table));

	inode_xlat_table[0].off=0L;
	inode_xlat_table[0].blk=misc_fsysstat.st_first_xtnt.xtnt_blk;
	inode_xlat_table[0].size=misc_fsysstat.st_first_xtnt.xtnt_size;

	curxtnt=0;

	fprintf(stdout,"XTNT %04d[FRST]: %6d blocks @%07x:000 [seek@%d]\n",
		curxtnt,
		inode_xlat_table[curxtnt].size,
		inode_xlat_table[curxtnt].blk,
		inode_xlat_table[curxtnt].off*512L);

	for ( curxtnt=1; curxtnt<misc_fsysstat.st_num_xtnts; ) {
    	/* get this extent */
		if (cur_xblk==0) {
			/* need to get 1st xblk */
			cur_xblk=misc_fsysstat.st_xblk;
			num_xblks++;
			
			if (cur_xblk==0) {
				fprintf(stderr,"%s: file '%s' has missing extents! (%d) (fatal)\n",Prog,misc_path,curxtnt);
				exit(EXIT_FAILURE);
			}

			f_block_read(Source_fd, cur_xblk, 1L,&cur_xblk_d);

			fprintf(stdout,"XBLK %04d: @%07x:000 %d xtnts; prev@%07x:000 next@%07x:000\n", 
				num_xblks, 
				cur_xblk,
				cur_xblk_d.xblk_num_xtnts,
				cur_xblk_d.xblk_prev_xblk,
				cur_xblk_d.xblk_next_xblk
				);
        }

		/* for each extent in the xblock, memorize it */
		for (xtnt_idx=0;xtnt_idx<cur_xblk_d.xblk_num_xtnts;xtnt_idx++,curxtnt++)
		{
			inode_xlat_table[curxtnt].off=inode_xlat_table[curxtnt-1].size +
							              inode_xlat_table[curxtnt-1].off;
			inode_xlat_table[curxtnt].blk=cur_xblk_d.xblk_xtnts[xtnt_idx].xtnt_blk;
			inode_xlat_table[curxtnt].size=cur_xblk_d.xblk_xtnts[xtnt_idx].xtnt_size;

			fprintf(stdout,"XTNT %04d[%04d]: %6d blocks @%07x:000 [seek@%d]\n",
				curxtnt,
				xtnt_idx+1,
				inode_xlat_table[curxtnt].size,
				inode_xlat_table[curxtnt].blk,
				inode_xlat_table[curxtnt].off*512L);
		}
			
		/* if we have not got all our extents */
		if (curxtnt<misc_fsysstat.st_num_xtnts) {
			/* get the next extent block */
			cur_xblk=cur_xblk_d.xblk_next_xblk;
			num_xblks++;
				
			if (cur_xblk==0) {
				fprintf(stdout,"curxtnt=%d\n",curxtnt);
				fprintf(stderr,"%s: file '%s' has missing extents! (%d) (fatal)\n",Prog,misc_path,curxtnt);
				exit(EXIT_FAILURE);
			}
	
			f_block_read(Source_fd, cur_xblk, 1L,&cur_xblk_d);

			fprintf(stdout,"XBLK %04d: @%07x:000 %d xtnts; prev@%07x:000 next@%07x:000\n", 
				num_xblks, 
				cur_xblk,
				cur_xblk_d.xblk_num_xtnts,
				cur_xblk_d.xblk_prev_xblk,
				cur_xblk_d.xblk_next_xblk
				);
		} 
	}

#ifdef NEVER
	for (xtnt_idx=0;xtnt_idx<inode_xlat_table_size;xtnt_idx++) {
		fprintf(stdout,"XTNT %d: off=%6d, blk=%6d, size=%6d\n",
			xtnt_idx,
			inode_xlat_table[xtnt_idx].off,
			inode_xlat_table[xtnt_idx].blk,
			inode_xlat_table[xtnt_idx].size);
	}
	fprintf(stdout,"TOTAL SIZE = %d blocks\n",inode_xlat_table[xtnt_idx-1].off+inode_xlat_table[xtnt_idx-1].size);
#endif
}

