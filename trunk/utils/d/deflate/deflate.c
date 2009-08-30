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
%C	[options]... [filename]...

Options:
 -b size    Compression block size may be 4K, 8K, 16K or 32K (default: 8K).
 -o fname   Output file name. A fname of - means stdout (default: inplace).
 -i         Inflate files (default: deflate).
 -d         Decompress (same as -i so it can be used as a filter by tar).
 -t 1|2     Compression type (default: 2).
              Compression Speed  Decompression Speed  Compression Amount
            1      fast            very fast          30% - 40% on executables
            2      slow                 fast          40% - 65% on executables
 -v         Verbose
filename    Files to compress. If no files are given or file is '-' then
            deflate will read from standard input and write to standard output
            allowing it to be used as a filter.
#endif

#include <lib/compat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif
#include _NTO_HDR_(gulliver.h)
#include <minilzo.h>
#include <ucl/ucl.h>


#define SIGNATURE	"iwlyfmbp"
#define CMP_LZO		0
#define CMP_UCL		1

#define OK				0
#define ERR_BIT			1
#define REMOVE_BIT		2

// This structure is always stored in litte endian on the media.
struct filehdr {
	char			signature[8];
	int				usize;		// Uncompressed size of the file
	short unsigned	blksize;	// Size of compression blocks
	char			cmptype;	// Type of compression (CMP_LZO, ...)
	char			flags;
};

// This structure is always stored in litte endian on the media.
struct cmphdr {
	short unsigned	prev;			// Offset to previous hdr
	short unsigned	next;			// Offset to next hdr
	short unsigned	pusize;			// Size of prev uncompressed blk
	short unsigned	usize;			// Size of this uncompressed blk
};

struct cmpblk {
	struct cmphdr	hdr;
	uint8_t			buf[32*1024];
} cmpblk;

uint8_t workbuf[32*1024];
uint8_t wrkmem[LZO1X_MEM_COMPRESS];
uint8_t pathbuf[PATH_MAX];
int verbose;

int process_file(const char *infile, const char *outfile, int inflate, int type, int blksize);

int compress  (const char *file, int infd, int outfd, int type, int blksize, const void *buf, int buf_size);
int decompress(                  int infd, int outfd, int type);
int copy      (                  int infd, int outfd,                        const void *buf, int buf_size);

#ifndef __MINGW32__
extern char	* __progname;
#else
static char *__progname;
#endif


