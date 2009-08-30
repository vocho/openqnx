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





/*
#ifdef __USAGE
%C - show all values for a file's stat structure (QNX)

%C	[-xl] file...
Options:
 -x        Include fsys_stat information.
 -l        Use lstat instead of fstat.
Description:
%C will display the stat structure for the file(s) named.
#endif
*/

/*
#ifdef __USAGENTO
%C - show all values for a file's stat structure (QNX)

%C	[-l] file...
Options:
 -l        Use lstat instead of fstat.
Description:
%C will display the stat structure for the file(s) named.
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <util/stdutil.h>
#include <util/util_limits.h>
#include <pwd.h>
#include <grp.h>
#ifndef __QNXNTO__
#include <sys/fsys.h>
#endif
#include <time.h>
#include <errno.h>

int main(int argc, char **argv)
{
	static struct stat statbuf;
#ifndef __QNXNTO__
	static struct _fsys_stat fsys_statbuf;
#endif
	static char miscbuf[256];
#ifndef __QNXNTO__
	static char fpbuf[UTIL_NAME_MAX];
	static char mountpoint[UTIL_NAME_MAX];
	static char devicename[UTIL_NAME_MAX];
#endif
	int err=0, use_fsys_stat=0, use_lstat=0, fd;
	struct passwd *pw;
	struct group *gr;
	
	while ((fd=getopt(argc,argv,"xl"))!=-1) {
		switch(fd) {
			case 'x': use_fsys_stat=!use_fsys_stat; break;
			case 'l': use_lstat=!use_lstat; break;
			default: err++; break;
		}
	}

	for (; optind<argc; optind++) {
		if ((fd = open(argv[optind], O_RDONLY))==-1) {
			fprintf(stderr,"fstat: could not open %s (%s)\n",
				argv[optind], strerror(errno));
			continue;
		}
		if (use_lstat) {
			if (lstat(argv[optind],&statbuf)==-1) {
				fprintf(stderr,"fstat: could not lstat %s (%s)\n",
					argv[optind],strerror(errno));
				continue;
			}
		} else if (fstat(fd,&statbuf)==-1) {
			fprintf(stderr,"fstat: could not stat %s (%s)\n",
				argv[optind],strerror(errno));
			continue;
		}

#ifndef __QNXNTO__
		if (use_fsys_stat && fsys_fstat(fd,&fsys_statbuf)==-1) {
			fprintf(stderr,"fstat: could not fsys_fstat %s (%s)\n",
				argv[optind], strerror(errno));
			continue;
		}
#endif
		close(fd);

#ifndef __QNXNTO__
		if (fsys_get_mount_dev(argv[optind],devicename)==-1) {
			devicename[0]=0;
		} else if (fsys_get_mount_pt(devicename,mountpoint)==-1) {		
			mountpoint[0]=0;
		}

		if (!qnx_fullpath(fpbuf,argv[optind])) {
			sprintf(fpbuf,"No full path for %s (%s) ",
				argv[optind], strerror(errno));
		}
		if (devicename[0])
			printf("%s (%s) resides on %s (%s)\n",
				argv[optind], fpbuf, mountpoint, devicename);
		else printf("%s (%s)\n",argv[optind],fpbuf);
#endif

		printf("st_ino     = %10lu (0x%08lx)\n",
			   (long unsigned)statbuf.st_ino,(long unsigned)statbuf.st_ino);
		printf("st_dev     = %10lu (0x%08lx)\n",
			   (long unsigned)statbuf.st_dev,(long unsigned)statbuf.st_dev);
		printf("st_size    = %10lu (0x%08lx)\n",
			   (long unsigned)statbuf.st_size,(long unsigned)statbuf.st_size);
		printf("st_rdev    = %10lu (0x%08lx)\n",
			   (long unsigned)statbuf.st_rdev,(long unsigned)statbuf.st_rdev);

#ifndef __QNXNTO__
		if (!strftime(miscbuf,sizeof(miscbuf),"%a %b %d %T %Z %Y",
			localtime(&statbuf.st_ftime)))	miscbuf[0]=0;
		printf("st_ftime   = %10lu (%s)\n",(long unsigned)statbuf.st_ftime,miscbuf);
#endif

		if (!strftime(miscbuf,sizeof(miscbuf),"%a %b %d %T %Z %Y",
			localtime(&statbuf.st_mtime)))	miscbuf[0]=0;
		printf("st_mtime   = %10lu (%s)\n",(long unsigned)statbuf.st_mtime,miscbuf);

		if (!strftime(miscbuf,sizeof(miscbuf),"%a %b %d %T %Z %Y",
			localtime(&statbuf.st_atime)))	miscbuf[0]=0;
		printf("st_atime   = %10lu (%s)\n",(long unsigned)statbuf.st_atime,miscbuf);

		if (!strftime(miscbuf,sizeof(miscbuf),"%a %b %d %T %Z %Y",
			localtime(&statbuf.st_ctime)))	miscbuf[0]=0;
		printf("st_ctime   = %10lu (%s)\n",(long unsigned)statbuf.st_ctime,miscbuf);

		printf("st_mode    = %10u (0%o, 0x%x)\n",statbuf.st_mode,statbuf.st_mode,statbuf.st_mode);

		pw=getpwuid(statbuf.st_uid);
		printf("st_uid     = %10u (%s)\n",statbuf.st_uid,pw?pw->pw_name:"Unknown");

		gr=getgrgid(statbuf.st_gid);
		printf("st_gid     = %10u (%s)\n",statbuf.st_gid,gr?gr->gr_name:"Unknown");

		printf("st_nlink   = %10u\n",statbuf.st_nlink);

#ifndef __QNXNTO__
		if (use_fsys_stat) {
			printf("st_first_xtnt.xtnt_blk  = %10lu (0x%08lx)\n",fsys_statbuf.st_first_xtnt.xtnt_blk,fsys_statbuf.st_first_xtnt.xtnt_blk);
			printf("st_first_xtnt.xtnt_size = %10lu (0x%08lx)\n",fsys_statbuf.st_first_xtnt.xtnt_size,fsys_statbuf.st_first_xtnt.xtnt_size);
      			printf("st_num_xtnts            = %10u (0x%04x)\n",fsys_statbuf.st_num_xtnts,fsys_statbuf.st_num_xtnts);
			printf("st_xblk                 = %10lu (0x%08lx)\n",fsys_statbuf.st_xblk,fsys_statbuf.st_xblk);
		}
#endif

		printf("\n");
	}
	
	return (err?EXIT_FAILURE:EXIT_SUCCESS);
}

