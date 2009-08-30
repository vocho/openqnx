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
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/procfs.h>
#include <sys/sched.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/syspage.h>
#include <sys/mman.h>
#include <sys/debug.h>
#include <sys/neutrino.h>
#include <limits.h>
#include <pthread.h>
#include <sgtty.h>
#include <inttypes.h>

#include "ttyin.h"
#include "procfs_util.h"


/* These are VT100 sequences. They should be obtained from termcap.
 * However, termcap is on the fritz right now. When its operational,
 * lets get these from there.
 */
char * cl_sequence = "\033[H\033[2J";
char * ho_sequence = "\033[H";
char * ce_sequence = "\033[K";


/* Data structures to contain the process model */

typedef struct process_tree_ process_tree;
typedef struct process_entry_ process_entry;
typedef struct thread_entry_ thread_entry;

const long MAGIC = 0x8BADF00D;

// globals for telltales
#define MAX_CPUS        32
unsigned  min_cpu_idle[ MAX_CPUS ];
unsigned  max_cpu_idle[ MAX_CPUS ];
unsigned  tot_cpu_idle[ MAX_CPUS ];

unsigned  min_mem_avail = 1000000;   // units - MB
unsigned  max_mem_avail;
unsigned  tot_mem_avail;

unsigned  min_procs = 1000000; 
unsigned  max_procs;
unsigned  tot_procs;

unsigned  min_threads = 1000000; 
unsigned  max_threads;
unsigned  tot_threads;

unsigned  times_thru;


/* The top of the process tree. It keeps the root of the process entry
 * list, as well as various statistics about the processes, such as
 * the number of processes, threads, and various times.
 */

struct process_tree_
{
    process_tree        *next;
    int                 n_processes;
    int                 n_threads;
    uint64_t            idle_time[ MAX_CPUS ];
    uint64_t            kernel_time;
    uint64_t            user_time;
    uint64_t            total_time;
    process_entry *     process_list_p;
    long                magic;
};


/* This structure represents a process. Keeps the process info
 * structure gleaned from /proc, along with the list of threads in the
 * process, the pid of the process, and the number of threads. Also
 * keep the name here for later.
 */

struct process_entry_
{
    process_entry *     next;
    process_tree *      up;
    procfs_info         info;
    int32_t             total_memory;
    int                 n_threads;
    char                name[_POSIX_PATH_MAX];
    thread_entry *      thread_list_p;
    long                magic;
};


/* This structure represents a thread. Keeps the status
 * structure obtained from /proc, along with the pid, tid,
 * and total time used (as computed in pick_active_threads()
 * below).
 */

struct thread_entry_
{
    thread_entry *      next;
    process_entry *     up;
    uint64_t            time_used;
    procfs_status       status;
    long                magic;
};


/* These are the heads of free lists */

process_tree * free_process_trees = 0;
process_entry * free_process_entries = 0;
thread_entry * free_thread_entries = 0;

/* Allocate and initialize a new process entry structure */

process_entry * new_process_entry (pid_t pid)
{
    process_entry * p;
    if (free_process_entries == NULL) {
        p = malloc(sizeof(*p));
    } else {
        p = free_process_entries;
        free_process_entries = p->next;
    }

    if (p != NULL) {
        memset(p, 0, sizeof(*p));
        p->info.pid = pid;
        p->magic = MAGIC;
    } else {
        perror("Couldn't allocate process entry");
        exit(1);
    }

    return p;
}

/* Allocate and init a new thread entry structure */

thread_entry * new_thread_entry (pid_t pid, int tid)
{
    thread_entry * t;
    if (free_thread_entries == NULL) {
        t = malloc(sizeof(*t));
    } else {
        t = free_thread_entries;
        free_thread_entries = t->next;
    }

    if (t != NULL) {
        memset(t, 0, sizeof(*t));
        t->status.pid = pid;
        t->status.tid = tid;
        t->magic = MAGIC;
    } else {
        perror("Couldn't allocate thread entry");
        exit(1);
    }
    return t;
}

