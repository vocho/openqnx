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
#if defined(HAVE_ETFS)
#include _NTO_HDR_(fs/etfs.h)


static void write_filetable(FILE *dst_fp, struct tree_entry *trp, unsigned parent);
static void write_filedata(FILE *dst_fp, struct tree_entry *trp, unsigned parent);
static int write_transaction(FILE *dst_fp, uint8_t *data, unsigned npages, unsigned fid);
static void assign_fids(struct tree_entry *trp);

//
//	Some globals used for the creation of the etfs filesystem
//
unsigned	totaldirs;
unsigned	totalfiles;
unsigned	totalclusters;
unsigned	totaltransactions;
uint8_t		*blockdata;
int			num_clusters;
int			files_per_cluster;
int			num_fids;


#define CLUSTERS(nfiles)	(((nfiles) + (files_per_cluster-1))/files_per_cluster)
#define NCLS(size)			(((size) + cluster_size - 1) / cluster_size)

#define MEG	(1024*1024)

//
//	We make all etfs data structures little endian
//
unsigned
etfs_make_fsys(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname) {
	struct tree_entry	*trp;
	int					i;

	if(mountpoint == NULL) mountpoint = "";

	// Set default to a 16M NAND flash part.
	if(cluster_size == 0) 	cluster_size = 1024;
	if(block_size == 0) 	block_size = 1024*16;

	num_clusters = block_size / cluster_size;
	blockdata = malloc(block_size);
	if (blockdata == NULL) {
		fprintf(stderr, "Not enough memory\n");
		return -1;
	}
	files_per_cluster = cluster_size/sizeof(struct etfs_ftable_file);

	trp = make_tree(list);
	num_fids = FID_FIRSTFILE;
	assign_fids(trp);

	// write_filetable() must be first
	write_filetable(dst_fp, trp, FID_ROOT);
	write_filedata(dst_fp, trp, FID_ROOT);

	// Pad out with ff's if num_blocks was specified.
	memset(&blockdata[0], 0xff, cluster_size + sizeof(struct etfs_trans));
	for(i = totalclusters ; i < num_blocks*num_clusters ; ++i)
		fwrite(&blockdata[0], cluster_size + sizeof(struct etfs_trans), 1, dst_fp);

	if(verbose)
		fprintf(stderr, "Dirs %u  Files %u  Clusters %u  Bytes %u  Transactions %u\n", totaldirs, totalfiles, totalclusters, totalclusters*cluster_size, totaltransactions);

	return 0;
}

static void
endian_file(struct etfs_ftable_file *fep) {
	fep->efid = swap16(0,fep->efid);
	fep->pfid = swap16(0,fep->pfid);
	fep->mode = swap32(0,fep->mode);
	fep->uid = swap32(0,fep->uid);
	fep->gid = swap32(0,fep->gid);
	fep->atime = swap32(0,fep->atime);
	fep->mtime = swap32(0,fep->mtime);
	fep->ctime = swap32(0,fep->ctime);
	fep->size = swap32(0,fep->size);
}

static void
endian_trans(struct etfs_trans *trp) {
	trp->fid = swap32(0,trp->fid);
	trp->cluster = swap32(0,trp->cluster);
	trp->sequence = swap32(0,trp->sequence);
	trp->nclusters = swap16(0,trp->nclusters);
}

static void
mkrootentry(struct etfs_ftable_file *ftp, char *name, mode_t md, int sz, time_t tm)
{
	ftp->efid = 0, ftp->pfid = FID_ROOT;
	ftp->size = sz;
	ftp->atime = ftp->mtime = ftp->ctime = tm;
	ftp->mode = md;
	ftp->gid = 0, ftp->uid = 0;
	strcpy(ftp->name, name);
	endian_file(ftp);
}

