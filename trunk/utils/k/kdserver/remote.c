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
 *
 *    The following gdb commands are supported:
 * 
 * command          function                               Return value
 * 
 *    g             return the value of the CPU registers  hex data or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 * 
 *    k             kill
 *    D				detach
 *    H[c,g]tid     set the current thread                 OK
 *    ?             return the signal number               Kxx
 *    qC            return the current thread id           <hex data>
 *    qRcmd,kprintf
 *                  return the kprintf ring buffer         O<hex data>
 *    qRcmd,pid <pid>[-<tid>]
 *                  switch pgtbl,regset to given pid,tid   O<hex data>
 *    q[f,s]ThreadInfo
 *                  return list of thread id's             m<tid> or l
 *
 * All commands and responses are sent with a packet which includes a 
 * checksum.  A packet consists of 
 * 
 * $<packet info>#<checksum>.
 * 
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 * 
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 * 
 * Example:
 * 
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 * 
 ****************************************************************************/


#include <ctype.h>
#include "kdserver.h"


static char				*inbuf;
static char				*outbuf;
static char				*scratch;
static unsigned			buff_max;
static struct kdump		*kdp;
static paddr_t			current_prp;
static unsigned			current_tid = 1;

static void (*gdb_expand)(char *src, char *dest);
static unsigned (*gdb_compress)(char *src, char *dest);


static int
dbg_getc(void) {
	unsigned char ch;

	read(0, &ch, 1);
	return ch;
}


static void
dbg_write(const void *p, unsigned len) {
	write(1, p, len);
}


/*
 * chartohex
 * Convert char to hex nibble
 */
