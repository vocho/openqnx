/*
 *  mig4nto.h - QNX 4 to QNX Neutrino migration header
 */
#ifndef __MIG4NTO_H_INCLUDED
#define __MIG4NTO_H_INCLUDED

#include <sys/types.h>
#include <sys/neutrino.h>
#include <sys/times.h>

#define MAXIMUM_PROXIES		2000
#define GET_PROXYNUM(pid)	((pid >> 16) & 0x07ff)

#define PROXY_CODE 			1		/* Secret code that is inserted into the 
									   code field of the pulse to indicate to 
									   the MsgReceive*() that this pulse 
									   represents a proxy */
#define MIG4NTO_UNSUPP		-1		/* migration functions do not have a
									   value for this */
#define far  
#define FP_SEG(x)			((int) x)
#define my_ds()				(0)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * QNX 4 signal.h miscellaneous
 */

#define SIGDEV				28


/*
 * QNX 4 sys/types.h miscellaneous
 */

#ifndef _GID_T_DEFINED_
 typedef short mgid_t;          /* Used for group in messages */
 #define _GID_T_DEFINED_
#endif

typedef short mpid_t;           /* Used for process & group IDs in messages */

#if defined(_QNX_SOURCE) || !defined(NO_EXT_KEYS)
typedef long        nid_t;      /* Used for network IDs         */
#endif

/*
 * magic is an area containing common globals that are used throughout
 */

typedef struct {
	int		ipc_chid;		/* channel id which the process receives on     */
	int		procmgr_fd;		/* fd for communicating with mig4nto-procmgr    */
	nid_t	nid;			/* local node id, obtained from mig4nto-procmgr */
	int		trigger_coid;	/* coid for sending trigger messages to			*/
} magic_t;

/*
 * QNX 4 sys/kernel.h miscellaneous
 */

#define PROC_PID			1

/* 
 * Note that STATE_READY under QNX 4 could be either STATE_READY or
 * STATE_RUNNING under QNX Neutrino.
 */

#define STATE_SEND_BLOCKED		STATE_SEND
#define STATE_RECEIVE_BLOCKED	STATE_RECEIVE
#define STATE_REPLY_BLOCKED		STATE_REPLY
#define STATE_HELD				STATE_STOPPED
#define STATE_SEM_BLOCKED		STATE_SEM


/* 
 * QNX 4 sys/name.h miscellaneous
 */

struct _nameinfo {
	nid_t			nid;
	pid_t			pid;	/* See note above */
	short unsigned	zero1,
					zero2[4];
	char            name[33];
};


/*
 * QNX 4 sys/sendmx.h miscellaneous
 */
 
struct _mxfer_entry {
	void			*iov_base;
	size_t			iov_len;
};

#define _setmx(_iov, _addr, _len)  ((_iov)->iov_base = (void *)(_addr), (_iov)->iov_len = (_len))


/*
 * QNX 4 sys/dev.h miscellaneous
 */

#define MAX_TTY_NAME 32

/*
 * members marked NA are not available through the dev_info() migration
 * function
 */
struct _dev_info_entry {
	short int       tty;                /* NA TTY number of this device    */
	nid_t           nid;                /* Logical network ID              */
	short unsigned  driver_pid;         /* Process ID of the driver        */
										/* task that controls device       */
	short unsigned  driver_pid_spare;   /* NA                              */
	short unsigned  nature;             /* NA Nature or type of the device */
	short unsigned  attributes;         /* NA Character attr. supported    */
	short unsigned  capabilities;       /* NA capabilities of this device  */
	char            driver_type[16];    /* Sym. name describing device     */
	char            tty_name[MAX_TTY_NAME];     /* A path to open device   */
	short unsigned  unit;               /* Unit number of this device      */  
										/* /dev/con2 has a unit of 2       */
	short unsigned  open_count;         /* NA Number of opens per device   */
	mpid_t          pgrp;               /* NA Process group                */
	short unsigned  pgrp_spare;         /* NA                              */
	mpid_t          session;            /* NA Session ID                   */
	short unsigned  session_spare;      /* NA                              */
	short unsigned  flags;              /* NA flag bits, see below         */
	short unsigned  major;              /* MA major device number          */
};

/*
 * Events recognized by dev_arm() and dev_state()
 * Those with an NA are not handled by the dev_arm() and dev_state()
 * migration functions
 */

#define _DEV_EVENT_INPUT        0x0001  
#define _DEV_EVENT_DRAIN        0x0002
#define _DEV_EVENT_LOGIN        0x0004	/* NA */
#define _DEV_EVENT_EXRDY        0x0008
#define _DEV_EVENT_OUTPUT       0x0010
#define _DEV_EVENT_TXRDY        0x0020	/* NA */
#define _DEV_EVENT_RXRDY        0x0040	/* NA */
#define _DEV_EVENT_HANGUP       0x0080	/* NA */
#define _DEV_EVENT_INTR         0x0100	/* NA */
#define _DEV_EVENT_WINCH        0x0200	/* NA */

