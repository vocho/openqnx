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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <pwd.h>
#include <errno.h>
#include <zlib.h>
#include <sys/elf.h>
#include <sys/elftypes.h>
#include <sys/elf_notes.h>
#include <sys/elf_nto.h>
#include <sys/procfs.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <sys/procmgr.h>
#include <sys/mman.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <sys/resource.h>

static iofunc_attr_t			attr;
static resmgr_connect_funcs_t	connect;
static resmgr_io_funcs_t		io;

static int requested = 0;

pid_t		match_pid = -1;
char		*dump_dir, *dump_path;
uintptr_t	pagesize;
char		*membuf;
pthread_t	cur_tid = 0;
int			verbose = 0;
int         gzlevel = -1;

long	max_core_size = RLIM_INFINITY;
int		sequential_dumps = 0;
int 	nodumpmem = 0;
int 	nodumpphys = 1;
int 	cur_tid_only = 0;
int 	world_readable = 0;

#define vprintf(x) { if (verbose) printf x; }

#define roundup(x, y)  ((((x)+((y)-1))/(y))*(y))

#ifndef NDEBUG
#define dprintf(x) printf x
#else
#define dprintf vprintf
#endif

int dumper_close_ocb(resmgr_context_t *ctp, void *reserved, iofunc_ocb_t *ocb);
int dumper_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb);
void DeliverNotifies(pid_t pid);
int get_ldd_mapinfos(int fd, procfs_mapinfo **infos, int *ninfos);

#define OFFENDING_THREAD(base, size, mapinfoptr) ((mapinfoptr)->vaddr >= base && \
						(mapinfoptr)->vaddr < (base + size))

off_t dump_tell( FILE *fp )
{
	if (gzlevel == -1) {
		return ftell(fp);
	} else {
		return gztell((gzFile)fp);
	}
}

static int dump_write(FILE *core_fp, const void *addr, int nr, long *size)
{
	int n; 		
	int wsize;  /* Size to be written. */

	/* According to Posix specs, if the file size "limit is exceeded, the
	 * writing of the core file shall terminate at this size." */

	if (*size < nr) {
		wsize = *size;
	} else {
		wsize = nr;
	}

	*size -= wsize;
	
	if (gzlevel == -1) {
		n = fwrite(addr, 1, wsize, core_fp );
	} else {
		n = gzwrite((gzFile)core_fp, (void *)addr, wsize);
	}

	if ((n<wsize) && (verbose))
	{
		perror("file write error");
	}

	if (wsize == nr) {
		return (n == nr);
	} else {
		return(-1);
	}
}

static int dump_seek(FILE *core_fp, off_t off)
{
	if (gzlevel == -1)
	  fseek( core_fp, off, SEEK_SET );
	else
	  if (gzseek((gzFile)core_fp, off, SEEK_SET) == -1) {
		  printf("gzseek backwards. Current %d, off %d, Failed.\n", dump_tell(core_fp), off);
		  exit(-1);
	  }
    return 1;
}

struct memelfnote {
	const char      *name;
	unsigned int    datasz;
	int             type;
	void            *data;
};

static int notesize(struct memelfnote *en)
{
    int sz;

    sz = sizeof(Elf32_Nhdr);
    sz += roundup(strlen(en->name), sizeof (Elf32_Word));
    sz += roundup(en->datasz, sizeof (Elf32_Word));

    return sz;
}

static int writenote(struct memelfnote *men, FILE *core_fp, long *size)
{
Elf32_Nhdr en;

    en.n_namesz = strlen(men->name)+1;
    en.n_descsz = men->datasz;
    en.n_type = men->type;

    if(dump_write( core_fp, &en, sizeof(en), size) == -1)
		return 1;

    if(dump_write( core_fp, men->name, en.n_namesz, size) == -1)
		return 1;
	
    dump_seek( core_fp, roundup(dump_tell(core_fp), sizeof (Elf32_Word)));  /* XXX */

    if(dump_write( core_fp, men->data, men->datasz, size) == -1)
		return 1;

    dump_seek( core_fp, roundup(dump_tell(core_fp), sizeof (Elf32_Word)));  /* XXX */

    dump_seek( core_fp, roundup(dump_tell(core_fp), sizeof (Elf32_Word)));

    return 1;
}

