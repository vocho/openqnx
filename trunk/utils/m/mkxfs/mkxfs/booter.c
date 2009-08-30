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
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/stat.h>
#include "struct.h"


struct attr_booter_entry	booter;
struct addr_space			default_image;
struct addr_space			default_ram;

enum {
	ATTR_ATTR,
	ATTR_DEF_IMAGE,
	ATTR_DEF_RAM,
	ATTR_FILTER,
	ATTR_LEN,
	ATTR_NOTLOADED,
	ATTR_PAGESIZE,
	ATTR_PADDR_BIAS,
	ATTR_RSVD_VADDR,
	ATTR_VBOOT
};

static struct attr_types attr_table[] = {
	{ "attr=",		ATTR_ATTR },
	{ "default_image=",ATTR_DEF_IMAGE },
	{ "default_ram=",	ATTR_DEF_RAM },
	{ "filter=",		ATTR_FILTER },
	{ "len=",			ATTR_LEN },
	{ "notloaded=",	ATTR_NOTLOADED },
	{ "pagesize=",	ATTR_PAGESIZE },
	{ "paddr_bias=",	ATTR_PADDR_BIAS },
	{ "rsvd_vaddr",	ATTR_RSVD_VADDR }, 
	{ "vboot=",		ATTR_VBOOT }, 
	{ NULL }
};


static void
parse_booter_attr(int tokenc, char *tokenv[]) {
	int			i;
	int			ival;
	char		*sval;
	char		attr_buf[512];
	unsigned	attr_len;

	attr_len = 0;
	for(i = 0 ; i < tokenc ; ++i) {
		switch(decode_attr(1, attr_table, tokenv[i], &ival, &sval)) {
		case ATTR_ATTR:
			attr_len += sprintf(&attr_buf[attr_len], " %s", sval);
			break;
		case ATTR_DEF_IMAGE:
			parse_addr_space_spec(&default_image, sval);
			break;
		case ATTR_DEF_RAM:
			parse_addr_space_spec(&default_ram, sval);
			split_image = 1;
			break;
		case ATTR_FILTER:
			booter.filter_spec = strdup(sval);
			while(*sval != '\0') {
				if(*sval++ == '%') {
					if(*sval++ == 'I') {
						/* The filter program makes a copy of the image,
						   rather than working on it in place. */
						booter.copy_filter = 1;
					}
				}
			}
			break;
		case ATTR_LEN:
			booter.boot_len = ival;
			break;
		case ATTR_NOTLOADED:
			booter.notloaded_len = ival;
			break;
		case ATTR_PAGESIZE:
			booter.pagesize = ival;
			break;
		case ATTR_PADDR_BIAS:
			booter.paddr_bias = ival;
			break;
		case ATTR_RSVD_VADDR:
			booter.rsvd_vaddr = ival;
			break;
		case ATTR_VBOOT:
			booter.vboot_addr = ival;
			break;
		}
	}
	if(attr_len > 0) {
		attr_len += 4;
		boot_attr_buf = malloc(attr_len+1);
		if(boot_attr_buf == NULL) {
			error_exit("Not enough memory for boot attributes.\n");
		}
		sprintf(boot_attr_buf, "[%s]\n", attr_buf);
	}
}

//
// If the boot data file is an elf executable, the REAL stuff we're
// interested in is in the first executable section.
//
static void *
handle_elf(char *p)
{
	Elf32_Ehdr			*ehdr;
	Elf32_Shdr			*shdr;
	int					endian;
	unsigned			i;
	unsigned			n;

	ehdr = (void *)p;
	if(memcmp (ehdr->e_ident, ELFMAG, SELFMAG) != 0) return(p);
	endian = (ehdr->e_ident[EI_DATA] != ELFDATA2LSB);
	shdr = (void *)(p + swap32(endian, ehdr->e_shoff));
	n = swap16(endian, ehdr->e_shnum);
	i = 0;
	for( ;; ) {
		if(i > n) {
			error_exit( "Can not find information section in %s\n", booter.name );
		}
		if(swap32(endian, shdr->sh_flags) & SHF_EXECINSTR) break;
		++shdr;
		++i;
	}
	p += swap32(endian, shdr->sh_offset);
	booter.data = p;
	booter.data_len = swap32(endian, shdr->sh_size);
	return(p);
}
	

