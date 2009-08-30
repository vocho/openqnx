/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

//#define DEBUG_GDB
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/elf.h>
#include <sys/syspage.h>
#include <sys/types.h>

#include "kdebug.h"

/*
 * Define the size of the buffers used for communications with the remote
 * GDB. This value must match the value used by GDB, or the protocol will
 * break.
 */
#define BUFMAX max(400, sizeof(gdb_register_context)*2 + 32)

boolean gdb_debug = FALSE;
boolean	connected = FALSE;
char	inbuf[BUFMAX];
char	outbuf[BUFMAX];
char	scratch[BUFMAX];

struct mapping_range {
	uintptr_t		start;
	uintptr_t		end;
};

static struct mapping_range direct  = { ~(uintptr_t)0, 0};
static struct mapping_range nocache = { ~(uintptr_t)0, 0};
static struct mapping_range raw     = { ~(uintptr_t)0, 0};
static struct mapping_range inout   = { ~(uintptr_t)0, 0};

#define MAP_IN_RANGE(r, p)	((((uintptr_t)(p) >= (r).start)) && ((uintptr_t)(p) < (r).end))
#define MAP_OFFSET(r, p)	((uintptr_t)(p) - (r).start)

int chartohex(unsigned char ch);
char tohexchar(unsigned char c);

void (*gdb_expand)(char *src, char *dest);
void (*gdb_compress)(char *src, char *dest);

#ifdef LONG_SIGNAMES
#define EXTRA_INFO(x)		x
#else
#define EXTRA_INFO(x)
#endif

const char * const sig_to_name[] = {
  "NULL Signal",
  EXTRA_INFO("SIG") "HUP" EXTRA_INFO(", Hangup"),
  EXTRA_INFO("SIG") "INT" EXTRA_INFO(", Interrupt"),
  EXTRA_INFO("SIG") "QUIT" EXTRA_INFO(", Quit"),
  EXTRA_INFO("SIG") "ILL" EXTRA_INFO(", Illegal instruction"),
  EXTRA_INFO("SIG") "TRAP" EXTRA_INFO(", Trace/breakpoint trap"),
  EXTRA_INFO("SIG") "ABRT" EXTRA_INFO(", Aborted"),
  EXTRA_INFO("SIG") "EMT" EXTRA_INFO(", Emulation trap"),
  EXTRA_INFO("SIG") "FPE" EXTRA_INFO(", Floating point exception"),
  EXTRA_INFO("SIG") "KILL" EXTRA_INFO(", Killed"),
  EXTRA_INFO("SIG") "BUS" EXTRA_INFO(", Bus error"),
  EXTRA_INFO("SIG") "SEGV" EXTRA_INFO(", Segmentation violation"),
  EXTRA_INFO("SIG") "SYS" EXTRA_INFO(", Bad argument to system call"),
  EXTRA_INFO("SIG") "PIPE" EXTRA_INFO(", Broken pipe"),
  EXTRA_INFO("SIG") "ALRM" EXTRA_INFO(", Alarm clock"),
  EXTRA_INFO("SIG") "TERM" EXTRA_INFO(", Terminated"),
  EXTRA_INFO("SIG") "USR1" EXTRA_INFO(", User defined signal 1"),
  EXTRA_INFO("SIG") "USR2" EXTRA_INFO(", User defined signal 2"),
  EXTRA_INFO("SIG") "CHLD" EXTRA_INFO(", Death of child"),
  EXTRA_INFO("SIG") "PWR" EXTRA_INFO(", Power-fail restart"),
  EXTRA_INFO("SIG") "WINCH" EXTRA_INFO(", Window change"),
  EXTRA_INFO("SIG") "UGR" EXTRA_INFO(", urgent condition on I/O channel"),
  EXTRA_INFO("SIG") "IO" EXTRA_INFO(", Asynchrounus I/O"),
  EXTRA_INFO("SIG") "STOP" EXTRA_INFO(", Program sent stopped signal"),
  EXTRA_INFO("SIG") "TSTP" EXTRA_INFO(", User sent stopped signal"),
  EXTRA_INFO("SIG") "CONT" EXTRA_INFO(", Continue a stopped process"),
  EXTRA_INFO("SIG") "TTIN" EXTRA_INFO(", Attempted background tty read"),
  EXTRA_INFO("SIG") "TTOU" EXTRA_INFO(", Attempted background tty write"),
  EXTRA_INFO("SIG") "DEV" EXTRA_INFO(", Dev event")
};
	

