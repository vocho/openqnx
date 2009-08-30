/*
 * $QNXtpLicenseC:
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





/*-
 * Copyright (c) 1983, 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#if !defined(lint) && defined(LIBC_SCCS)
static char     sccsid[] = "@(#)gmon.c      8.1 (Berkeley) 6/4/93";
#endif

#include <sys/neutrino.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/dcmd_prof.h>
#include <sys/link.h>
#include <sys/profiler.h>
#include "gcov-io.h"
#include "gmon.h"

// Use the libgcc name, else we run into problems
struct __bb __attribute__((__weak__)) *bb_head = 0;
extern struct gmonparam _gmonparam;

static int      s_scale;
static int      prof_fd = -1;
static int      __shmem_key = 0;

/* see profil(2) where this is describe (incorrectly) */
#define         SCALE_1_TO_1    0x10000L

#define ERR(s) write(2, s, sizeof(s))

void moncontrol __P((int));
extern char *__progname;

static int
qconn_map_add(struct gmon_arc_param *p) 
{
	int       shmem_fd, ret = 0;
	char      path[128];
	char      *cp;
	struct __prof_clientinfo *c;
	int size;
	
	sprintf(path, "/dev/shmem/prof-%d-%d", getpid(), ++__shmem_key);
#ifdef DEBUG
	fprintf(stderr, "shmem name %s\n", path);
#endif
	if ((shmem_fd = shm_open(path, O_RDWR|O_EXCL|O_CREAT|O_TRUNC, S_IRWXU)) < 0) {
		return -1;
	}
	size = p->pc_size + p->index_size + 1; // put a status at the end.
	//Did we succeed ?
	if (ftruncate(shmem_fd, size) == -1) {
		unlink(path);
		close(shmem_fd);
		return - 1;
	}
	if ((cp = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, shmem_fd, 0)) == MAP_FAILED) {
		unlink(path);
		close(shmem_fd);
		return -1;
	}
	//Set up structures
	p->pc = (struct pc_struct *) cp;
	cp += p->pc_size;
	p->pc_index = (u_short *) cp;
	*((unsigned char *)cp + p->index_size) = p->state_flags;
	c = calloc(1, sizeof(struct __prof_clientinfo) + ((p->name == NULL) ? 0 : strlen(p->name)));
	if (c != NULL) {
		if (p->gmon->flags & GMON_PARAM_FLAGS_FROM_TO_ARCS) {
			c->cmd = PROF_CMD_ARCS | PROF_CMD_ADD_MAPPING;
			c->tos_off = 0;
			c->tos_size = p->pc_size;
			c->from_off = p->pc_size;;
			c->from_size = p->index_size;
		} else {
			c->cmd = PROF_CMD_ARCS_2 | PROF_CMD_ADD_MAPPING;
			c->tos_off = p->pc_size;
			c->tos_size = p->index_size;
			c->from_off = 0;
			c->from_size = p->pc_size;
		}
		c->lowpc = p->lowpc;
		c->highpc = p->highpc;
		c->arcdata = (void *) cp;
		c->hash_frac = p->hashfraction;
		c->shmem_key = __shmem_key;
		if (p->name != NULL) {
			strcpy(c->map_name, p->name);
			c->map_name_len = strlen(p->name);
		} else {
			c->map_name_len = 0;
		}
		if (devctl(prof_fd, DCMD_PROF_MAPPING_ADD, c, sizeof *c + c->map_name_len, 0) != EOK) {
			ret = -1;
		}
		free(c);
	} else {
		ret = -1;
	}
	if (ret == -1) {
		munmap(p->pc, size);
	}
	unlink(path);
	close(shmem_fd);
	return ret;
}

static void
qconn_map_remove(struct gmon_arc_param *p)
{
	struct __prof_clientinfo c;

	munmap(p->pc, p->pc_size + p->index_size + 1);
	memset(&c, 0, sizeof c);
	if (p->gmon->flags & GMON_PARAM_FLAGS_FROM_TO_ARCS) {
		c.cmd = PROF_CMD_ARCS_2 | PROF_CMD_REMOVE_MAPPING;
	} else {
		c.cmd = PROF_CMD_ARCS | PROF_CMD_REMOVE_MAPPING;
	}
	c.lowpc = p->lowpc;
	c.highpc = p->highpc;		
	devctl(prof_fd, DCMD_PROF_MAPPING_REM, &c, sizeof c, 0);
}

