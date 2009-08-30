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



#include "pidin.h"

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <gulliver.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <sys/netmgr.h>
#include <sys/netmsg.h>
#include <sys/sysmgr.h>
#include <sys/sysmsg.h>
#include <sys/conf.h>
#include <sched.h>
#include <sys/sched_aps.h>
#include <sys/procfs.h>
#include <sys/mman.h>


char		   *opts = "d:klM:n:f:F:ho:p:P:";

void dspinfo(pid_t pid, char *pidname, int flags, const char *fmt, const char *mem_format);
void piddspinfo(int fd, char *pidname, pid_t pid, int flags, const char *fmt, const char *mem_format);
int  find_netdir(char *nodename, char *buf, size_t bufsize, int flag);
char* tail(char *name);

char		   *node = "";

static char	   *format = "%a %b %N %p %J %B";
static int		exit_on_warning = 0;

static struct shorthand {
	char		   *handle;
	char		   *format;
	char		   *mem_format;
} shorthands[] =
{
	// these need to be in reverse lexigraphical order for lookup to work
	{
		"users", "%a %N %U %V %W %X %Y %Z"
	},							// users
	 {
		"ttimes", "%a %b %N %J %t %y %z"
	},							// ttimes
	 {
		"times", "%a %N %L %t %u %v %w %x"
	},							// times
	 {
		"timers", "%a %b %N %R"
	},							// pidin timers
	 {
		"threads", "%a %N %h %J %B"
	},							// pidin threads
	 {
		"syspage", 0, "Page"
	},							// syspage
	 {
		"signals", "%a %b %N %S %s"
	},							// signals
	 {
	 	"session","%L %a %P %e %N"
	},							// session
	 {
	 	"sched", "%a %b %N %p %l %H %J",
	},							// scheduling
	 {
		"rmasks", "%a %b %N %i"
	},							// runmasks
	 {
		"regs", "%a %b %N %r"
	},							// registers
	 {
		"rc", 0, "rc"
	},							// remote connections
	 {
		"pmem", "%a %b %N %p %J %c %d %m", 
	},							// memory (process only)
	 {
		"net", 0, "net"
	},							// node info on network
	 {
		"memory", "%a %b %N %p %J     %c %d %m", 
		"            %M @%> %? %< %="
	},							// memory
	 {
		"mapinfo", "%a %b %N %p %J     %c %d %m", 
		"             %: @%> %; %< %=  %@"
	},							// mapinfo
	 {
		"irqs", "%a %b %N %Q"
	},							// interrupts
	 {
		"channels", "%a %b %N %["
	},							// channels
	 {
		"info", 0, "system"
	},							// flags
	 {
		"flags", "%a %N %f"
	},							// flags
	 {
		"fds", "%a %N %o"
	},							// fd
	 {
		"family", "%a %N %L %P %e %G %C"
	},							// family
	 {
	 	"extsched", NULL, "ExtSchedulers"
	},							// ext_schedulers
	 {
		"environment", "%a %N %E"
	},							// environment
#ifndef NO_BACKTRACE_LIB
	 {
		"backtrace", "%I %q"
	},
#endif
	 {
		"arguments", "%a %A"
	},							// arguments
	 {
		0, 0
	}
};

static char	   *normalize_format(const char *f);
static struct format *get_format(FILE * fp, const char **f);
static struct shorthand *lookup_shorthand(const char *f);
static void validate_formats();

void dspsys(const char *type, const char *argv);

