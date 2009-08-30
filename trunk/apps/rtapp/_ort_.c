/*
 * apps/rtapp/_ort_.c
 *
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

/* TODO:
 * - manage task priorities
 * - adapt task actions (done in handler) load and timing
 * - get app duration from command line
 */

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/siginfo.h>

#include <sys/kercalls.h>
#include <sys/neutrino.h>
#include <sys/trace.h>
#include "../../services/system/public/sys/trace.h"

struct thread_info {
	int sec;
	int nsec;
	int sig;
	void *handler;
};
#define SET_THREADINFO(st, s, ns, signal, hand) \
	st.sec = s; \
	st.nsec = ns; \
	st.sig = signal; \
	st.handler = hand \

pthread_t my_thread1_th, my_thread2_th, my_thread3_th;
int id1 = 0, id2 = 0;
int stop = 0;

void rtsig_dummy_handler(int sig, siginfo_t *extra, void *cruft); /* prototype */

/* sample task function that computes factorial */
unsigned long long factorielle(unsigned n) {
	unsigned long long res = 1;
	unsigned	i;

	if (n == 0)
		return res;

	/* increase load by looping the calculation */
	int j = 10000;
	while(j--) {
		res = 1;

		/* this is the real computation: this tiny loop would be sufficient */
		for (i = 1; i <= n; ++i)
			res *= i;

	}
	return res;
}
/* this is the periodicly executed code */
void handler1 (int sig, siginfo_t *extra, void *cruft) {
	unsigned long long fac;
	unsigned i;

	i = 10;

	fac = factorielle(i);
	printf("action 1: factorielle(%u) = %llu\n", i, fac);

	return;
}
void handler2 (int sig, siginfo_t *extra, void *cruft) {
	unsigned long long fac;
	unsigned i;

	i = 11;

	fac = factorielle(i);
	printf("action 2: factorielle(%u) = %llu\n", i, fac);

	return;
}
void handler3 (int sig, siginfo_t *extra, void *cruft) {
	unsigned long long fac;
	unsigned i;

	i = 12;

	fac = factorielle(i);
	printf("action 3: factorielle(%u) = %llu\n", i, fac);

	return;
}
/* dummy handler */
void rtsig_dummy_handler(int sig, siginfo_t *extra, void *cruft) {
	//printf("dummy\n");
}
/* obsolete, replaced by sleep */
void alarm_handler(int sig, siginfo_t *extra, void *cruft) {
	printf("alarm_handler\n");
	return;
}

/* this function is a thread, takes a pointer to a thread_info structure
 * with timing, signal, and handler info
 * create any new thread invoking this function
 */
void * any_thread(void *pt0) {

	struct thread_info *ti = pt0;
	struct sigaction sa;
	struct sigevent sig_spec;
	sigset_t allsigs;
	struct itimerspec tmr_setting;
	timer_t timer_h;

	printf("thread init, pid = %u, tid = %u\n", getpid(), pthread_self());

	/* setup signal to respond to timer */
	sigemptyset(&sa.sa_mask);
	//sigaddset( &sa.mask, SIGRTMIN ); /* add signals to be masked while in handler */
	sa.sa_flags = SA_SIGINFO;
	//sa.sa_sigaction = ti->handler;	/* set handler */
	sa.sa_handler = ti->handler;	/* set handler (QNX) */
	if (sigaction(ti->sig, &sa, NULL) < 0)
		perror("sigaction");

	SIGEV_SIGNAL_THREAD_INIT(&sig_spec, ti->sig, ti->sig, SI_MINAVAIL);

	/* create timer, which uses the REALTIME clock */
	if (timer_create(CLOCK_REALTIME, &sig_spec, &timer_h) < 0)
		perror("timer create");

	/* set the initial expiration and frequency of timer */
	tmr_setting.it_value.tv_sec = ti->sec;
	tmr_setting.it_value.tv_nsec = ti->nsec;
	tmr_setting.it_interval.tv_sec = ti->sec;
	tmr_setting.it_interval.tv_nsec = ti->nsec;
	if ( timer_settime(timer_h, 0, &tmr_setting, NULL) < 0)
		perror("settimer");


	int i = 0;

	/* wait for signals */
	sigemptyset(&allsigs);
	while (!stop) {
		if (i++ == 2) {
			tmr_setting.it_interval.tv_sec = ti->sec / 1.5;
			tmr_setting.it_interval.tv_nsec = ti->nsec / 1.5;
			timer_settime(timer_h, 0, &tmr_setting, NULL);
			setprio(0, 25);
		}

		//printf("waiting %d\n", sig);
		sigsuspend(&allsigs);
	}

	timer_delete(timer_h);
	pthread_exit(0);
	return NULL;
}


int main()
{
	int status;
	struct thread_info thr_info1, thr_info2, thr_info3;
	sched_param_t sched_param;
	pthread_attr_t thr_attr, *thr_attr_pt;
	struct timespec tspec;

	thr_attr_pt = NULL;

	printf("main begin\n");
	printf("main pid = %u, tid %u\n", getpid(), pthread_self());

	//signal(SIGALRM, alarm_handler);

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

	thr_attr_pt = &thr_attr;
#endif

	SET_THREADINFO(thr_info1, 0, 2500000, SIGRTMIN+1, &handler1);
	status = pthread_create(&my_thread1_th, thr_attr_pt, &any_thread, &thr_info1);
	if (status != 0)
		printf ("Error while creating thread %d\n", status);

	SET_THREADINFO(thr_info2, 0, 5000000, SIGRTMIN+2, &handler2);
	status = pthread_create(&my_thread2_th, thr_attr_pt, &any_thread, &thr_info2);
	if (status != 0)
		printf ("Error while creating thread %d\n", status);

	SET_THREADINFO(thr_info3, 0, 7500000, SIGRTMIN+3, &handler3);
	status = pthread_create(&my_thread3_th, thr_attr_pt, &any_thread, &thr_info3);
	if (status != 0)
		printf ("Error while creating thread %d\n", status);

	//alarm(3);
	sleep(3);	/* this is the time the threads keep running, set is as you want */
	stop = 1;

	//pthread_timedjoin(my_thread1_th, NULL, &tspec);
	pthread_join(my_thread1_th, NULL);
	pthread_join(my_thread2_th, NULL);
	pthread_join(my_thread3_th, NULL);

	printf("main end\n");
	return 0;
}