/*
 * gdb_putstr - low level string output routine
 */ 
void
gdb_putstr(const char *text, int len) {
	int i;

	if(len == 0) len = strlen(text);

	for(i = 0; i < len; i++) dbg_putc(text[i]);
}

/*
 * gdb_printf - gdb debugging information printout routine.
 * 
 * It prints out message to the same port as gdb
 * target-host communication port.
 */ 
int
gdb_printf(const char *fmt, ... ) {
	char				buffer[100];
	va_list				ap;
	unsigned			len;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, sizeof buffer, fmt, ap);
	gdb_putstr(buffer, len);
	va_end(ap);
	return(len);
}
	

/*
 *
 *    The following gdb commands are supported:
 * 
 * command          function                               Return value
 * 
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    i				get target information                 hex data or ENN
 * 
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 * 
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 * 
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 * 
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
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

boolean
gethexnum(char *srcstr, char **retstr, int *retvalue) {
    char *str = srcstr;
    ulong_t value = 0;

    /* Convert all of the digits until we get a non-hex digit */

    while(*str && (((*str >= 'a') && (*str <= 'f')) ||
		    ((*str >= '0') && (*str <= '9')))) {
		value = value*16 + (*str <= '9' ? (*str++ -'0') : 
					(*str++ -'a'+10));
    }
    
    /* Return failure if we are still pointing at the start */
    
    if(str == srcstr) return(FALSE);
    
    /* Set up the return values and return success */
    
    *retvalue = value;
    *retstr = str;
    return(TRUE);
}

/*
 * parsehexnum - Parse a single hex number
 *
 * This routine is used to convert a hex value into binary. It uses gethexnum
 * to perform the actual conversion, and ignores any invalid characters at
 * the end of the string.
 */

boolean
parsehexnum(char *srcstr, int *retvalue) {
    char *dummy;
    
    return(gethexnum(srcstr, &dummy, retvalue));
}

/*
 * parse2hexnum - Parse two hex numbers
 *
 * This routine converts a string of two numbers, seperated by commas,
 * into two binary values. Note that if either of the values can not
 * be returned, this routine will return failure and not update either
 * return value.
 */
boolean
parse2hexnum(char *srcstr, int *retvalue1, int *retvalue2) {
    char *str;
    int value1, value2;
    
    if(!gethexnum(srcstr, &str, &value1) || (*str++ != ',') ||
	  !gethexnum(str, &str, &value2)) {
		return(FALSE);
    }
    
    *retvalue1 = value1;
    *retvalue2 = value2;
    return(TRUE);
}

/* 
 * scan for the sequence $<data>#<checksum>
 */