/* Special "proxy" value to disarm pending armed proxies */
#define _DEV_DISARM                     (-1)

/* Modes recognized by dev_mode() */
#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST)


/*
 * QNX 4 sys/disk.h miscellaneous
 */

#define _DRIVER_NAME_LEN   12

/*
 *  Disk types
 *  Types 10 up are available for user written drivers
 *  that don't exactly fit any of the standard types,
 *  however they are assumed to be fixed (non-removable) drive
 *  and cached blocks will not timeout.
 *  Types below 10 are reserved (even if not explicitly)
 *  for known types or expansion.
 *  If you have a removable disk which can be locked in the drive,
 *  call it _REMOVABLE (type 4).
 *  If the removable disk can be removed at any time, call it a _FLOPPY.
 */
#define _UNMOUNTED  0
#define _FLOPPY     1   /*  Cache times out after 2 seconds of inactivity   */
#define _HARD       2
#define _RAMDISK    3   /*  Built-in, not via a driver                      */
#define _REMOVABLE  4   /*  A removable hard disk, e.g. Iomega Bernoulli    */
#define _TAPE       5   /*  Streaming tape: it has no known size, so all    */
						/*  writes are synchronous.  I/O errors are assumed */
						/*  to be the end-of-tape.  Seeking is an error.    */
#define _CDROM      6
#define _WORM       7
#define _UNKNOWN    8   /*  catchall for things we don't know               */
#define _PRINTER    9   /*  SCSI (?) printer                                */
#define _COMMS      10  /*  SCSI (?) communications device                  */
#define _PROCESSOR  11  /*  SCSI (?) processor device                       */
#define _SCANNER    12  /*  SCSI (?) scanner                                */
#define _MEDIA_CHG  13  /*  SCSI (?) media changer                          */
#define _OPTICAL    14  /*  SCSI (?) optical memory device                  */


/*
 *  Info about a QNX drive.
 *
 *  NOTE:  It is possible that cylinders, heads and track_sectors may be 0,
 *          in which case only disk_sectors describes the physical disk size.
 */
struct _disk_entry {
	unsigned long   blk_offset,
					num_sectors, /* Num sectors in logical drive (partition) */
					disk_sectors;/* Num sectors on physical disk    */
	unsigned short  cylinders,
					heads,
					track_sectors;
	unsigned char   disk_type,
					disk_drv;           /*  Drive number as known to driver */
	long            reserved[3];
	unsigned char   driver_name[_DRIVER_NAME_LEN];
};


/*
 * QNX 4 sys/osinfo.h miscellaneous
 */

/*
 * members marked NA are not available through the qnx_osinfo() migration
 * function
 */

struct _osinfo {
	short unsigned  cpu_speed,  	/* NA A PC is around 960             	*/
					num_procs,  	/* NA Max number of processes          	*/
					tick_size,  	/* Tick size, in microsecond units  	*/
					version,    	/* Version number of QNX * 100      	*/
					timesel,    	/* NA Segment in which the time is kept	*/
					totmemk,    	/* Total memory in the system       	*/
					freememk;   	/* Free memory in the system 	       	*/

	char            primary_monitor,    /* NA monitor type     				*/
					secondary_monitor;  /* NA monitor type     				*/
	short unsigned  machinesel;     /* NA */
	unsigned short  disksel;        /* NA pointer to _diskinfo structure 	*/
	unsigned long   diskoff;        /* NA */
	unsigned long   ssinfo_offset;  /* NA */
	short unsigned  ssinfo_sel,     /* NA */
					microkernel_size;	/* NA */
	char            release,        /* NA Release letter               		*/
					zero2;
	long            sflags;         /* System flags, as defined below   	*/
	nid_t           nodename;       /* Logical node number of this cpu
									   Note that this is the QNX 4 network
									   id as obtained by sending to the
									   migration name manager process and
									   not a QNX Neutrino node descriptor 	*/
	long            cpu,            /* Processor type 486, 586, ...     	*/
					fpu;            /* NA Floating-point unit 287, 387,... 	*/
	char            machine[16],    /* Machine name                     	*/
					bootsrc,        /* NA 'F'loppy 'H'ard disk 'N'etwork   	*/
					zero3[9];
	short unsigned  num_names,      /* NA Maximum number of names      		*/
					num_timers,     /* NA Maximum number of timers     		*/
					num_sessions,   /* NA Maximum number of sessions   		*/
					num_handlers,   /* NA Maximum number of interrupt handlers */
					reserve64k,     /* NA Relocation offset 				*/
					num_semaphores, /* MA */
					prefix_len,     /* NA */
					zero4[4],
					max_nodes,      /* NA Number of nodes you are licensed for */
					proc_freemem,   /* NA */
					cpu_loadmask,   /* NA */
					fd_freemem,     /* NA */
					ldt_freemem,    /* NA */
					num_fds[3],     /* NA Number of fd's					*/
					pidmask,        /* NA Process ID bit mask				*/
					name_freemem;	/* NA */
	long unsigned   top_of_mem;		/* NA */
	long unsigned   freepmem;		/* Free physical memory					*/
	long unsigned   freevmem;       /* NA */
	long unsigned   totpmem;        /* Total physical memory				*/
	long unsigned   totvmem;        /* NA */
	long unsigned   cpu_features;   /* Contains CPU speed in Mhz			*/
	short unsigned  zero5[13];
};