/* Allocate and init a process tree structure */

process_tree * new_process_tree (void)
{
    process_tree * tree_p;

    if (free_process_trees == NULL) {
        tree_p = malloc(sizeof(*tree_p));
    } else {
        tree_p = free_process_trees;
        free_process_trees = tree_p->next;
    }

    if (tree_p == NULL) {
        perror("Couldn't allocate process tree");
        exit(1);
    }
    memset(tree_p, 0, sizeof(*tree_p));
    tree_p->magic = MAGIC;

    return tree_p;
}

/* Free a tree entry. */

void free_thread_entry (thread_entry * te_p)
{
    if (te_p != NULL) {
        if (te_p->magic != MAGIC) {
            leave_cbreak_mode();
            abort();
        }
        te_p->magic = 0;

        te_p->next = free_thread_entries;
        free_thread_entries = te_p;
    }
}

/* free a process_entry
 */

void free_process_entry (process_entry * pe_p)
{
    if (pe_p != NULL) {
        if (pe_p->magic != MAGIC) {
            leave_cbreak_mode();
            abort();
        }
        pe_p->magic = 0;

        pe_p->next = free_process_entries;
        free_process_entries = pe_p;
    }
}

/* Free a process tree, freeing the process list as well */

void free_process_tree (process_tree * tree_p)
{
    process_entry * pe_p;
    void * tmp;

    if (tree_p->magic != MAGIC) {
        leave_cbreak_mode();
        abort();
    }

    pe_p = tree_p->process_list_p;
    while (pe_p != NULL) {
        thread_entry * te_p = pe_p->thread_list_p;

        while (te_p != NULL) {
            tmp = te_p->next;
            free_thread_entry(te_p);
            te_p = tmp;
        }

        tmp = pe_p->next;
        free_process_entry(pe_p);
        pe_p = tmp;
    }

    tree_p->magic = 0;
    tree_p->next = free_process_trees;
    free_process_trees = tree_p;
}

/*
 * get_memory_usage_from_map ()
 */

int32_t get_memory_usage_from_map (int fd, procfs_mapinfo *mem, int num)
{
    int32_t             total = -1;

    if (num > 0) {
        int i;

        total = 0;

        for (i = 0; i < num; i++) {
            if ((mem[i].flags & MAP_TYPE) == MAP_SHARED) {
                /* Ignore it */
                continue;
            }
            switch(mem[i].dev) {
            case 1:
                if (mem[i].flags & MAP_ELF) {
                    total += mem[i].size;
                }
                break;
            case 2:
                /* This is mapped memory */
                if (mem[i].flags & MAP_FIXED) {
                    /* This is device memory most likely. Ignore it. */
                } else if (
                    (mem[i].flags & (MAP_STACK|PG_HWMAPPED)) ==
                    (MAP_STACK|PG_HWMAPPED)) {
                    /* This is stack that has physical backing store */
                    total += mem[i].size;
                } else if (mem[i].flags & PG_HWMAPPED) {
                    /* This is not device memory. and it has a
                     * physical backing store. Count is as data
                     */
                    total += mem[i].size;
                }
                break;
            case 3:
                /* This is shared memory */
                break;
            case 4:
                /* This is DLL text. Ignore it */
                break;
            }
        }
    }
    return total;
}

/*
 * Collect process information from the /proc file system. Allocate and
 * return a process_entry for the process with the given pid on the given
 * node.
 *
 * In addition, get info on all the threads in the process, and allocate
 * a list of thread_entry structures with them.
 *
 * Note that this routine puts the thread structures on the list in
 * reverse order from how it discovers them.
 */

