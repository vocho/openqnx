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





#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/memmsg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/statvfs.h>

char *__progname;

#define DEFSIZE		(64*1024*1024)

int
usage(void) {
	execlp("use", "use", __progname, NULL);
	return 0;
}

#define 	SWAP_SIG	"QNX.SWAP"

struct _swap_sig {
	char		name[8];
	unsigned	size;
	unsigned	flags;
	unsigned	reserved[4];
};

int
check_signature(int fd) {
	struct _swap_sig sig;

	lseek(fd, 0, SEEK_SET);
	read(fd, &sig, sizeof sig);

	if(strncmp(sig.name, SWAP_SIG, 8) == 0) {
		return 0;
	}	
	return -1;
}

int
add_signature(int fd) {
	struct _swap_sig sig;

	memset(&sig, 0, sizeof sig);
	memcpy(&sig, SWAP_SIG, 8);

	sig.size = 0;

	lseek(fd, 0, SEEK_SET);
	write(fd, &sig, sizeof sig);

	return 0;
}

int
check_space(char *name, unsigned size, unsigned min_free) {
	struct statvfs 		vfs;

	memset(&vfs, 0, sizeof vfs);
	if((statvfs(name, &vfs) == 0) && vfs.f_frsize && (vfs.f_frsize * vfs.f_bfree >= min_free + size)) {
		return 1;
	}

	return 0;
}

int
display_stats(void) {
	mem_swap_t		msg;
	iov_t			iov[2];

	memset(&msg, 0, sizeof msg);

	msg.swap_stat.type = _MEM_SWAP;
	msg.swap_stat.subtype = _MEM_SWAP_STAT;
	SETIOV(&iov[0], &msg, sizeof (struct _mem_swap_stat));
	SETIOV(&iov[1], &msg, sizeof (struct _mem_swap_stat_reply));
	if(MsgSendv(MEMMGR_COID, &iov[0], 1, &iov[1], 1) == 0) {
		printf("Swap stats:\n");
		printf("\tswap space size: %u\n", (unsigned) msg.swap_stat_reply.total);
		printf("\tswap space in use: %u\n", (unsigned) msg.swap_stat_reply.inuse);
		printf("\tnumber of pages swapped out: %u\n", (unsigned) msg.swap_stat_reply.swapouts);
		printf("\tnumber of pages swapped in: %u\n", (unsigned) msg.swap_stat_reply.swapins);
		printf("\tnumber of pages deleted from swap: %u\n", (unsigned) msg.swap_stat_reply.swaprems);
	} else {
		printf("Error obtaining swapper stats!\n");
	}
	return 0;
}

int
make_swap(char *name) {
	mem_swap_t		msg;
	iov_t			iov[2];
	int				len = strlen(name);

	msg.swap_on.type = _MEM_SWAP;
	msg.swap_on.subtype = _MEM_SWAP_ON;
	msg.swap_on.flags = 0;
	msg.swap_on.size = 0;
	msg.swap_on.len = len + 1;
	SETIOV(&iov[0], &msg, sizeof (struct _mem_swap_on));
	SETIOV(&iov[1], (void *)name, len + 1);
	return MsgSendv(MEMMGR_COID, iov, 2, 0, 0);
}