static void
write_filetable(FILE *dst_fp, struct tree_entry *trp, unsigned parent) {
	static int					index;
	struct tree_entry			*tp;
	struct file_entry			*fip;
	unsigned					perms;
	struct etfs_ftable_file		*fep;
	time_t						now = no_time ? 0 : time(NULL);

	// We make a special case for the root of the filesystem and
	// the special files.
	if(parent == FID_ROOT) {
		if(verbose)
			fprintf(stderr, " Fid Pfid  Mode Clusters Name\n");
		memset(&blockdata[0], 0xff, block_size);

		fep = (struct etfs_ftable_file *)blockdata;
		mkrootentry(&fep[FID_ROOT], "", S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, 0, now);
		mkrootentry(&fep[FID_FILETABLE], ".filetable", S_IFREG | S_IRUSR | S_IRGRP | S_IROTH, CLUSTERS(num_fids) * cluster_size, now);
		mkrootentry(&fep[FID_BADBLKS], ".badblks", S_IFREG | S_IRUSR | S_IRGRP | S_IROTH, cluster_size, now);
		mkrootentry(&fep[FID_COUNTS], ".counts", S_IFREG | S_IRUSR | S_IRGRP | S_IROTH, num_blocks * (2 * sizeof(uint16_t)), now);
		mkrootentry(&fep[FID_LOSTFOUND], ".lost+found", S_IFDIR | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH, 0, now);
		mkrootentry(&fep[FID_RESERVED], ".reserved", S_IFREG | S_IRUSR | S_IRGRP | S_IROTH, 0, now);

		index = FID_FIRSTFILE;
	}

	for(tp = trp ; tp ; tp = tp->sibling) {
		struct stat	sbuf;

		if(index == 0)
			memset(&blockdata[0], 0xff, block_size);
		fep = (struct etfs_ftable_file *)((char *)blockdata + ((index / files_per_cluster) * cluster_size) + ((index % files_per_cluster) * sizeof(struct etfs_ftable_file)));

		fep->efid = 0;
		if(strlen(tp->name) > ETFS_FNAME_SHORT_LEN)
			fep->efid = tp->flags + 1;
		fep->pfid = parent;
		fep->size = 0;

		fip = tp->fip;
		perms = fip->host_perms;

		switch(fip->attr->mode & S_IFMT) {
		case S_IFIFO:
			fep->size = 0;
			perms &= fip->attr->perms_mask;
			perms |= fip->attr->perms_set;
			++totalfiles;
			break;

		case S_IFREG:
			if(stat(fip->hostpath, &sbuf) == 0)
				fep->size = sbuf.st_size;
			perms &= fip->attr->perms_mask;
			perms |= fip->attr->perms_set;
			++totalfiles;
			break;

		case S_IFDIR:
			fep->size = 0;
			perms &= fip->attr->dperms_mask;
			perms |= fip->attr->dperms_set;
			++totaldirs;
			break;

		case S_IFLNK:
			fep->size = strlen(fip->hostpath);
			perms &= fip->attr->perms_mask;
			perms |= fip->attr->perms_set;
			++totalfiles;
			break;
		}

		fep->atime = fep->mtime = fep->ctime = fip->host_mtime;
		fep->mode = fip->attr->mode | perms;
		fep->gid = fip->attr->gid;
		fep->uid = fip->attr->uid;
		strncpy(fep->name, tp->name, ETFS_FNAME_SHORT_LEN);

		if(verbose)
			fprintf(stderr, "%4x %4x  %4.4x %5d %s\n",
				tp->flags, fep->pfid, fep->mode, NCLS(fep->size), tp->name);

		endian_file(fep);

		if(++index >= files_per_cluster * num_clusters) {
			write_transaction(dst_fp, blockdata, num_clusters, FID_FILETABLE);
			index = 0;
		}

		// Check for an extended filename and put the name out
		if(strlen(tp->name) > ETFS_FNAME_SHORT_LEN) {
			struct etfs_ftable_extname	*efep;

			efep = (struct etfs_ftable_extname *)((char *)blockdata + ((index / files_per_cluster) * cluster_size) + ((index % files_per_cluster) * sizeof(struct etfs_ftable_file)));

			efep->efid = swap16(0,ETFS_FTABLE_EXTENSION);
			efep->pfid = swap16(0,tp->flags);
			efep->type = ETFS_FTABLE_TYPE_EXTNAME;
			strncpy(efep->name, tp->name + ETFS_FNAME_SHORT_LEN, ETFS_FNAME_EXT_LEN);
			if(++index >= files_per_cluster * num_clusters) {
				write_transaction(dst_fp, blockdata, num_clusters, FID_FILETABLE);
				index = 0;
			}
		}

		if(tp->child && S_ISDIR(tp->fip->attr->mode))
			write_filetable(dst_fp, tp->child, tp->flags);
	}

	// Do a flush just before final return
	if(parent == FID_ROOT  &&  index != 0) {
		write_transaction(dst_fp, blockdata, CLUSTERS(index), FID_FILETABLE);
		}
}