int 
main(int argc, char *argv[])
{
	int				c;
	int				p = 1;
	char			*pidname = NULL;
	int				flags = 0;
	const char		*f;
	int				loopdelay = 10;
	int				loop = 0;
	int				pri = 0;
	const char *			mem = 0;

#ifndef NO_EASTER_EGGS
	if(argc == 2) {
		//
		// By long QSSL tradition, the QNX process info utility must
		// have easter eggs.
		//
		if(strcmp(argv[1], "tsk") == 0) {
			printf("Boy, you've been using QNX for a loooong time!\n");
			exit(0);
		}
		if(strcmp(argv[1], "toomuch") == 0) {
			printf("Oh, wow. What OS are you tripping with man?\n");
			exit(0);
		}
		if(strcmp(argv[1], "sin") == 0) {
			printf("Since we're without sin, we're casting the first stone.\n");
			exit(0);
		}
		if(strcmp(argv[1], "english") == 0) {
			printf("Can't even spell pidgin english properly?\n");
			exit(0);
		}
		if(strcmp(argv[1], "pie") == 0) {
			printf("Mmmm, pudding pie. Them's good eating.\n");
			exit(0);
		}
	}
#endif

	while ((c = getopt(argc, argv, opts)) != -1)
		switch (c) {
		case 'd':
			loopdelay = strtoul(optarg, 0, 0);
			break;

		case 'h':
			execlp("use", "use", argv[0], (char*)NULL);
			perror("Unable to start 'use' to display help");
			exit(EXIT_FAILURE);
			break;

		case 'f':
			format = normalize_format(optarg);
			if(format == NULL) {
				error_exit(1, "\nno memory for format\n");
			}
			break;

		case 'F':
			format = optarg;
			break;

		case 'k':
			exit_on_warning = 1;
			break;

		case 'l':
			loop++;
			break;

		case 'M':
			mem = optarg;
			break;

		case 'o':
			pri = strtoul(optarg, 0, 0);
			break;

		case 'p':
			flags = DONT_RECURSE;
			/*
			 * fall through 
			 */
		case 'P':
			if(*optarg >= '0' && *optarg <= '9') {
				p = atoi(optarg);
			} else {
				pidname = optarg;
			}
			break;

		case 'n':
			node = optarg;
			break;

		case '?':
			//getopt already reported the error
			exit(1);

		default:
			error_exit(1, "invalid option '%c'\n", c);
			break;
		}

	// simple check to validate formats */
	validate_formats();

	if (pri) {
		struct sched_param p;
		p.sched_priority = pri;
		if (SchedSet(0, 0, SCHED_NOCHANGE, &p) == -1)
			error_exit(1, "sched_setprio: %s\n", strerror(errno));
	}

	do {
		do {
			const char		   *g;
			const char		   *mf = 0;
			struct shorthand	   *s = 0;

			if (optind < argc &&
			   !(s = lookup_shorthand(argv[optind]))) {
				error_exit(1, "%s invalid shorthand\n", argv[optind]);
			}

			if (!s) {
				f = g = format;
			} else if(s->format) {
				f = g = s->format;
				mf = s->mem_format;
			} else {
				dspsys(s->mem_format, argv[optind]);
				if (loop)
        				delay(loopdelay * 100);
				continue;
			}
			if (mem) {
				mf = mem;
			}

			while (*f) {
				struct format  *format;

				if (!(format = get_format(stdout, &f)))
					break;
				if (format->flags & THREAD_UNIQUE)
					flags |= DO_THREADS;
				if (format->flags & MULTI_LINE)
					continue;
				format_title_string(stdout, format, format->title);
			}
			fputc('\n', stdout);

			dspinfo(p, pidname, flags, g, mf);
			if (loop)
				delay(loopdelay * 100);
		} while (loop);
	} while (++optind < argc);

	return EXIT_SUCCESS;
}

/* node = "", local node
 * flag = 0, find the root of "node" (default: /net/node)
 * flag = 1, find the network mountpoint of node (default: /net/node/net)
 */
int find_netdir(char *node, char *buf, size_t bufsize, int flag)
{
	int len, fd;
	
	if (*node == 0) {
		*buf = '/';
		*(buf + 1) = 0;
		len = 1;
		if (flag)
		  len = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_NAME_HIDE, ND_LOCAL_NODE, buf, bufsize);
		return len;
	}
	
	if (*node == '/') {
		len = snprintf(buf, bufsize, "%s/", node);
	} else {
		if ((len = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_NAME_HIDE, ND_LOCAL_NODE, buf, bufsize)) == -1)
		  return -1;
		len--;
		len += snprintf(&buf[len], bufsize - len, "%s/", node);
	}
	
	if (flag) {
		char *netmgr;
		netmgr_ndtostr_t msg;
		
		if (!(netmgr = alloca(len + 12)))
		  return -1;

		sprintf(netmgr, "%s/dev/netmgr", buf);
		
		if ((fd = open(netmgr, O_RDONLY)) == -1)
		  return -1;
		
		msg.i.hdr.type = _IO_MSG;
		msg.i.hdr.combine_len = sizeof msg.i;
		msg.i.hdr.mgrid = _IOMGR_NETMGR;
		msg.i.hdr.subtype = _NETMGR_NDTOSTR;
		msg.i.len = bufsize - len;
		msg.i.flags = ND2S_DIR_SHOW | ND2S_NAME_HIDE;
		msg.i.nd = ND_LOCAL_NODE;
		if (MsgSend(fd, &msg.i, sizeof(msg.i), &buf[len], bufsize - len) == -1)
		  return -1;
		close(fd);
		len = strlen(buf);
	}
	return len;
}

char* tail(char *name) {
	register char *p;
	
	p = name + strlen(name);
	while(p > name  &&  *p != '/' && *p != '\\' )	--p;

	return( (*p == '/' || *p == '\\') ? p + 1 : p );
}

void 
dsprc(DIR *dp)
{
	int  len, fd, coid, nd;
	struct _server_info qinfo, sinfo;
	char noderoot[PATH_MAX + 1], nodename[20];

	if ((len = find_netdir(node, noderoot, PATH_MAX, 0)) == -1)
	  error_exit(1, "can't find node %s: %s\n", node, strerror(errno));

	strcat(noderoot, "dev/netmgr");
	if ((fd = open(noderoot, O_RDONLY)) == -1)
	  error_exit(1, "can't open %s: %s\n", noderoot, strerror(errno));
	
	if (ConnectServerInfo(0, fd, &qinfo) == -1)
	  error_exit(1, "can't find server info for %s: %s\n", noderoot, strerror(errno));
	close(fd);
	
	printf(" From             Pid         Arguments\n");
	for (coid = 0; (coid = ConnectServerInfo(qinfo.pid, coid, &sinfo)) >= 0; coid++)
	{
		if (sinfo.chid != -1)
		  continue;

		if ((nd = netmgr_remote_nd(qinfo.nd, sinfo.nd)) == -1 ||
			(len = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_NAME_SHOW | ND2S_LOCAL_STR | ND2S_QOS_HIDE,
								  nd, noderoot, PATH_MAX)) == -1)
		  continue;
		
		nodename[16] = 0;
		netmgr_ndtostr(ND2S_DIR_HIDE | ND2S_LOCAL_STR, nd, nodename, 15);
		snprintf(&noderoot[len - 1], PATH_MAX - len, "proc/%d/as", sinfo.pid);
		if ((fd = open(noderoot, O_RDWR)) == -1)
		  continue;
		
		printf("%-15s   %-10d  ", nodename, sinfo.pid);
		piddspinfo(fd, 0, sinfo.pid, 0, "%A", 0);
		close(fd);
	}
	return;
}