int
main(int argc, char *argv[]) {
	int			on, off, dynamic, create, stats, sig, space, force;
	unsigned	size, min_free;
	int			i;
	char		elfhdr;
	char		*name;
	extern char *__progname;

	on = off = dynamic = create = stats = sig = space = 0;
	force = 0;
	min_free = 0;
	name = NULL;

	size = DEFSIZE;

	if(strcmp(__progname,"swapon") == 0) {
		on = 1;
		if(argc < 2) {
			usage();
			exit(1);
		}
		name = argv[argc-1];
	} else if(argc < 2) {
		display_stats();
		exit(0);
	}

	while((i = getopt(argc, argv, "a:cdF:mrsS:f")) != -1) {
		switch (i) {
		case 'a':
			on = 1;
			name = optarg;
			break;
		case 'c':
			create = 1;
			break;
		case 'd':
			dynamic = 1;
			break;
		case 'F':
			min_free = strtol(optarg, &optarg, 10);
			switch(*optarg) {
			case 'k':
			case 'K':
				min_free <<= 10;
				break;
			case 'm':
			case 'M':
				min_free <<= 20;
				break;
			case 'g':
			case 'G':
				min_free <<= 30;
				break;
			case '\0':
				break;
			default:
				usage();
				exit(1);
			}
			break;
		case 'm':
			sig = 1;
			break;
		case 'r':
			off = 1;
			break;
		case 's':
			stats = 1;
			break;
		case 'S':
			size = strtol(optarg, &optarg, 10);
			switch(*optarg) {
			case 'k':
			case 'K':
				size <<= 10;
				break;
			case 'm':
			case 'M':
				size <<= 20;
				break;
			case 'g':
			case 'G':
				size <<= 30;
				break;
			case '\0':
				break;
			default:
				usage();
				exit(1);
			}
			break;
		case 'f':
			force = 1;
			break;
		case '?':
		default:
			usage();
			exit(0);
			break;
		}
	}

	if(stats) {
		display_stats();
	}

	if(off) {
		fprintf(stderr, "Cannot remove swapfile\n");
		exit(1);
	}

	if(on && name) {
		struct stat st;

		//Only allow root to perform this operation, initial sanity check
		if(geteuid() != 0 && getegid() != 0) {
			fprintf(stderr, "Swapfile can only be activated by root user \n");
			exit(1);
		}

		//Some level of sanity checking on the file, only on regular files
		if(stat(name, &st) != -1) {
			if(!(S_ISREG(st.st_mode) || S_ISBLK(st.st_mode) || S_ISNAM(st.st_mode))) {
				fprintf(stderr, "Swapfile %s is not a regular/block/name file\n", name);
				exit(1);
			}

			/* Older swapfiles were created forgetting to put the mode on so were
			   created with wx permissions =;-( */
			if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH) && !force &&
			   ((st.st_mode & S_IPERMS) != (S_IXUSR | S_IWUSR))) {
				fprintf(stderr, "Swapfile %s is executable, use -f to force\n", name);
				exit(1);
			}
		} 

		if((i = open(name, O_RDWR)) == -1 && !create) {
			fprintf(stderr, "Swapfile %s cannot be opened; specify -c to create one\n", name);
			exit(1);
		}

		//Check for ELF file and warn the user
		if(!force && i != -1) {
			if(read(i, &elfhdr, 1) == 1 && elfhdr == 0x7f &&
			   read(i, &elfhdr, 1) == 1 && elfhdr == 'E' &&
			   read(i, &elfhdr, 1) == 1 && elfhdr == 'L' &&
			   read(i, &elfhdr, 1) == 1 && elfhdr == 'F') {
				fprintf(stderr, "Swapfile %s is ELF file, use -f to force\n", name);
				exit(1);
			}
			lseek(i, 0, SEEK_SET);
		} 

		if(i == -1) {		//Create must have been specified to go through here
			umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH|S_IXOTH);
			
			/* No access */
			if((i = open(name, O_RDWR|O_CREAT, 0000)) == -1) {
				fprintf(stderr, "Swapfile %s cannot be created\n", name);
				exit(1);
			}
			
			if((space = check_space(name, size, min_free)) == 0) {
				fprintf(stderr, "Filesystem has insufficient free space (%u requested)\n", size);
				unlink(name);
				exit(1);
			}

			add_signature(i);
			if(lseek(i, 0, SEEK_SET) == -1 || ftruncate(i, size) == -1) {
				fprintf(stderr, "Swapfile %s cannot be grown to %u bytes \n", name, size);
				unlink(name);
				exit(1);
			}

			create++; //Indicate that we actually did create a file
			close(i);
		} else if(check_signature(i) && !sig) {
			fprintf(stderr, "Swapfile %s does not contain a valid signature\n", name);
			exit(1);
		} else if(check_signature(i)) {
			add_signature(i);
			close(i);
		} else {
			close(i);
		}

		if(make_swap(name) == -1) {
			fprintf(stderr, "Could not make file %s a swap device: %s\n", name, strerror(errno));
			if(create > 1) {
				unlink(name);
			}
			exit(1);
		} else {
//
// Should add a 'verbose' switch later to print this out if requested
//			fprintf(stderr, "File %s successfully added as a swap device\n", name);
			exit(0);
		}
	} else if(!stats) {
		usage();
	}

	return 0;
}
