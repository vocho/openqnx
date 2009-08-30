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



#ifdef __USAGE		/* mount.c */
%C - mount partitions and filesystems (UNIX)

%C	[-v] [-w secs] -p block_special_file...
%C	[-v] [-w secs] block_special_file directory [flags]...
%C	[-v] -t type "type specific options, check mount_'type'"
Options:
 -p        Mount partitions on this device as block special files in /dev.
           (Works only with block special files which contain partitions 
           such as /dev/hd0.)
 -P        Mount partitions as per '-p', but skip DOS Extended (t5) partitions.
 -t        Type of filesystem. (defaults to "qnx4" or "nfs" if ':' or '@' used)
 -w secs   Wait up to the indicated number of seconds for the blocks special
           file to appear.  Useful at boot time with slow resetting hardware.
           (Default: 60 seconds)
 -v        be verbose.
Flags:
 -a        All metadata (system) updates are asynchronous.
 -e        Don't allow executables to load.
 -g        Don't allow persistent pregrown files.
 -r        Mount as a read-only file system.
 -s        All data (user) updates are synchronous.
 -u        Don't honor setuid bits.
#endif

/*
#define MTAB
#define MOUNT_TABLE		"/etc/mtab"
*/

/*---------------------------------------------------------------------



	$Header$

	$Source$

	$Log$
	Revision 1.4  2005/06/03 01:37:52  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.3  1999/06/07 21:11:47  steve
	Attempt #3 to get an initial working setup....
	
	Revision 1.27  1999/04/08 20:07:09  jclarke
	ÿ¡ÿ©ÿ¡ÿ©ÿ¡fixed bug caused by goto statement in function install_extended_partitions()

	Revision 1.26  1999/03/30 17:49:33  jclarke
	added functionality for type 15 extended partitions .

	Revision 1.25  1999/03/22 14:41:30  jclarke
	fixed extened partition stuff so that t5 can be in any position in extended partition table.
	ÿ¡ÿ¡ÿ¡ÿ¡ÿ¡.

	Revision 1.24  1999/01/04 15:11:37  jgarvey
	Add -P option to skip mounting contents of extended partitions
	This is because the mount code is too simplistic (Linux/Dos t5 ordering)

	Revision 1.23  1997/03/07 03:25:37  waflowers
	Reading the bios signature into an "unsigned" is bad --
	the variable might be 32-bits but the signature is only
	2 bytes.

	Revision 1.22  1997/02/26 01:35:17  waflowers
	Support for new mount flags added for 4.24

	Revision 1.21  1996/09/03 15:35:29  waflowers
	Fix for extended partitions (they can be type '0' -- not allocated).
	Also added a final exit(0) -- there never was one.  Amazing!

	Revision 1.20  1996/08/15 16:49:17  waflowers
	Use block_read() instead of lseek and read for DOS extended partitions.
	This lets us get beyond the 2G point on the drive.

	Revision 1.19  1996/07/18 18:27:07  steve
	*** empty log message ***

 * Revision 1.18  1994/12/05  18:18:44  steve
 * Added mountp_error() to try and guess at why the mount failed.
 * Not perfect, but should be an improvement...
 *
 * Revision 1.17  1993/08/25  18:29:54  brianc
 * Don't exit on duplicate paritition types, just skip it
 * Use strerror() in preference to perror()
 *
 * Revision 1.16  1993/08/24  18:42:13  brianc
 * *** empty log message ***
 *
 * Revision 1.15  1993/07/16  20:13:14  peterv
 * Changed the default wait time from 10secs to 60secs
 *
 * Revision 1.14  1993/06/11  15:22:05  eric
 * when mount() fails with ENOREMOTE, mount will now display an extensive
 * message about what to do
 *
 * Revision 1.13  1993/06/07  20:09:30  peterv
 * Changed execing from "?_mount" to "mount_?" to be consistant with
 * other unix's
 *
 * Revision 1.12  1993/04/08  17:22:03  peterv
 * Added "-t type" option for nfs and other file systems
 *
 * Revision 1.11  1992/10/27  17:17:53  eric
 * added usage one-liner
 *
 * Revision 1.10  1992/06/23  19:18:39  waflowers
 * Small revision to usage message for -w option.
 *
 * Revision 1.9  1992/06/23  19:11:27  waflowers
 * Added new option '-w secs' to override wait period.
 * Instead of 10 seconds, some devices seem to need more time!
 *
 * Revision 1.8  1992/03/23  21:23:14  eric
 * will now only rety open or stat waiting for a driver to come up
 * if the errno from the failed call is ENOENT.
 *
 * Revision 1.7  1992/02/13  17:45:28  waflowers
 * Changed to check via stat() instead of open() for presence.
 * This way you can mount something which Dosfsys has mounted
 * and floppies won't spin at mount time.
 *
 * Revision 1.6  1992/01/26  15:04:34  waflowers
 * Preserve errno when calling fprintf().
 * It was changing it internally (sometimes at least).
 * This was very confusing!
 *
 * Revision 1.5  1991/10/16  16:22:27  waflowers
 * Hmmm, forgot to save and restore current seek position
 * when mounting DOS extended partitions.
 * Simplest way is to re-open the device.
 *
 * Revision 1.4  1991/10/16  16:15:44  waflowers
 * Oops, forgot to include <errno.h>.
 *
 * Revision 1.3  1991/10/16  16:13:33  waflowers
 * Made mount more tolerant when using it with an old Fsys
 * and mounting DOS extended partitions (they won't work together).
 *
 * Revision 1.2  1991/10/16  15:58:40  waflowers
 * Added (correct) support for DOS extended partitions.
 * Sends a new message (currently only supported by experimental Fsys)
 * to mount extended partitions.
 * No longer mounts partition type 5.
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <i86.h>
#include <limits.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/psinfo.h>
#include <sys/kernel.h>
#include <sys/vc.h>
#include <string.h>
#include <sys/fsys.h>
#include <sys/fsys_msg.h>
#include <sys/disk.h>
#include <errno.h>

/* #include <stdutil.h> */
int		wait_secs = 60;