void dump_stack_memory( int fd, FILE *fp, procfs_mapinfo *mem, long *size )
{
int		num, i, max, min, base;
off_t	here, there;

	if ( (mem->flags & PG_HWMAPPED) == 0 ) {
		dprintf(("Ignoring non-mapped stack region: %#llx @ %#llx\n", 
			mem->size, mem->vaddr ));
		return;
	}
	base = mem->offset;
//	base = mem->vaddr;

	dprintf(("blanking %lld bytes of stack memory at %#x\n", mem->size, base ));
//printf("vaddr=%#x, offset=%#x, size=%lld\n", mem->vaddr, mem->offset, mem->size );

	here = dump_tell(fp);
	max = roundup( mem->size, pagesize );
	min = (mem->offset - mem->vaddr);
	dprintf(("max=%#x, min = %#x (%#llx->%#llx)\n", max, min, mem->vaddr+min, mem->vaddr+max ));

	if (gzlevel != -1) {
		int rsize;

		/* forward dumping from mem->offset to mem->offset + mem->size in gz mode */
		memset(membuf, 0, pagesize);
		for (i = 0; i < min; i += pagesize) {
			if( dump_write( fp, membuf, min(pagesize, min - i), size) == -1)
			  return;
		}

		for (i = min; i < mem->size; i += pagesize) {
			if ( lseek( fd, mem->vaddr + i, SEEK_SET ) == -1 ) {
				memset(membuf, 0, pagesize);
				num = pagesize;
			} else {
				rsize = (i & ~(pagesize - 1)) + pagesize;
				if (rsize >= mem->size) {
					rsize = mem->size - i;
				} else {
					rsize -= i;
				}
				if ((num = read( fd, membuf, rsize)) != rsize) {
					memset(membuf, -1, pagesize);
				}
			}
			if( dump_write( fp, membuf, num, size) == -1)
			  return;
		}
		return;
	}
	
	memset( membuf, 0, sizeof membuf );
	for (i = 0; i < mem->size; i+= pagesize ) {
		if(dump_write( fp, membuf, min( pagesize, mem->size - i ), size) == -1)
			return;
	}
	there = dump_tell(fp);

	for ( i = max - pagesize; i >= min; i -= pagesize ) {
		dump_seek( fp, here + i );
		if ( lseek( fd, mem->vaddr + i, SEEK_SET ) == -1 )
			continue;
		/* read memory here */
//		dprintf(("attempting to read @ %#llx\n", mem->vaddr + i ));
		if ( (num = read( fd, membuf, pagesize )) != pagesize ) {
			memset( membuf, -1, sizeof membuf );
			num = pagesize;
		}
		if(dump_write( fp, membuf, num, size) == -1)
			return;
		fflush( fp );
		if ( num != pagesize ) {
			dprintf(("cut short at %d+%d\n", i, num ));
			break;
		}
//		else
//			dprintf(("read %d bytes ok\n", num ));
	}
	dump_seek( fp, there );
}

void dump_memory( int fd, FILE *fp, procfs_mapinfo *mem, long *size )
{
int		num, i, ok = 1;

	dprintf(("dumping %lld bytes of memory at %#llx\n", mem->size, mem->vaddr ));

	for (i = 0; i < mem->size; i+= pagesize ) {
		/* read memory here */
		if ( ok ) {
			num = read( fd, membuf, min(pagesize, mem->size - i) );
		} else {
			memset( membuf, -1, sizeof membuf );
			num = pagesize;
		}
		if(dump_write( fp, membuf, min(pagesize, mem->size - i), size) == -1)
			return;
		if ( num != pagesize ) {
			dprintf(("cut short at %d+%d\n", i, num ));
			ok = 0;
		}
	}
}

