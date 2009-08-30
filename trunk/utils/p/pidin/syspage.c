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



#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/image.h>
#include <sys/startup.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <sys/elf.h>
#include <hw/sysinfo.h>
#include <confname.h>
#include <sys/debug.h>
#include <sys/procfs.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <gulliver.h>

#include "pidin.h"


extern char	   *node;
extern unsigned node_nd;
extern int  find_netdir(char *nodename, char *buf, size_t bufsize, int flag);
struct syspage_entry *mysyspage;

static void
real_print_char(int c) {
	putchar(c);
}

static void	(*print_char)(int) = real_print_char;

static void
one_char(char c) {
	if(c == '\n') print_char('\r');
	print_char(c);
}

const static char	c[] = "0123456789abcdef";

static void
vmsg(const char *fmt, va_list args) {
	char 			*vs;
	uint64_t		num;
	unsigned		radix;
	int				dig;
	char			buf[64];
	
	for(; *fmt; ++fmt) {
		if(*fmt != '%') {
			one_char(*fmt);
			continue;
		}
		radix = dig = 0;
		switch(*++fmt) {
		case 'b':
			num = va_arg(args, unsigned);
			dig = sizeof(uint8_t)*2;
			radix = 16;
			break;
		case 'w':
			num = va_arg(args, unsigned);
			dig = sizeof(uint16_t)*2;
			radix = 16;
			break;
		case 'P':
			num = va_arg(args, paddr_t);
			dig = sizeof(paddr_t)*2;
			radix = 16;
			break;
		case 'x':
		case 'X':
		case 'l':
			num = va_arg(args, unsigned long);
			dig = sizeof(unsigned long)*2;
			radix = 16;
			break;
		case 'L':
			num = va_arg(args, uint64_t);
			dig = sizeof(uint64_t)*2;
			radix = 16;
			break;
		case 'd':
			num = va_arg(args, unsigned);
			radix = 10;
			break;
		case 's':
			vs = va_arg(args, char *);
			while(*vs) {
				one_char(*vs++);
			}
			continue;
		default:
			one_char(*fmt);
			continue;
		}
		vs = &buf[sizeof(buf)];

		*--vs = '\0';
		do {
			*--vs = c[num % radix];
			num /= radix;
		} while(num);
		for(dig -= &buf[sizeof(buf)-1] - vs; dig > 0; --dig) {
			one_char('0');
		}
		while(*vs) {
			one_char(*vs++);
		}
	}
}
				
void
kprintf(const char *fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	vmsg(fmt, args);
	va_end(args);
}	

static char *string_ptr;

static void
string_print_char(int c) {
	*string_ptr++ = c;
}


void
ksprintf(char *buff, const char *fmt, ...) {
	va_list args;
	void	(*old_print_char)(int) = print_char;

	print_char = string_print_char;
	string_ptr = buff;
	va_start(args, fmt);
	vmsg(fmt, args);
	va_end(args);
	*string_ptr = '\0';
	print_char = old_print_char;
}
				
void
crash(const char *fmt, ...) {
	va_list args;
	
	va_start(args, fmt);
	vmsg(fmt, args);
	va_end(args);
	for( ;; ) {}
}	

void
set_print_char(void (*prt)(int)) {
	print_char = prt;
}

void	(*
get_print_char(void))(int) {
	return print_char;
}

// Defines
//
#define NULL_PADDR		(~(paddr_t)0)
#define NULL_PADDR32	(~(paddr32_t)0)

#define TRUNC(_x,_a)	((unsigned long)(_x)&~((_a)-1))
#define ROUND(_x,_a)	TRUNC(((unsigned long)(_x))+((_a)-1),_a)

#define TRUNCPG(_x)		TRUNC((_x), __PAGESIZE)
#define ROUNDPG(_x)		ROUND((_x), __PAGESIZE)

#define KILO(k)			((k) * 1024UL)
#define MEG(m)			((m) * (1024UL * 1024UL))
#define GIG(m)			((m) * (1024UL * 1024UL * 1024UL))

#define TOPTR(x)		((void *)(x))