process_entry * do_process(char * node, pid_t pid, int calc_mem)
{
    int                 fd;
    char                buff[128];
    static char *       e = "Unable to build the process tree";
    process_entry *     pe_p;

    pe_p = new_process_entry(pid);
    if (pe_p == NULL) {
        perror(e);
        exit(-1);
    }

    if (node == 0 || node[0] == 0) {
        sprintf(buff, "/proc/%d", pid);
    } else {
        sprintf(buff, "/net/%s/proc/%d", node, pid);
    }

    if ((fd = open(buff, O_RDONLY)) == -1) {
        /* The pid may have died, just ignore it */
        free_process_entry(pe_p);
        return NULL;
    }

    if (devctl(fd, DCMD_PROC_INFO,
               &pe_p->info, sizeof pe_p->info, 0) != -1) {
        int             lasttid, tid;

        if (pe_p->info.flags & _NTO_PF_ZOMBIE) {
            free_process_entry(pe_p);
            pe_p = NULL;
        } else {
            if (pe_p->info.pid == 1) {
                strcpy(pe_p->name, "kernel");
            } else {
                procfs_get_process_name(fd, pe_p->name, sizeof(pe_p->name));
         	}

            if (calc_mem) {
                procfs_mapinfo  *map;
                int             num;

				/* This can be an expensive operation if a process has
				a large number of mappings.  It's also not currently
				used in the displayed output.  Make it optional. */

                num = procfs_get_process_mapinfo(fd, &map);
	            pe_p->total_memory = get_memory_usage_from_map(fd, map, num);
                free(map);

            } else {
                pe_p->total_memory = 0;
            }

            for (lasttid = tid = 1; ; lasttid = ++tid) {
                thread_entry * te_p = new_thread_entry(pid, tid);
                if (te_p == NULL) {
                    perror(e);
                    exit(-1);
                }

                te_p->status.tid = tid;
                if (devctl(fd, DCMD_PROC_TIDSTATUS, &te_p->status,
                       sizeof(te_p->status), 0) != EOK) {
                    free_thread_entry(te_p);
                    break;
                }
                tid = te_p->status.tid;
                if (tid < lasttid) {
                	free_thread_entry(te_p);
                	break;
                }
                te_p->up = pe_p;
                te_p->next = pe_p->thread_list_p;
                pe_p->thread_list_p = te_p;
                pe_p->n_threads++;
            }
        }
    }
    close(fd);
    return(pe_p);
}


/* Walk through /proc, and for each pid found, call do_process(). It
 * should return either NULL or a process_entry block, containing info
 * for the process. Link them up on the supplied tree.
 */

process_tree * build_process_tree (char * node)
{
    DIR          *dir;
    char          fname[_POSIX_PATH_MAX];
    process_tree *tree_p;

    /* allocation failure taken care of in new_process_tree */
    tree_p = new_process_tree();

    if (node == NULL) {
        dir = opendir("/proc");
    } else {
        sprintf(fname, "/net/%s/proc", node);
        dir = opendir(fname);
    }

    if (dir == NULL) {
        leave_cbreak_mode();
        printf(cl_sequence);
        printf("/proc can't be opened: %s\n", strerror(errno));
        exit(-2);
    } else {
        struct dirent * dirent;

        while ((dirent = readdir(dir)) != NULL) {
            if (isdigit(dirent->d_name[0])) {
                process_entry * pe_p =
                    do_process(node, atoi(dirent->d_name), 0);

                if (pe_p != NULL) {
                    pe_p->next = tree_p->process_list_p;
                    pe_p->up = tree_p;
                    tree_p->process_list_p = pe_p;
                    tree_p->n_processes++;
                    tree_p->n_threads += pe_p->n_threads;
                }
            }
        }
        closedir(dir);
    }
    return tree_p;
}


/* find_thread_entry() searches a process tree for a particular thread.
 * as denoted by its pid/tid pair. If found, that thread_entry is returned.
 */