void dspsched(void)
{
struct sched_query			query;

	if (!SchedCtl(SCHED_QUERY_SCHED_EXT, &query, sizeof(query))) {
		switch (query.extsched) {
		case SCHED_EXT_APS: {
		sched_aps_info				aps_info;
		sched_aps_partition_info	aps_pinfo;
		sched_aps_partition_stats	*aps_pstats;
		uint64_t					round[2];
		int							n;
		char						notify[32], security[16];
		APS_INIT_DATA(&aps_info);
		APS_INIT_DATA(&aps_pinfo);
		

			if (SchedCtl(SCHED_APS_QUERY_PARMS, &aps_info, sizeof(aps_info)))
				error_exit(!0, "Unable to query APS - %s\n", strerror(errno));
			if ((aps_pstats = alloca(aps_info.num_partitions * sizeof(sched_aps_partition_stats))) == NULL)
				error_exit(!0, "Unable to query APS statistics - %s\n", strerror(ENOMEM));

			aps_pstats=(sched_aps_partition_stats*)calloc(aps_info.num_partitions, sizeof(sched_aps_partition_stats));
			if (!aps_pstats) error_exit(!0, "Unable to allocate memory\n", strerror(ENOMEM)); 
			
			APS_INIT_DATA(&aps_pstats[0]); 
			aps_pstats->id = APS_SYSTEM_PARTITION_ID;
			if (SchedCtl(SCHED_APS_PARTITION_STATS, aps_pstats, aps_info.num_partitions * sizeof(sched_aps_partition_stats)))
				error_exit(!0, "Unable to query APS statistics - %s\n", strerror(errno));
			round[0] = aps_info.windowsize_cycles / 2 / 100, round[1] = aps_info.cycles_per_ms / 2;
			printf("APS scheduler");
			printf(", %d partitions", aps_info.num_partitions);
			printf(", window %d ms", (int)(aps_info.windowsize_cycles / aps_info.cycles_per_ms));
			if (aps_info.sec_flags == SCHED_APS_SEC_OFF)
				strcpy(security, "off");
			else if (aps_info.sec_flags & SCHED_APS_SEC_PARTITIONS_LOCKED)
				strcpy(security, "LOCKED");
			else if (aps_info.sec_flags == SCHED_APS_SEC_BASIC)
				strcpy(security, "basic");
			else if (aps_info.sec_flags == SCHED_APS_SEC_FLEXIBLE)
				strcpy(security, "flexible");
			else if (aps_info.sec_flags == SCHED_APS_SEC_RECOMMENDED)
				strcpy(security, "recommended");
			else
				sprintf(security, "bits %04X", aps_info.sec_flags);
			printf(", security %s", security);
			printf("\n");
			for (n = 0; n < aps_info.num_partitions; ++n) {
				if (SchedCtl(SCHED_APS_QUERY_PARTITION, (aps_pinfo.id = n, &aps_pinfo), sizeof(aps_pinfo)))
					error_exit(!0, "Unable to query APS partition - %s\n", strerror(errno));
				sprintf(notify, (aps_pinfo.notify_pid != -1) ? "%d/%d" : "", aps_pinfo.notify_pid, aps_pinfo.notify_tid);
				printf("%-*.*s   %3d%% (%3d%%)   %3dms (%3dms)%c   %s\n",
					APS_PARTITION_NAME_LENGTH, APS_PARTITION_NAME_LENGTH, aps_pinfo.name,
					aps_pinfo.budget_percent, (int)((aps_pstats[n].run_time_cycles + round[0]) * 100 / aps_info.windowsize_cycles),
					(int)(aps_pinfo.critical_budget_cycles / aps_info.cycles_per_ms), (int)((aps_pstats[n].critical_time_cycles + round[1]) / aps_info.cycles_per_ms),
					(aps_pstats[n].stats_flags & SCHED_APS_PSTATS_IS_BANKRUPT_NOW) ? '!' : ' ',
					notify);
			}
			} break;
		}
	}
}


int confstr_get(int fd, int name, unsigned int *value, char *str) 
{
	/* Copied from confstr lib file with added fd allowing
	 * retrieval of information from existing connection. */
	sys_conf_t		msg;
	iov_t			iov[2];
	
	msg.i.type = _SYS_CONF;
	msg.i.subtype = _SYS_SUB_GET;
	msg.i.cmd = _CONF_STR;
	msg.i.name = name;
	msg.i.value = *value;
	
	SETIOV(iov + 0, &msg.o, sizeof(msg.o));
	SETIOV(iov + 1, str, *value);

	if (MsgSendvnc(fd, (iov_t *)&msg.i, -sizeof (msg.i), iov, 2) == -1) {
		return(-1);
	}
	*value = msg.o.value;
	
	return (msg.o.match);
}