int main(int argc, char *argv[])
{
	struct stat	st;
	char *		outfile;
	int			blksize;
	int			inflate;
	int			type;
	int			opt;
	int			error;
	
#ifdef __MINGW32__
	__progname = argv[0];
#endif

	// Setup defaults
	outfile    = NULL;
	blksize    = 8*1024;
	inflate    = 0;
	type       = CMP_UCL;

	while ((opt = getopt(argc, argv, "db:io:p:t:v")) != -1) {
		switch (opt) {
			case 'o':
				outfile = optarg;
				break;

			case 'b':
				switch (*optarg) {
					case '4':
						blksize =  4*1024;
						break;
					case '8':
						blksize =  8*1024;
						break;
					case '1':
						blksize = 16*1024;
						break;
					case '3':
						blksize = 32*1024;
						break;
					default:
						fprintf(stderr, "Invalid size %s\n", optarg);
						exit(EXIT_FAILURE);
					}
				break;

			case 'i':
			case 'd':
				inflate = 1;
				break;

			case 'p':
				break;

			case 't':
				switch (*optarg) {
					case '1':
						type = CMP_LZO;
						break;
					case '2':
						type = CMP_UCL;
						break;
					default:
						fprintf(stderr, "Invalid compression type %s\n", optarg);
						exit(EXIT_FAILURE);
				}
				break;

			case 'v':
				verbose = 1;
				break;

			default:
				exit(EXIT_FAILURE);
		}
	}

#if defined(__NT__) || defined (__MINGW32__)
	/* Disable automatic ASCII conversion */
	setmode(STDIN_FILENO,  O_BINARY);
	setmode(STDOUT_FILENO, O_BINARY);
#endif

	/* If no input files are given */
	if (optind == argc) {
		error = process_file(NULL, outfile, inflate, type, blksize);

	/* If handling a single file */
	} else if (optind + 1 == argc) {
		if (outfile == NULL) {
			error = process_file(argv[optind], argv[optind], inflate, type, blksize);

		} else if ((outfile[0] == '-') && (outfile[1] == '\0')) {
			error = process_file(argv[optind], NULL, inflate, type, blksize);

		} else {
			error = process_file(argv[optind], outfile, inflate, type, blksize);
		}

	/* Otherwise, handling multiple files */
	} else {
		/* If an outfile was given it better be a directory */
		if (outfile != NULL) {
			if ((outfile[0] == '-') && (outfile[1] == '\0')) {
				fprintf(stderr, "Cannot output multiple files to stdout\n");
				error = EINVAL;
				goto error;
			}

			if (stat(outfile, &st)) {
				fprintf(stderr, "Cannot access output directory %s\n", outfile);
				error = errno;
				goto error;
			}

			if (!S_ISDIR(st.st_mode)) {
				fprintf(stderr, "Cannot process multiple input files into a single output file\n");
				error = ENOTDIR;
				goto error;
			}

			/* Compress each file */
			for( ; optind < argc ; optind++) {
				/* Ouput file to given directory */
				strcpy(pathbuf, outfile);
				strcat(pathbuf, "/");
				strcat(pathbuf, argv[optind]);

				error = process_file(argv[optind], pathbuf, inflate, type, blksize);
				if (error) fprintf(stderr, "Error processing file %s: %s\n",
				                   argv[optind], strerror(error));
			}
			error = EOK;

		/* Just do everything in place */
		} else {
			for( ; optind < argc ; optind++) {
				error = process_file(argv[optind], argv[optind], inflate, type, blksize);
				if (error) fprintf(stderr, "Error processing file %s: %s\n",
				                   argv[optind], strerror(error));
			}
			error = EOK;
		}
	}
	if (error) {
		fprintf(stderr, "Error processing file %s: %s\n",
		        argv[optind], strerror(error));
		error = EOK;
		goto error;
	}

	return (EXIT_SUCCESS);
error:
	if (error) fprintf(stderr, "Error: %s\n", strerror(error));
	return (EXIT_FAILURE);
}