#define PTR_DIFF(a,b)	((uint8_t *)(a) - (uint8_t *)(b))

#define NUM_ELTS(__array)	(sizeof(__array)/sizeof(__array[0]))

#define COMMON_OPTIONS_STRING   CPU_COMMON_OPTIONS_STRING "AD:f:K:M:N:P:R:S:vr:j:"

#define PRT_SYSPAGE_RTN(flags, name)	\
	{ #name, offsetof(struct syspage_entry, name), flags, print_##name }

#define CPU_PRT_SYSPAGE_RTN(_cpu, flags, name)	\
	{ #name, offsetof(struct syspage_entry, un._cpu.name), flags, print_##_cpu##_##name }

void
print_typed_strings() {
	struct typed_strings_entry	*string = _SYSPAGE_ENTRY(mysyspage,typed_strings);
	unsigned	type;
	unsigned	i;

	i = 0;
	for( ;; ) {
		type = *(uint32_t *)&string->data[i];
		if(type == _CS_NONE) break;
		i += sizeof(uint32_t);
		kprintf("  off:%d type:%d string:'%s'\n", i-sizeof(uint32_t), type, &string->data[i]);
		i += strlen(&string->data[i]) + 1;
		i = ROUND(i, sizeof(uint32_t));
	}
}


void
print_strings() {
	char		*p = ((struct strings_entry *)_SYSPAGE_ENTRY(mysyspage,strings))->data;
	char		*start = p;
	unsigned	off;
	unsigned	len;
	char		buff[80];

	kprintf(" ");
	off = 1;
	while(*p != '\0') {
		ksprintf(buff, " [%d]'%s'", p - start, p);
		len = strlen(buff);
		if((off + len) >= 80) {
			kprintf("\n ");
			off = 1;
		}
		kprintf("%s", buff);
		off += len;
		p += strlen(p) + 1;
	}
	kprintf("\n");
}


void
print_system_private() {
	struct system_private_entry	*private = _SYSPAGE_ENTRY(mysyspage,system_private);
	unsigned				i;

	kprintf("  syspage ptr user:%l kernel:%l\n", private->user_syspageptr, private->kern_syspageptr);
	kprintf("  cpupage ptr user:%l kernel:%l spacing:%d\n", private->user_cpupageptr, private->kern_cpupageptr, private->cpupage_spacing);
	kprintf("  kdebug info:%l callback:%l\n", private->kdebug_info, private->kdebug_call);
	kprintf("  boot pgms: idx=%d\n", private->boot_idx);
	i = 0;
	for( ;; ) {
		if(i >= NUM_ELTS(private->boot_pgm)) break;
		if(private->boot_pgm[i].entry == 0) break;
		kprintf("    %d) base paddr:%l start addr:%l\n",
				i, private->boot_pgm[i].base, private->boot_pgm[i].entry);
		++i;
	}
	kprintf("  pagesize:%l\n", private->pagesize);
}


char *
__hwi_find_string(unsigned off) {
    return &_SYSPAGE_ENTRY(mysyspage,strings)->data[off];
}

void dump_asinfo( const char *parent_name, uint16_t owner, int level )
{
	struct asinfo_entry 	*as = _SYSPAGE_ENTRY(mysyspage,asinfo);
	int						num = mysyspage->asinfo.entry_size / sizeof(*as);
	int						i;
	char					name[1024+1];

	for(i = 0; i < num; ++i) {
		if ( owner == as->owner ) {
			ksprintf( name, "%s%s%s", parent_name, parent_name[0]?"/":"", __hwi_find_string(as->name));
			printf(" %4hx) %16llx-%-16llx o:%04hx a:%04hx p:%d n:%s\n",
					i * sizeof(*as),
					as->start,
					as->end,
					as->owner,
					as->attr,
					as->priority,
					name );
			if ( as->attr & AS_ATTR_KIDS )
				dump_asinfo( name, i*sizeof(*as), level+1 );
		}
		++as;
	}
}

void
print_asinfo() {
	dump_asinfo( "", -1, 0 );
}

//
// Gets overridden when used in startup.
//
void *
__hwi_base() {
    return _SYSPAGE_ENTRY(mysyspage,hwinfo);
}

void
print_hwinfo() {
	hwi_tag				*tag = (hwi_tag *)((struct hwinfo_entry *)_SYSPAGE_ENTRY(mysyspage,hwinfo))->data;
	void				*base;
	void				*next;
	char				*name, *itemname;

	while(tag->prefix.size != 0) {
		next = (hwi_tag *)((uint32_t *)tag + tag->prefix.size);
		base = (void *)(&tag->prefix + 1);
		name = __hwi_find_string(tag->prefix.name);
		kprintf("  %d) size:%d tag:%d(%s)", hwi_tag2off(tag), tag->prefix.size, tag->prefix.name, name);
		if(*name >= 'A' && *name <= 'Z') {
			itemname = __hwi_find_string(tag->item.itemname);
			base = (void *) (&tag->item + 1);
			kprintf(" isize:%d, iname:%d(%s), owner:%d, kids:%d",
					tag->item.itemsize, tag->item.itemname, itemname,
					tag->item.owner, tag->item.kids);
		}
		if(base != next) {
			kprintf("\n    ");
			while(base < next) {
				uint8_t		*p = base;
	
				kprintf(" %b", *p);
				base = p + 1;
			}
		}
		kprintf("\n");
		tag = next;
	}
}


void
print_qtime() {
	struct qtime_entry *qtime = _SYSPAGE_ENTRY(mysyspage,qtime);

	kprintf("  boot:%l CPS:%l%l rate/scale:%d/-%d intr:%d\n",
		qtime->boot_time,
		(unsigned long)(qtime->cycles_per_sec >> 32),
		(unsigned long)qtime->cycles_per_sec,
		qtime->timer_rate,
		-(int)qtime->timer_scale,
		(int)qtime->intr
		);
}

void
print_cpuinfo() {
 	struct cpuinfo_entry *cpu = _SYSPAGE_ENTRY(mysyspage,cpuinfo);
	unsigned i;

	for( i = 0; i < mysyspage->num_cpu; ++i ) {
		kprintf("  %d) cpu:%l flags:%l speed:%l cache i/d:%d/%d name:%d\n",
			i,
			cpu[i].cpu,
			cpu[i].flags,
			cpu[i].speed,
			cpu[i].ins_cache,
			cpu[i].data_cache,
			cpu[i].name);
	}
}

void
print_cacheattr() {
 	struct cacheattr_entry *cache = _SYSPAGE_ENTRY(mysyspage,cacheattr);
	int						num = mysyspage->cacheattr.entry_size / sizeof(*cache);
	int						i;

	for( i = 0; i < num; ++i ) {
		kprintf("  %d) flags:%b size:%w #lines:%w control:%l next:%d\n",
			i,
			cache[i].flags,
			cache[i].line_size,
			cache[i].num_lines,
			cache[i].control,
			cache[i].next);
	}
}


void
print_callout() {
	struct callout_entry	*call = _SYSPAGE_ENTRY(mysyspage,callout);
	unsigned				i;

	kprintf("  reboot:%l power:%l\n", call->reboot, call->power);
	kprintf("  timer_load:%l reload:%l value:%l\n",
			call->timer_load, call->timer_reload, call->timer_value);
	for(i = 0; i < NUM_ELTS(call->debug); ++i) {
		struct debug_callout	*dbg = &call->debug[i];

		kprintf("  %d) display:%l poll:%l break:%l\n", i,
			dbg->display_char, dbg->poll_key, dbg->break_detect);
	}
}

static void
print_intrgen(char *name, struct __intrgen_data *gen) {
	kprintf("     %s => flags:%w, size:%w, rtn:%l\n",
		name, gen->genflags, gen->size, gen->rtn);
}

void
print_intrinfo() {
 	struct intrinfo_entry *intr = _SYSPAGE_ENTRY(mysyspage,intrinfo);
	int						num = mysyspage->intrinfo.entry_size / sizeof(*intr);
	int						i;

	for( i = 0; i < num; ++i ) {
		kprintf("  %d) vector_base:%l, #vectors:%d, cascade_vector:%l\n",
				i, intr[i].vector_base, intr[i].num_vectors, intr[i].cascade_vector);
		kprintf("     cpu_intr_base:%l, cpu_intr_stride:%d, flags:%w\n",
				intr[i].cpu_intr_base, intr[i].cpu_intr_stride, intr[i].flags);
		print_intrgen(" id", &intr[i].id);
		print_intrgen("eoi", &intr[i].eoi);
		kprintf("     mask:%l, unmask:%l, config:%l\n",
			intr[i].mask, intr[i].unmask, intr[i].config);
	}
}


void
print_smp() {
	struct smp_entry *smp = _SYSPAGE_ENTRY(mysyspage,smp);

	kprintf("  send_ipi:%l cpu:%l\n", smp->send_ipi, smp->cpu);
}

#define INFO_SECTION		0x0001
#define EXPLICIT_ENABLE		0x8000
#define EXPLICIT_DISABLE	0x4000

struct debug_syspage_section {
	const char 		*name;
	unsigned short	loc;
	unsigned short	flags;
	void			(*print)(void);
};

static struct debug_syspage_section sp_section[] = {
	PRT_SYSPAGE_RTN(1, system_private),
	PRT_SYSPAGE_RTN(1, qtime),
	PRT_SYSPAGE_RTN(1, callout),
	PRT_SYSPAGE_RTN(1, cpuinfo),
	PRT_SYSPAGE_RTN(1, cacheattr),
	PRT_SYSPAGE_RTN(1, asinfo),
	PRT_SYSPAGE_RTN(1, hwinfo),
	PRT_SYSPAGE_RTN(1, typed_strings),
	PRT_SYSPAGE_RTN(1, strings),
	PRT_SYSPAGE_RTN(1, intrinfo),
	PRT_SYSPAGE_RTN(1, smp),
};

static int
enable_print_syspage(const char *name) {
	unsigned	i;
	unsigned	on_bit;
	unsigned	off_mask;
	
	if( *name == '~') {
		++name;
		on_bit = EXPLICIT_DISABLE;
		off_mask = ~EXPLICIT_ENABLE;
	} else {
		on_bit = EXPLICIT_ENABLE;
		off_mask = ~EXPLICIT_DISABLE;
	}
	for(i = 0; i < NUM_ELTS(sp_section); ++i) {
		if(strcmp(sp_section[i].name, name) == 0) {
			sp_section[i].flags &= off_mask;
			sp_section[i].flags |= on_bit;
		}
	}
	return(on_bit & EXPLICIT_ENABLE);
}

int debug_flag = 10;

void
print_syspage(char *enables) {
	unsigned	i;
	unsigned	flags;
	int			have_enables = 0;

	if(debug_flag > 1) {
		if ( enables )
		for ( enables = strtok(enables,"=,"); enables; enables = strtok(NULL,",") ) {
			have_enables |= enable_print_syspage(enables);
		}
		for(i = 0; i < NUM_ELTS(sp_section); ++i) {
			flags = sp_section[i].flags;
			if(!(flags & EXPLICIT_DISABLE) && !have_enables) {
				flags |= EXPLICIT_ENABLE;
			}
			if(flags & EXPLICIT_ENABLE) {
				kprintf("Section:%s ", sp_section[i].name);
				if(sp_section[i].flags & INFO_SECTION) {
					syspage_entry_info	*info;

					info = (void *)((uint8_t *)mysyspage + sp_section[i].loc);
					kprintf("offset:0x%x size:0x%x\n", info->entry_off, info->entry_size);
					if(info->entry_size > 0 && debug_flag > 2) {
						sp_section[i].print();
					}
				} else {
					kprintf("offset:0x%x\n", sp_section[i].loc);
					if(debug_flag > 2) {
						sp_section[i].print();
					}
				}
			}
		}
	}
}

int dspsyspage(char *enables)
{
char netpath[PATH_MAX+1], path[PATH_MAX+1];
int i, fd;

	/* get a handle on the /proc dir, even on another node */
	if ( node && *node ) {
		if ((i = find_netdir("", netpath, PATH_MAX, 1)) == -1)
		  error_exit(1, "can't find node %s: %s\n", node, strerror(errno));
		if ((fd = open("/dev/netmgr", O_RDONLY)) == -1)
		  error_exit(1, "Network manager is not running.\n");
		close(fd);
		strcat( netpath, node );
	}
	else
		netpath[0] = '\0';
	sprintf( path, "%s/proc/1/as", netpath );

	fd = open(path, O_RDONLY );
	if ( fd == -1 ) {
		fprintf( stderr, "can't display syspage! %s\n", strerror(errno));
		return 1;
	}

	if ((mysyspage = load_syspage(fd, !0)) == NULL) {
		fprintf(stderr, "can't load syspage! %s\n", strerror(errno));
		return 1;
	}

	print_syspage(enables);
	free(mysyspage);

	return 0;
}

struct syspage_entry *load_syspage(int fd, int full)
{
struct syspage_entry	*ptr;
struct asinfo_entry		*as;
struct cpuinfo_entry	*cpu;
struct qtime_entry		*qtm;
int						len, err, i, n;

	if ((err = devctl(fd, DCMD_PROC_SYSINFO, NULL, 0, &len)) == EOK) {
		if ((ptr = (struct syspage_entry *)malloc(len)) != NULL) {
			if ((err = devctl(fd, DCMD_PROC_SYSINFO, ptr, len, NULL)) == EOK) {
				if ((err = devctl(fd, -1, NULL, 0, NULL)) != EENDIAN || !full) {
					if (err == EENDIAN) {
						ENDIAN_SWAP16(&ptr->size);
						ENDIAN_SWAP16(&ptr->total_size);
						ENDIAN_SWAP16(&ptr->type);
						ENDIAN_SWAP16(&ptr->num_cpu);
						ENDIAN_SWAP16(&ptr->asinfo.entry_off);
						ENDIAN_SWAP16(&ptr->asinfo.entry_size);
						as = _SYSPAGE_ENTRY(ptr, asinfo);
						for (n = ptr->asinfo.entry_size / sizeof(struct asinfo_entry), i = 0; i < n; ++i, ++as) {
							ENDIAN_SWAP64(&as->start);
							ENDIAN_SWAP64(&as->end);
							ENDIAN_SWAP16(&as->owner);
							ENDIAN_SWAP16(&as->name);
							ENDIAN_SWAP16(&as->attr);
							ENDIAN_SWAP16(&as->priority);
						}
						ENDIAN_SWAP16(&ptr->cpuinfo.entry_off);
						ENDIAN_SWAP16(&ptr->cpuinfo.entry_size);
						cpu = _SYSPAGE_ENTRY(ptr, cpuinfo);
						for (i = 0; i < ptr->num_cpu; ++i, ++cpu) {
							ENDIAN_SWAP32(&cpu->cpu);
							ENDIAN_SWAP32(&cpu->speed);
							ENDIAN_SWAP32(&cpu->flags);
							ENDIAN_SWAP16(&cpu->name);
						}
						ENDIAN_SWAP16(&ptr->qtime.entry_off);
						ENDIAN_SWAP16(&ptr->qtime.entry_size);
						qtm = _SYSPAGE_ENTRY(ptr, qtime);
						ENDIAN_SWAP32(&qtm->boot_time);
						ENDIAN_SWAP16(&ptr->strings.entry_off);
						ENDIAN_SWAP16(&ptr->strings.entry_size);
					}
					return(ptr);
				}
			}
			free(ptr);
		}
		else {
			err = ENOMEM;
		}
	}
	errno = err;
	return(NULL);
}

uint64_t get_total_mem(struct syspage_entry *ptr)
{
char				*str = _SYSPAGE_ENTRY(ptr, strings)->data;
struct asinfo_entry	*as = _SYSPAGE_ENTRY(ptr, asinfo);
uint64_t			total = 0;
unsigned			num;

	for (num = ptr->asinfo.entry_size / sizeof(*as); num > 0; --num) {
		if (strcmp(&str[as->name], "ram") == 0) {
			total += as->end - as->start + 1;
		}
		++as;
	}
	return(total);
}

__SRCVERSION("syspage.c $Rev: 153052 $");