typedef struct thread_entry_ thread_entry;
typedef struct process_entry_ process_entry;

/**
 * int do_process(char * node, pid_t pid, process_entry * pe_p)
 *
 * Scan the process for number of threads.  If this thread is a zombie,
 * then return 0 and fail, otherwise, return 1 (success)
 */
int do_process(char * node, pid_t pid, process_entry * pe_p)
{
	int fd;
	char buff[512];
	
	if (pe_p == NULL) {
		return NULL;
	}
	
	pe_p->n_threads = 0;

	if (node == 0 || node[0] == 0) {
		snprintf(buff, sizeof(buff), "/proc/%d", pid);
	} else {
		snprintf(buff, sizeof(buff), "/net/%s/proc/%d", node, pid);
	}

	if ((fd = open(buff, O_RDONLY)) == -1) {
		return 0;
	}

	if (devctl(fd, DCMD_PROC_INFO, &pe_p->info, sizeof pe_p->info, 0) != -1) {
		int lasttid, tid;
		if (pe_p->info.flags & _NTO_PF_ZOMBIE) {
			close(fd);
			return 0;
		} else {
			
			for (lasttid = tid = 1; ; lasttid = ++tid) {
				thread_entry te_p;
				memset(&te_p, 0, sizeof(thread_entry));
				te_p.status.tid = tid;
				if (devctl(fd, DCMD_PROC_TIDSTATUS, &te_p.status, sizeof(te_p.status), 0) != EOK) {
					break;
				}
				tid = te_p.status.tid;
				if (tid < lasttid) {
					break;
				}
				pe_p->n_threads++;
			}
		}
	}
	close(fd);
	return 1;
}

/**
 * int get_node_procs (char * node, proc_info_t *tree_p)
 *
 * Scan all processes, and fill in proc_info_t with # of processes
 * and total number of threads.  This is not a completely trivial
 * operation, as it takes a bit to do
 */
int get_node_procs (char * node, proc_info_t *tree_p)
{
    DIR			*dir;
	char		fname[PATH_MAX];

	if (!tree_p){
		return NULL;
	}
	tree_p->totalprocs = 0;
	tree_p->totalthreads = 0;

	if (node == NULL || node[0] == '\0') {
		dir = opendir("/proc");
	} else {
		snprintf(fname, sizeof(fname), "/net/%s/proc", node);
		dir = opendir(fname);
	}
	
	if (dir == NULL) {
		fprintf(stderr, "Unable to open %s\n", fname);
		return 0;
	} else {
		struct dirent * dirent;
		/* The only value that matters gets reset */
		process_entry pe_p;
		while ((dirent = readdir(dir)) != NULL) {
			if (isdigit(dirent->d_name[0])) {
				if (do_process(node, atoi(dirent->d_name), &pe_p)) {
					tree_p->totalprocs++;
					tree_p->totalthreads += pe_p.n_threads;
                }
            }
        }
        closedir(dir);
    }
    return 1;
}

/* pr_scale formats a number into the supplied buffer. It
 * prints the number as meg, kilo, giga, or bytes, appending
 * the appropriate suffix.
 */

char * pr_scale (uint32_t v, char * buf, int len)
{
    if (v >= 1024 * 1024 * 1024) {
        snprintf(buf, len, "%dG", v / (1024 * 1024 * 1024));
    } else if (v >= 1024 * 1024) {
        snprintf(buf, len, "%dM", v / (1024 * 1024));
    } else if (v >= 1024) {
        snprintf(buf, len, "%dK", v / 1024);
    } else {
        snprintf(buf, len, "%d", v);
    }

    return buf;
}