boolean
getpacket() {
    unsigned char checksum;
    unsigned char xmitcsum;
    int  i;
    int  count;
    int ch;
	int cs1;
	int cs2;
	int	(*init_getc)(void);
  
 	init_getc = connected ? dbg_getc : dbg_getc_connect_check;
    for( ;; ) {
try_again:
		/* wait around for the start character, ignore all other characters */
		do {
			ch = init_getc();
			if(ch == -1) return(FALSE);
		} while(ch != '$');

try_again2:		
		checksum = 0;
		count = 0;
		cs1 = cs2 = 0;
		
		/* now, read until a # or end of buffer is found */
		for( ;; ) {
			if(count >= BUFMAX) goto try_again;
			ch = dbg_getc();
			if(ch == -1) return(FALSE);
			if(ch == '#') break;
			if(ch == '$') goto try_again2;
			checksum = checksum + ch;
			scratch[count++] = ch;
		}
		/* collect the checksum */
		cs1 = dbg_getc();
		if(cs1 == -1) return(FALSE);
		cs2 = dbg_getc();
		if(cs2 == -1) return(FALSE);

		scratch[count] = 0;
		gdb_expand(scratch, inbuf);
		
		xmitcsum = (chartohex(cs1) << 4) + chartohex(cs2);
		if(checksum == xmitcsum) break;
		if(gdb_debug) {
			gdb_printf("bad checksum.  My count = 0x%x, sent=0x%x. buf=%s\n",
			   checksum,xmitcsum,inbuf);
		}
		dbg_putc('-');  /* failed checksum */ 
    } 
	dbg_putc('+');  /* successful transfer */
	/* if a sequence char is present, reply the sequence ID */
	if(inbuf[2] == ':') {
		dbg_putc(inbuf[0]);
		dbg_putc(inbuf[1]);
		/* remove sequence chars from buffer */
		i = 3;
		for( ;; ) {
			inbuf[i-3] = inbuf[i];
			if(inbuf[i] == '\0') break;
			++i;
		}
	} 
    return(TRUE);
}

/* 
 * send the packet in buffer.  The host gets one chance to read it.  
 * This routine does not wait for a positive acknowledge.
 */

void
putpacket(void) {
    unsigned char checksum;
    int  count;
    char ch;

    gdb_compress(outbuf, scratch);
  
#ifdef DEBUG_GDB
	kprintf("Response packet '%s'\n", scratch);
#endif	

    /*  $<packet info>#<checksum>. */

    dbg_putc('$');
    checksum = 0;
    count    = 0;
  
    while((ch = scratch[count])) {
		dbg_putc(ch);
		checksum += ch;
		count += 1;
    }
	
    dbg_putc('#');
    dbg_putc(tohexchar(checksum >> 4));
    dbg_putc(tohexchar(checksum));
}

/* 
 * convert the memory pointed to by mem into hex, placing result in buf
 * return a pointer to the last char put in buf (null)
 */
char *
mem2hex(char *mem, char *buf, int count) {
    int i;
    unsigned char ch;
	union {
		uint8_t		u8;
		uint16_t	u16;
		uint32_t	u32;
	}	temp;

	// Handle 1/2/4 bytes reads specially - they might be "in8/16/32"
	// requests, or we might be talking to a mem mapped register that
	// doesn't like handing out information one byte at a time.

	if(cpu_handle_alignment(mem, count)) {
		switch(count) {
		case 1:	
			if(MAP_IN_RANGE(inout, mem)) {
				temp.u8 = in8(MAP_OFFSET(inout, mem));
			} else {
				temp.u8 = *(uint8_t *)mem;
			}
			mem = (void *)&temp;
			break;
		case 2:	
			if(MAP_IN_RANGE(inout, mem)) {
				temp.u16 = in16(MAP_OFFSET(inout, mem));
			} else {
				temp.u16 = *(uint16_t *)mem;
			}
			mem = (void *)&temp;
			break;
		case 4:
			if(MAP_IN_RANGE(inout, mem)) {
				temp.u32 = in32(MAP_OFFSET(inout, mem));
			} else {
				temp.u32 = *(uint32_t *)mem;
			}
			mem = (void *)&temp;
			break;
		default:
			break;
		}
	}

    for(i=0;i<count;i++) {
		ch = *mem++;
		*buf++ = tohexchar(ch >> 4);
		*buf++ = tohexchar(ch);
    }
    *buf = 0; 
    return(buf);
}

/*
 * convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte written
 */