thread_entry * find_thread_entry (process_tree * tree_p, pid_t pid, int tid)
{
    process_entry * pe_p;

    for (pe_p = tree_p->process_list_p;
         pe_p != NULL;
         pe_p = pe_p->next) {

        thread_entry * te_p;

        if (pe_p->info.pid != pid) {
            continue;
        }

        for (te_p = pe_p->thread_list_p;
             te_p != NULL;
             te_p = te_p->next)
        {
            if (te_p->status.tid == tid) {
                return te_p;
            }
        }
    }
    return NULL;
}


/*
 * pick_active_threads() is the real heart of top. Given two process
 * trees, it attempts to order the threads from the newer tree into
 * decending order of first CPU usage, and then total CPU usage. It does
 * this by comparing the values of the ticks field in the tid
 * information.
 *
 * Every 10 ms, the ticks field is incremented for the running thread.
 * This means that the tid info returned by procfs can be compared to the
 * prior tid info, and an indication of how many ticks happened during
 * the delay between tree building can be determined for each thread.
 *
 * One other thing of note is that pid 1, tid 1 is the idle thread. This
 * is the thread that is run when there isn't anything else to do.
 *
 * So, the algorithm is
 *
 * for each thread in the new tree
 *
 *  find the thread in the old tree.
 *  time used during this interval is (new ticks - old ticks)
 *  total ticks is incremented by this amount
 *
 *  skipping the idle thread, sort the current thread into
 *  the supplied array.
 *
 * Return the number of elements added to the array.
 */

int pick_active_threads (
    int nCPUs,
    int nthreads,
    thread_entry ** stat_array,
    process_tree * prior_p,
    process_tree * now_p
    )
{
    process_entry * pe_p;
    int last_element = -1;

    memset(stat_array, 0, sizeof(thread_entry *) * nthreads);

    for (pe_p = now_p->process_list_p;
         pe_p != NULL;
         pe_p = pe_p->next) {

        thread_entry * te_p;

        for (te_p = pe_p->thread_list_p;
             te_p != NULL;
             te_p = te_p->next)
        {
            thread_entry * prior_te_p =
                find_thread_entry(prior_p, te_p->status.pid,
                                  te_p->status.tid);

            if (prior_te_p == NULL ||
                te_p->status.sutime < prior_te_p->status.sutime) {
                /* Here, prior_te_p either isn't set, or points to a
                 * thread with the same TID, but which isn't the same
                 * as the new one (since the elapsed_time is more than
                 * the new thread). Consequently, the time_used field is
                 * just the number of ticks in this one.
                 */

                te_p->time_used = te_p->status.sutime;
            } else {
                /* Here, we use the difference between this time
                 * and the last time.
                 */
                te_p->time_used =
                    te_p->status.sutime -
                    prior_te_p->status.sutime;
            }

            /* Count total time running */
            now_p->total_time += te_p->time_used;

            if (te_p->status.pid == 1 && te_p->status.tid <= nCPUs ) {
                /* We exclude the idle thread for fun */
                /* We also trust we never see a tid of 0. */
                now_p->idle_time[ te_p->status.tid - 1 ] = te_p->time_used;
            } else {
                int i;
                thread_entry * i_p = te_p;

                if (i_p->status.pid == 1) {
                    now_p->kernel_time += i_p->time_used;
                } else {
                    now_p->user_time += i_p->time_used;
                }

                for (i = 0; i < nthreads; i++) {
                    if (stat_array[i] == 0) {
                        /* We always replace null entries */
                        stat_array[i] = i_p;
                        last_element = i;
                        break;
                    }

                    if (stat_array[i]->time_used < i_p->time_used
                        ||
                        (stat_array[i]->time_used == i_p->time_used
                         &&
                         (stat_array[i]->status.sutime <
                          i_p->status.sutime))) {

                        /* The new one will replace the one at 'i' */
                        thread_entry * swap_p = i_p;
                        i_p = stat_array[i];
                        stat_array[i] = swap_p;
                    }
                }
            }
        }
    }
    return last_element + 1;
}