void fixup_stack_boundary( procfs_status *thread, procfs_mapinfo *mapinfos, int n )
{
int i;
uint64_t vaddr, size;

	for ( i = 0; i < n; i++ ) {
		if ( (mapinfos[i].flags & MAP_STACK) )
			mapinfos[i].offset = mapinfos[i].vaddr;
	}
	dprintf(("Thread %d's SP is at %#llx\n", thread->tid, thread->sp ));
	for ( i = 0; i < n; i++ ) {
		if ( thread->sp >= mapinfos[i].vaddr &&
				thread->sp < mapinfos[i].vaddr+mapinfos[i].size ) {

			if ( (mapinfos[i].flags & MAP_STACK) == 0 )
				continue;

			
			vaddr = thread->sp & ~(pagesize-1);
			size = mapinfos[i].size - (vaddr - mapinfos[i].vaddr);

			dprintf(("Adjusting %lld bytes @ %#llx->%#llx to %lld bytes @ %#llx->%#llx\n",
				mapinfos[i].size, mapinfos[i].vaddr, mapinfos[i].vaddr+mapinfos[i].size,
				size, vaddr, vaddr+size ));

			mapinfos[i].offset = vaddr;

			return;
		}
	}
}

#ifndef _SLOGC_DUMPER
#define _SLOGC_DUMPER _SLOGC_TEST
#endif

void slog_tid( procfs_status *status, const char *path )
{
	switch (status->why) {
	case _DEBUG_WHY_SIGNALLED:
	case _DEBUG_WHY_FAULTED:
		slogf( _SLOG_SETCODE( _SLOGC_DUMPER, 0 ), _SLOG_INFO,
			"run fault pid %d tid %d signal %d code %d ip %#llx %s", 
				(int)status->pid, (int)status->tid, (int)status->info.si_signo, (int)status->info.si_code, (uint64_t)status->ip, path ?:"" );
		break;
	default:
		slogf( _SLOG_SETCODE( _SLOGC_DUMPER, 0 ), _SLOG_INFO,
			"run fault pid %d tid %d why %d what %d ip %#llx %s", 
				(int)status->pid, (int)status->tid, (int)status->why, (int)status->what, (uint64_t)status->ip, path ?:"" );
		break;
	}
}

int elfcore(int fd, FILE *fp, const char *path, long coresize)
{
procfs_sysinfo		*sysinfo;
int					sysinfo_len;
procfs_info			info;
procfs_status		status;
int					ret;
procfs_mapinfo		*mem = NULL, *mapinfos = NULL, *ldd_infos = NULL;
int					numnote=0, num, i, j, seg = 0, err, n_ldd_infos = 0;
Elf32_Ehdr			elf;
Elf32_Phdr 			phdr;
struct memelfnote	notes[20], thread_note;
off_t				offset = 0, dataoff;
uint64_t			cur_tid_base = 0, cur_tid_size = 0;


	if (nodumpmem) {
		if (-1 == get_ldd_mapinfos(fd, &ldd_infos, &n_ldd_infos)) {
			/* should we bail out here? */
			n_ldd_infos = 0;
		}
	}

	if((ret = devctl(fd, DCMD_PROC_SYSINFO, 0, 0, &sysinfo_len)) != EOK) {
		errno = ret;
		goto bailout;
	}
	if(sysinfo = alloca(sysinfo_len)) {
		if((ret = devctl(fd, DCMD_PROC_SYSINFO, sysinfo, sysinfo_len, 0)) != EOK) {
			errno = ret;
			goto bailout;
		}
	}

	if((ret = devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0)) != EOK) {
		errno = ret;
		goto bailout;
	}

	pagesize = sysconf( _SC_PAGESIZE );
	if ( membuf == NULL && ((membuf = malloc( pagesize )) == NULL) ) {
		goto bailout;
	}

	// write elf header
	memcpy(elf.e_ident, ELFMAG, SELFMAG);
	elf.e_ident[EI_CLASS] = ELFCLASS32;
	elf.e_ident[EI_DATA] = ELFDATANATIVE;
	elf.e_ident[EI_VERSION] = EV_CURRENT;
