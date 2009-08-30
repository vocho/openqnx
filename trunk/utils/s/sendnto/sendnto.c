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
%C	[options]... imagefilename

Options:
 -b baudrate  Serial baud rate.
 -d device    Device to send image to.
 -e           Request an echo every record (default: don't request).
 -i host[:port] Host (with optional port) to send image to.
 -l speed     Output is to a parallel port with a LAPLINK cable. Speed may
              be 1..3 with 1 being the fastest.
 -p (nto|gdb) Set download protocol to NTO (the default) or GDB.
 -P regnum[,size] Set the GDB PC register number and size.
 -q           Quiet. Don't print an ongoing percentage of image downloaded.
 -v           be more verbose when sending.
 -w <timeout>[:<response>] Wait for <timeout> seconds or until <response>
              has been received before closing the connection.
#endif
*/

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <sys/stat.h>
#include <time.h>

#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/startup.h)

#define DEFN


#include "sendnto.h"

uint8_t	data[512];
char	*imagename;

struct xfer {
	const char	*name;
	int	(*start)(int devicefd);
	int	(*data)(int devicefd, int seq, unsigned long addr, const void *data, int nbytes);
	int (*done)(int devicefd, int seq, unsigned long start_addr);
};

const struct xfer xfers[] = {
	{"nto", xfer_start_nto, xfer_data_nto, xfer_done_nto},
	{"gdb", xfer_start_gdb, xfer_data_gdb, xfer_done_gdb},
};

const struct xfer *xfer = &xfers[0];


#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

#define SWAP16( val ) ( (((val) >> 8) & 0x00ff)	\
					  | (((val) << 8) & 0xff00) )

long 
swap32(long val) {
	return(host_endian != target_endian ? SWAP32(val) : val);
}

int
swap16(int val) {
	return(host_endian != target_endian ? SWAP16(val) : val);
}


static int
send_image(FILE *imagefp, int devicefd, void *h, int nbytes) {
	struct startup_header	*hdr;
	struct stat				sbuf;
	int						seq;
	int						percentage;
	int						scan_idx = 0;
	unsigned long			total;
	unsigned long			sofar;
	unsigned long			start_addr;
	unsigned long			curr_addr;

	do {
		// RAW format images place a section of code to jump to 
		// the start address in front of the standard BINARY header. 
		// Scan forward in order to detect the header.
		hdr = (struct startup_header*) (((char*) h) + scan_idx);

		if(scan_idx > (nbytes - sizeof(struct startup_header))) 
			return(-1);

		// We assume that images will all start on a 4 byte boundary.
		scan_idx += 4;
		
		target_endian = ((hdr->flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN) != 0);
	} while(swap32(hdr->signature) != STARTUP_HDR_SIGNATURE);

	if(fstat(fileno(imagefp), &sbuf) != 0) {
		fprintf(stderr, "Unable to get image size\n");
		return(EXIT_FAILURE);
	}
	total = sbuf.st_size;

	curr_addr = swap32(hdr->image_paddr) + swap32(hdr->paddr_bias);
	start_addr = swap32(hdr->startup_vaddr);

	gdb_cputype = swap16(hdr->machine);

	percentage = -1;
	sofar = 0;
	seq = xfer->start(devicefd);
	do {
		//First block of data is already in data buffer
		seq = xfer->data(devicefd, seq, curr_addr, data, nbytes);

		curr_addr += nbytes;
		sofar += nbytes;

		if(!quiet  &&  total  &&  (percentage != (sofar*100)/total)) {
			printf("Percent Complete: %2d%%\r", percentage = (sofar*100)/total);
			fflush(stdout);
		}
	} while((nbytes = fread(data, 1, sizeof(data), imagefp)));

	if(!quiet) {
		printf("\n");
	}

	if(!feof(imagefp)) {
		fprintf(stderr, "Error reading %s: %s\n", imagename, strerror(errno));
		return(EXIT_FAILURE);
	}

	xfer->done(devicefd, seq, start_addr);
	return(0);
}

