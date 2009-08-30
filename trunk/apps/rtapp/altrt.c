/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>	
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/kercalls.h>
#include <sys/neutrino.h>

#include <sys/trace.h>
#include "../../services/system/public/sys/trace.h"

#include <time.h>
#include <pthread.h>
//#include <sched.h>
#include <sys/siginfo.h>


#define RTSIG1 RTSIGMIN + 1
#define RTSIG2 RTSIGMIN + 2
#define RTSIG3 RTSIGMIN + 3


pthread_t my_thread1_th;
sched_param_t sched_param;
pthread_attr_t thr_attr;


void hand_1(int sig, siginfo_t *extra, void *cruft) {

}

void * new_timer_create(int num_secs, int num_nsecs, unsigned sig, void *handler, unsigned times) {

struct sigaction sa;
	struct sigevent sig_spec;
	sigset_t allsigs;
	struct itimerspec tmr_setting;
	timer_t timer_h;

	/* setup signal to respond to timer */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	if (sigaction(sig, &sa, NULL) < 0)
		perror("sigaction");

	sig_spec.sigev_notify = SIGEV_SIGNAL;
	sig_spec.sigev_signo = sig;
	/* create timer, which uses the REALTIME clock */
	if (timer_create(CLOCK_REALTIME, &sig_spec, &timer_h) < 0)
		perror("timer create");
#if 0
	TraceEvent(_montt_TRACE_TASKPERIODICITY,
		  getpid(), pthread_self(), num_secs, num_nsecs);
#endif
	/* set the initial expiration and frequency of timer */
	tmr_setting.it_value.tv_sec = 1;
	tmr_setting.it_value.tv_nsec = 0;
	tmr_setting.it_interval.tv_sec = num_secs;
	tmr_setting.it_interval.tv_sec = num_nsecs;
	if ( timer_settime(timer_h, 0, &tmr_setting, NULL) < 0)
		perror("settimer");

	/* wait for signals */
	sigemptyset(&allsigs);
	char i = times;
	while (--i) {
		sigsuspend(&allsigs);
	}
	if (timer_delete(timer_h) < 0)
		perror("timer delete");

	return NULL;
}
/* routine that is called when timer expires */
void hand_1(int sig, siginfo_t *extra, void *cruft)
{
    /* perform periodic processing and then exit */

	printf("periodic task 1\n");

}

void hand_2(int sig, siginfo_t *extra, void *cruft)
{
    /* perform periodic processing and then exit */

	printf("periodic task 2\n");

}



int main()
{
	int status;

	printf("main begin\n");

#if 1
	status = pthread_attr_init(&thr_attr);
	if (status != 0)
		printf ("Error while getting thread attributes %d\n", status);

	status = pthread_attr_getschedparam(&thr_attr, &sched_param);
	if (status != 0)
		printf("Error while getting scheduler parameters : code <%d>\n", status);

	status = pthread_attr_setschedparam(&thr_attr, &sched_param);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);
#endif

//	status = pthread_create(&my_thread1_th, &thr_attr, &th1, NULL);
	status = pthread_create(&my_thread1_th, NULL, &th1, NULL);

	pthread_join(my_thread1_th, NULL);

	printf("main end\n");
	return 0;
}