void
dspsys(const char *type, const char *argv)
{
	char					buffer[PATH_MAX + 1];
	int						fd;
	procfs_sysinfo			*sysinfo;
	struct stat64			st;
	unsigned int			size;
	uint64_t				stat_size;
	uint64_t				total;
	char					*size_sym;
	char					*total_sym;
	time_t					boot_time;
	struct cpuinfo_entry	*cpu;
	int						i;
	char                    nodepath[PATH_MAX + 1], *n;
	DIR                     *dp;
	struct dirent           *dent;
	proc_info_t procinfo;
	
	dp = 0;
	dent = 0;
	n = ".";
	
	switch(*type) {
	  case 's':
	  case 'n':
		if (*type == 'n' && !*node) {
			/* 'pidin net' show info for everyone in /net */
			if ((i = find_netdir("", nodepath, PATH_MAX, 1)) == -1)
			  error_exit(1, "can't find node %s: %s\n", node, strerror(errno));
			
			/* make sure the network manager is running, otherwise, we jsut
			 * got a "/"
			 */
			if ((fd = open("/dev/netmgr", O_RDONLY)) == -1)
			  error_exit(1, "Network manager is not running.\n");
			close(fd);
			
			if ((dp = opendir(nodepath)) == NULL)
			  error_exit(1, "can't opendir %s: %s\n", nodepath, strerror(errno));
			
			if (!(dent = readdir(dp))) {
				closedir(dp);
				return;
			}
			
			n = dent->d_name;
			printf(" ND    Node             CPU      Release FreeMem       BootTime\n");
		} else {
			/* 'pidin -n node info' or 'pidin -n node net' generate same result */
			if ((i = find_netdir(node, nodepath, PATH_MAX, 0)) == -1)
			  error_exit(1, "can't find node %s: %s\n", node, strerror(errno));
		}
		break;
	  case 'r':
		dsprc(dp);
		return;

	  case 'P':
		dspsyspage(strchr(argv,'='));
		return;

	  case 'E':
		if (*node != '\0')
			error_exit(!0, "can only query 'extsched' on local node\n");
		dspsched();
		return;

	  default:
		error_exit(1, "invalid option\n");
	}
	
	do {
		snprintf(buffer, PATH_MAX, "%s%s/proc", nodepath, n);
		if (dent) {
			if (strlen(dent->d_name) > 15)
			  dent->d_name[15] = 0;
			printf(" %-4d  %-15s ", (int)dent->d_ino, dent->d_name);
		}
		
		if ((fd = open64(buffer, O_RDONLY)) == -1) {
			if (dp) {
				printf("%s\n", strerror(errno));
				goto next;
			}
			error_exit(1, "couldn't open %s: %s\n", buffer, strerror(errno));
		}
		if ((sysinfo = load_syspage(fd, 0)) == NULL) {
			if (dp) {
				printf("%s\n", strerror(errno));
				goto next;
			}
			error_exit(!0, "couldn't load syspage info for %s: %s\n", buffer, strerror(errno));
		}

		if (fstat64(fd, &st) == -1)
		  error_exit(1, "couldn't get stat info for %s: %s\n", buffer, strerror(errno));
		stat_size = st.st_size;
		total = get_total_mem(sysinfo);
		boot_time = _SYSPAGE_ENTRY(sysinfo, qtime)->boot_time;

		
		if (dent) {
			printf("%2d ", sysinfo->num_cpu);
		} else {
			printf("CPU:");
		}
		

		switch(sysinfo->type) {
		  case SYSPAGE_X86:
			printf("X86 ");
			break;
		  case SYSPAGE_PPC:
			printf("PPC ");
			break;
		  case SYSPAGE_MIPS:
			printf("MIPS ");
			break;
		  case SYSPAGE_ARM:
			printf("ARM ");
			break;
		  case SYSPAGE_SH:
			printf("SH  ");
			break;
		  default:
			printf("Unknown(%d) ", sysinfo->type);
			break;
		}
		
		size = PATH_MAX;
		if (confstr_get(fd,_CS_RELEASE,&size, buffer) == -1) {
			error_exit(1, "couldn't retrieve release info for %s: %s\n", buffer, strerror(errno));
		}
		
		if (dent) {
			printf("   %-8s",buffer);
		} else {
			printf("Release:%-7s", buffer);
		}

		size = normalize_data_size(stat_size, &size_sym);
		total = normalize_data_size(total, &total_sym);
		strftime(buffer, sizeof buffer, "%b %d %T %Z %Y", localtime(&boot_time));
		if (dent) {
			char sizebuf[14];
			snprintf(sizebuf, 14, "%u%sb/%u%sb", size, size_sym, (unsigned)total, total_sym);
			printf("%-14s", sizebuf);
			if (boot_time != 0){
				printf("%s", buffer);
			}else{
				printf("------------------------");
			}
		} else {
			printf("FreeMem:%u%sb/%u%sb ", size, size_sym, (unsigned)total, total_sym);
			printf("BootTime:%s", buffer);
		}
		
		printf("\n");		

		if (dent){
			if(get_node_procs(dent->d_name, &procinfo)){
				printf("           Processes: %d, Threads: %d\n", procinfo.totalprocs, procinfo.totalthreads);
			}
		}else{
			if(get_node_procs(node, &procinfo)){
				printf("Processes: %d, Threads: %d\n", procinfo.totalprocs, procinfo.totalthreads);
			}
		}

		if (dent) {
			
			for(i = 1, cpu = _SYSPAGE_ENTRY(sysinfo, cpuinfo); i <= sysinfo->num_cpu; i++, cpu++) {
				char cpubuf[20];
				char fmt[20];
				printf("           CPU %1d: ", i);
				if(sysinfo->type == SYSPAGE_PPC){
					printf("%10x ", cpu->cpu);
				} else{
					printf("%10u ", cpu->cpu);
				}
				snprintf(cpubuf, sizeof(cpubuf), "%s", &_SYSPAGE_ENTRY(sysinfo, strings)->data[cpu->name]);
				if (strlen(cpubuf) == sizeof(cpubuf) - 1 && sizeof(cpubuf) > 6){
					cpubuf[sizeof(cpubuf) - 4] = '.';
					cpubuf[sizeof(cpubuf) - 3] = '.';
					cpubuf[sizeof(cpubuf) - 2] = '.';
				}
				snprintf(fmt, sizeof(fmt), "%%-%ds ", sizeof(cpubuf));
				printf(fmt, cpubuf);
				printf("%6dMHz ", cpu->speed);
				if(cpu->flags & CPU_FLAG_FPU) {
					printf("FPU ");
				}else{
					printf("    ");
				}
				if(!(cpu->flags & CPU_FLAG_MMU)) {
					printf("Physical ");
				}
				printf("\n");
			}
		}else{
			for(i = 1, cpu = _SYSPAGE_ENTRY(sysinfo, cpuinfo); i <= sysinfo->num_cpu; i++, cpu++) {
				printf("Processor%d: ", i);
				if(sysinfo->type == SYSPAGE_PPC){
					printf("%x ", cpu->cpu);
				} else{
					printf("%u ", cpu->cpu);
				}
				printf("%s ", &_SYSPAGE_ENTRY(sysinfo, strings)->data[cpu->name]);
				printf("%dMHz ", cpu->speed);
				if(cpu->flags & CPU_FLAG_FPU) {
					printf("FPU ");
				}
				if(!(cpu->flags & CPU_FLAG_MMU)) {
					printf("Physical ");
				}
				printf("\n");
			}
		}

		free(sysinfo);
		close(fd);
next:			
		if (dent && (dent = readdir(dp)))
		  n = dent->d_name;
	} while (dent);
	
	if (dp)
	  closedir(dp);
}