#if defined (__ARM__)
	elf.e_ident[EI_OSABI] = ELFOSABI_ARM;
#endif
	
	memset(elf.e_ident+EI_PAD, 0, EI_NIDENT-EI_PAD);

	if((ret = devctl(fd, DCMD_PROC_PAGEDATA, NULL, 0, &num)) != EOK) {
		errno = ret;
		goto bailout;
	}

	mapinfos = malloc( num * sizeof *mem );
	if ( mapinfos == NULL ) {
		goto bailout;
	}

	if((ret = devctl(fd, DCMD_PROC_PAGEDATA, mapinfos, num*sizeof(*mapinfos), &num)) != EOK) {
		errno = ret;
		goto bailout;
	}

	mem = malloc( (n_ldd_infos + num) * sizeof(*mem) );
	if ( mem == NULL ) {
		goto bailout;
	}

	/* find the offending thread */
	for(status.tid = 1; devctl(fd, DCMD_PROC_TIDSTATUS, &status, sizeof status, 0) == EOK; status.tid++) {
		dprintf(("thread %d.flags is %#x\n", status.tid, status.flags ));
		if(status.why == _DEBUG_WHY_SIGNALLED) {
			// This is the faulting thread...
			dprintf(("thread %d is was SIGNALLED\n", status.tid ));
			cur_tid = status.tid;
			cur_tid_base = status.stkbase;
			cur_tid_size = status.stksize;
		}
		dprintf(("thread %d.why is %#x\n", status.tid, status.why ));
	}

	if(cur_tid == 0) {
		/* can't find the faulting thread then we need to dump all stack information */
		cur_tid_only = 0;
	}

	for(seg = 0, i = 0; i < num; i++) {
	  	if(!(mapinfos[i].flags & PG_HWMAPPED) ||
			(nodumpmem && !(mapinfos[i].flags & MAP_STACK)) ) {
			continue;
		}
		if ( (nodumpphys && (mapinfos[i].flags & MAP_PHYS) && !(mapinfos[i].flags & MAP_ANON)) ) {
			continue;
		}

		/* if we only want to dump the offending tid's stack */
		if(cur_tid_only && (mapinfos[i].flags & MAP_STACK) && 
			!OFFENDING_THREAD(cur_tid_base, cur_tid_size, &mapinfos[i])) {
			continue;
		}

		memcpy(&mem[seg], &mapinfos[i], sizeof(*mem));
		seg++;
	}

	dprintf(("ldd mapinfos:\n"));
	for(i = 0; i < n_ldd_infos; i++) {
		dprintf(("%svaddr=%#llx, offset=%#llx, size=%#llx, flags=%#x\n", ldd_infos[i].flags & PG_HWMAPPED?"*":"", ldd_infos[i].vaddr, ldd_infos[i].offset, ldd_infos[i].size, ldd_infos[i].flags ));
		memcpy( &mem[seg], &ldd_infos[i], sizeof(*mem));;
		seg++;
	}
	free(mapinfos);
	mapinfos = NULL;
	if(n_ldd_infos) {
		free(ldd_infos);
		ldd_infos = NULL;
	}
	num = seg;

	elf.e_type = ET_CORE;
	elf.e_machine = EM_NATIVE;
	elf.e_version = EV_CURRENT;
	elf.e_entry = 0;
	elf.e_phoff = sizeof(elf);
	elf.e_shoff = 0;
#ifdef __SH__
	{
		struct cpuinfo_entry	*cpu;

		cpu = SYSPAGE_ENTRY(cpuinfo);
		switch ( SH4_PVR_FAM(cpu[0].cpu) ) {
			case SH4_PVR_SH4A:
				dprintf(("Noting SH4-A CPU\n"));
				elf.e_flags = EF_SH4A;
				break;
			case SH4_PVR_SH4:
			default:
				elf.e_flags = EF_SH4;
				break;
		}
	}
