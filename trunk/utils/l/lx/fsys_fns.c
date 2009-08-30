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




#include <sys/stat.h>
#include <util/fs_qnx4_util.h>
#include <util/util_limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __QNXNTO__
int fsys_get_mount_dev(char *filename, char *devicename)
{
	/* ok how the heck do we do this? */

}
#endif

#ifdef __QNXNTO__
int qnx4fs_fsys_stat( char *filename, struct _qnx4fs_fsys_stat *buf )
{
	char full_path[UTIL_PATH_MAX];
	char *directory_name, *file_name;
	struct stat statbuf;
    qnx4fs_dir_entry_t direntrybuf;
	int dirfd;

    /* get fullpath of file */
	if (_fullpath(full_path, filename,sizeof(full_path))==NULL)
		return -1; /* leave errno intact */

	/* get regular stat info for the file's full path */
	if (stat(full_path,&statbuf)==-1) return -1; /* leave errno intact */

	/* copy pertinent fields to buf */
    memset(buf,0,sizeof(*buf));
	buf->st_ino = statbuf.st_ino;
	buf->st_dev = statbuf.st_dev;
	buf->st_size = statbuf.st_size;
	buf->st_rdev = statbuf.st_rdev;
    buf->st_mtime = statbuf.st_mtime;
    buf->st_atime = statbuf.st_atime;
	buf->st_ctime = statbuf.st_ctime;
	buf->st_mode = statbuf.st_mode;
	buf->st_uid = statbuf.st_uid;
	buf->st_gid = statbuf.st_gid;
	buf->st_nlink = statbuf.st_nlink;
    /* missing: st_first_xtnt, st_num_xtnts, st_xblk, st_ftime, st_status */

    /* derive parent directory pathname from fullpath */
	directory_name = dirname(full_path);
    /* derive file basename from fullpath */
    file_name = basename(full_path);

    /* open parent directory read-only */
    if (-1==(dirfd=open(directory_name,O_RDONLY))) return -1; /* leave errno intact */

    /* read directory entries looking for basename */
	for (;;) {
		int nbytes, bytesread;
		qnx4fs_dir_entry_t *ptr;
		
		/* read a dir entry */
        nbytes=sizeof(direntrybuf);
		ptr=&direntrybuf;
		bytesread=0;
        for (nbytes=sizeof(direntrybuf);nbytes!=0;nbytes-=bytesread) {
			bytesread=read(dirfd,ptr,nbytes);
			if (bytesread==-1) {
				close(dirfd);
				return -1;
			}
		}

		if (0==strncmp(direntrybuf.d_inode.i_fname,file_name,strlen(file_name)+1)) 
			break;
	}
	close(dirfd);

    /* direntrybuf points to directory entry for this file. There are two
       cases: file has a directory inode, or file has a separate inode. This
       function only deals with the former case i.e. files with short
       file names that have never had multiple links */

	if (direntrybuf.d_link.l_status & QNX4FS_FILE_INODE) {
		errno = ENOTSUP;
		return -1;
	}

    /* when a match is found, duplicate pertinent fields,
       perform sanity check on fields obtained via stat(), and return */
	buf->st_first_xtnt = direntrybuf.d_inode.i_first_xtnt;
	buf->st_num_xtnts = direntrybuf.d_inode.i_num_xtnts;	
	buf->st_xblk = direntrybuf.d_inode.i_xblk;

    /* ftime, status is not returned by stat in neutrino */
	buf->st_ftime = direntrybuf.d_inode.i_ftime;
	buf->st_status = direntrybuf.d_inode.i_status;

	return 0;
}
#else

int qnx4fs_fsys_stat( char *filename, struct _qnx4fs_fsys_stat *buf )
{
	return fsys_stat(filename, buf);
}

#endif