static int
send_elf(FILE *imagefp, int devicefd, void *h, int nbytes) {
	Elf32_Ehdr		*hdr = h;
	Elf32_Phdr		*phdrv;
	Elf32_Phdr		*phdr;
	unsigned		size;
	unsigned		phnum;
	int				seq;
	int				percentage;
	unsigned long	total;
	unsigned long	sofar;
	unsigned long	start_addr;
	unsigned long	curr_addr;
	unsigned		i;

	if(memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0) return(-1);

	target_endian = (hdr->e_ident[EI_DATA] != ELFDATA2LSB);

	gdb_cputype = swap16(hdr->e_machine);

	phnum = swap16(hdr->e_phnum);
	if(phnum == 0) {
		fprintf(stderr, "No program table found\n");
		return(EXIT_FAILURE);
	}
	size = phnum * swap16(hdr->e_phentsize);
	phdrv = malloc(size);
	if(fseek(imagefp, swap32(hdr->e_phoff), SEEK_SET) != 0) {
		fprintf(stderr, "Unable to seek to program headers: %d\n", errno);
		return(EXIT_FAILURE);
	}
	if(fread(phdrv, 1, size, imagefp) != size) {
		fprintf(stderr, "Unable to read program headers: %d\n", errno);
		return(EXIT_FAILURE);
	}
	start_addr = swap32(hdr->e_entry);
	total = 0;
	for(i = 0; i < phnum; ++i) {
		phdr = &phdrv[i];
		if(swap32(phdr->p_type) == PT_LOAD) {
			total += swap32(phdr->p_memsz);
		}
	}
	percentage = -1;
	sofar = 0;
	seq = xfer->start(devicefd);
	for(i = 0; i < phnum; ++i) {
		phdr = &phdrv[i];
		if(swap32(phdr->p_type) == PT_LOAD) {
			unsigned long	memsz;
			unsigned long	filesz;
			unsigned long	done;

			if(fseek(imagefp, swap32(phdr->p_offset), SEEK_SET) != 0) {
				fprintf(stderr, "Unable to seek to program data: %d\n", errno);
				return(EXIT_FAILURE);
			}
			memsz = swap32(phdr->p_memsz);
			filesz = swap32(phdr->p_filesz);
			curr_addr = swap32(phdr->p_vaddr);
			done = 0;
			while(done < memsz) {
				unsigned	max;
				unsigned	nbytes;
		
				if(done >= filesz) {
					nbytes = memsz - done;
					if(nbytes > sizeof(data)) nbytes = sizeof(data);
					memset(data, 0, nbytes);
				} else {
					max = filesz - done;
					if(max > sizeof(data)) max = sizeof(data);
					nbytes = fread(data, 1, max, imagefp);
					if(nbytes == 0) {
						fprintf(stderr, "Can't read data: %d\n", errno);
						return(EXIT_FAILURE);
					}
				}
		
				seq = xfer->data(devicefd, seq, curr_addr, data, nbytes);
		
				curr_addr += nbytes;
				sofar += nbytes;
				done += nbytes;
		
				if(!quiet  &&  total  &&  (percentage != (sofar*100)/total)) {
					printf("Percent Complete: %2d%%\r", percentage = (sofar*100)/total);
					fflush(stdout);
				}
			}
		}
	}

	if(!quiet) {
		printf("\n");
	}

	xfer->done(devicefd, seq, start_addr);
	return(0);
}

static int
send_error(FILE *imagefp, int devicefd, void *h, int nbytes) {
	fprintf(stderr, "Unrecognized source file type\n");
	return 1;
}

static int (*send_func[])(FILE *, int, void *, int) = {
	send_image,
	send_elf,
	send_error // must be last entry
};