/*
 *  Structure of a partition table entry
 */
struct part_entry
	{
	unsigned char	boot_ind,
					beg_head,
					beg_sector,
					beg_cylinder,
					os_type,
					end_head,
					end_sector,
					end_cylinder;
	long			part_offset,
					part_size;
	};

#ifdef MTAB
/* block_special_file directory\n */
char			path[PATH_MAX];
char			line[(2*PATH_MAX)+2];
#endif
char			fname[NAME_MAX];

int      verbose = 0;

void
vprint(char *fmt, ...)
{
	va_list v;
	if (!verbose)
		return;
	va_start(v, fmt);
	vfprintf(stdout, fmt, v);
	fflush(stdout);
}

main(argc, argv)
int		 argc;
char	*argv[];
	{
	register char	*p;
	register int	 n;
	char			*spec,
					*dir;
	int				 mount_flags;
	int				 qnx4flag = 0;
#ifdef MTAB
	FILE			*fp;
#endif

#ifndef MTAB
	if(need_usage(argv) || argc <= 1) print_usage(argv);
#else
	if ( argc <= 1 ) {
		if ( !( fp = fopen( MOUNT_TABLE, "r" ) ) ) {
			fprintf(stderr, "%s:  Unable to open %s: %s\n",
				"mount", MOUNT_TABLE, strerror(errno));
			exit( EXIT_FAILURE );
			}
		while( fgets( &line, sizeof( line ), fp ) != NULL )
			fprintf( stdout, "%s", line );
		fclose( fp );
		exit( EXIT_SUCCESS );
		}
#endif

	for(n = 1; n < argc; ++n)
		{
		p = argv[n];
		if (p[0] == '-' && p[1] == 'v') {
			verbose = 1;
			continue;
		}
		if(p[0] == '-'  &&  p[1] == 't')
			{
			if(p[2])
				p += 2;
			else
				{
				if(++n == argc)
					{
					fprintf(stderr, "%s: missing argument for -t option\n", "mount");
					exit(EXIT_FAILURE);
					}
				p = argv[n];
				}
			if(strcmp(p, "qnx4"))
				{
				sprintf(fname, "mount_%s", p);
				argv[0] = fname;
				execvp(fname, argv);
				fprintf(stderr, "%s: could not exec \"%s\", %s\n",
					"mount", argv[0], strerror(errno));
				exit(EXIT_FAILURE);
				}
			else
				qnx4flag = 1;
			}
		else if(p[0] == '-'  &&  p[1] == 'w')
			{
			if(p[2] == '\0')
				{
				if(++n == argc)
					{
					fprintf(stderr, "%s: missing argument for -w option\n", "mount");
					exit(EXIT_FAILURE);
					}
				else
					wait_secs = atoi(argv[n]);
				}
			else
				wait_secs = atoi(&p[2]);
			}
		else if(p[0] == '-'  &&  (p[1] == 'p' || p[1] == 'P'))
			{
			if(p[2] != '\0')
				spec = p + 2;
			else if(++n < argc)
				spec = argv[n];
			else {
				fprintf( stderr, "%s: invalid 'device with partitions'\n", "mount");
				exit(EXIT_FAILURE);
				}

			install_partitions(spec, p[1] == 'p');
			}
        else if(p[0] == '-')
            {
            fprintf(stderr, "%s: invalid option '%s' ignored\n", "mount", p);
            continue;
            }
		else if(argc - n < 2) {
			fprintf( stderr, "%s: invalid number of arguments\n", "mount" );
			exit(EXIT_FAILURE);
			}
		else
			{
			spec = argv[n++];
			dir = argv[n];
			mount_flags = 0;
			while(n + 1 < argc)
				{
				p = argv[n + 1];
                if(*p != '-')
                    break;

                while(*++p)
                    switch(*p)
                        {
                    case 'a':
                        mount_flags |= _MOUNT_ASYNCH;
                        break;

                    case 'e':
                        mount_flags |= _MOUNT_NOEXEC;
                        break;

                    case 'g':
                        mount_flags |= _MOUNT_NOGROW;
                        break;

                    case 'r':
                        mount_flags |= _MOUNT_RDONLY;
                        break;

                    case 'u':
                        mount_flags |= _MOUNT_NOSUID;
                        break;

                    case 's':
                        mount_flags |= _MOUNT_SYNCHRONOUS;
                        break;

                    default:
                        if(p == argv[n + 1] + 1)
                            goto mount_it;

                        fprintf(stderr, "%s: unknown mount flag '%c'\n",
                            argv[0], *p);
                        }

				++n;
				}

			/*
			 * If "-t qnx4" was not specified, and the special name has a
			 * ':' or '@' in it, exec into mount_nfs
			 */
mount_it:
			if(!qnx4flag && (strchr(spec, ':') || strchr(spec, '@')))
				{
				argv[0] = "mount_nfs";
				execvp(argv[0], argv);
				fprintf(stderr, "%s: could not exec \"%s\", %s\n",
					"mount", argv[0], strerror(errno));
				exit(EXIT_FAILURE);
				}

			mount_fsys(spec, dir, mount_flags);
			}
		}

	exit(EXIT_SUCCESS);
	}