static void
write_filedata(FILE *dst_fp, struct tree_entry *trp, unsigned parent) {
	static int			index;
	struct tree_entry	*tp;
	FILE				*src_fp;
	unsigned			n;

	for(tp = trp ; tp ; tp = tp->sibling) {
		if(tp->child) {
			write_filedata(dst_fp, tp->child, tp->flags);
		} else {
			switch(tp->fip->attr->mode & S_IFMT) {
			case S_IFREG:
				if((src_fp = fopen(tp->fip->hostpath, "rb")) == NULL) {
					fprintf(stderr, "Unable to open '%s': %s - skipping.\n", tp->fip->hostpath, strerror(errno));
					continue;
				}

				for(index = 0;;) {
					n = (num_clusters - index) * cluster_size;
					memset(&blockdata[index*cluster_size], 0xff, n);
					n = fread(&blockdata[index*cluster_size], 1, n, src_fp);
					if(n <= 0 && index == 0)
						break;
					index = write_transaction(dst_fp, blockdata, index + NCLS(n), tp->flags);
				}

				fclose(src_fp);
				break;

			case S_IFLNK:
				memset(&blockdata[0], 0, cluster_size);
				strcpy(&blockdata[0], tp->fip->hostpath);
				index = write_transaction(dst_fp, blockdata, 1, tp->flags);
				break;
			}
		}
	}

	// Do a final flush just before final return
	if(parent == 0 && index != 0)
		write_transaction(dst_fp, blockdata, INT_MAX, INT_MAX);
}


static int
write_transaction(FILE *dst_fp, uint8_t *data, unsigned nclusters, unsigned fid) {
	struct etfs_trans	trans;
	static int			cur = 0, sequence = 0, cluster = 0, lastfid = INT_MAX;
	int					i, n;

	// Check for a flush request.
	if(fid == INT_MAX) {
		if(cur == 0)
			return(0);
		fid = lastfid;
	// Check for a new transaction stream
	} else if(fid != lastfid) {
		lastfid = fid;
		cluster = 0;
	}

	// Limit data to space left in current etfs block.
	n = min(nclusters, num_clusters - cur);
	
	for(i = 0 ; i < n ; ++i) {
		trans.fid       = fid;
		trans.cluster   = cluster + i;
		trans.nclusters = n - i;
		trans.tacode    = ETFS_TRANS_OK;
		trans.dacode    = ETFS_TRANS_OK;
		trans.sequence  = sequence;
		endian_trans(&trans);

		fwrite(data + i*cluster_size, cluster_size, 1, dst_fp);
		fwrite(&trans, sizeof(trans), 1, dst_fp);
	}

	cluster += n;
	totalclusters += n;
	totaltransactions += 1;

	if((cur += n) >= num_clusters) {
		cur = 0;
		++sequence;
	}

	if(n != nclusters  &&  nclusters != INT_MAX)
		memcpy(data, data + n*cluster_size, (nclusters - n)*cluster_size);

	return(nclusters - n);
}


void
assign_fids(struct tree_entry *trp) {
	struct tree_entry	*tp;

	for(tp = trp ; tp ; tp = tp->sibling) {
		tp->flags = num_fids++;
		if(strlen(tp->name) > ETFS_FNAME_SHORT_LEN)
			++num_fids;
		if(tp->child)
		if(S_ISDIR(tp->fip->attr->mode))
			assign_fids(tp->child);
		else {
			fprintf(stderr, "%s : Declared as both a directory and a file. Ignoring all files below directory declaration.\n", tp->name);
			tp->child = NULL;
		}
	}
}

#else

unsigned
etfs_make_fsys(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname) {
	error_exit("Embedded transaction file system generation not supported\n");
	return 0;
}

#endif

int
etfs_need_seekable(struct file_entry *list) {
	return(0);
}

__SRCVERSION("mk_et_fsys.c $Rev: 153052 $");