/* Structure used for sorting process information. This is currently
 * set up to sort on a pid type key */
typedef struct sortentry{
	pid_t pid;
	pid_t key;
} sortentry_t;

static int 
sortkey(const void *a, const void *b)
{
	const sortentry_t *as = (const sortentry_t *)a;
	const sortentry_t *bs = (const sortentry_t *)b;
	
	return(as->key - bs->key );
}


static int 
sortpid(const void *a, const void *b) 
{
	const sortentry_t *as = (const sortentry_t *)a;
	const sortentry_t *bs = (const sortentry_t *)b;
	
	return(as->pid - bs->pid);
}



static int 
getinfo(char *procname, struct shared_info *info_p) 
{
	int ret, fd;
	
	if ((fd = open64(procname, O_RDONLY)) == -1) {
		if(errno != ENOENT) {
			warning_exit(1, 0, "couldn't open %s: %s\n", procname, strerror(errno));
		}
		return(1);
	}
	memset(info_p, 0, sizeof(*info_p));
	ret = fill_info(info_p, fd);
	close(fd);
	return(ret);
}





static int * 
getpidlist(char *nodepath, const char *fmt)
{


	char buf[50];
	struct dirent  *dirent;
	DIR			   *dir;
	int 			entries = 0;
	pid_t			*pid_list;
	int				i, cur = 0;
	sortentry_t		*sort_list;
	int				pidsort = 0;
	char 			keyid = 'a';
	
	snprintf(buf, 50, "%sproc", nodepath);
	if (!(dir = opendir(buf))) {
		error_exit(1, "couldn't open %s: %s\n", buf, strerror(errno));
	}

	/* Find number of entries. */	
	while (dirent = readdir(dir)) {
		entries++;
	}
	
	/* Add one entry for terminator value. */
	pid_list = malloc(sizeof(pid_t)*(entries + 1));

	if (pid_list == NULL) {
		closedir(dir);
		return(NULL);
	}
	
	sort_list = (sortentry_t *)malloc(sizeof(sortentry_t)*(entries));
	if (sort_list == NULL) {
		free(pid_list);
		closedir(dir);
		return(NULL);
	}
	
	rewinddir(dir);

	if (fmt != NULL) {
		/* Find first character identifier in output format to determine
		 * which key should be used as primary for sorting. */
		char *firstid = strchr(fmt, '%');
		if (firstid != NULL) {
			keyid = *(firstid+1);
		}
	}
	
	while (dirent = readdir(dir)) {
		pid_list[cur] = atoi(dirent->d_name);
		
		/* Only analyze numeric entries.  0 indicates all ASCII... */
		/* Potential problem here with numeric + ASCII, but these
		* should never occur in the proc dir. */
		if (pid_list[cur] != 0) {
			struct shared_info info;
			
			snprintf(buf, 50, "%sproc/%d/as", nodepath, pid_list[cur]);
			if (getinfo(buf, &info) == 0) {
				sort_list[cur].pid = pid_list[cur];
				
				/* Bit of a nuisance having to fill all of these in, but
				 * there aren't really many that are likely to be useful
				 * for grouping other than sessions and process groups. */
				 
				switch(keyid) {
				case 'L':
					/* Session ID. */
					sort_list[cur].key = info.info->sid;
					break;
				case 'P':
					/* Process group. */
					sort_list[cur].key = info.info->pgrp;
					break;
				default:
					/* Sort on pid only.*/
					sort_list[cur].key = info.info->pid;
					pidsort = 1;
					break;
				}						
				/* Include pid in list to be displayed... */
				cur++;
			}
			
			if (cur >= entries) {
				/* Fall out if more entries found than allocated.
				 * No need to produce error since the output is only a sample
				 * at a given point in time anyway.
				 */	
				break;
			}
		}
	}
	closedir(dir);
	
	/* Sort on primary key. */
	qsort (sort_list, cur, sizeof(sortentry_t), sortkey);
	
	/* Now perform secondary sort based on pid (if necessary). */
	if (!pidsort) {
		i = 0;
		while (i < cur) {
			int ns = 0;
			pid_t curkey = sort_list[i].key;
			while ((sort_list[i+ns].key == curkey) && (i+ns < cur)) {
				ns++;
			}
			qsort(&sort_list[i], ns, sizeof(sortentry_t), sortpid);
			i+=ns;
		}
	}
	
	
	/* Copy sorted pids int pidlist for return to caller. */	
	for (i = 0; i < cur; i++) {
		pid_list[i] = sort_list[i].pid;
	}
	
	/* End of array = 0. */
	pid_list[cur] = 0;
	free (sort_list);

	return(pid_list);

}