static int 
get_arc_density() {
	char *user_value, *end;
	int density = 0;

	user_value = getenv("PROFARCDENSITY");
	if(user_value != NULL) {
		density = strtoul(user_value, &end, 0);
		if(user_value == end) {
			density = 0;
		}
	}
	
	if(density <= 0 || density >= 100) {
		density = ARCDENSITY;
	}

	return density;
}

static int 
ldd_handler(Ldd_Eh_Data_t *ehd, void *data, unsigned flags) 
{
	char *libname = ehd->l_map->l_path;
	struct gmonparam *p = (struct gmonparam *)data;
	
	if (flags & LDD_EH_DLL_LOAD) {
		libname = strrchr(libname, '/');
		if (libname != NULL) {
			libname++;
		} else {
			libname = ehd->l_map->l_path;
		}
		if (strcmp(libname,__progname) != 0) {	
			struct gmon_arc_param *arc_param = calloc(1, sizeof(struct gmon_arc_param));
			if (arc_param != NULL) {
				u_long lowpc = ehd->text_addr;
				u_long highpc = ehd->text_addr + ehd->text_size;
				arc_param->gmon = p;
				arc_param->lowpc = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
				arc_param->highpc = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
				arc_param->textsize = ehd->text_size;
				arc_param->hashfraction = HASHFRACTION;
				arc_param->index_size = arc_param->textsize / HASHFRACTION;
				arc_param->pc_limit = arc_param->textsize * get_arc_density() / 100;
				arc_param->name = ehd->l_map->l_path;
				if (arc_param->pc_limit < MINARCS)
					arc_param->pc_limit = MINARCS;
				else if (arc_param->pc_limit > MAXARCS)
					arc_param->pc_limit = MAXARCS;
				arc_param->pc_size = arc_param->pc_limit * sizeof(struct pc_struct);
				if (qconn_map_add(arc_param) != -1) {
					arc_param->next = _gmonparam.arc_param.next;
					_gmonparam.arc_param.next = arc_param;
				} else {
					free(arc_param);
				}
			}
		}
	} else if (flags & LDD_EH_DLL_UNLOAD) {
		struct gmon_arc_param *arc_param = _gmonparam.arc_param.next;
		struct gmon_arc_param *prev = NULL;
		while(arc_param) {
			if (strcmp(libname, arc_param->name) == 0) { 
				if (prev == NULL) {
					_gmonparam.arc_param.next = arc_param->next;
				} else {
					prev->next = arc_param->next;
				}
				qconn_map_remove(arc_param);
				free(arc_param);
				break;
			}
			prev = arc_param;
			arc_param = arc_param->next;
		}
	}
	return 0;
}