/*
 * pr_state()
 *
 *  return a print string for the state of a press.
 */

char * pr_state (_uint8 state)
{
    /* These need to be 4 characters */
    switch(state) {
    case STATE_DEAD:
        return "Dead";
        break;

    case STATE_RUNNING:
        return "Run ";
        break;

    case STATE_READY:
        return "Rdy ";
        break;

    case STATE_STOPPED:
        return "Stop";
        break;

    case STATE_SEND:
        return "Send";
        break;

    case STATE_RECEIVE:
        return "Rcv ";
        break;

    case STATE_REPLY:
        return "Rply";
        break;

    case STATE_STACK:
        return "Stck";
        break;

    case STATE_WAITTHREAD:
        return "WtTr";
        break;

    case STATE_WAITPAGE:
        return "WtPg";
        break;

    case STATE_SIGSUSPEND:
        return "SigS";
        break;

    case STATE_SIGWAITINFO:
        return "SigW";
        break;

    case STATE_NANOSLEEP:
        return "NSlp";
        break;

    case STATE_MUTEX:
        return "Mtx ";
        break;

    case STATE_CONDVAR:
        return "CdV ";
        break;

    case STATE_JOIN:
        return "Join";
        break;

    case STATE_INTR:
        return "Intr";
        break;

    case STATE_SEM:
        return "Sem ";
        break;
    }
    return "Unkn";
}


/* pr_elapsed_time prints the elapsed time into the supplied
 * buffer, and returns the buffer as a pointer.
 */

char * pr_elapsed_time (thread_entry * te_p, char * buffer, int len)
{
    uint32_t hours, minutes, seconds;

    seconds = te_p->status.sutime / 1000000000;
    minutes = seconds / 60;
    seconds = seconds % 60;
    hours   = minutes / 60;
    minutes = minutes % 60;
    snprintf(buffer, len, "%6d:%02d:%02d",
           hours, minutes, seconds);

    return buffer;
}


/* pr_real_name copies the real name into the supplied
 * buffer, after skipping the path.
 */