void 
dspinfo(pid_t pid, char *pidname, int flags, const char *fmt, const char *mi_fmt)
{
	char			buffer[50], nodepath[50];
	struct dirent  *dirent;
	DIR			   *dir;
	int				fd, len;
	pid_t			*pid_list;

	if ((len = find_netdir(node, nodepath, 50, 0)) == -1)
	  error_exit(1, "could't find node %s: %s\n", node);

	if (flags & DONT_RECURSE && pidname == NULL) {
		snprintf(buffer, 50, "%sproc/%d/as", nodepath, pid);
		if ((fd = open64(buffer, O_RDONLY)) == -1)
			error_exit(1, "couldn't open %s: %s\n", buffer, strerror(errno));
		piddspinfo(fd, NULL, pid, flags | DONT_RECURSE, fmt, mi_fmt);
		close(fd);
		return;
	}
	snprintf(buffer, 50, "%sproc", nodepath);

	/* Try and get sorted list of pids. */
	pid_list = getpidlist(nodepath, fmt);
	if (pid_list != NULL) {
		int *pid_e = pid_list;
		while (*pid_e != 0) {
			snprintf(buffer, 50, "%sproc/%d/as", nodepath, *pid_e);
			if ((fd = open64(buffer, O_RDONLY)) != -1) {
				piddspinfo(fd, pidname, *pid_e, flags | DONT_RECURSE, fmt, mi_fmt);
				close(fd);
			} else if(errno != ENOENT) {
				warning_exit(1, 0, "couldn't open %s: %s\n", buffer, strerror(errno));
			}
			pid_e++;
		}
		/* pid_list was malloc'ed... */
		free(pid_list);
		return;
	}
	
	
	/* Sorted list unavailable (e.g. no memory...). Get unsorted entries. */
	if (!(dir = opendir(buffer)))
		error_exit(1, "couldn't open %s: %s\n", buffer, strerror(errno));
	while (dirent = readdir(dir)) {
		int				fd;

		/* skip over "/proc/self" entry */
		if(!isdigit(dirent->d_name[0])) continue;

		snprintf(buffer, 50, "%sproc/%s/as", nodepath, dirent->d_name);
		if ((fd = open64(buffer, O_RDONLY)) != -1) {
			piddspinfo(fd, pidname, atoi(dirent->d_name), flags | DONT_RECURSE, fmt, mi_fmt);
			close(fd);
		} else if(errno != ENOENT) {
			warning_exit(1, 0, "couldn't open %s: %s\n", buffer, strerror(errno));
		}
	}
	closedir(dir);
}

void 
piddspinfo(int fd, char *pidname, pid_t pid, int flags, const char *fmt, const char *mf)
{
	int				tid, lasttid;
	const char		*f;
	struct shared_info info;
	int				special = 0;

	info.name = 0;
	info.mem = 0;
	info.info = 0;
	info.irqs = 0;
	info.channels = 0;
	info.num_channels = 0;
	info.num_irqs = 0;
	info.timers = 0;
	info.num_timers = 0;
	info.coids = 0;
	info.num_coids = 0;
	info.tls = 0;
	info.text = info.data = info.stack = 0;
	info.memobjects = 0;
	info.num_memobjects = 0;
	info.flags = 0;
	info.gprs = 0;
	info.meminfo = 0;
	
	if (fill_info(&info, fd)) {
		return;
	}

	if(pidname) {
		fill_name(&info, fd);

		if(!info.name) {
			return;
		}
		if(!(f = strrchr(info.name->path, '/'))) {
			f = info.name->path;
		}
		while(*f && *f == '/') { f++; }
		if(strcmp(tail((char*) f), pidname) != 0) {
			return;
		}
	}

	if (mf) {
		info.flags |= SEPARATE_MEMORY;
	}

	for (lasttid = tid = 1; lasttid >= tid; lasttid = ++tid) {
		f = fmt;

		if (!(info.info->flags & _NTO_PF_ZOMBIE)) {
			if (fill_status(1, &info, &tid, fd) || tid < lasttid)
				break;
		}

		if (info.memobjects) {
			int i;
			struct memobjects *mo;

			for(mo = info.memobjects, i = 0;i < info.num_memobjects; mo++, i++) {
				free(mo->name);
			}
			free(info.memobjects);
			info.memobjects = 0;
			info.num_memobjects = 0;
		}
		info.text = info.data = info.stack = 0;

		while (*f) {
			struct format  *format;

			if (!(format = get_format(stdout, &f)))
				break;
			info.status = 0;
			if (format->flags & MULTI_LINE) {
				special++;
				continue;
			}
			/* if we can do it as a zombie or this isn't a zombie ... */
			if (!(format->flags & ZOMBIE_INVALID) || 
			    !(info.info->flags & _NTO_PF_ZOMBIE)) {
				if (format->print(stdout, pid, &tid, format, fd, &info))
					break;
			} else {
				if (format->flags & NA) {
					format_data_string(stdout, format,
						(info.info->flags & _NTO_PF_ZOMBIE) ? "(Zombie)" : na);
				} else {
					format_data_string(stdout, format, spaces);
				}
			}
		}
		fprintf(stdout, "\n");
		if (special) {
			f = fmt;
			while (*f) {
				struct format  *format;

				if (!(format = get_format(0, &f)))
					break;
				if ((format->flags & MULTI_LINE) &&
					!(format->flags & PROCESS_ONLY) &&
					format->print(stdout, pid, &tid, format, fd, &info))
					break;
			}
		}

		if (info.info->flags & _NTO_PF_ZOMBIE)
			break;
		if (!(flags & DO_THREADS))
			break;
	}

	if (special) {
		fpos_t p1, p2;
		fgetpos(stdout, &p1);
		f = fmt;
		while (*f) {
			struct format  *format;

			if (!(format = get_format(0, &f)))
				break;
			if ((format->flags & (MULTI_LINE | PROCESS_ONLY)) == (MULTI_LINE | PROCESS_ONLY) &&
				format->print(stdout, pid, &tid, format, fd, &info))
				break;
		}
		fgetpos(stdout, &p2);
		//Avoid putting two newlines when there is no extra output
		if(memcmp(&p1, &p2, sizeof(p1)) != 0) {
			fprintf(stdout, "\n");
		}
	}

	if (mf) {
		int i;
		for (i = info.num_memobjects; i; i--) {
			f = mf;
			while(*f) {
				struct format *format;

				if (!(format = get_format(stdout, &f)))
					break;
				format->print(stdout, pid, &tid, format, fd, &info);
			}
			info.next_memobject++;
			fprintf(stdout, "\n");
		}
	}

	// need to clean up resources
	if (info.mem)
		free(info.mem);
	if (info.tls)
		free(info.tls);
	if (info.irqs)
		free(info.irqs);
	if (info.gprs)
		free(info.gprs);
	if (info.timers)
		free(info.timers);
	if (info.memobjects) {
		int i;
		struct memobjects *mo;

		for(mo = info.memobjects, i = 0;i < info.num_memobjects; mo++, i++) {
			free(mo->name);
		}
		free(info.memobjects);
		info.memobjects = 0;
	}
	if (info.coids) {
		int i;
		for(i = 0; i < info.num_coids; i++) {
			if(info.coids[i].name) {
				free(info.coids[i].name);
			}
		}
		free(info.coids);
		info.coids = 0;
		info.num_coids = 0;
	}
	if (info.meminfo) {
		free_meminfo(&(info.meminfo));
	}
}