//
// Set the CPU and boot data file - process the boot data
//

void
proc_booter_data(char *boot, int virtual) {
	char			hbuf[256];
	char			name[256];
	struct stat		sbuf;
	char			*p;
	char			*end;
	unsigned		len;
	FILE			*fp;

	booter.virtual = virtual;
	len = strcspn(boot, boot[0] == '/' ? "," : ",/");
	end = &boot[len];
	if(*end != '\0') {
		*end = '\0';
		set_cpu(boot, 1);
		boot = end + 1;
	}
	end = strchr(boot, ' ');
	
	if(end != NULL) {
		*end = '\0';
		booter.filter_args = strdup(end + 1);
	} else {
		booter.filter_args = "";
	}
	sprintf(name, "%s.boot", boot);
	booter.name = strdup(find_file(NULL, hbuf, &sbuf, name, 0));
	booter.data_len = sbuf.st_size;
	booter.data = p = malloc(booter.data_len + 1);
	if(p == NULL) {
		error_exit("Can not allocate %u bytes for boot information.\n",
				booter.data_len);
	}
	fp = fopen(booter.name, "rb");
	if(fp == NULL) {
		error_exit("Unable to open %s: %s\n", booter.name, strerror(errno));
	}
	fread(p, 1, booter.data_len, fp);
	fclose(fp);
	p = handle_elf(p);
	p[booter.data_len] = '\0'; // makes processing easier
	end = p + booter.data_len;
	if(*p == '[' ) {
		//
		// process booter attributes
		//
		push_token_state();
		p = tokenize(p + 1, " \r\t\n", ']');
		if(*p++ != ']') {
			error_exit("Missing ] in %s\n", booter.name);
		}
		parse_booter_attr(token->c, token->v);
		pop_token_state();
	}
	while(isspace(*p)) ++p;
	booter.data = p;
	booter.data_len = end - p;
	if(p < end) {
		for( ;; ) {
			if(p == end) {
				error_exit( "Can not find boot signature.\n" );
			}
			if( p[0] == 'b'
			 && p[1] == 'o'
			 && p[2] == 'o'
			 && p[3] == 't' ) break;
			++p;
		}
		p += 4;
		booter.data = p;
		booter.data_len = end - p;
	}
}

void
proc_booter_filter(unsigned startup_offset, char *intermediate_path, char *output_path) {
	char	buf[1024];
	char	*fmt;
	char	*p;
	char	c;

	p = buf;
	fmt = booter.filter_spec;
	for( ;; ) {
		c = *fmt++;
		if(c == '\0') break;
		if(c == '%') {
			c = *fmt++;
			switch(c) {
			case 'a':
				p += sprintf(p, "%s", booter.filter_args);
				break;
			case 's':
				p += sprintf(p, "0x%x", startup_offset);
				break;
			case 'I':
				p += sprintf(p, "%s", intermediate_path);
				break;
			case 'i':
				p += sprintf(p, "%s", output_path);
				break;
			default:
				*p++ = c;
				break;
			}
		} else {
			*p++ = c;
		}
	}
	*p = '\0';
#if defined (__WIN32__) || defined(__NT__)
	fixenviron(buf, sizeof(buf));
#endif
	if(verbose >= 3)
		fprintf(debug_fp, "Execute: %s\n", buf);
	if(system(buf) != 0)
		error_exit("Unable to run filter program '%s'.\n", buf);
}

__SRCVERSION("booter.c $Rev: 200993 $");
