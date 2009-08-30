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





#include "sendnto.h"


#define START_RECORD	0x80	// Sent by host
#define DATA_RECORD		0x81	// Sent by host
#define GO_RECORD		0x82	// Sent by host
#define ECHO_RECORD		0x83	// Sent by host
#define ABORT_RECORD	0x88	// Sent by target (or host)

#define ABORT_CHKSUM	1
#define ABORT_SEQUENCE	2
#define ABORT_PROTOCOL	3

struct start_record {
	uint8_t	cmd;
};

struct data_record {
	uint8_t		cmd;
	uint8_t		seq;
	uint8_t		cksum;
	uint8_t		nbytes;
	uint32_t	addr;
	uint8_t		data[256];	// Actual length given by 'nbytes' member
};

struct go_record {
	uint8_t		cmd;
	uint8_t		seq;
	uint8_t		cksum;
	uint8_t		spare;
	uint32_t	addr;
};

struct echo_record {
	uint8_t	cmd;
};

struct abort_record {
	uint8_t	cmd;
	uint8_t	reason;
};


//
// Calculate the checksum.
// The sum of the buffer including the checksum should be zero.
//
static int
cksum(const uint8_t *buf, int len) {
	uint8_t sum = 0;

	while(len--) {
		sum += *buf++;
	}

	return(-sum);
}

static char *reasons[] = {
	"Checksum error",
	"Sequence error",
	"Protocol error",
};

static int
aborted(int fd, int code) {

	if(code == 0) code = output.get(fd, 5);
	fprintf(stderr, "Download aborted by target. Code %d", code);

	if( (code >= 1) && (code <= (NUM_ELTS(reasons)+1))) {
		fprintf(stderr, " (%s)", reasons[code-1]);
	}
	fprintf(stderr, "\n");

	return(EXIT_FAILURE);
}

int
xfer_start_nto(int devicefd) {
	struct start_record		startrec;

	// Write the start record
	startrec.cmd = START_RECORD;
	output.write(devicefd, &startrec, sizeof(startrec));
	return(0);
}

int
xfer_data_nto(int devicefd, int seq, unsigned long addr, const void *data, int nbytes) {
	struct data_record		datarec;
	struct echo_record		echorec;
	int						n;

	if(nbytes > sizeof(datarec.data)) {
		//
		// Don't let a packet go over buffer size
		//
		unsigned 	half = nbytes >> 1;

		seq = xfer_data_nto(devicefd, seq, addr, data, half);
		seq = xfer_data_nto(devicefd, seq, addr + half, (uint8_t *)data + half, nbytes - half);
		return(seq);
	}

	datarec.cmd = DATA_RECORD;
	datarec.addr = swap32(addr);
	datarec.seq = seq;

	// Adjust nbytes so (00 means 1) and (ff means 256).
	datarec.nbytes = nbytes - 1;
	memcpy(datarec.data, data, nbytes);

	// Calculate checksum over entire record.
	n = nbytes + (sizeof(datarec) - 256);
	datarec.cksum = 0;
	datarec.cksum = cksum(data, n);
	if(force_error == 1) {
		++datarec.cksum;
	}

	// Write the data record
	output.write(devicefd, &datarec, n);

	// Setup for the next record
	seq = (seq + 1) & 0x7f;
	if(force_error == 2) {
		++seq;
	}

	if(echo && !laplink) {
		int c;

		echorec.cmd = ECHO_RECORD;
		output.write(devicefd, &echorec, sizeof(echorec));

		// Wait for echo or .1 sec. We swallow all extra chars.
		if((c = output.get(devicefd, 20)) == -1) {
			fprintf(stderr, "Download aborted: Target not responding\n");
			exit(EXIT_FAILURE);
		} else {
			if(c == ABORT_RECORD) {
				exit(aborted(devicefd, 0));
			}
		}
	}

	// Every so often we look for an abort
	if((seq & (output.check_rate-1)) == 0) {
		int ch = output.get(devicefd, -1);
		switch(ch) {
		case ABORT_RECORD:	
		case -2:	
			exit(aborted(devicefd, 0));
			break;
		}
	}

	return(seq);
}

int
xfer_done_nto(int devicefd, int seq, unsigned long start_addr) {
	struct go_record		gorec;

	// Calculate the checksum and write the go record
	gorec.cmd = GO_RECORD;
	gorec.seq = seq;
	gorec.addr = swap32(start_addr);
	gorec.spare = 0;
	gorec.cksum = 0;
	gorec.cksum = cksum((uint8_t *)&gorec, sizeof(gorec));
	output.write(devicefd, &gorec, sizeof(gorec));
	return(0);
}