mountp_error(int fd, char *special, struct part_entry *p)
{
	struct _disk_entry   dinfo;
	char    *mountprog = "mount -p";
	fprintf(stderr, "%s: cannot mount partition %d (0%xh) on %s:\n   error %s\n",
			mountprog, p->os_type, p->os_type, special, strerror(errno));
	if (disk_get_entry(fd, &dinfo) == -1) {
		switch (errno) {
		case ENOSYS:
			fprintf(stderr, "   '%s' is not a disk, error- %s\n",
				special, strerror(errno));
		default:
			fprintf(stderr, "   cannot get disk information about '%s' (%s)\n",
				special, strerror(errno));
		}
		exit(EXIT_FAILURE);
	}
/*-
 * Print out disk info, and try and figure out why mount partition failed.
 */
 	if (dinfo.blk_offset) {
 		fprintf(stderr, "   %s is a partition.   It is unusual to mount onto a partition\n",
 				special);
 		fprintf(stderr, "   %s occupies sectors %ld..%ld of the disk\n",
 				special,
				dinfo.blk_offset, dinfo.blk_offset+dinfo.num_sectors);
	} else {
		fprintf(stderr, "   disk %s has %ld sectors\n",
				special, dinfo.disk_sectors);
	}

	if (p->part_offset < 0 || p->part_size < 0 ||
	    p->part_offset+p->part_size < 0) {
	    fprintf(stderr, "   partition is invalid, offset is %ld, size %ld\n",
	    	p->part_offset, p->part_size);
	} else if (p->part_offset + p->part_size > dinfo.num_sectors) {
		fprintf(stderr, "   partition (%ld..%ld) does not fit within disk\n",
			p->part_offset, p->part_offset+p->part_size);
	}
	exit(EXIT_FAILURE);
}