static int
chartohex(unsigned char ch) {
    if((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
    if((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
    if((ch >= '0') && (ch <= '9')) return (ch-'0');
    return 0;
}


static char
tohexchar(unsigned char c) {
	static unsigned char const hexchars[] = "0123456789abcdef";

    return hexchars[c & 0xf];
}


static unsigned
cisco_gdb_compress(char *src, char *dest) {
	char	*start = dest;
    char 	previous = 0;
    int 	repeat = 0;

    do {
		if((*src == previous) && (repeat != 255)) {
			repeat++;
		} else {
			if(repeat > 3) {
				dest = dest-repeat;
				*dest++ = '*';
				*dest++ = tohexchar(repeat >> 4);
				*dest++ = tohexchar(repeat);
			}
			repeat = 0;
		}
		*dest++ = *src;
		previous = *src;
    } while(*src++);
	return dest - start;
}

static void
cisco_gdb_expand(char *src, char *dest) {
    int i;
    int repeat;

    do {
		if(*src == '*') {
			repeat = (chartohex(src[1]) << 4) + chartohex(src[2]);
			for (i = 0; i < repeat; i++) {
				*dest++ = *(src-1);
			}
			src += 2;
		} else {
			*dest++ = *src;
		}
    } while(*src++);
}

static unsigned
generic_gdb_compress(char *src, char *dest) {
	char		*start = dest;
    char 		previous = 0;
    int 		repeat = 0;
	unsigned	min_repeat = 3;

    do {
		if((*src == previous) && (repeat < 97)) {
			repeat++;
		} else {
			if(repeat > min_repeat) {
				dest -= repeat;
				*dest++ = '*';
				*dest++ = repeat + 29;
				/* make sure four '|' characters never come out in a row */
				min_repeat = (repeat == ('|' - 29)) ? 2 : 3;
			}
			repeat = 0;
		}
		*dest++ = *src;
		previous = *src;
    } while(*src++);
	return dest - start;
}

static void
generic_gdb_expand(char *src, char *dest) {
    int i;
    int repeat;

    do {
		if(*src == '*') {
			repeat = src[1] - 29;
			for(i = 0; i < repeat; i++) {
				*dest++ = src[-1];
			}
			src += 2;
		} else {
			*dest++ = *src;
		}
    } while(*src++);
}

/*
 * gethexnum - parse a hex string returning a value
 *
 * This is a low-level routine for converting hex values into binary. It
 * is used by the 'parse' routines to actually perform the conversion.
 *
 * Entry : srcstr	- Pointer to string to convert
 *	   retstr	- Pointer to cell which will contain the address 
 *			  of the first byte not converted
 *			- Pointer to cell to return value
 */
static int
gethexnum(char *srcstr, char **retstr, unsigned *retvalue) {
    char *str = srcstr;
    unsigned long value = 0;

    /* Convert all of the digits until we get a non-hex digit */

    while(*str && (((*str >= 'a') && (*str <= 'f')) ||
		    ((*str >= '0') && (*str <= '9')))) {
		value = value*16 + (*str <= '9' ? (*str++ -'0') : 
					(*str++ -'a'+10));
    }
    
    /* Return failure if we are still pointing at the start */
    
    if(str == srcstr) return 0;
    
    /* Set up the return values and return success */
    
    *retvalue = value;
    *retstr = str;
    return 1;
}


/*
 * parse2hexnum - Parse two hex numbers
 *
 * This routine converts a string of two numbers, seperated by commas,
 * into two binary values. Note that if either of the values can not
 * be returned, this routine will return failure and not update either
 * return value.
 */
static int
parse2hexnum(char *srcstr, unsigned *retvalue1, unsigned *retvalue2) {
    char *str;
    unsigned value1, value2;
    
    if(!gethexnum(srcstr, &str, &value1) || (*str++ != ',') ||
	  !gethexnum(str, &str, &value2)) {
		return 0;
    }
    
    *retvalue1 = value1;
    *retvalue2 = value2;
    return 1;
}


/* 
 * scan for the sequence $<data>#<checksum>
 */
static int
getpacket() {
    unsigned char checksum;
    unsigned char xmitcsum;
    int	i;
    int	count;
    int	ch;
	int cs1;
	int cs2;
  
    for( ;; ) {
try_again:
		/* wait around for the start character, ignore all other characters */
		do {
			ch = dbg_getc();
			if(ch == -1) return 0;
		} while(ch != '$');

try_again2:		
		checksum = 0;
		count = 0;
		cs1 = cs2 = 0;
		
		/* now, read until a # or end of buffer is found */
		for( ;; ) {
			if(count >= buff_max) goto try_again;
			ch = dbg_getc();
			if(ch == -1) return 0;
			if(ch == '#') break;
			if(ch == '$') goto try_again2;
			checksum = checksum + ch;
			scratch[count++] = ch;
		}
		/* collect the checksum */
		cs1 = dbg_getc();
		if(cs1 == -1) return 0;
		cs2 = dbg_getc();
		if(cs2 == -1) return 0;

		scratch[count] = 0;
		gdb_expand(scratch, inbuf);
		
		xmitcsum = (chartohex(cs1) << 4) + chartohex(cs2);
		if(checksum == xmitcsum) break;
		if(debug_flag) {
			fprintf(stderr, "bad checksum.  My count = 0x%x, sent=0x%x. buf=%s\n",
			   checksum,xmitcsum,inbuf);
		}
		dbg_write("-", 1);  /* failed checksum */ 
    } 
	dbg_write("+", 1);  /* successful transfer */
	/* if a sequence char is present, reply the sequence ID */
	if(inbuf[2] == ':') {
		dbg_write(&inbuf[0], 2);
		/* remove sequence chars from buffer */
		i = 3;
		for( ;; ) {
			inbuf[i-3] = inbuf[i];
			if(inbuf[i] == '\0') break;
			++i;
		}
	} 
    return 1;
}


/* 
 * send the packet in buffer.  The host gets one chance to read it.  
 * This routine does not wait for a positive acknowledge.
 */
static void
putpacket(void) {
    unsigned char 	checksum;
	unsigned		len;
	unsigned		i;

    len = gdb_compress(outbuf, &scratch[1]);
  
    /*  $<packet info>#<checksum>. */

    checksum = 0;
  
	for(i = 1; i <= len; ++i) {
		checksum += scratch[i];
    }
	
	scratch[0] = '$';
	scratch[len + 1] = '#';
    scratch[len + 2] = tohexchar(checksum >> 4);
    scratch[len + 3] = tohexchar(checksum >> 0);
	dbg_write(scratch, len + 4);
}


/* 
 * convert the memory pointed to by mem into hex, placing result in buf
 * return a pointer to the last char put in buf (null)
 */
static char *
mem2hex(char *mem, char *buf, int count) {
    int i;
    unsigned char ch;

    for(i=0;i<count;i++) {
		ch = *mem++;
		*buf++ = tohexchar(ch >> 4);
		*buf++ = tohexchar(ch);
    }
    *buf = 0; 
    return buf;
}


/*
 * convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte written
 */

char *
hex2mem(char *buf, char *mem, int count) {
    int i;
    unsigned char ch;

    for(i=0;i<count;i++) {
		ch = chartohex(*buf++) << 4;
		ch = ch + chartohex(*buf++);
		*mem++ = ch;
    }
    return(mem);
}


/*
 * gdb_read_membytes:
 * Read bytes from our memory and return to gdb client
 */
static void
gdb_read_membytes(void) {
    unsigned 	vaddr;
	unsigned	length;
	unsigned	got;

    if(parse2hexnum(&inbuf[1],&vaddr,&length)) {
		got = core_read_vaddr(vaddr, scratch, length);
		mem2hex(scratch, outbuf, got);
		if(got == 0) {
			strcpy(outbuf,"E03");
			if(debug_flag) fprintf(stderr, "bus error");
		}
    } else {
		strcpy(outbuf,"E01");
		if(debug_flag) fprintf(stderr, "malformed read memory command: %s", inbuf);
    }     
}


static void
monitor_kprintf(void) {
	char		*p;
	unsigned	len;

	if(kdp == NULL) {
		// fill in kdp info 
		struct kdump	kdump;

		core_read_paddr(kdump_paddr, &kdump, sizeof(kdump));
		len = sizeof(kdump) + endian_native32(kdump.kp_size);
		kdp = malloc(len);
		if(kdp == NULL) {
			if(debug_flag > 0) {
				fprintf(stderr, "no memory for kdump info\n");
			}
			return;
		}
		// We allocated one byte more than we really want
		// so we can ensure there's always a trailing '\0'
		core_read_paddr(kdump_paddr, kdp, len - 1);
		kdp->kp_buff[len-1] = '\0';
	}
	p = &kdp->kp_buff[0];
	while(*p != '\0') {
		len = strlen(p);
		if(len > 100) len = 100;
		outbuf[0] = 'O';
		mem2hex(p, &outbuf[1], len);
		putpacket();
		p += len;
	}
	strcpy(outbuf, "OK");
}


static void
monitor_pid(char *p) {
	unsigned	pid;
	unsigned	tid;
	paddr64_t	pid_paddr;
	paddr64_t	tid_paddr;
	unsigned	len;

	while(isspace(*p)) ++p;
	pid = strtoul(p, &p, 0);
	if(pid == 0) {
		set_default_dump_state();
		current_prp = 0;
		current_tid = 1;
		#define ORIG_MSG	"Returned to original dump state\n"
		mem2hex(ORIG_MSG, outbuf, sizeof(ORIG_MSG)-1);
		return;
	}
	while(isspace(*p)) ++p;
	if(*p == '-') {
		++p;
		while(isspace(*p)) ++p;
		tid = strtoul(p, &p, 0);
	} else {
		tid = 0;
	}
	pid_paddr = find_pid(pid);
	if(pid_paddr == 0) {
		#define PID_MSG	"No such process\n"
		mem2hex(PID_MSG, outbuf, sizeof(PID_MSG)-1);
		return;
	}
	tid_paddr = find_tid(pid_paddr, &tid);
	if(tid_paddr == 0) {
		#define TID_MSG	"No such thread\n"
		mem2hex(TID_MSG, outbuf, sizeof(TID_MSG)-1);
		return;
	}
	current_prp = pid_paddr;
	current_tid = tid;
	set_pgtbl(pid_paddr);
	set_regset(tid_paddr);
	len = sprintf(scratch, "switched to pid %d, thread %d\n", pid, tid);
	mem2hex(scratch, outbuf, len);
}


/*
 * This function does all command procesing for interfacing to gdb.
 */
void
server() {
	char		*p;
	unsigned	tid;
	paddr64_t	tid_paddr;


	if(protocol == 0) {
		/* generic GDB protocol compression */
		gdb_expand = generic_gdb_expand;
		gdb_compress = generic_gdb_compress;
	} else {
		/* cisco GDB protocol compression */
		gdb_expand = cisco_gdb_expand;
		gdb_compress = cisco_gdb_compress;
	}

	/*
	 * Define the size of the buffers used for communications with the remote
	 * GDB. This value must match the value used by GDB, or the protocol will
	 * break.
	 */
	buff_max = max(400, cpu->gdb_regset_size*2 + 32);

	inbuf = malloc(buff_max);
	outbuf = malloc(buff_max);
	scratch = malloc(buff_max + 10);
	if((inbuf == NULL) || (outbuf == NULL) || (scratch == NULL)) {
		fprintf(stderr, "No memory for buffers\n");
		return;
	}

	while(getpacket()) {
		outbuf[0] = 0;

        switch(inbuf[0]) {

		case 'c': // continue
		case 'C': // continue with signal
		case 's': // step
		case 'S': // step with signal
		case '?': // Tell GDB our signal number
			sprintf(outbuf, "T%02xthread:%X;", note->sig_num, current_tid);
			break;
		
		case 'g' : // return the value of the CPU registers
			cpu->cvt_regset(regset, scratch);
			mem2hex(scratch, outbuf, cpu->gdb_regset_size);
			break;
		  
		case 'm' : // mAA..AA,LLLL  Read LLLL bytes at address AA..AA
			gdb_read_membytes();
			break;

		case 'k' :	// kill program
		case 'D' :	// detach program
			putpacket(); /*ACK the packet early (since we're going bye-bye) */
			return;

		case 'H': // set active thread
			if((inbuf[1] == 'g') && (inbuf[1] != '-')) {
				gethexnum(&inbuf[2], &p, &tid);
				if(current_prp == 0) {
					if((tid != 0) && (tid != 1)) {
						// bad tid
						strcpy(outbuf, "E03");
						break;
					}
				} else {
					tid_paddr = find_tid(current_prp, &tid);
					if(tid_paddr == 0) {
						// bad tid
						strcpy(outbuf, "E03");
						break;
					}
					set_regset(tid_paddr);
					current_tid = tid;
				}
			}
			strcpy(outbuf, "OK");
			break;

		case 'T': // test if thread is alive
			gethexnum(&inbuf[1], &p, &tid);
			if(current_prp == 0) {
				if((tid != 0) && (tid != 1)) {
					// bad tid
					strcpy(outbuf, "E03");
					break;
				}
			} else {
				unsigned check_tid = tid;

				tid_paddr = find_tid(current_prp, &check_tid);
				if((tid_paddr == 0) || (check_tid != tid)) {
					// bad tid
					strcpy(outbuf, "E03");
					break;
				}
			}
			strcpy(outbuf, "OK");
			break;

		case 'q': // generic query
			if(memcmp(&inbuf[1], "C", 2) == 0) {
				// return current thread ID
				sprintf(outbuf, "%X", current_tid);

			} else if(memcmp(&inbuf[2], "ThreadInfo", 11) == 0) {
				// return thread list
				static unsigned	next_tid;
				paddr64_t		tid_paddr;
				unsigned		tid;

				if(inbuf[1] == 'f') {
					// first time, reset the index
					next_tid = 1;
				}
				tid = next_tid;
				if(current_prp != 0) {
					tid_paddr = find_tid(current_prp, &tid);
				} else if(tid <= 1) {
					tid_paddr = 1;
				} else {
					tid_paddr = 0;
				}
				if(tid_paddr == 0) {
					strcpy(outbuf, "l");
				} else {
					sprintf(outbuf, "m%X", tid);
					next_tid = tid + 1;
				}

			} else if(memcmp(&inbuf[1], "Rcmd,", 5) == 0) {
				// remote command
				p = &inbuf[6];
				hex2mem(p, scratch, strlen(p));
				if(strcmp(scratch, "kprintf") == 0) {
					// We're going to use the "monitor kprintf" command
					// to return the information from the kprintf ring buffer
					// log.
					monitor_kprintf();
				} else if(memcmp(scratch, "pid ", 4) == 0) {
					// The "monitor pid <pid>[-<tid>]" command is used to switch
					// the the page table & register set to the given <pid>
					monitor_pid(&scratch[4]);
				}
			}
			break;

		} /* switch */ 
		
		/* reply to the request */
		putpacket();
    }
}