char *
hex2mem(char *buf, char *mem, int count) {
    int i;
    unsigned char ch;
	char		*out;
	int			special;
	union {
		uint8_t		u8;
		uint16_t	u16;
		uint32_t	u32;
	}	temp;

	// Handle 1/2/4 bytes writes specially - they might be "out8/16/32"
	// requests, or we might be talking to a mem mapped register that
	// doesn't like taking information one byte at a time.

	special = 0;
	out = mem;
	if(cpu_handle_alignment(mem, count)) {
		switch(count) {
		case 1:	
		case 2:	
		case 4:
			special = 1;
			out = (void *)&temp;
			break;
		default:
			break;
		}
	}

    for(i=0;i<count;i++) {
		ch = chartohex(*buf++) << 4;
		ch = ch + chartohex(*buf++);
		*out++ = ch;
    }

	if(special) {
		switch(count) {
		case 1:	
			if(MAP_IN_RANGE(inout, mem)) {
				out8(MAP_OFFSET(inout, mem), temp.u8);
			} else {
				*(uint8_t *)mem = temp.u8;
			}
			break;
		case 2:	
			if(MAP_IN_RANGE(inout, mem)) {
				out16(MAP_OFFSET(inout, mem), temp.u16);
			} else {
				*(uint16_t *)mem = temp.u16;
			}
			break;
		case 4:
			if(MAP_IN_RANGE(inout, mem)) {
				out32(MAP_OFFSET(inout, mem), temp.u32);
			} else {
				*(uint32_t *)mem = temp.u32;
			}
			break;
		default:
			break;
		}
	}
    return(mem + count);
}

/* These procedures manage a semaphore that controls the protocol for sending
 * relocation packets.  gdb_set_reloc_sem should be called to set the
 * semaphore whenever the router enters a debugging session to force the
 * protocol to transmit a relocation packet in response to the first '?'
 * prompt from the gdb client.
 *
 * We use a semaphore instead of a parameter because the connection between
 * gdb_command and gdb_interface *may* be through the breakpoint exception
 * handler, which prohibits knowing how we were invoked.
 */

static boolean gdb_send_reloc;

void
gdb_set_reloc_sem(void) {
  gdb_send_reloc = TRUE;
}

boolean
gdb_test_reloc_sem(void) {
  return gdb_send_reloc;
}

void
gdb_clear_reloc_sem(void) {
  gdb_send_reloc = FALSE;
}


static void *
gdb_mapping_add(uintptr_t vaddr, unsigned len, unsigned prot, size_t *valid) {
//kprintf("map add %x, nocache,dir,raw=%x/%x %x/%x %x/%x\n", vaddr, nocache.start, nocache.end, direct.start, direct.end, raw.start, raw.end);
	if(MAP_IN_RANGE(inout, vaddr)) {
		if(debug_flag > 2) kprintf("in/out access for 0x%x\n", vaddr);
		*valid = len;
		return (void *)vaddr;
	}
	if(MAP_IN_RANGE(raw, vaddr)) {
		if(debug_flag > 2) kprintf("raw access for 0x%x\n", vaddr);
		*valid = len;
		return (void *)vaddr;
	}
	if(MAP_IN_RANGE(direct, vaddr)) {
		paddr_t	paddr;

		if(vaddrinfo(NULL, vaddr, &paddr, valid) == PROT_NONE) return(NULL);
		if(debug_flag > 2) kprintf("direct access for 0x%x\n", vaddr);
		return (void *)vaddr;
	}
	if(MAP_IN_RANGE(nocache, vaddr)) {
		if(debug_flag > 2) kprintf("nocache access for 0x%x\n", vaddr);
		prot |= PROT_NOCACHE;
	}
	return mapping_add(vaddr, len, prot, valid);
}


static void
gdb_mapping_del(void *p, size_t len) {
	if(  !MAP_IN_RANGE(inout, p) 
	  && !MAP_IN_RANGE(raw, p) 
	  && !MAP_IN_RANGE(direct, p)) {
		mapping_del(p, len);
	}
}


/*
 * gdb_read_membytes:
 * Read bytes from our memory and return to gdb client
 */