#else
	elf.e_flags = 0;
#endif
	elf.e_ehsize = sizeof(elf);
	elf.e_phentsize = sizeof(phdr);
	elf.e_phnum = seg+1; /* xxxx */
	elf.e_shentsize = 0;
	elf.e_shnum = 0;
	elf.e_shstrndx = 0;

	if(dump_write( fp, &elf, sizeof elf, &coresize ) == -1)
		goto bailout;
	
	offset += sizeof elf;
	offset += (elf.e_phnum) * sizeof phdr;

	if(sysinfo) {
		// write QNT_CORE_SYSINFO note
		memset( notes, 0, sizeof notes );
		notes[numnote].name = QNX_NOTE_NAME;
		notes[numnote].type = QNT_CORE_SYSINFO;
		notes[numnote].datasz = roundup(sysinfo_len, sizeof (Elf32_Word));
		notes[numnote].data = sysinfo;
		numnote++;
	}

	// write QNT_CORE_INFO note
	notes[numnote].name = QNX_NOTE_NAME;
	notes[numnote].type = QNT_CORE_INFO;
	notes[numnote].datasz = sizeof(info);
	notes[numnote].data = &info;
	numnote++;

	/* Write notes phdr entry */
	{
		int sz = 0;

		memset( &phdr, 0, sizeof phdr );

		for(i = 0; i < numnote; i++)
		    sz += notesize(&notes[i]);

		for(status.tid = 1; devctl(fd, DCMD_PROC_TIDSTATUS, &status, sizeof status, 0) == EOK; status.tid++) {
			procfs_greg					greg;
			procfs_fpreg				fpreg;
			int							size;

			if ( (err = devctl(fd, DCMD_PROC_CURTHREAD, &status.tid, sizeof status.tid, 0 )) != EOK ) {
				continue;
			}

			if (cur_tid_only && (cur_tid != status.tid)) {
				continue;
			}

			fixup_stack_boundary( &status, mem, seg );

			thread_note.name = QNX_NOTE_NAME;
			thread_note.type = QNT_CORE_STATUS;
			thread_note.datasz = sizeof(status);
			thread_note.data = &status;
			sz += notesize( &thread_note );

			if(devctl(fd, DCMD_PROC_GETGREG, &greg, sizeof greg, &size) == EOK) {
				thread_note.name = QNX_NOTE_NAME;
				thread_note.type = QNT_CORE_GREG;
				thread_note.datasz = size;
				thread_note.data = &greg;
				sz += notesize( &thread_note );
			}
			if(devctl(fd, DCMD_PROC_GETFPREG, &fpreg, sizeof fpreg, &size) == EOK) {
				thread_note.name = QNX_NOTE_NAME;
				thread_note.type = QNT_CORE_FPREG;
				thread_note.datasz = size;
				thread_note.data = &fpreg;
				sz += notesize( &thread_note );
			}
		}

		phdr.p_type = PT_NOTE;
		phdr.p_offset = offset;
		phdr.p_vaddr = 0;
		phdr.p_paddr = 0;
		phdr.p_filesz = sz;
		phdr.p_memsz = 0;
		phdr.p_flags = 0;
		phdr.p_align = 0;

		offset += phdr.p_filesz;
		if(dump_write( fp, &phdr, sizeof(phdr), &coresize) == -1)
			goto bailout;
	}

	/* Page-align dumped data */
	dataoff = offset = roundup(offset, pagesize);

	for ( i = 0; i < seg; i++ ) {
		memset( &phdr, 0, sizeof phdr );

		phdr.p_type = PT_LOAD;
		phdr.p_offset = offset;
		phdr.p_vaddr = mem[i].vaddr;
		phdr.p_paddr = 0;
		phdr.p_memsz = mem[i].size;
		phdr.p_flags = PF_W|PF_R;
		if ( mem[i].flags & MAP_ELF )
			phdr.p_flags |= PF_X;
		phdr.p_align = pagesize;
		phdr.p_filesz = phdr.p_memsz;

		offset += phdr.p_filesz;
		if(dump_write( fp, &phdr, sizeof(phdr), &coresize) == -1)
			goto bailout;
	}

	for(i = 0; i < numnote; i++) {
	    if (!writenote(&notes[i], fp, &coresize ))
	    	goto bailout;
	}

	for(status.tid = 1; devctl(fd, DCMD_PROC_TIDSTATUS, &status, sizeof status, 0) == EOK; status.tid++) {
		procfs_greg					greg;
		procfs_fpreg				fpreg;
		int							size;

		if ( devctl(fd, DCMD_PROC_CURTHREAD, &status.tid, sizeof status.tid, 0 ) != EOK ) {
			continue;
		}

		if ( cur_tid == 0 )
			cur_tid = status.tid;

		if (cur_tid_only && (cur_tid != status.tid)) {
			continue;
		} else if ( status.tid == cur_tid ) {
			dprintf(("thread %d is current thread!\n", status.tid ));
			slog_tid( &status, path );
			status.flags |= _DEBUG_FLAG_CURTID;
		}

		// write QNT_CORE_STATUS note
		thread_note.name = QNX_NOTE_NAME;
		thread_note.type = QNT_CORE_STATUS;
		thread_note.datasz = sizeof(status);
		thread_note.data = &status;
		if ( !writenote( &thread_note, fp, &coresize ) )
			goto bailout;

		if(devctl(fd, DCMD_PROC_GETGREG, &greg, sizeof greg, &size) == EOK) {
			// write QNT_CORE_GREG note
			thread_note.name = QNX_NOTE_NAME;
			thread_note.type = QNT_CORE_GREG;
			thread_note.datasz = size;
			thread_note.data = &greg;
			if ( !writenote( &thread_note, fp, &coresize ) )
				goto bailout;
		}

		if(devctl(fd, DCMD_PROC_GETFPREG, &fpreg, sizeof fpreg, &size) == EOK) {
			// write QNT_CORE_FPREG note
			thread_note.name = QNX_NOTE_NAME;
			thread_note.type = QNT_CORE_FPREG;
			thread_note.datasz = size;
			thread_note.data = &fpreg;
			if ( !writenote( &thread_note, fp, &coresize ) )
				goto bailout;
		}
	}

	dump_seek( fp, dataoff );

	for ( j = 0; j < seg; j++ ) {
		if ( lseek( fd, mem[j].vaddr, SEEK_SET ) == -1 )
			goto bailout;
		if ( mem[j].flags & MAP_STACK )
			dump_stack_memory( fd, fp, &mem[j], &coresize );
		else
		  if (!nodumpmem)
			dump_memory( fd, fp, &mem[j], &coresize );
	}

	// Return EOK when accually writing ELF files
	free(mem);
	return EOK;
