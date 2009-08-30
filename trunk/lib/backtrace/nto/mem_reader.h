/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
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

#ifndef _MEM_READER_H_INCLUDED
#define _MEM_READER_H_INCLUDED

#include <setjmp.h>
#include "utils.h"

typedef struct mem_reader_t {
	int (*fn_safe)(struct mem_reader_t *rdr, void *mem, bt_addr_t pos, size_t sz);
	void (*fn)(struct mem_reader_t *rdr, void *mem, bt_addr_t pos, size_t sz);
	int fd;
	/* 4096 is the smallest page size on nto... */
#define MEM_RDR_CACHE_SZ 4096
	char *cache;
	bt_addr_t cache_offset;
	int err;
	int tid;
	int cnt;
	sigjmp_buf env;
	int sig_handler_installed;
	struct {
		struct sigaction sigsegv;
		struct sigaction sigbus;
	} oldact;
	sigset_t oldset;
} mem_reader_t;


void _BT(mem_reader_init)(mem_reader_t *rdr,
						  void *fn, void *fn_nocheck,
						  int fd, void *cache);
int _BT(mem_reader_install_sig_trap)(mem_reader_t *rdr);
int _BT(mem_reader_uninstall_sig_trap)(mem_reader_t *rdr);
void _BT(read_mem_direct)(mem_reader_t *rdr,
						  void *mem, bt_addr_t position, size_t size);
#ifndef _BT_LIGHT
int _bt_read_mem_direct_safe(mem_reader_t *rdr,
							 void *mem, bt_addr_t position, size_t size);
void _bt_read_mem_indirect(mem_reader_t *rdr,
						   void *mem, bt_addr_t position, size_t size);
int _bt_read_mem_indirect_safe(mem_reader_t *rdr,
							   void *mem, bt_addr_t position, size_t size);
#endif

/*============================================================*/
#define MEM_FAULT do{(*(char*)0)=0;}while(0)
#ifdef _BT_LIGHT

#define MEM_BUF(type,size) size_t _mem_buf_sz __attribute__((unused)) = size; \
                           size_t _mem_sz_1_elt __attribute__((unused)) = sizeof(type); \
						   char *_mem_buf_p
#define MEM_BUF_SZ              _mem_buf_sz
#define MEM_GET(addr,len)       do { _mem_buf_p = (void*)(addr); } while(0)
#define MEM_SLIDE_FORWARD(len)  do { _mem_buf_p += len; } while(0)
#define MEM_SLIDE_BACKWARD(len) do { _mem_buf_p -= len; } while(0)
#define MEM_SZ_1_ELT            _mem_sz_1_elt
#define MEM_P                   ((void*)_mem_buf_p)
#define MEM_ADDR                ((bt_addr_t)_mem_buf_p)

/* Read memory without using the buffer. */
#define MEM_READ_VAR(var,addr) do { _BT(memcpy)(&(var),(void*)(addr),sizeof(var)); } while(0)

#else
#define MEM_BUF(type,size)  type _mem_buf[size]; bt_addr_t _mem_addr=0
#define MEM_BUF_SZ          sizeof(_mem_buf)
#define MEM_GET(addr,len)   do { _mem_addr=(addr); \
                                 rdr->fn(rdr,(void*)_mem_buf,_mem_addr,len); \
                               } while(0)
#define MEM_SZ_1_ELT        sizeof(_mem_buf[0])
#define MEM_SLIDE_FORWARD(len) do {                                 \
        size_t sz = (len);                                          \
        _mem_addr += sz;                                            \
		(void)_BT(memmove)(_mem_buf, ((char*)_mem_buf)+sz,          \
		                   sizeof(_mem_buf)-sz);                    \
		rdr->fn(rdr,                                                \
		        ((char*)_mem_buf)+sizeof(_mem_buf)-sz,              \
		        _mem_addr+sizeof(_mem_buf)-sz, sz);                 \
	} while(0)		

#define MEM_SLIDE_BACKWARD(len)	do {                                \
		size_t sz = (len);                                          \
		if (_mem_addr > sz) _mem_addr -= sz; else MEM_FAULT;        \
		(void)_BT(memmove)(((char*)_mem_buf)+sz, _mem_buf,          \
		                  sizeof(_mem_buf)-sz);                     \
		rdr->fn(rdr, (void*)_mem_buf, _mem_addr, sz);               \
	} while(0)

#define MEM_P                   ((void*)&(_mem_buf[0]))
#define MEM_ADDR                _mem_addr

/* Read memory without using the buffer. */
#define MEM_READ_VAR(var,addr) do { rdr->fn(rdr,&(var),(addr),sizeof(var)); }while(0)


#endif

#endif