void
gdb_read_membytes(CPU_REGISTERS *ctx) {
    int 		addr;
	int 		length;
	size_t		actual;
	char		*p;
	size_t		valid;
	void		*trans;

    if(parse2hexnum(&inbuf[1],&addr,&length)) {
#ifdef DEBUG_GDB
		kprintf("Readmem: addr = %x, ", addr);
		kprintf("data = %x\n", safe_read(addr));
#endif
		p = outbuf;
		for( ;; ) {
			trans = gdb_mapping_add(addr, length, PROT_READ, &valid);
			if(trans == NULL) break;
			if(valid == 0) break;
			actual = min(valid, length);
			mem2hex(trans, p, actual);
			gdb_mapping_del(trans, valid);
			p += actual * 2;
			addr += actual;
			length -= actual;
			if(length == 0) break;
		}
		if(p == outbuf && valid == 0) {
			strcpy(outbuf,"E03");
			if(gdb_debug) gdb_printf("bus error");
		}
    } else {
		strcpy(outbuf,"E01");
		if(gdb_debug) gdb_printf("malformed read memory command: %s", inbuf);
    }     
}

/*
 * gdb_write_membytes:
 * Write bytes from the gdb client command buffer to our memory
 */
void
gdb_write_membytes(CPU_REGISTERS *ctx) {
    int 		addr;
	int			length;
    char		*ptr;
	char		*p;
	size_t		actual;
	size_t		valid;
	void		*trans;

    if(parse2hexnum(&inbuf[1],&addr,&length)) {
		ptr = strchr(inbuf,':') + 1; /* point 1 past the colon */
#ifdef DEBUG_GDB
		kprintf("Writemem: addr = %x\n", addr);
#endif
		p = ptr;
		for( ;; ) {
			trans = gdb_mapping_add(addr, length, PROT_READ|PROT_WRITE, &valid);
			if(trans == NULL) break;
			actual = min(valid, length);
			hex2mem(p, trans, actual);
			CPU_CACHE_FLUSH(trans, addr, actual);
			gdb_mapping_del(trans, valid);
			p += actual * 2;
			addr += actual;
			length -= actual;
			if(length == 0) break;
		}
		if((p == ptr) && (trans == NULL)) {
			strcpy(outbuf,"E03");
			if(gdb_debug) gdb_printf("bus error");
		}
    } else {
		strcpy(outbuf,"E02");
		if(gdb_debug) gdb_printf("malformed write memory command: %s",inbuf);
    } 
}

/*
 * gdb_get_info
 *
 * Get target information
 */
void
gdb_get_info(void) {
    struct kdebug_info		*kinfo;
    const struct kdebug_private	*kprivate;
	unsigned				len;
	void					*thread;
	void					*process;
	unsigned				pid;
	char					*name;

    kinfo = private->kdebug_info;
    if(kinfo == NULL || (kprivate = kinfo->kdbg_private) == NULL) {
		strcpy(outbuf, "E00");		
		return;
    }
	//NYI: what about SMP?
	thread = ((void **)kprivate->actives)[0];
	if(thread == NULL) {
		strcpy(outbuf, "E00");		
		return;
	}
    switch (inbuf[1]) {
	case 'p':
		process = *(void **)((uint8_t *)thread + kprivate->th_process_off);
		pid = *(unsigned *)((uint8_t *)process + kprivate->pr_pid_off);
		name = *(char **)((uint8_t *)process + kprivate->pr_debug_name_off);
	    len = ksprintf(scratch, "Current process = \"%s\", PID = %d\n", name, pid);
	    outbuf[0] = '0';
	    mem2hex(scratch, &outbuf[1], len);
	    break;
	case '0':
	    strcpy(outbuf, "E00");		
	    break;
	default:
	    strcpy(outbuf, "E00");		
		break;
    }
}