bailout:
	if ( mapinfos != NULL ) {
		free(mapinfos);
	}
	if ( ldd_infos != NULL ) {
		free(ldd_infos);
	}
	if ( mem != NULL ) {
		free(mem);
	}
	return errno;
}

char *gen_dump_path(char *base)
{
static char corepath[PATH_MAX+1];
static char savepath[PATH_MAX+1];
int n;
	char suf[20];
	
	if (gzlevel == -1) {
		sprintf(suf, "core");
	} else {
		sprintf(suf, "core.gz");
	}
	
	sprintf( corepath, "%s.%s", base, suf );

	if ( sequential_dumps ) {
		for ( n = 1; ; n++ ) {
			sprintf( savepath, "%s.%d.%s", base, n, suf );
			if ( access( savepath, F_OK ) == -1 ) {
				break;
			}
		}
	}
	if ( access( corepath, F_OK ) == EOK ) {
		if ( sequential_dumps ) {
			vprintf(("renaming %s to %s\n", corepath, savepath ));
			if ( rename( corepath, savepath ) == -1 ) {
				vprintf(("rename failed, dumping to %s\n", savepath ));
				return savepath;
			}
		}
		else {
			vprintf(("unlinking old %s\n", corepath ));
			if ( unlink( corepath ) == -1 )
				return NULL;
		}
	}
	vprintf(("dumping to %s\n", corepath ));
	return corepath;
}