int 
main(int argc, char *argv[]) {
	int			opt;
	char		*devicename = NULL;
	int			baud;
	int			nbytes;
	FILE		*imagefp;
	int			devicefd;
	int			n;
	int			r;
	unsigned	i;
	char		*expect = NULL;
	int			timeout = -1;

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = (*(char *)&n != 1);
	// assume target is the same for the moment
	target_endian = host_endian;

	gdb_pc_regnum = -1;
	gdb_pc_regsize = 4;

	baud = -1;
	while ((opt = getopt(argc, argv, "b:d:ei:l:LvqE:p:P:w:")) != -1) {
		switch(opt) {
		case 'E':
			force_error = atoi(optarg);
			break;

		case 'b':
			baud = atoi(optarg);
			break;

		case 'd':
			devicename = optarg;
			outputdevice();
			break;

		case 'i':
			devicename = optarg;
			outputinet();
			break;

		case 'v':
			verbose++;
			break;

		case 'e':
			echo = 1;
			break;

		case 'l':
			laplink = atoi(optarg);
			break;

		case 'L':
			devicename = "loopback";
			outputpipe();
			break;

		case 'q':
			quiet = 1;
			break;

		case 'p':
			i = 0;
			for( ;; ) {
				if(i >= NUM_ELTS(xfers)) {
					fprintf(stderr, "unrecognized protocol '%s'\n", optarg);
					return(EXIT_FAILURE);
				}
				if(strcmp(optarg, xfers[i].name) == 0) break;
				++i;
			}
			xfer = &xfers[i];
			break;

		case 'P':
			{
				char	*p;

				gdb_pc_regnum = strtoul(optarg, &p, 0);
				if(*p == ',') {
					gdb_pc_regsize = strtoul(p + 1, NULL, 0);
				}
			}
			break;

		case 'w':	
			{
				char	*p;
				
				timeout = strtoul(optarg, &p, 0);
				if(timeout == 0) timeout = 10;
				if(*p == ':') ++p;
				if(*p != '\0') expect = p;
			}
			break;

		default:
			fprintf(stderr, "Unknown option.\n");
			break;
		}
	}

	if(devicename == NULL) {
		fprintf(stderr, "You must specify a device name using the -d option.\n");
		exit(EXIT_FAILURE);
	}

	output.init();

	// Open the device to send the image over.
	devicefd = output.open(devicename, baud);
	if(devicefd == -1) {
		fprintf(stderr, "%s : %s\n", optarg, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// If a file open it, otherwise assume piped input from stdin
	if(optind < argc) {
		if((imagefp = fopen(imagename = argv[optind], "rb")) == NULL) {
			fprintf(stderr, "%s : %s\n", argv[optind], strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		imagename = "-stdin-";
		imagefp = stdin;
		MAKE_BINARY_FP(imagefp);
	}

	//Discard any echo'd input
    output.flush(devicefd);

	nbytes = fread(data, 1, sizeof(data), imagefp);
	if(nbytes != sizeof(data)) {
		fprintf(stderr, "Short read for source file header %d\n", nbytes);
		return(EXIT_FAILURE);
	}

	i = 0;
	do {
		r = send_func[i++](imagefp, devicefd, data, nbytes);
	} while(r < 0);

	if(timeout > 0) {
		int		c;
		char	*p;
		time_t	start_time;

		start_time = time(0);
		p = expect;

		for( ;; ) {
			c = output.get(devicefd, 10);
			if(expect != NULL) {
				if(c == *p) {
					++p;
					if(*p == '\0') break;
				} else if(c != -1) {
					// didn't match the expected char - restart the
					// search from the begining
					p = expect;
				}
			}
			if((time(0) - start_time) > timeout) {
				// We've timed out
				if(expect != NULL) {
					// If we were expecting a response string, that's
					// a failure
					fprintf(stderr, "Timed out waiting for '%s'\n", expect);
					r = EXIT_FAILURE;
				}
				break;
			}
		}
	}

	output.close(devicefd);

	return(r);
}