static void
monitor_mem(char *p) {
	struct mapping_range	*range;
	int						adding;

	while(isspace(*p)) ++p;
	#define NOCACHE_CMD	"nocache"
	#define DIRECT_CMD	"direct"
	#define RAW_CMD	"raw"
	#define INOUT_CMD	"inout"
	if(memcmp(p, NOCACHE_CMD, sizeof(NOCACHE_CMD)-1) == 0) {
		p += sizeof(NOCACHE_CMD)-1;
		range = &nocache;
	} else if(memcmp(p, DIRECT_CMD, sizeof(DIRECT_CMD)-1) == 0) {
		p += sizeof(DIRECT_CMD)-1;
		range = &direct;
	} else if(memcmp(p, RAW_CMD, sizeof(RAW_CMD)-1) == 0) {
		p += sizeof(RAW_CMD)-1;
		range = &raw;
	} else if(memcmp(p, INOUT_CMD, sizeof(INOUT_CMD)-1) == 0) {
		p += sizeof(INOUT_CMD)-1;
		range = &inout;
	} else {
		return;
	}
	while(isspace(*p)) ++p;
	if(*p == '\0') {
		range->start = ~(uintptr_t)0;
		range->end   = 0;
	} else if(isdigit(*p)) {
		adding = 0;
		range->start = strtoul(p, &p, 0x10);
		while(isspace(*p)) ++p;
		if(*p == '+') {
			++p;
			while(isspace(*p)) ++p;
			adding = 1;
		}
		range->end = strtoul(p, &p, 0x10);
		if(adding) range->end += range->start - 1;
	} else {
		return;
	}
	if(debug_flag > 1) {
		kprintf("mon mem nocache,dir,raw,inout=%x/%x %x/%x %x/%x %x/%x\n", 
			nocache.start, nocache.end, 
			direct.start, direct.end, 
			raw.start, raw.end,
			inout.start, inout.end);
	}
	strcpy(outbuf, "OK");
}


/*
 * This function does all command procesing for interfacing to gdb.
 */
static boolean
do_gdb_interface(struct kdebug_entry *entry, CPU_REGISTERS *ctx, ulong_t signal) {
    int    length;

struct kdebug_info		*kinfo;
const struct kdebug_private	*kprivate;
THREAD *thread;

    /* 
     * Indicate that we've gone back to debug mode
     */
    for (length = 0; length < 4; length++) dbg_putc('|');
	if(protocol == 0) {
		// generic GDB 4.16 wants the response to the continue/step
		// command sent before it transmits anything else.
		ksprintf(outbuf,"S%02xk", (unsigned)signal);
		putpacket();
	}

	while(getpacket()) {
		connected = TRUE;

		outbuf[0] = 0;

#ifdef DEBUG_GDB
kprintf("Processing packet '%s'\n", inbuf);		
#endif
        switch(inbuf[0]) {

		/* Tell the gdb client our signal number */
		case '?' :
			if(gdb_test_reloc_sem()) {
				paddr_t					base;
				char					*str = SYSPAGE_ENTRY(strings)->data;
				struct asinfo_entry		*as = SYSPAGE_ENTRY(asinfo);
		
				while(strcmp(&str[as->name], "imagefs") != 0) {
					++as;
				}
				base = gdb_image_base(as->start);
				gdb_clear_reloc_sem();
				ksprintf(outbuf,"N%02x%P;%P;%P",
					(unsigned)signal, base, base, (paddr_t)(base + as->end - as->start + 1));
			} else {
				ksprintf(outbuf,"S%02xk", (unsigned)signal);
			}
	
			for(length=1;outbuf[length];length++) {
				if((outbuf[length] >= 'A') && 
					(outbuf[length] <='Z'))
					outbuf[length]=outbuf[length]+('a'-'A');
			}
			if(gdb_debug) gdb_printf("%s", outbuf);
	
			break;
	
		/* toggle debug flag */
		case 'd' :
			gdb_debug = !(gdb_debug);
			break; 
		
		/* return the value of the CPU registers */
		case 'g' :
/* temp solution, need to add an offset item in kdebug_private for fpu data	*/
			if((kinfo = private->kdebug_info)== NULL || (kprivate = kinfo->kdbg_private) == NULL ||
				(thread = ((void **)kprivate->actives)[0]) == NULL) {
				gdb_get_cpuregs(ctx,NULL);
			} else {
				gdb_get_cpuregs(ctx,thread->fpudata);
			}
			break;
	
		/* set the value of the CPU registers - return OK */
		case 'G' : 
/* temp solution, need to add an offset item in kdebug_private for fpu data	*/
			if((kinfo = private->kdebug_info)== NULL || (kprivate = kinfo->kdbg_private) == NULL ||
				(thread = ((void **)kprivate->actives)[0]) == NULL) {
				gdb_set_cpuregs(ctx,NULL);
			} else {
				gdb_set_cpuregs(ctx,thread->fpudata);
			}
			strcpy(outbuf,"OK");
			break;

		/* get target information */
		case 'i':
			gdb_get_info();
			break;
		  
		/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
		case 'm' : 
			gdb_read_membytes(ctx);
			break;
		  
		/* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
		case 'M' : 
			gdb_write_membytes(ctx);
			break;
		 
		/* cAA..AA    Continue at address AA..AA(optional) */
		case 'c' : 
			gdb_proc_continue(ctx, 0);	/* continue the process */
			return(TRUE);
	
		/* sAA..AA   Step one instruction from AA..AA(optional) */
		case 's' : 
			gdb_proc_continue(ctx, 1);	/* step one instruction */
			return(TRUE);

		/* q???? Generic query */	

		case 'q': 
			if(memcmp(&inbuf[1], "Rcmd,", 5) == 0) {
				// remote command
				char	*p;

				p = &inbuf[6];
				hex2mem(p, scratch, strlen(p));
				#define MEM_CMD	"mem "
				if(memcmp(scratch, MEM_CMD, sizeof(MEM_CMD)-1) == 0) {
					monitor_mem(&scratch[sizeof(MEM_CMD)-1]);
				}
			}
			break;

		/* k		Kill program */
		case 'k' :
			putpacket(); /*ACK the packet early (since we're going bye-bye) */
			gdb_prep_reboot();
			SYSPAGE_ENTRY(callout)->reboot(_syspage_ptr, 0);
			break;

		/* D		Detach from host */
		case 'D' :
			connected = FALSE;
			return(FALSE);
			  
		} /* switch */ 
		
		/* reply to the request */
		putpacket();
    }
	if(!connected) {
		kprintf( "Aborted!\n" );
	}
    return(FALSE);
}

