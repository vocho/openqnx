/***** USAGE *****************************************************************/
/*

tracectrl   ...    Used to change/monitor trace operation.
*/

#ifdef __USAGE
%C [options*] 
Options:
 -n <node>          Node to query. Default is the current node.
 -i                 Display the current trace parameters.
 -s <severity>      Only save messages of this severity or higher.
                    Severities range from 0 (most critical) to 7 (least
                    critical).
                    Only trace events of an equal or higher severity than
                    'severity' will be saved; all others will be discarded.
                    This affects all future Trace() calls. 
 -p <percent full>  Trigger the logging process (e.g. tracelogger) when the
                    trace buffer becomes this full (default is 50%).
                    This option only applies when a logging process (such as
                    'tracelogger') is being used. When the trace buffer
                    hits this high water mark, a proxy will be triggered on
                    the logging process.
                    A percentage of 0 indicates that a trigger will occur on
                    every trace event.

e.g. tracectrl -s 3 -p 60
     Only save events of severity 0 to 3, and trigger a logging process
     ('tracelogger') when the trace buffer is 60% full.
#endif

/***** INCLUDE ***************************************************************/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/stat.h>
#include <sys/timers.h>
#include <sys/types.h>
#include <sys/osinfo.h>
#include <sys/psinfo.h>
#include <sys/trace.h>
#include <sys/vc.h>

/***** GLOBALS ***************************************************************/

pid_t  proc_pid    = PROC_PID;         /* the pid of Proc */

/***** MAIN ******************************************************************/

void main(int argc, char **argv) {
unsigned opt;
pid_t vid;
nid_t nid;
int percentage = 0;
int severity   = 0;
int ret;
char sev_flag      = 0;
char info_flag     = 0;
char hi_water_flag = 0;

	while((opt=getopt(argc,argv,"in:s:p:"))!=-1) {
		switch(opt) {
 			case 's': /* severity */
                      sev_flag = 1;
                      severity = atoi(optarg);
                      break;
 			case 'p': /* percentage for high water mark */
                      hi_water_flag = 1;
                      percentage = atoi(optarg);
                      break;
 			case 'i': /* information */
                      info_flag = 1;
                      break;
 			case 'n': /* try this node */
                      nid = atoi(optarg);
                      if ( nid != getnid() ) {
                         if ((vid = qnx_vc_attach(nid, PROC_PID, 100, 0)) == -1) {
                            fprintf(stderr,"Unable to attach to node %d.\n",nid);
                            exit(EXIT_FAILURE);
                            }
                         else
                            proc_pid = vid;
                         }
                      break;
			default:  /* invalid option */
                      exit(EXIT_FAILURE);	
                      break;
		    }
	    }

    /* now do the operations */
    
    if ( hi_water_flag )
       ret = update_hi_water(percentage);
    if ( sev_flag )
       ret = update_severity(severity);
    if ( info_flag || !(hi_water_flag || sev_flag) )
       ret = display_info();

    exit(ret);
}

/***** UPDATE_HI_WATER *******************************************************/

update_hi_water(int percentage)
{
struct _trace_info info;
long hi_water;

if(qnx_trace_info(proc_pid, &info) == -1) {
  fprintf(stderr,"Error on read on trace_info (%s)\n",strerror(errno));
  return(EXIT_FAILURE);
  }
else {
  /* read datasize and calculate bytes */
  hi_water = info.buffsize * (long)percentage / 100L;
  if ( qnx_trace_trigger( proc_pid, hi_water, -1 ) == 0 ) {
     fprintf(stderr,"High water mark updated to %d percent.\n",percentage);
     return(EXIT_SUCCESS);
     }
  else {
     fprintf(stderr,"Failure: High water mark NOT updated to %d percent (%s).\n",percentage,strerror(errno));
     return(EXIT_FAILURE);
     }
  }
}

/***** UPDATE_SEVERITY *******************************************************/

update_severity(int severity)
{
if ( qnx_trace_severity( proc_pid, severity ) == 0 ) {
   fprintf(stderr,"Severity changed to %d.\n",severity);
   return(EXIT_SUCCESS);
   }
else {
   fprintf(stderr,"Failure: Severity NOT changed to %d (%s).\n",severity,strerror(errno));
   return(EXIT_FAILURE);
   }
}

/***** DISPLAY_INFO **********************************************************/

display_info()
{
struct _trace_info info;

if(qnx_trace_info(proc_pid, &info) == -1) {
  fprintf(stderr,"Error on trace_info call (%s)\n",strerror(errno));
  return(EXIT_FAILURE);
  }
else {
  fprintf(stdout,"Number of overruns   = %u\n",  info.overruns);
  fprintf(stdout,"Severity to save     = 0 to %u\n",  info.severity);
  fprintf(stdout,"Reader's pid         = %d\n",  info.reader);
  fprintf(stdout,"Proc's buffer size   = %ld  (bytes)\n", info.buffsize);
  fprintf(stdout,"Trace data in buffer = %ld  (bytes)\n", info.datasize);
  fprintf(stdout,"Trace selector       = %04X\n",info.tracesel);
  fprintf(stdout,"Hi_water mark        = %ld  (bytes)\n", info.hi_water);
  fprintf(stdout,"Proxy to trigger     = %d\n",  info.proxy);
  return(EXIT_SUCCESS);
  }
}