/* System flag definitions */

#define _PSF_PROTECTED          0x0001	/* Running in protected mode.	*/
#define _PSF_NDP_INSTALLED      0x0002 	/* An 80x87 is installed.		*/
#define _PSF_EMULATOR_INSTALLED 0x000c 	/* An 80x87 emulator is running	*/
#define _PSF_32BIT_KERNEL       0x1000 	/* 32-bit kernel is being used.	*/
#define _PSF_PCI_BIOS           0x2000 	/* A PCI BIOS is present.		*/


/*
 * QNX 4 process.h miscellaneous
 */

/*
 * struct _proc_spawn was in sys/proc_msg.h
 * note that the qnx_spawn() function ignores the parameter that uses
 * the struct _proc_spawn type
 */
 
struct _proc_spawn {                    /*  Same structure as _PROC_EXEC    */
	short unsigned  type;
	char            priority,
					algorithm;
	short unsigned  zero1;
	short unsigned  flags;
	char            zero2,
					ctfd;
	char            stdfds[10];
	short unsigned  argc,
					envc;
	char            data[1];            /*  Can be up to 60K in usr program */
};

/*
 *  Spawn flags passed to qnx_spawn()
 *  These are unique to QNX 4
 */

#define _SPAWN_DEBUG        0x0001
#define _SPAWN_HOLD         0x0002
#define _SPAWN_BGROUND      0x0004
#define _SPAWN_NEWPGRP      0x0008
#define _SPAWN_TCSETPGRP    0x0010
#define _SPAWN_NOZOMBIE     0x0020
#define _SPAWN_SIGCLR       0x0080
#define _SPAWN_SETSID       0x0100
#define _SPAWN_NOHUP        0x0200


/*
 * QNX 4 sys/psinfo.h miscellaneous
 */

/*
 * members marked NA are not available through the qnx_psinfo() migration
 * function
 *
 * the pid and related fields were all short int in QNX 4,
 * they have been changed to pid_t which in QNX Neutrino is 32 bit
 */

struct _psinfo {
	pid_t       	pid,
					pid_zero,
					blocked_on,
					blocked_on_zero,
					pid_group,
					pid_group_zero;
	long        	flags;     		/* NA */
	short int   	rgid,
					ruid,
					egid,
					euid;
	long        	sp_reg;			/* NA */
	short unsigned  ss_reg;			/* NA */
	long            magic_off;		/* NA */
	short unsigned  magic_sel,		/* NA */
					ldt,			/* NA */
					umask;
	long            signal_ignore,
					signal_pending,	/* NA */
					signal_mask,
					signal_off;		/* NA */
	short unsigned  signal_sel;		/* NA */
	char            state,
					zero0,
					zero0a,
					priority,
					max_priority,
					sched_algorithm;
	short unsigned  sid;
	nid_t           sid_nid;		/* NA */
	short unsigned  zero1[5];
	union {
		struct {
			pid_t       	father,
							father_zero,
							son,
							son_zero, 
							brother,
							brother_zero,
							debugger,		/* NA */
							debugger_zero,
							mpass_pid,		/* NA */
							mpass_pid_zero;
			short unsigned  mpass_sel,		/* NA */
							mpass_flags;	/* NA */

			char            name[100];
			short unsigned  links;			/* NA */
			time_t          file_time;		/* NA */

			short unsigned  nselectors;		/* NA */
			time_t          start_time;		/* NA */
			struct tms      times;
			short unsigned  mxcount;		/* NA */
			short unsigned  zero2[7];
		} proc;
		struct {
			pid_t       local_pid,          /* NA */
						local_pid_zero,
						remote_pid,         /* NA */
						remote_pid_zero,
						remote_vid,         /* NA */
						remote_vid_zero;
			nid_t       remote_nid;         /* NA */
			short unsigned  vidseg,         /* NA */
							links;          /* NA */
			char            substate,       /* NA */
							zero_v1;
			short unsigned  zero2[49];
		} vproc;
		struct {
			short unsigned           count, /* NA */
									 zero2[50];
		} mproc;
	} un;
	short unsigned zero3[12];
};