boolean
gdb_interface(struct kdebug_entry *entry, CPU_REGISTERS *ctx, ulong_t sigcode) {
	boolean ret;
	ulong_t	signal = SIGCODE_SIGNO(sigcode);

	cpu_save_extra(&extra_state);
	if(!connected) {
		char				buff[_POSIX_PATH_MAX + 1];

		kprintf("KDEBUG at 0x%x,", gdb_location(ctx));
		if(signal < sizeof sig_to_name / sizeof *sig_to_name) {
			kprintf(" (%s)", sig_to_name[signal]);
		}
		kprintf(" S/C/F=%d/%d/%d",
			(unsigned)signal,
			(unsigned)SIGCODE_CODE(sigcode),
			(unsigned)SIGCODE_FAULT(sigcode));
		if(debugpath(entry, buff, sizeof buff) > 0) {
			kprintf(" in %s", buff);
		}
		if(debug_flag > 1) {
			kprintf(" loc=0x%x ctx=0x%p", (unsigned)(sigcode >> 24), ctx);
		}
		kprintf("\n");
	}

	if(signal >= NSIG) signal = NSIG;
	ret = do_gdb_interface(entry, ctx, signal);

	if(async_check) {
		//Wait for any break to clear before going back to kernel.
		do {
		} while(dbg_break_detect());
	}
	cpu_restore_extra(&extra_state);
	return(ret);
}


/*
 * chartohex
 * Convert char to hex nibble
 */

int
chartohex(unsigned char ch) {
    if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
    if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
    if ((ch >= '0') && (ch <= '9')) return (ch-'0');
    return(0);
}

unsigned char const hexchars[] = "0123456789abcdef";

char
tohexchar(unsigned char c) {
	c &= 0x0f;

    return(hexchars[c]);
}