int dump(uint32_t nd, pid_t pid, long size ) {
	int							fd;
	int							ret;
	FILE						*fp;
	char						path[PATH_MAX + 1];
	char						buff[PATH_MAX + 1];
	struct _procfs_debug_info	*map = (struct _procfs_debug_info *)buff;
	uid_t						uid = 0;
	gid_t						gid = 0;
	procfs_info					info;

	if(pid == 0) {
		return ENOENT;
	}

	if(ND_NODE_CMP(nd, ND_LOCAL_NODE) && match_pid != -1 && match_pid != pid) {
		return ENXIO;
	}

	sprintf(buff, "/proc/%d/as", pid);
	if ( requested ) {
		if((fd = open(buff, O_RDWR|O_NONBLOCK)) == -1) {
			fprintf(stderr,"dumper: error attaching to process %d - %s\n", pid, strerror(errno) ); 
			return errno;
		}
		if ((ret = devctl(fd, DCMD_PROC_STOP, NULL, 0, 0)) != EOK) {
			vprintf(("failed PROC_STOP %s\n", strerror(ret) ));
			close(fd);
			return ret;
		}
	}
	else {
		if((fd = open(buff, O_RDONLY|O_NONBLOCK)) == -1) {
			return errno;
		}
	}

	if((ret = devctl(fd, DCMD_PROC_MAPDEBUG_BASE, map, sizeof buff, 0)) != EOK) {
		close(fd);
		return ret;
	}

	if((ret = devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0)) == EOK) {
		uid = geteuid();
		if ( uid != 0 && uid != info.uid ) {
			vprintf(("Permission Denied\n"));
			close(fd);
			return EPERM;
		}
		uid = info.euid;
		
		if(uid != info.uid) /* suid binary */
			gid=0;
		else
			gid = info.egid;
	}

	path[0] = '\0';
	if(nd != -1) {
		if(dump_dir) {
			strcpy(path, dump_dir);
		} else {
	  		struct passwd				*pwp;

			if((ret = devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0)) == EOK && (pwp = getpwuid(info.uid))) {
				strcpy(path, pwp->pw_dir);
			} else {
				strcpy(path, "/tmp");
			}

		}

		strcat(path, "/");
	}
	strcat(path, basename(map->path));
	dump_path = gen_dump_path(path);

	if (gzlevel == -1) {
		fp = fopen(dump_path, "w");
	} else {
		fp = (FILE *)gzopen(dump_path, "w");
		if (fp)
		  gzsetparams((gzFile)fp, gzlevel, 0);
	}
	
	if(!fp) {
		perror(dump_path);
		close(fd);
		return errno;
	}
	chown( dump_path, uid, gid );
	chmod( dump_path, world_readable ? 0644 : S_IRUSR|S_IWUSR );
	ret = elfcore(fd, fp, map->path, size);
	
	if (gzlevel == -1) {
		fclose(fp);
	} else {
		gzclose((gzFile)fp);
	}
	close(fd);



	return ret;
}

#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

int dumper_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb) {
	int				status;
	int				nonblock;
	long			size = max_core_size;
	char			*p, *tmp;
	pid_t pid;

	if((status = iofunc_write_verify(ctp, msg, ocb, &nonblock)) != EOK) {
		return status;
	}

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		return EINVAL;
	}

	if(msg->i.nbytes > sizeof ctp->msg - sizeof msg->i) {
		return ENOSPC;
	}

	p = (char *)(&msg->i) + sizeof msg->i;
	p[msg->i.nbytes] = '\0';

	pid = strtol(p, &tmp, 10);

	/* eat whitespace */
	while(*tmp && !ISDIGIT(*tmp))
		tmp++;

	if(*tmp){
		size = strtol(tmp, NULL, 10);
		if(size > max_core_size)
			size = max_core_size;
	}
	
	DeliverNotifies(pid); // send all notifications first, 
                        // since the call to dump might return an error
	if((status = dump(ctp->info.nd, pid, size)) != EOK) {
		return status;
	}

	_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
	return EOK;
}