static char	   *
normalize_format(const char *f)
{
	const char	   *p;
	char		   *r, *s;

	if (!(r = malloc(strlen(f) * 3 + 1)))
		return 0;
	for (s = r, p = f; *p; p++)
		*s++ = '%', *s++ = *p, *s++ = ' ';
	*s = 0;
	return r;
}

// note this returns a pointer to static storage
static struct format *
get_format(FILE * fp, const char **f)
{
	static struct format r;
	const char		*p = *f;
	int				w, c;

	while (*p && *p != '%') {
		if (fp)
			fputc(*p, fp);
		p++;
	}
	if (!*p || !*++p)
		return *f = p, (struct format *) 0;		// trailing % is ignored

	w = strtol(p, (char **)&p, 10);
	c = *p++;
	if (formats[c].letter != c) {
		error_exit(1, "Format letter '%c'(%#x) <=> '%c'(%#x) is invalid.\n", c, c, formats[c].letter, formats[c].letter);
	}
	if (!formats[c].print)
		return errno = EINVAL, (struct format *) -1;
	r.width = w ? w : formats[c].width;
	r.title = formats[c].title;
	r.print = formats[c].print;
	r.flags = formats[c].flags;
	if (r.width < 0)
		r.flags |= DATA_LEFT_JUSTIFIED, r.width *= -1;
	r.letter = c;
	*f = p;
	return &r;
}

static void validate_formats()
{
	/* very simple check to validate the formats array
	* instead of trying to validate every entry, we check the format letter 'z' 
  * and verify that its position is correct in the table. Since 'z' is 
  * the last entry used entry in the table, we assume that if this one is
  * correct, all previous are too */

	if (formats['z'].letter != 'z') {
		error_exit(1, "Format table invalid: '%c'(%#x) <=> '%c'(%#x)\n", 'z', 'z', formats['z'].letter, formats['z'].letter);
	}
	return;
}

static struct shorthand *
lookup_shorthand(const char *f)
{
	struct shorthand	*s, *match;
	int					len;

	for (len =0; f[len] && isalnum(f[len]); len++ );
	for (match = 0, s = shorthands; s->handle; s++) {
		if (!strnicmp(s->handle, f, len)) {
			if (match) {			
				match = 0;
				break;
			}
			match = s;
		}
	}
	return match;
}

void 
error_exit(int printmsg, const char *fmt,...)
{
	extern char	*__progname;

	if (printmsg) {
		va_list			arglist;

		fprintf(stderr, "%s: ", __progname);
		va_start(arglist, fmt);
		vfprintf(stderr, fmt, arglist);
		va_end(arglist);
	}
	exit(EXIT_FAILURE);
}

void 
warning_exit(int printmsg, int expectwarn, const char *fmt,...)
{
	if (printmsg) {
		va_list			arglist;
		va_start(arglist, fmt);
		vfprintf(stderr, fmt, arglist);
		va_end(arglist);
	}
	if (!expectwarn && exit_on_warning)
		exit(EXIT_FAILURE);
}


__SRCVERSION("pidin.c $Rev: 210372 $");