void
monstartup(u_long lowpc, u_long highpc, int argc, char **argv)
{
	register int    o;
	char           *profiler_devicename, *cp;
	struct gmonparam *p = &_gmonparam;
	struct __prof_clientinfo c;

	if ((profiler_devicename = getenv("QCONN_PROFILER")) != NULL) {
		// use connect to create a fd on a side channel.
		if ((prof_fd = _connect(_NTO_SIDE_CHANNEL, profiler_devicename, 0, O_RDWR, 0, _IO_CONNECT_OPEN, 1, _IO_FLAG_RD | _IO_FLAG_WR, 0, 0, 0, 0, 0, 0, 0)) != -1) {
			memset(&c, 0, sizeof c);
			//Init capabilities
			c.cap_flags = PROF_CAP_ARCCNTS | PROF_CAP_THREAD | PROF_CAP_BBINFO | PROF_CAP_SAMPLER | PROF_CAP_SHLIB;
			c.bb_head = &bb_head;
			if (devctl(prof_fd, DCMD_PROF_ATTACH, &c, sizeof c, 0) != EOK) {
				fprintf(stderr, "monstartup: external profiler error (devctl - %s)\n", strerror(errno));
				close(prof_fd);
				prof_fd = -1;
			}
		} else {
			fprintf(stderr,"monstartup: external profiler error (connect - %s)\n", strerror(errno));
		}
	}

	p->arc_param.gmon = p;
	/*
     * round lowpc and highpc to multiples of the density we're using
     * so the rest of the scaling (here and in gprof) stays in ints.
     */
	p->arc_param.lowpc = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
	p->arc_param.highpc = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
	p->arc_param.textsize = p->arc_param.highpc - p->arc_param.lowpc;
	p->arc_param.hashfraction = HASHFRACTION;
	p->arc_param.index_size = p->arc_param.textsize / HASHFRACTION;
	p->arc_param.pc_limit = p->arc_param.textsize * get_arc_density() / 100;
	if (p->arc_param.pc_limit < MINARCS) {
		p->arc_param.pc_limit = MINARCS;
	} else if (p->arc_param.pc_limit > MAXARCS) {
		p->arc_param.pc_limit = MAXARCS;
	}
	p->arc_param.pc_size = p->arc_param.pc_limit * sizeof(struct pc_struct);
	p->arc_param.name = __progname;
	p->arc_param.next = NULL;
	
	if (prof_fd != -1) {
		p->kcountsize = 0; // qconn sampler being used.
		p->kcount = NULL;
		
		memset(&c, 0, sizeof(c));
		c.cmd = PROF_CMD_QUERY_SHLIB;
		if (devctl(prof_fd, DCMD_PROF_QUERY, &c, sizeof c, 0) == EOK) {
			if (qconn_map_add(&p->arc_param) != -1) {	
				p->ldd_handle = __ldd_register_eh(ldd_handler, p, LDD_EH_DLL_LOAD | LDD_EH_DLL_UNLOAD | LDD_EH_DLL_REPLAY);
			} else {
				SET_GMON_STATE(p->arc_param.state_flags, GMON_PROF_OFF);
				fprintf(stderr, "monstartup: external profiler error (qconn_map_add - %s)\n", strerror(errno));
			}
		} else { // old qconn switch to old arc format
			p->flags |= GMON_PARAM_FLAGS_FROM_TO_ARCS;
			if (qconn_map_add(&p->arc_param) == -1) {	
				SET_GMON_STATE(p->arc_param.state_flags, GMON_PROF_OFF);
				fprintf(stderr, "monstartup: external profiler error (qconn_map_add - %s)\n", strerror(errno));
			}			
		}
		memset(&c, 0, sizeof(c));
		c.cmd = PROF_CMD_QUERY_THREAD;
		if (devctl(prof_fd, DCMD_PROF_QUERY, &c, sizeof c, 0) == EOK) {
			p->flags |= GMON_PARAM_FLAGS_THREAD;
		}
	} else {
		int size;
		p->kcountsize = p->arc_param.textsize / HISTFRACTION;
		size = p->kcountsize + p->arc_param.pc_size + p->arc_param.index_size + 1;
		cp = malloc(size);
		if (cp == (char *) NULL) {
			ERR("monstartup: out of memory\n");
			return;
		}
		memset(cp, 0, size);
		p->arc_param.pc = (struct pc_struct *) cp;
		cp += p->arc_param.pc_size;
		p->kcount = (u_short *) cp;
		cp += p->kcountsize;
		p->arc_param.pc_index = (u_short *) cp;
	}
	p->arc_param.pc[0].link = 0;

	o = p->arc_param.highpc - p->arc_param.lowpc;
	if (p->kcountsize != 0 && p->kcountsize < o) {
#if !defined(hp300) && !defined(__QNXNTO__)
		s_scale = ((float) p->kcountsize / o) * SCALE_1_TO_1;
#else				/* avoid floating point */
		int             quot = (unsigned)o / p->kcountsize;

		if (quot >= 0x10000)
			s_scale = 1;
		else if (quot >= 0x100)
			s_scale = 0x10000 / quot;
		else if (o >= 0x800000)
			s_scale = 0x1000000 / ((unsigned)o / (p->kcountsize >> 8));
		else
			s_scale = 0x1000000 / (((unsigned)o << 8) / p->kcountsize);
#endif
	} else
		s_scale = SCALE_1_TO_1;

	moncontrol(1);
}