/*
 * struct _seginfo was in sys/seginfo.h
 * note that the qnx_psinfo() function ignores the parameter that uses
 * the struct _seginfo type
 */

struct _seginfo {
	short unsigned  selector,
					flags;
	long            addr,
					nbytes;
};


extern int mig4nto_init(void);
extern int ipc_init(void);
extern void qnx_hint_table_init(void);
extern int name_init(void);

extern pid_t Receivemx(pid_t pid, unsigned parts, struct _mxfer_entry *msgmx);
extern pid_t Receive(pid_t pid, void *msg, unsigned nbytes);
extern int Send(pid_t pid, void *smsg, void *rsmg, unsigned snbytes, 
		 unsigned rnbytes);
extern int Sendmx(pid_t pid, unsigned sparts, unsigned rparts, 
		   struct _mxfer_entry *smsgmx, struct _mxfer_entry *rmsgmx);
extern int Reply(pid_t pid, void *msg, unsigned nbytes);
extern int Replymx(pid_t pid, unsigned parts, struct _mxfer_entry *msgmx);

extern unsigned Readmsg(pid_t pid, unsigned offset, void *msg, unsigned nbytes);
extern unsigned Readmsgmx(pid_t pid, unsigned offset, unsigned parts, 
	struct _mxfer_entry *msgmx);

extern unsigned Writemsg(pid_t pid, unsigned offset, void *msg, unsigned nbytes);
extern unsigned Writemsgmx(pid_t pid, unsigned offset, unsigned parts,
	struct _mxfer_entry *msgmx);

extern pid_t Trigger(pid_t pid);
extern pid_t qnx_proxy_attach(pid_t pid, const void *data, int nbytes, int priority);
extern int qnx_proxy_detach(pid_t pid);

extern int qnx_name_attach(nid_t nid, char *name);
extern int qnx_name_detach(nid_t nid, int name_id);
extern int qnx_name_locate(nid_t nid, char *name, unsigned size, unsigned *copies);
extern int qnx_name_query(pid_t proc_pid, int name_id, struct _nameinfo *buffer);
extern int qnx_vc_name_attach(nid_t nid, unsigned length, char *name);
extern int qnx_vc_attach(nid_t nid, pid_t pid, unsigned length, int flags);
extern int qnx_vc_detach(pid_t vid);

extern int dev_info(int __fd, struct _dev_info_entry *__info);
extern int dev_fdinfo(pid_t __server, pid_t __pid,
					int __fd, struct _dev_info_entry *__info);
extern int dev_insert_chars(int __fd, int __n, const char *__buf);
extern unsigned dev_state(int __fd, unsigned __bits, unsigned __mask);
extern unsigned dev_mode(int __fd, unsigned __mask, unsigned __mode);
extern int dev_arm(int __fd, pid_t __proxy, unsigned __events);
extern int dev_read(int __fd, void *__buf, unsigned __nbytes,
				  unsigned __minimum, unsigned __time, unsigned __timeout,
				  pid_t __proxy, int *__triggered);
extern int dev_ischars(int __fd);
extern int dev_size(int __fd, int __set_rows, int __set_columns,
				  int *__rows, int *__cols);
extern int dev_readex(int __fd, char *__buf, int __nbytes);

extern int disk_get_entry(int __fd, struct _disk_entry *__d_bfr);
extern int disk_space(int __fd, long *__free_blocks, long *__tot_blocks);
extern int block_read(int __fd, long __block, unsigned __nblock, void *__buf);
extern int block_write(int __fd, long __block, unsigned __nblock,
					 const void *__buf );
extern int fsys_get_mount_dev(const char *path, char *device);
extern int fsys_get_mount_pt(const char *device, char *directory);

extern nid_t getnid(void);

extern int qnx_hint_attach(unsigned intnum, pid_t (* handler)(void), unsigned ds);
extern int qnx_hint_detach(int id);

extern int qnx_osinfo(nid_t, struct _osinfo *);

extern pid_t qnx_psinfo(pid_t, pid_t, struct _psinfo *, unsigned, struct _seginfo *__segdata);

extern pid_t qnx_spawn(int __mode, struct _proc_spawn *__msgbuf, nid_t __node,
				 int __prio, int __sched_algo, int __flags,
				 const char *__path, char **__argv, char **__envp,
				 char *__iov, int __ctfd);

extern void Yield(void);

#ifdef __cplusplus
};
#endif

#endif