char * pr_real_name (char * name, char * buf, int len)
{
    char * p = strrchr(name, '/');

    if (p == NULL || *p == '\0') {
        p = name;
    } else {
        /* We found a '/', skip past it */
        p++;
    }

    strncpy(buf, p, len - 1);
    buf[len - 1] = '\0';

    return buf;
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


/*
 * print_phys_memory()
 *
 * Total up the physical memory and print it out. At some point, we want to
 * print the current, low, and highwater marks for page allocation too.
 */

void print_memory (char * suffix)
{
    struct mem_info info;
    struct system_private_entry *private;
    char rs[32];
    char fs[32];
    char ps[32];

    mem_get_info(NOFD, 0, &info); 
    private = SYSPAGE_ENTRY(system_private);
    printf("Memory: %s total, %s avail, page size %s%s\n",
           pr_scale(private->ramsize, rs, sizeof rs),
           pr_scale(info.mi_length, fs, sizeof fs),
           pr_scale(private->pagesize, ps, sizeof ps),
           suffix);

    {
        unsigned this_mem_avail = info.mi_length / (1024 * 1024);

        min_mem_avail = min(this_mem_avail, min_mem_avail);
        max_mem_avail = max(this_mem_avail, max_mem_avail);
        tot_mem_avail+= this_mem_avail;
    }
}


/* Get a number from the user for interactive processing */

int get_number (char * prompt, int low, int high)
{
    char input[128];
    int n;

    printf(cl_sequence);
    printf(prompt);
    leave_cbreak_mode();
    for (;;) {
        gets(input);
        if (sscanf(input, "%d", &n) != 1 || (n > high) || (n < low)) {
            printf("Please enter a number between %d and %d\n", low, high);
            printf(prompt);
        } else {
            break;
        }
    }
    enter_cbreak_mode();
    return n;
}


/* Kill a process on the user's behalf */

void kill_process ()
{
    pid_t pid;

    printf(cl_sequence);

    pid = get_number("Enter the pid of the process to kill: ", 2, 9999999);

    if (kill(pid, SIGINT) != 0) {
        leave_cbreak_mode();
        perror("Kill fails");
        printf(" ---- type any character to continue ----\n");
        getchar();
        enter_cbreak_mode();
    }
}


/* Print helpful information */

void print_help_info ()
{
    printf(cl_sequence);
    printf("top - print out top users of resources\n"
           "Commands:\n"
           " ? - print this help\n"
           " q - quit\n"
           " n - modify number of threads to display\n"
           " d - modify delay between updates\n"
           " l - refresh screen\n"
           " k - kill a process\n"
           "\n"
           " ---- type any character to continue ---- \n"
        );
    getchar();
    printf(cl_sequence);
}


/* Handle a SIGINT by cleaning up, clearing the screen, and dying */

void die (int x)
{
    leave_cbreak_mode();
    printf(cl_sequence);
    exit(0);
}


/* Main program.
 *
 * The algorithm goes something like this:
 *
 * Create a model of the processes in the system.  Then, wait a
 * bit. After a while, create another model, and find those threads
 * which have used the most cpu time between the two models were
 * built.  sort those thread structures into an array, and then print
 * them out.
 *
 * Once you've printed them out, you need to wait a bit and do it
 * again, after tossing the old model and making the new one the old
 * one. (you get it).
 *
 * Oh, and while you are waiting, find out if the user wants to do
 * anything like quit, or kill a process, or that kind of thing.
 *
 */

void top (char * node, int iterations)
{
    process_tree * prior_p;
    process_tree * now_p;
    thread_entry **thread_array = NULL;
    char elapsed_time_buffer[32];
    char namebuf[64];
    int number_of_threads = 10;
    int number_of_threads_allocated = 0;
    int quit = 0;
    int delay = 5;
    int loop_count = 0;
    int number_of_CPUs = _syspage_ptr->num_cpu;
    int i;

    if ( number_of_CPUs > MAX_CPUS )
    {
        fprintf( stderr, "too many CPUs." );
        exit( EXIT_FAILURE );
    }


    signal(SIGINT, &die);

    /* Check for the existence of the node */
    if (node != NULL) {
        DIR *dir;
        
        snprintf(namebuf, sizeof(namebuf), "/net/%s", node);
        dir = opendir(namebuf);

        if (dir == NULL) {
            printf("node %s does not exist\n",node);
            exit(-2);
        }
        closedir(dir);
    }

    /* This is so commands aren't echoed, and we don't have
     * to get a '\n' character before receiving anything
     */
    if (enter_cbreak_mode()) {
        exit(1);
    }

    for ( i = 0; i < number_of_CPUs; ++i )
    {
        min_cpu_idle[i] = 100;
    }

    /**
     * Need to know how many CPUs there are in order
     * to know which threads are the idle threads.
     * 
     * It seems that the idle threads are pid == 1, tid = 1..n
     * where n is the number of CPUs.
     */

    /* The first time, we need a differential sample, so
     * we just wait one second
     */
    printf("Computing times...");
    fflush(stdout);

    prior_p = build_process_tree(node);

    sleep(1);
    printf(cl_sequence);

    while (!quit) {
        int active;

        /* Create a new process tree for the new model, and build it */
        now_p = build_process_tree(node);

        if (number_of_threads != number_of_threads_allocated) {
            /* realloc the array when it changes size */
            thread_array =
                realloc(thread_array,
                        sizeof(thread_entry *) * number_of_threads);
            if (thread_array == 0) {
                perror("Unable to allocate thread array");
                leave_cbreak_mode();
                exit(1);
            }
            number_of_threads_allocated = number_of_threads;
        }

        /*
         * Now, pick out the most active threads. These are threads that
         * accrued ticks between the times when the two models were
         * built. Sort them in decending order (not including the idle
         * thread (pid == 1, tid == 1). Return how many we got.
         *
         */
        active =
            pick_active_threads(
                number_of_CPUs,
                number_of_threads, thread_array,
                prior_p, now_p);


        {   // update cpu telltales
            int this_cpu_idle[ MAX_CPUS ];
            int this_procs = now_p->n_processes;
            int this_threads = now_p->n_threads;

            for ( i = 0; i < number_of_CPUs; ++i )
            {
                this_cpu_idle[i] = (100 * now_p->idle_time[i]) / ( now_p->total_time / number_of_CPUs );
                min_cpu_idle[i] = min( this_cpu_idle[i], min_cpu_idle[i] );
                max_cpu_idle[i] = max( this_cpu_idle[i], max_cpu_idle[i] );
                tot_cpu_idle[i] += this_cpu_idle[i];
            }
      
            min_procs = min(this_procs, min_procs);
            max_procs = max(this_procs, max_procs);
            tot_procs+= this_procs;
      
            min_threads = min(this_threads, min_threads);
            max_threads = max(this_threads, max_threads);
            tot_threads+= this_threads;
        }

        /*
         * Now, print them out. The ce_sequence thing is to erase to end
         * of line.
         */

        printf("%s%d processes; %d threads;%s\n",
               ho_sequence,
               now_p->n_processes, now_p->n_threads,
               ce_sequence);

        if ( number_of_CPUs > 1 )
        {
            uint64_t total_idle;
            total_idle = now_p->total_time - now_p->user_time - now_p->kernel_time;
            printf("CPU states: "
                   "%lld.%1lld%% user, %lld.%1lld%% kernel%s\n",
                   (100 * now_p->user_time) / now_p->total_time,
                   ((1000 * now_p->user_time) / now_p->total_time) % 10,
                   (100 * now_p->kernel_time) / now_p->total_time,
                   ((1000 * now_p->kernel_time) / now_p->total_time) % 10,
                   ce_sequence );
            for ( i = 0; i < number_of_CPUs; ++i )
            {
                printf( "CPU %2d Idle: %lld.%1lld%% %s\n",
                        i,
                        (100 * now_p->idle_time[i]) / ( now_p->total_time / number_of_CPUs ),
                        ((1000 * now_p->idle_time[i]) / ( now_p->total_time / number_of_CPUs) )% 10,
                        ce_sequence );
            }
        }
        else
        {
            printf("CPU states: %lld.%1lld%% idle, "
                   "%lld.%1lld%% user, %lld.%1lld%% kernel%s\n",
                   (100 * now_p->idle_time[0]) / now_p->total_time,
                   ((1000 * now_p->idle_time[0]) / now_p->total_time) % 10,
                   (100 * now_p->user_time) / now_p->total_time,
                   ((1000 * now_p->user_time) / now_p->total_time) % 10,
                   (100 * now_p->kernel_time) / now_p->total_time,
                   ((1000 * now_p->kernel_time) / now_p->total_time) % 10,
                   ce_sequence);
        }

        print_memory(ce_sequence);
        printf("%s\n", ce_sequence);

        printf( "      PID   TID PRI STATE    HH:MM:SS    CPU  COMMAND%s\n",
                ce_sequence);

        for (i = 0; i < active; i++) {
            thread_entry * te_p = thread_array[i];

            printf("%9d %5d %3d %4s %s %3lld.%02lld%% %s%s\n",
                   te_p->status.pid,
                   te_p->status.tid,
                   te_p->status.priority,
                   pr_state(te_p->status.state),
                   pr_elapsed_time(te_p,
                                   elapsed_time_buffer,
                                   sizeof elapsed_time_buffer),
                   ((100 * te_p->time_used) / now_p->total_time),
                   ((10000 * te_p->time_used) / now_p->total_time) % 100,
                   pr_real_name(te_p->up->name, namebuf, sizeof namebuf),
                   ce_sequence);
        }

        printf("%s\n", ce_sequence);

        times_thru++;

        printf("             Min        Max       Average %s\n",
          ce_sequence);

        if ( number_of_CPUs > 1 )
        {
            for ( i = 0; i < number_of_CPUs; ++i )
            {
                printf( "CPU%2d idle:  %3d%%       %3d%%       %3d%% %s\n",
                        i, min_cpu_idle[i], max_cpu_idle[i],
                        tot_cpu_idle[i]/times_thru, ce_sequence);
            }
        }
        else
        {
            printf( "CPU idle:    %3d%%       %3d%%       %3d%% %s\n",
                    min_cpu_idle[0], max_cpu_idle[0],
                    tot_cpu_idle[0]/times_thru, ce_sequence);
        }

        printf("Mem Avail:   %3dMB      %3dMB      %3dMB  %s\n",
          min_mem_avail, max_mem_avail, tot_mem_avail/times_thru, ce_sequence);

        printf("Processes:   %3d        %3d        %3d    %s\n",
          min_procs, max_procs, tot_procs/times_thru, ce_sequence);

        printf("Threads:     %3d        %3d        %3d    %s\n",
          min_threads, max_threads, tot_threads/times_thru, ce_sequence);

        printf("%s\n", ce_sequence);

        /*
         * Free the 'old' model, and make the current one the prior one
         * for next time.
         */

        free_process_tree(prior_p);
        prior_p = now_p;

	if (iterations && (++loop_count == iterations))
	    break;

        /*
         * Wait a while, and at the same time process user commands, if
         * any.
         */
        for (i = 0; !quit && i < delay; i++) {
            if (!input_to_read()) {
                sleep(1);
            } else {
                int c = getchar();
                switch(c) {
                case 'q':
                    quit = 1;
                    break;
                case 'n':
                    number_of_threads =
                        get_number("Enter number of threads to display: ",
                                   5, 40);
                    i = delay;
                    break;
                case '?':
                case 'h':
                    print_help_info();
                    i = delay;
                    break;
                case 'k':
                    kill_process();
                    i = delay;
                    break;
                case 'd':
                    delay =
                        get_number("Enter seconds between updates: ",
                                   1, 5000);
                    i = delay;
                    break;
                case 'l':
                    printf(cl_sequence);
                    i = delay;
                    break;
                }
            }
        }
    }

    /* Fix tty */
    leave_cbreak_mode();
    exit(0);
}

/* Main, collects arguments, calls top() */

int main (int ac, char * av[])
{
    int     c = 0;
    char   *node = NULL;
    int	    iterations = 0;

    while ((c = getopt(ac, av, "p:i:dn:")) != EOF) {
        switch (c) {
        case 'p':
	    {
		    struct sched_param p;
		    p.sched_priority = strtol(optarg, 0, 0);
		    if (SchedSet(0, 0, SCHED_NOCHANGE, &p) == -1) {
		        perror("SchedSet");
		        exit(1);
		    }
	    }
        break;

        case 'd':
            /* Dumb terminal */
            cl_sequence = "";
            ho_sequence = "";
            ce_sequence = "";
        break;

        case 'n':
            node = optarg;
        break;

        case 'i':
            iterations = strtol(optarg, 0, 10);
        break;
   
        case '?':
        default:
            printf("usage: %s [-i <number>] [-d] [-n <node>]\n", av[0]);
            printf(" -d         dumb terminal\n");
            printf(" -n <node>  remote node\n");
            printf(" -p <pri>   run at priority\n");
            printf(" -i <iter>  # of iterations\n");
            exit(0);
        break;
        }
    }

    top(node, iterations);

    return 0;
}