static void init_memory()
{
	void		*dummy;
	/* Make sure we always have some memory at hand.  */
	mallopt(MALLOC_MEMORY_HOLD, 1);
	dummy = calloc(32 * 1024, 1); /* 32K for RAM */
	free (dummy);
}

static void init_stack()
{
	const int size = 32 * 1024;  /* 32K for stack. */
	void *p = alloca (size);
	memset (p, 0, size);
}

int main(int argc, char *argv[])
{
	int 				c;
	char				*size_suffix;
	pid_t 				process = 0;
	dispatch_t			*dpp;
	resmgr_attr_t		resmgr_attr;
	resmgr_context_t	*ctp;

	umask(0222);

	/* We want some pshysical memory reserved for dumper process. */
	init_memory();

	/* We also want some physical memory for our stack.  */
	init_stack();
	
	while ( (c = getopt( argc, argv, "Dd:p:ns:vmPwtz:" )) != -1 ) {
		switch(c) {
		case 'd':
			dump_dir = optarg;
			break;
		case 'n':
			sequential_dumps = 1;
			break;
		case 'p':
			process = atoi(optarg);
			break;
		case 's':
			max_core_size = strtol(optarg, &size_suffix, 10);
			switch(*size_suffix){
				case 'g':
				case 'G':
					max_core_size *= 1024;
				case 'm':
				case 'M':
					max_core_size *= 1024;
				case 'k':
				case 'K':
					max_core_size *= 1024;
					break;
				case 0:
					break;
				default:
					fprintf(stderr, "Warning: suffix '%c' unrecognized\n", *size_suffix);
			}
			break;
		case 'v':
			verbose = 1;
			break;
		case 'm':
			nodumpmem = 1;
			break;
		case 'P':
			nodumpphys = 0;
			break;
		case 't':
			cur_tid_only = 1;
			break;
		case 'w':
			world_readable = 1;
			break;
	    case 'z':
			gzlevel = strtol(optarg, &size_suffix, 10);
			if (gzlevel < 1 || gzlevel > 9) {
				fprintf(stderr, "Compress level must between 1 and 9.\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
	}

	if ( process ) {
		requested = 1;
		return dump( ND_LOCAL_NODE, process, max_core_size );
	}

	if((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch handle.\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&resmgr_attr, 0, sizeof resmgr_attr);
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect, _RESMGR_IO_NFUNCS, &io);
	io.write = dumper_write;
	io.devctl = dumper_devctl; 
	io.close_ocb = dumper_close_ocb; 

	iofunc_attr_init(&attr, S_IFNAM | 0600, 0, 0);

	if ( -1 == resmgr_attach(dpp, &resmgr_attr, "/proc/dumper", _FTYPE_DUMPER, 0, &connect, &io, &attr)) {
		fprintf( stderr, "%s: Couldn't attach as /proc/dumper: %s\n", argv[0], strerror(errno) );
		exit(EXIT_FAILURE);
	}

	if((ctp = resmgr_context_alloc(dpp)) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch handle.\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	if ( -1 == procmgr_daemon(EXIT_SUCCESS, verbose ? 
		PROCMGR_DAEMON_NODEVNULL | PROCMGR_DAEMON_KEEPUMASK :
		PROCMGR_DAEMON_KEEPUMASK) ) {
		fprintf(stderr, "%s: Couldn't become daemon.\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	while(1) {
		if((ctp = resmgr_block(ctp)) == NULL) {
			fprintf(stderr, "block error\n");
			exit(EXIT_FAILURE);
		}
		resmgr_handler(ctp);
	}
	return EXIT_SUCCESS;
}