void
_mcleanup()
{
	int             fd;
	int		curr_index;
	int             end_index;
	u_long          pc;
	int 			pc_index;
	struct gmonparam *p = &_gmonparam;
	struct gmon_arc_param *a = &_gmonparam.arc_param;
		
	struct gmonhdr  gmonhdr, *hdr;
	struct gmon_hdr gmon_hdr, *ghdr;
	struct gmon_hist_hdr gmon_hist, *hist;
	struct gmon_cg_arc_record gmon_cg, *cg;
	struct _clockperiod period;
	struct __bb    *ptr;
	unsigned char   c;
	int             i;
	char			gmon_out[PATH_MAX], *tmp;
	extern char		*__progname;
#ifdef DEBUG
	int             log, len;
	char            buf[200];
#endif

	if (p->state == GMON_PROF_MCOUNT_OVERFLOW)
		ERR("_mcleanup: tos overflow\n");

	moncontrol(0);

	if (prof_fd != -1) {
		struct __prof_clientinfo clocal;
		if (p->ldd_handle != NULL) {
			(void) __ldd_deregister_eh(p->ldd_handle);
		}
		/* External profiler, it will cleanup on its own */
		memset(&clocal, 0, sizeof clocal);
		devctl(prof_fd, DCMD_PROF_DETACH, &clocal, sizeof clocal, 0);
		close(prof_fd);
		prof_fd = -1;
		a = p->arc_param.next;
		while(a != NULL) {
			struct gmon_arc_param *next = a->next;
			free(a);
			a = next;
		}
		return;
	}
	
	/* Else we output the traditional gmon.out file */
	
	tmp = getenv("PROFDIR");
	if(tmp)
		sprintf(gmon_out,"%s/gmon.out.%d.%s", tmp, getpid(), __progname);
	else
		strcpy(gmon_out, "gmon.out");

	fd = open(gmon_out, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	/* open failed so, if not opening PROFDIR, fall back to opening gmon.out */
	if (fd < 0){
		int ii = errno;
		if(tmp && (fd = open("gmon.out", O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0){
			fprintf(stderr, "mcount: unable to open %s or gmon.out: %s\n", gmon_out, strerror(errno));
			return;
		}
		if (fd < 0){
			perror("mcount: gmon.out");
			return;
		}
		fprintf(stderr, "warning: unable to open %s (%s)\nfalling back to gmon.out\n", gmon_out, strerror(ii));
	}
#ifdef DEBUG
	log = open("gmon.log", O_CREAT | O_TRUNC | O_WRONLY, 0664);
	if (log < 0) {
		perror("mcount: gmon.log");
		return;
	}
	len = sprintf(buf, "[mcleanup1] kcount 0x%x ssiz %d\n",
		      p->kcount, p->kcountsize);
	write(log, buf, len);
#endif
	hdr = (struct gmonhdr *) & gmonhdr;
	ghdr = (struct gmon_hdr *) & gmon_hdr;
	memset(ghdr, 0, sizeof(*ghdr));
	memcpy(ghdr->cookie, GMON_MAGIC, 4);
	*(unsigned *) ghdr->version = 1;

	write(fd, (char *) ghdr, sizeof *ghdr);
	hist = &gmon_hist;
	/* Write counts */
	c = GMON_TAG_TIME_HIST;
	write(fd, &c, sizeof c);
	hist->low_pc = a->lowpc;
	hist->high_pc = a->highpc;
	hist->hist_size = p->kcountsize / HISTFRACTION;
	(void)ClockPeriod(CLOCK_REALTIME, NULL, &period, 0);
	hist->prof_rate = 1000000000 / period.nsec;
	strncpy(hist->dimen, "seconds", sizeof(hist->dimen));
	hist->dimen_abbrev = 's';
	write(fd, (char *) hist, sizeof *hist);
	write(fd, p->kcount, p->kcountsize);

	/* Write arc data */
	cg = &gmon_cg;
	c = GMON_TAG_CG_ARC;
	end_index = a->index_size / sizeof(*a->pc_index);
	for (curr_index = 0; curr_index < end_index; curr_index++) {
		if (a->pc_index[curr_index] == 0)
			continue;

		pc = a->lowpc;
		pc += curr_index * a->hashfraction * sizeof(*a->pc_index);
		for (pc_index = a->pc_index[curr_index]; pc_index != 0; pc_index = a->pc[pc_index].link) {
			if (p->flags & GMON_PARAM_FLAGS_FROM_TO_ARCS) {
				cg->self_pc = pc;
				cg->from_pc = a->pc[pc_index].pc;
			} else {
				cg->from_pc = pc;
				cg->self_pc = a->pc[pc_index].pc;
			}
			cg->count = a->pc[pc_index].count;
			write(fd, &c, sizeof c);
			write(fd, cg, sizeof *cg);
		}
	}

	/* Write basic-block info */
	c = GMON_TAG_BB_COUNT;
	for (ptr = bb_head; ptr; ptr = ptr->next) {
		write(fd, &c, sizeof c);
		write(fd, &ptr->ncounts, sizeof ptr->ncounts);
		for (i = 0; i < ptr->ncounts; i++) {
			write(fd, &ptr->addresses[i], sizeof *ptr->addresses);
			write(fd, &ptr->counts[i], sizeof *ptr->counts);
		}

	}
	close(fd);
	free(a->pc), a->pc = NULL;
}

/*
 * Control profiling
 *      profiling is what mcount checks to see if
 *      all the data structures are ready.
 */
void moncontrol(int mode)
{
	struct gmonparam *p = &_gmonparam;

	if (mode) {
		/* start */
		if (prof_fd == -1 ) {
			(void)profil((char *) p->kcount, p->kcountsize, (int) p->arc_param.lowpc, HISTFRACTION);
		}
		p->state = GMON_PROF_ON;
	} else {
		/* stop */
		if (prof_fd == -1) {
			profil((char *) 0, 0, 0, 0);
		}
		p->state = GMON_PROF_OFF;
	}
}

/*
 * Code coverage cleanup called on executable exit
 * Output data in gcov format if we have no external profiler
 */
void 
__attribute__((weak)) __bb_exit_func(void)
{
	FILE *da_file;
	int i;
	struct __bb *ptr;

	if(bb_head && prof_fd == -1) {
		ptr = bb_head;

		i = strlen(ptr->filename);
		if(strcmp(ptr->filename + i - 3, ".da")) {
			/* Old style coverage, not supported */
			return;
		}
		while(ptr) {
			/* Open for modification */
			da_file = fopen (ptr->filename, "wb");
			if (da_file == NULL) {
				da_file = fopen(basename((char *)ptr->filename), "wb");
			}
			
			if (!da_file)
			{
				fprintf (stderr, "arc profiling: Can't open output file %s.\n",
						 ptr->filename);
				ptr->filename = 0;
				ptr = ptr->next;
				continue;
			}
			
			/* We have an FD, write out bb info */
			(void) __write_long(ptr->ncounts, da_file, 8);
			for(i = 0; i < ptr->ncounts; i++) {
				(void) __write_long(ptr->counts[i], da_file, 8);
			}
			fclose(da_file);
			ptr = ptr->next;
		}
	}
}

/*
 * Constructor hook to link all basic-block and coverage structures on 
 * executable startup
 */
void 
__attribute__((weak)) __bb_init_func(struct __bb * block)
{

	if (block->zero_word) {
		return;
	}
	if(bb_head == NULL) {
		(void)atexit(__bb_exit_func);
	}
	block->zero_word = 1;
	block->next = bb_head;	
	bb_head = block;
}

__SRCVERSION("gmon.c $Rev: 159933 $");