install_partitions(special, ext)
char	*special;
int		ext;
	{
	register int		n;
	int					fd;
	short unsigned      signature;
	struct part_entry	partition;

	/*
	 *	Try 60 times, once per second, to open the device.
	 *	This is in case the driver is just coming up.
	 */

	vprint("waiting to open %s...", special);
	fd = open(special, O_RDONLY);
	for(n = 0; fd == -1  &&  errno == ENOENT  &&  n < wait_secs; ++n)
		{
		sleep(1);
		fd = open(special, O_RDONLY);
		}

	if(fd == -1)
		{
		fprintf(stderr, "%s: open() on %s failed: %s\n",
			"mount -p", special, strerror(errno));
		exit(EXIT_FAILURE);
		}

	/*
	 *	Look for 55 AA partition signature.
	 */
	if(lseek(fd, 510L, 0) == -1L)
		{
		fprintf(stderr, "%s: lseek() to BIOS signature on %s failed: %s\n",
			"mount -p", special, strerror(errno));
		exit(EXIT_FAILURE);
		}
	vprint("reading block 0...");
	if(read(fd, &signature, 2) != 2)
		{
		fprintf(stderr, "%s: read() of BIOS signature on %s failed: %s\n",
			"mount -p", special, strerror(errno));
		exit(EXIT_FAILURE);
		}

	/*
	 *	Remember the byte ordering on Intel machines.
	 */
	if(signature != 0xAA55)
		{
		fprintf(stderr, "%s: No BIOS signature in partition sector on %s\n",
			"mount -p", special);
		exit(EXIT_FAILURE);
		}

	/*
	 *	Seek to the partition table.
	 */
	if(lseek(fd, 512L - (4 * sizeof(struct part_entry) + 2), 0) == -1L)
		{
		fprintf(stderr, "%s: lseek() to start of partition table on %s failed: %s\n",
			"mount -p", special, strerror(errno));
		exit(EXIT_FAILURE);
		}
	vprint("\n");
	/*	Scan the partition table... */
	for(n = 0; n < 4; ++n)
		{
		vprint("reading partition %d...", n);
		if(read(fd, &partition, sizeof(partition)) != sizeof(partition))
			{
			fprintf(stderr, "%s: read() of partition entry on %s failed: %s\n",
				"mount -p", special, strerror(errno));
			exit(EXIT_FAILURE);
			}

		if(partition.os_type != 0) {
			vprint("installing partition type %d\n", partition.os_type);
			if((partition.os_type == 5)||(partition.os_type ==15)) {	/*	Install extended partitions	*/
				if (ext)
					install_extended_partitions(special, partition.part_offset,
														partition.part_size);
			}
			else if(fsys_mount_partition(special, partition.os_type,
							partition.part_offset, partition.part_size) == -1)
				if (errno == EEXIST) {
					fprintf(stderr, "%s: skipping mount of duplicate partition type %d on %s\n",
						"mount -p", partition.os_type, special);
				} else {
					mountp_error(fd, special, &partition);
				}
		} else {
			vprint("partition is empty\n");
		}
	}

	close(fd);
	}


#define NUM_PARTS	4
#define LOADER_SIZE	(_BLOCK_SIZE - (NUM_PARTS * sizeof(struct part_entry) + 2))

struct part_block
	{
	char				filler[LOADER_SIZE];
	struct part_entry	part_entry[NUM_PARTS];
	short unsigned		signature;
	};


install_extended_partitions(special, base_offset, base_size)
char	*special;
long	 base_offset,
		 base_size;
	{
	int					ext_num,
						fd;
	int 				ext_par_ndx,i;
	long				ext_offset = 0;

    /*
     *  Re-open the drive so we don't mess up the seek position
     */
	if((fd = open(special, O_RDONLY)) == -1)
		{
		fprintf(stderr, "%s: open() on %s failed: %s\n",
			"mount -p", special, strerror(errno));
		exit(EXIT_FAILURE);
		}

	for(ext_num = 1; ; ++ext_num)
	{
        struct part_block    part_table;
        struct part_entry   *partition;

        vprint("reading block %d for DOS extended partition #%d\n",
                base_offset + ext_offset, ext_num);
        if(block_read(fd, base_offset + ext_offset + 1, 1, &part_table) != 1)
			{
			fprintf(stderr, "%s: block_read() of DOS extended partition #%d failed: %s\n",
				"mount -p", ext_num, strerror(errno));
				fprintf(stderr, "%s: giving up on the DOS extended partitions.\n", "mount -p");
			return;
			}

		/*
		 *	Remember the byte ordering on Intel machines.
		 */
		if(part_table.signature != 0xAA55)
		{
			fprintf(stderr, "%s: No signature in DOS extended partition #%d\n",
				"mount -p", ext_num);
			fprintf(stderr, "%s: giving up on the DOS extended partitions.\n", "mount -p");
			return;
		}

		ext_par_ndx = -1;
   	partition = &part_table.part_entry[0];

		for(i=0;i<4;i++)
		{
			if(partition->os_type == 0)
			{	
				++partition;
           	continue;
      	} 

      	if((partition->os_type == 5)||(partition->os_type == 15))
			{
				ext_par_ndx = i;
				++partition;
				continue;
			}
      

			if(ext_offset + partition->part_size > base_offset + base_size)
			{
				fprintf(stderr, "%s: DOS extended partitions either non-stardard or corrupt\n"
							"mount -p");
weird2:
				fprintf(stderr, "%s: giving up on the DOS extended partitions.\n", "mount -p");
				return;
			}

			if(fsys_mount_ext_part(special, partition->os_type, ext_num,
							base_offset + ext_offset + partition->part_offset,
												partition->part_size) == -1)
			{
				if(errno == ENOSYS)
				{
					fprintf(stderr, "%s: mounting DOS extended partitions not supported by this Fsys.\n",
					"mount -p");
					goto weird2;
				}

				fprintf(stderr, "%s: mounting DOS extended partition #%d on %s failed: %s\n",
				"mount -p", ext_num, special, strerror(errno));
				exit(EXIT_FAILURE);
			}
			
			++partition;
		}

		if(ext_par_ndx != -1)
		{
        	partition = &part_table.part_entry[0];
        	partition += ext_par_ndx;
			ext_offset = partition->part_offset;
		}
		else
			break;
	}

	close(fd);
	}


