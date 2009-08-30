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

#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/siginfo.h>

#include <sys/kercalls.h>
#include <sys/neutrino.h>
#include <sys/trace.h>
#include "../../services/system/public/sys/trace.h"


pthread_t my_thread1_th;
pthread_t my_thread2_th;
sched_param_t sched_param;
pthread_attr_t thr_attr;

//void timer_intr(int sig, siginfo_t *extra, void *cruft);	/* prototype */
//void timer_intr2(int sig, siginfo_t *extra, void *cruft);	/* prototype */

void new_timer_create(int num_secs, int num_nsecs, void * hand_task) {

  struct sigaction sa;
  struct sigevent sig_spec;
  sigset_t allsigs;
  struct itimerspec tmr_setting;
  timer_t timer_h;

  printf("in new_timer_create\n");

  /* setup signal to respond to timer */
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  //sa.sa_sigaction = timer_intr;
  sa.sa_sigaction = hand_task;
//  sa.sa_handler = hand_task;
  if (sigaction(SIGRTMIN, &sa, NULL) < 0)
    perror("sigaction");

  sig_spec.sigev_notify = SIGEV_SIGNAL;
  sig_spec.sigev_signo = SIGRTMIN;

  /* create timer, which uses the REALTIME clock */
  if (timer_create(CLOCK_REALTIME, &sig_spec, &timer_h) < 0)
    perror("timer create");

  /* set the initial expiration and frequency of timer */
  tmr_setting.it_value.tv_sec = 1;
  tmr_setting.it_value.tv_nsec = 0;
  tmr_setting.it_interval.tv_sec = num_secs;
  tmr_setting.it_interval.tv_nsec = num_nsecs;
  if ( timer_settime(timer_h, 0, &tmr_setting,NULL) < 0)
    perror("settimer");

  /* wait for signals */
  sigemptyset(&allsigs);
  int i = 3;
  while (i--) {
	  printf("waiting\n");
    sigsuspend(&allsigs);
  }
  timer_delete(timer_h);
}

/* routine that is called when timer expires */
//void * timer_intr()
void timer_intr(int sig, siginfo_t *extra, void *cruft)
{
    /* perform periodic processing and then exit */
	//printf("task 1\n");
}

/* routine that is called when timer expires */
//void * timer_intr2()
void timer_intr2(int sig, siginfo_t *extra, void *cruft)
{
    /* perform periodic processing and then exit */
	printf("task 2\n");
}

void my_action1 () {
	printf("action 1\n");
}
void my_action2 () {
	printf("action 2\n");
}
void dummy (int sig, siginfo_t *extra, void *cruft) {
}

void * th1() {
	printf("in thread 1\n");

#if 0
new_timer_create(0, 500000000, timer_intr);
#else
	struct sigaction sa;
	struct sigevent sig_spec;
	sigset_t allsigs;
	struct itimerspec tmr_setting;
	timer_t timer_h;

	printf("in new_timer_create 1\n");
	/* setup signal to respond to timer */
#if 1
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = dummy;
	if (sigaction(SIGRTMIN, &sa, NULL) < 0)
		perror("sigaction 1");
#else

#endif
	//sig_spec.sigev_notify = SIGEV_SIGNAL;
	sig_spec.sigev_notify = SIGEV_SIGNAL_THREAD;
	sig_spec.sigev_signo = SIGRTMIN;

	/* create timer, which uses the REALTIME clock */
	if (timer_create(CLOCK_REALTIME, &sig_spec, &timer_h) < 0)
		perror("timer create 1");

	/* set the initial expiration and frequency of timer */
	tmr_setting.it_value.tv_sec = 1;
	tmr_setting.it_value.tv_nsec = 0;
	tmr_setting.it_interval.tv_sec = 0;
	tmr_setting.it_interval.tv_nsec = 500000000;

	printf("task1 tid %u\n", pthread_self());
	TraceEvent(_montt_TRACE_TASKPERIODICITY,
			getpid(), pthread_self(), tmr_setting.it_interval.tv_sec, tmr_setting.it_interval.tv_nsec);

	if ( timer_settime(timer_h, 0, &tmr_setting,NULL) < 0)
		perror("settimer 1");

	  /* wait for signals */
	  sigemptyset(&allsigs);
	  int i = 4;
	  while (i--) {
		  printf("waiting 1\n");
		  sigsuspend(&allsigs);
		  my_action1();
	  }
#endif
	  timer_delete(timer_h);
	  //TimerDestroy_r()

	pthread_exit(&my_thread1_th);
	return;
}

void * th2() {
	printf("in thread 2\n");

#if 0
new_timer_create(1, 0, timer_intr2);
#else
	struct sigaction sa2;
	struct sigevent sig_spec2;
	sigset_t allsigs2;
	struct itimerspec tmr_setting2;
	timer_t timer_h2;

	printf("in new_timer_create 2\n");

#if 1
	/* setup signal to respond to timer */
	sigemptyset(&sa2.sa_mask);
	sa2.sa_flags = SA_SIGINFO;
	//sa2.sa_sigaction = timer_intr2;
	//sa2.sa_sigaction = NULL;
	sa2.sa_sigaction = dummy;
	if (sigaction(SIGRTMIN, &sa2, NULL) < 0)
		perror("sigaction 2");
#else
	//if (sigaction(SIGRTMIN, NULL, NULL) < 0)
		//	perror("sigaction 2");
#endif
	//sig_spec2.sigev_notify = SIGEV_SIGNAL;
	sig_spec2.sigev_notify = SIGEV_SIGNAL_THREAD;
	sig_spec2.sigev_signo = SIGRTMIN;

	/* create timer, which uses the REALTIME clock */
	if (timer_create(CLOCK_REALTIME, &sig_spec2, &timer_h2) < 0)
		perror("timer create 2");

	/* set the initial expiration and frequency of timer */
	tmr_setting2.it_value.tv_sec = 1;
	tmr_setting2.it_value.tv_nsec = 0;
	tmr_setting2.it_interval.tv_sec = 1;
	tmr_setting2.it_interval.tv_nsec = 0;
	if ( timer_settime(timer_h2, 0, &tmr_setting2, NULL) < 0)
		perror("settimer 2");

	  /* wait for signals */
	  sigemptyset(&allsigs2);
	  int i = 4;
	  while (i--) {
		  printf("waiting 2\n");
		  sigsuspend(&allsigs2);
		  my_action2();
	  }
#endif
	  timer_delete(timer_h2);
	pthread_exit(&my_thread2_th);
	return;
}

int main()
{
	int status;

	printf("main begin\n");
	printf("main tid %u\n", pthread_self());

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
//	status = pthread_create(&my_thread1_th, NULL, timer_intr, NULL);
	if (status != 0)
		printf ("Error while creating thread %d\n", status);

	status = pthread_create(&my_thread2_th, NULL, &th2, NULL);
	//status = pthread_create(&my_thread2_th, NULL, timer_intr2, NULL);
	if (status != 0)
		printf ("Error while creating thread %d\n", status);

#if 0
	new_timer_create(1, 0, timer_intr);
	//new_timer_create(1, 0, &timer_intr2);
#endif

	pthread_join(my_thread1_th, NULL);
	pthread_join(my_thread2_th, NULL);

	printf("main end\n");
	return 0;
}
