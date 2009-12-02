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

#include "mt_test.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>

#include <sys/neutrino.h>
#include <sys/mt_trace.h>

/******************* DEADLOCK & INVERSION ******************
struct syncs {
	pthread_mutex_t mut1;
	pthread_mutex_t mut2;
	sem_t sem1;
};
******************* DEADLOCK & INVERSION ******************/

/******************* DEADLOCK ******************
void * thread1( void *arg ) {
	struct syncs* pSync = arg;
	sem_wait(&pSync->sem1);
	sleep(1);
	pthread_mutex_lock(&pSync->mut1);
	return 0;
}
void * thread2( void *arg ) {
	struct syncs* pSync = arg;
	pthread_mutex_lock(&pSync->mut2);
	sleep(1);
	sem_wait(&pSync->sem1);
	return 0;
}
void * thread3( void *arg ) {
	struct syncs* pSync = arg;
	pthread_mutex_lock(&pSync->mut1);
	sleep(1);
	pthread_mutex_lock(&pSync->mut2);
	return 0;
}
****************** DEADLOCK *********************/

/******************* INVERSION ******************
void * thread1( void *arg ) {
	struct syncs* pSync = arg;
	pthread_mutex_lock(&pSync->mut1);
	usleep(500000);
	sem_wait(&pSync->sem1);
	return 0;
}
void * thread2( void *arg ) {
	int i;
	struct syncs* pSync = arg;
	while (1);
	return 0;
}
void * thread3( void *arg ) {
	int i;
	struct syncs* pSync = arg;
	sem_wait(&pSync->sem1);
	usleep(5000000);
	sem_post(&pSync->sem1);
	return 0;
}
******************* INVERSION *******************/

void handler(signo)
{
	unsigned int i, j;
	int x = 20000000;
	static int bool = 1;

	if (bool) {

		printf("Caught signal\n");
		//usleep(1300000);

		for (i=1; i<10000; i++) {
			for (j=1; j<10000; j++) {
				x /= 3;
				x += 2;
				x *= 2;
			}
		}

		printf("X = %d\n", x);

		bool = 0;
	}
	else
		bool = 1;
}


int main () {

/******************* DEADLOCK & INVERSION ******************
	int hThread;
	struct syncs sync;

	pthread_mutex_init(&sync.mut1, NULL);
	pthread_mutex_init(&sync.mut2, NULL);
	sem_init(&sync.sem1, NULL, 1);
******************* DEADLOCK & INVERSION ******************/

/* ****************** DEADLOCK ******************
	pthread_create(&hThread, NULL, thread1, &sync);
	pthread_create(&hThread, NULL, thread2, &sync);
	pthread_create(&hThread, NULL, thread3, &sync);

	void* pRet;
	pthread_join(hThread, &pRet);
****************** DEADLOCK ****************** */

/******************* INVERSION ******************
	pthread_mutex_lock(&sync.mut1);
	pthread_create(&hThread, NULL, thread1, &sync);
	pthread_setschedprio(hThread, 10);
	usleep(500000);

	pthread_create(&hThread, NULL, thread2, &sync);
	pthread_setschedprio(hThread, 8);
	usleep(500000);

	pthread_create(&hThread, NULL, thread3, &sync);
	pthread_setschedprio(hThread, 5);
	usleep(500000);

	pthread_mutex_unlock(&sync.mut1);

	void* pRet;
	pthread_join(hThread, &pRet);
******************* INVERSION *******************/

	timer_t timer;
	struct itimerspec time;
	struct sigaction act;
	sigset_t set;
	int i;

	time.it_value.tv_sec = 1;
	time.it_value.tv_nsec = 0;
	time.it_interval.tv_sec = 1;
	time.it_interval.tv_nsec = 0;


	timer_create(CLOCK_REALTIME, NULL, &timer);
	timer_settime(timer, 0, &time, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &handler;
	sigaction(SIGALRM, &act, NULL);

	for (i=0; i<5; i++)
		sleep(10);

	return 0;
}