void
acme_gdb_compress(char *src, char *dest) {
    char previous = 0;
    int repeat = 0;

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
}

void
acme_gdb_expand(char *src, char *dest) {
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

void
generic_gdb_compress(char *src, char *dest) {
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
}

void
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
 * init_kerdebug - Kernel debugger initialization.
 * Here it means GDB initialization.
 */

int
init_kerdebug(const char *parm) {

	if(protocol == 0) {
		/* generic GDB protocol compression */
		gdb_expand = generic_gdb_expand;
		gdb_compress = generic_gdb_compress;
	} else {
		/* acme GDB protocol compression */
		gdb_expand = acme_gdb_expand;
		gdb_compress = acme_gdb_compress;
	}
	return(0);
}


unsigned
safe_read(uintptr_t addr) {
	unsigned	*p;
	unsigned	val;
	unsigned	valid;

	p = gdb_mapping_add(addr, sizeof(unsigned), PROT_READ|PROT_WRITE, &valid);
	if(p == NULL) return 0;
	val = *p;
	gdb_mapping_del(p, valid);
	return val;
}


break_opcode bp_opcode;
uintptr_t bp_addr;

int
set_break(uintptr_t addr) {
	void	*p;
	size_t	valid;

#ifdef DEBUG_GDB
	kprintf("break at %x old = %x ", addr, safe_read(addr));
#endif
	p = mapping_add(addr, sizeof(break_opcode), PROT_READ|PROT_WRITE, &valid);
	if((p == NULL) || (valid < sizeof(break_opcode))) {
		if(gdb_debug) kprintf("!%x not valid addr!n", bp_addr);
		return 0;
	}

	bp_addr = addr;
	bp_opcode = *(break_opcode *)p;
	*(volatile break_opcode *)p = OPCODE_BREAK;
	CPU_CACHE_FLUSH(p, addr, sizeof(break_opcode));
	mapping_del(p, valid);
#ifdef DEBUG_GDB
	kprintf("new_data = %x\n", safe_read(addr));
#endif
	return 1;
}

boolean
restore_break() {
	void	*p;
	size_t	valid;

    if(bp_addr == 0) return(FALSE);
#ifdef DEBUG_GDB
	kprintf("Restore break: addr = %x, opcode = %x\n",
		bp_addr, bp_opcode);
#endif
	/*
	 * If we are taking a breakpoint exception and we have
	 * registered the fact that we are doing single stepping
	 * and the address corresponds to the single stepping pc,
	 * restore the original code
	 */
	p = mapping_add(bp_addr, sizeof(break_opcode), PROT_READ|PROT_WRITE, &valid);
	*(break_opcode *)p = bp_opcode;
	CPU_CACHE_FLUSH(p, bp_addr, sizeof(break_opcode));
	mapping_del(p, valid);
	bp_addr = 0;
	bp_opcode = 0;
	return(TRUE);
}


int
watch_entry(struct kdebug_entry *entry, uintptr_t vaddr) {
#ifdef DEBUG_GDB
	kprintf("Watch entry = %x\n", vaddr);
#endif
	if(set_break(vaddr)) {
		entry->flags |= KDEBUG_FLAG_STOP_ON_ENTRY;
	} else {
		kprintf("Invalid address %x\n", vaddr);
		return(-1);
	}
	return(0);
}

int
is_watch_entry(struct kdebug_entry *entry, uintptr_t ip) {
	if((entry != NULL) && (entry->flags & KDEBUG_FLAG_STOP_ON_ENTRY) && (ip == bp_addr)) {
		entry->flags &= ~KDEBUG_FLAG_STOP_ON_ENTRY;
		return 1;
	}
	return 0;
}


void
msg_entry(const char *msg, unsigned len) {
	if(len == 0) len = strlen(msg);
	out_text(msg, len);
}


void
go(uintptr_t start_vaddr, int kerdbg) {
	if(kerdbg) set_break(start_vaddr);
	startnext(start_vaddr);
}