mount_fsys( char *spec, char *dir, int rwflag )
	{
	int				 n,
					 retval;
	struct	stat	 statbuf;
#ifdef MTAB
	FILE			*fp,
					*fp2;
	struct	_psinfo	 psbuf;
	pid_t			 vid;
#endif

	/*
	 *	Try 60 times, once per second, to stat the device.
	 *	This is in case the driver is just coming up.
	 */
	vprint("waiting to stat '%s'...", spec);
	retval = stat(spec, &statbuf);
	for(n = 0; retval == -1  &&  errno == ENOENT  &&  n < wait_secs; ++n)
		{
		sleep(1);
		retval = stat(spec, &statbuf);
		}

	if(retval == -1) {
		fprintf(stderr,"%s: stat() on %s failed: %s\n",
			"mount", spec, strerror(errno));
		exit(EXIT_FAILURE);
		}
	vprint("mounting '%s' on '%s' ...", spec, dir);
	if(mount(spec, dir, rwflag) == -1) {
		fprintf(stderr,"%s: mount() of %s failed: %s\n",
			"mount", spec, strerror(errno));
		if (errno==ENOREMOTE) {
			fprintf(stderr,"mount: To mount a remote disk, the disk must first be mounted on the\n");
			fprintf(stderr,"       remote system. Then, to obtain access to the remote mounted disk\n");
			fprintf(stderr,"       under your local /, you should either use the prefix utility to\n");
			fprintf(stderr,"       create a prefix alias to the remote mount point, or create a\n");
			fprintf(stderr,"       symbolic link to the remote mount point using ln -s.\n");
		}

		exit(EXIT_FAILURE);
		}
	vprint("done\n");
#ifdef MTAB
	/*
	 *	Remove the MOUNT_TABLE file if it was created before the system
	 *	who holds the file booted.
	 *
	 *	Update the MOUNT_TABLE file. (block_special_file directory)
	 */
	if ( qnx_fullpath( &path, MOUNT_TABLE ) != NULL ) {
		if ( stat( path, &statbuf ) != -1 ) {
			if ( ( vid = qnx_vc_attach( atoi( path + 2 ), PROC_PID,
				sizeof( struct _psinfo ) + 10, 0 ) ) != -1 ) {
				if ( qnx_psinfo( vid, PROC_PID, &psbuf, 0, 0 ) != -1 ) {
					if ( statbuf.st_ctime < psbuf.un.proc.start_time ) {
						unlink( path );
						}
					}
				}
			}
		}

	if ( fp = fopen( path, "r" ) )
		unlink( path );
	if ( !( fp2 = fopen( path, "w" ) ) ) {
		fprintf( stderr, "%s: unable to open %s: %s\n",
			"mount", path, strerror(errno));
		exit( EXIT_FAILURE );
		}
	qnx_fullpath( &path, spec );
	while( fp  &&  fgets( &line, sizeof( line ), fp ) != NULL ) {
		if ( strncmp( line, path, strchr( line, ' ' )-line ) != 0 )
			fprintf( fp2, "%s", line );
		}
	fprintf( fp2, "%s ", path );
	qnx_fullpath( &path, dir );
	fprintf( fp2, "%s\n", path );
	if ( fp ) fclose( fp );
	fclose( fp2 );
#endif
	}