int process_file(const char *infile, const char *outfile, int inflate, int type, int blksize)
{
	struct stat		st;
	struct filehdr	hdr;
	const char *	file     = NULL;
	char *			tmpfile  = NULL;
	int				infd     = -1;
	int				outfd    = -1;
	int				st_valid;
	int				raw      =  1;
	int				xfer;
	int				error;

	/* Prepare the input file */
	if (infile == NULL) {
		infd = STDIN_FILENO;
		file = "--stdin--";

	} else {
 		/* Only compress regular files */
		error = lstat(infile, &st);
		if ( error == -1 ) {
			error = errno;
			goto done; 
		}
		if (!S_ISREG(st.st_mode)) {
			/* return without error to bypass compression of symbolic links */
			if (verbose) fprintf(stderr, "%s: Bypassing compresson of non-regular file: %s\n", __progname, infile);  	
			error = EOK;
			goto done;			
		} 
		
		infd = open(infile, O_RDONLY | O_BINARY);
		if (infd == -1) return (errno);
		file = infile;
	} 
	/* Don't compress the file twice */
	xfer = read(infd, &hdr, sizeof(hdr));
	if (xfer < 0) {error = errno; goto done;}

	if (xfer == sizeof(hdr)) {
		/* First check for the signature */
		if (!memcmp (hdr.signature, SIGNATURE, sizeof(hdr.signature))) {
			/* Validate the block size */
			blksize = ENDIAN_LE16(hdr.blksize);
			if ((blksize == (blksize & ((blksize - 1) ^ blksize))) &&
			    (blksize >= 4096) && (blksize <= 32768))
			{
				raw = 0;
			}
		}
	}
 

	/* 
	 * Always use a temp file when compressing to judge percentage or when an
	 * outfile is given.
	 */
	if ((outfile != NULL) || (!inflate)) {
		/* Output file already exists, use a temporary file instead */
		tmpfile = tmpnam(NULL);
		if (tmpfile == NULL) {error = ENOMEM; goto done;}

		outfd = open(tmpfile,
		             O_RDWR | O_CREAT | O_TRUNC | O_BINARY | O_EXCL,
		             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (outfd == -1) {error = errno; goto done;}

	/* Otherwise, we're decompressing to stdout */
	} else {
		tmpfile = NULL;
		outfd = STDOUT_FILENO;
	}

	/* Do decompression */
	if (inflate) {
		if (raw) {
			if (verbose) fprintf(stderr, "%s: Unrecognized header, copying instead\n", __progname);
			error = copy(infd, outfd, &hdr, xfer);

		} else {
			error = decompress(infd, outfd, type);
		}

	} else {
		if (raw) {
			error = compress(file, infd, outfd, type, blksize, &hdr, xfer);
		} else {
			if (verbose) fprintf(stderr, "%s: File already compressed, copying instead\n", __progname); 
			error = copy(infd, outfd, &hdr, xfer);
		}
		if (error) goto done;
	}
	if (error) goto done;

	/* If we used a temp file */
	if (tmpfile) {
		/* If we're targeting a file, replace it */
		if (outfile) {
			/* Store permissions */
			if (fstat (infd, &st)) {
				st.st_mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
				st_valid    = 0;
			} else {
				st_valid    = 1;
			}

			/* Close the output file (so Windows doesn't complain) */
			if (outfd != -1) close(outfd);
			if (infd  != -1) close(infd);

			/* Remove the original file */
			unlink(outfile);

			/* Replace it with the temp file */
			if (rename(tmpfile, outfile)) {
				infd = open(tmpfile, O_RDONLY | O_BINARY);
				outfd = open(outfile, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, st.st_mode);
				copy(infd, outfd, NULL, 0);

			} else {
				tmpfile = NULL;
				if (st_valid) chmod(outfile, st.st_mode);
			}

			if (st_valid) chown(outfile, st.st_uid, st.st_gid);

		/* Otherwise, dump to stdout */
		} else {
			lseek(outfd, 0, SEEK_SET);
			copy(outfd, STDOUT_FILENO, NULL, 0);
		}
	}

	error = EOK;
done:
	if ((outfd != -1) && (outfd != STDOUT_FILENO)) close(outfd);
	if ((infd  != -1) && (infd  != STDIN_FILENO))  close(infd);
	if (tmpfile)                                   unlink(tmpfile);

	return (error);
}

int compress(const char *file, int infd, int outfd, int type, int blksize, const void *buf, int buf_size)
{
	struct filehdr	hdr;
	uint32_t		intotal  = 0;
	uint32_t		outtotal;
	uint16_t		prev     = 0;
	uint16_t		pusize   = 0;
	int				temp;
	int				xfer;
	int				error;

	/* Seek past the header */
	lseek(outfd, sizeof(hdr), SEEK_SET);
	outtotal = sizeof(hdr);

	/* Do the first iteration with the stuff we've already read */
	cmpblk.hdr.prev   = 0;
	cmpblk.hdr.pusize = 0;

	memcpy(workbuf, buf, buf_size);
	xfer = read(infd, workbuf + buf_size, blksize - buf_size);
	if (xfer < 0) return (errno);

	pusize  = xfer + buf_size;
	intotal = pusize;

	if (type == CMP_LZO) {
		error = lzo1x_1_compress(workbuf, pusize, cmpblk.buf, (lzo_uint *)&temp, wrkmem);
		if (error) return (EIO);

	} else if (type == CMP_UCL) {
		error = ucl_nrv2b_99_compress(workbuf, pusize, cmpblk.buf, (ucl_uintp)&temp, NULL, 9, NULL, NULL);
		if (error) return (EIO);

	} else {
		return (EINVAL);
	}

	prev             = sizeof(cmpblk.hdr) + temp;
	cmpblk.hdr.next  = ENDIAN_LE16(prev);
	cmpblk.hdr.usize = ENDIAN_LE16(pusize);

	xfer = write(outfd, &cmpblk, prev);
	if      (xfer < 0)     return (errno);
	else if (xfer != prev) return (EIO);

	outtotal += prev;

	/* Do the rest of the file */
	while (1) {
		cmpblk.hdr.prev   = ENDIAN_LE16(prev);
		cmpblk.hdr.pusize = ENDIAN_LE16(pusize);

		xfer = read(infd, workbuf, blksize);
		if (xfer < 0) return (errno);
		if (xfer == 0) break;

		pusize   = xfer;
		intotal += xfer;

		if (type == CMP_LZO) {
			error = lzo1x_1_compress(workbuf, pusize, cmpblk.buf, (lzo_uint *)&temp, wrkmem);
			if (error) return (EIO);

		} else if (type == CMP_UCL) {
			error = ucl_nrv2b_99_compress(workbuf, pusize, cmpblk.buf, (ucl_uintp)&temp, NULL, 9, NULL, NULL);
			if (error) return (EIO);

		} else {
			return (EINVAL);
		}

		prev             = sizeof(cmpblk.hdr) + temp;
		cmpblk.hdr.next  = ENDIAN_LE16(prev);
		cmpblk.hdr.usize = ENDIAN_LE16(pusize);

		xfer = write(outfd, &cmpblk, prev);
		if      (xfer < 0)     return (errno);
		else if (xfer != prev) return (EIO);

		outtotal += prev;
	}

	/* Write out the end-of-file marker */
	cmpblk.hdr.prev   = ENDIAN_LE16(prev);
	cmpblk.hdr.next   = 0;
	cmpblk.hdr.pusize = ENDIAN_LE16(pusize);
	cmpblk.hdr.usize  = 0;

	xfer = write(outfd, &cmpblk, sizeof(cmpblk.hdr));
	if      (xfer < 0)                   return (errno);
	else if (xfer != sizeof(cmpblk.hdr)) return (EIO);

	outtotal += sizeof(cmpblk.hdr);

	/* Write the header */
	memcpy(hdr.signature, SIGNATURE, sizeof(hdr.signature));
	hdr.usize   = ENDIAN_LE32(intotal);
	hdr.blksize = ENDIAN_LE16(blksize);
	hdr.cmptype = type;
	hdr.flags   = 0;

	lseek(outfd, 0, SEEK_SET);
	write(outfd, &hdr, sizeof(hdr));

	if (verbose) {
		if(intotal == 0) intotal = 1; // avoid divide by zero below
		fprintf(stderr, "Compressed %s %d%%\n",
	                     file, 100 - ((100 * outtotal) / intotal));
	}

	return (EOK);
}

int decompress(int infd, int outfd, int type)
{
	int		blksize;
	int		xfer;
	int		temp;
	int		error;

	while (1) {
		xfer = read(infd, &cmpblk.hdr, sizeof(cmpblk.hdr));
		if      (xfer < 0)  return (errno);
		else if (xfer == 0) break;

		if (cmpblk.hdr.next == 0) break;

		blksize = ENDIAN_LE16(cmpblk.hdr.next) - sizeof(cmpblk.hdr);
		xfer = read(infd, cmpblk.buf, blksize);
		if      (xfer < 0)        return (errno);
		else if (xfer != blksize) return (EIO);

		if (type == CMP_LZO) {
			error = lzo1x_decompress(cmpblk.buf, blksize, workbuf, (lzo_uint *)&temp, NULL);
			if (error) return (EIO);

		} else if (type == CMP_UCL) {
			error = ucl_nrv2b_decompress_8(cmpblk.buf, blksize, workbuf, (ucl_uintp)&temp, NULL);
			if (error) return (EIO);

		} else {
			return (EINVAL);
		}

		xfer = write(outfd, workbuf, temp);
		if      (xfer < 0)     return (errno);
		else if (xfer != temp) return (EIO);
	}

	return (EOK);
}

int copy(int infd, int outfd, const void *buf, int buf_size)
{
	int xfer1;
	int xfer2;

	xfer1 = write(outfd, buf, buf_size);
	if      (xfer1 < 0)         return (errno);
	else if (xfer1 != buf_size) return (EIO);

	while (1) {
		xfer1 = read(infd, workbuf, sizeof(workbuf));
		if      (xfer1 < 0)  return (errno);
		else if (xfer1 == 0) break;

		xfer2 = write(outfd, workbuf, xfer1);
		if      (xfer2 < 0)      return (errno);
		else if (xfer2 != xfer1) return (EIO);
	}

	return (EOK);
}
