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

/* ANSI C headers */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>

/*Posix and QNX headers*/
#include <unistd.h>	/* alarm() */
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/neutrino.h>	/* TimerAlarm_r() ; TimerCreate_r()*/
#include <hw/inout.h>


/* this sets the standard stack size for spawned tasks used by the model.
 * this can be changed by compiling with '-DSTACK_SIZE=nnn' where nnn is
 * the stack size desired.
 */
#define	_PDB_threads_	/* this is for using my threads instead of Cedric ones */
//#define _PDB_use_normal_timer_	/* instead of TimerAlarm */
#define MAX_PRIO											 255

#ifndef _PDB_threads_
sem_t rtClockSem;
sem_t startStopSem;
#endif
sem_t rtTaskSem1;
sem_t rtTaskSem2;
sched_param_t parametre;
pthread_attr_t attribut;
pthread_t tSubRate1Desc;
pthread_t tSubRate2Desc;
pthread_t my_thread1_th, my_thread2_th;
volatile int id1;
volatile int erreur;
struct sigevent event;

const struct _itimer itime = {
	.nsec = 1000000000,			/* '* 1000000' makes seconds */
	.interval_nsec= 1000000000l	/* '* 1000000' makes miliseconds */
};
//const struct _itimer *ipoint= &itime;
//struct _itimer otime;
//struct _itimer *opoint= &otime;
int task = 0;
int handled = 0;
#define MAX	10	/* 0 for infinite */


void cleanExit(int dummy);


#ifdef _PDB_threads_
void * my_thread1(void * cookie)	/* PDB */
{
	while (1) {
		sem_wait(&rtTaskSem1);
		printf("my_thread1\n");
	}
}

void * my_thread2(void * cookie)	/* PDB */
{
	while (1) {
		sem_wait(&rtTaskSem2);

		printf("my_thread2\n");
	}
}
#else
void * tSubRate_1(void * cookie)
{
	int valeur_sem;
	printf("tSubRate_1\n");	/* PDB */
	while (1) {
		/*une valeur sup�rieure a 0 indique que la tache n'a pas fini son cycle...*/
		sem_getvalue(&rtTaskSem1, &valeur_sem);
		if (valeur_sem > 0) {
			printf("Rate for SubRateTask1 too fast.\n");
			erreur = -1;
			sem_post(&startStopSem);
		}

		sem_wait(&rtTaskSem1);

		/* Write model outputs associated to subrate here */
	}
}

void * tSubRate_2(void * cookie)
{
	printf("tSubRate_2\n");	/* PDB */
	int valeur_sem;
	while (1) {
		/*une valeur sup�rieure a 0 indique que la tache n'a pas fini son cycle...*/
		sem_getvalue(&rtTaskSem2, &valeur_sem);
		if (valeur_sem > 0) {
			printf("Rate for SubRateTask2 too fast.\n");
			erreur = -1;
			sem_post(&startStopSem);
		}

		sem_wait(&rtTaskSem2);

		/* Write model outputs associated to subrate here */
	}
}
#endif

#ifndef _PDB_threads_
void * tBaseRate(void * cookie)
{
	int valeur_sem;
	printf("tBaseRate\n");	/* PDB */
	while (1) {

		sem_getvalue(&rtClockSem, &valeur_sem);
		if (valeur_sem > 0) {
			printf("Rate for baserate too fast.\n");
			erreur = -1;
			sem_post(&startStopSem);
		}

		sem_wait(&rtClockSem);

		/* Get model outputs associated with base rate here */
	}
}
#endif

void alarm_handler(int signum)	/* PDB */
{

//*
	printf("alarm_handler: signum= %u", signum);
	if (signum == SIGALRM)
		printf(" (is alarm)");
	else
		printf(" OTHER SIGNAL");
	printf("\n");

	if (++handled == MAX) {
		cleanExit(handled);
		signal(SIGALRM, NULL);
		return;
	} else {
		/* PDB: it looks like
		the handler expires and
		must be reregistered each time */
		//signal(SIGALRM, alarm_handler);
	}

	task= task % 2 + 1;
	//printf("task is %u\n", task);	//*/
//*
	switch (task) {
	case 1: {
		//printf("task 1\n");
		sem_post(&rtTaskSem1);
		break;
	}
	case 2: {
		//printf("task 2\n");
		sem_post(&rtTaskSem2);
		break;
	}
	default: {
		printf("alarm_handler ERROR: task= %u\n", task);
		break;
	}
	}
	//*/
}


void cleanExit(int dummy)
{
	printf("cleanup...\n");

	/* Task deletion */
#ifdef _PDB_threads_
	pthread_exit(&my_thread1_th);
	pthread_exit(&my_thread2_th);
#else
	pthread_exit(&tSubRate2Desc);
	pthread_exit(&tSubRate1Desc);
#endif
	/* Semaphores deletion */
	sem_destroy(&rtTaskSem2);
	sem_destroy(&rtTaskSem1);
#ifndef _PDB_threads_
	sem_destroy(&rtClockSem);
	sem_destroy(&startStopSem);
#endif
	printf("quit..\n");
}

int main(void)
{
	int status;

	//itime.interval_nsec= 100 * 1000000;
	printf("itime duration: %lld [nsec], Interval= %lld [nsec]\n",
			itime.nsec, itime.interval_nsec);
	//printf("size itime= %u\n", sizeof(itime));

	/* signal handlers registration */
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	signal(SIGALRM, alarm_handler);	/* PDB */

	mlockall(MCL_CURRENT | MCL_FUTURE);
#ifndef _PDB_threads_
	status = sem_init(&rtClockSem, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore : %d\n", status);

	status = sem_init(&startStopSem, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore startStopSem : %d\n", status);
#endif
	/* recuperation attribut par defaut thread*/
	status = pthread_attr_init(&attribut);
	if (status != 0)
		printf ("Error while getting thread attributs %d\n", status);

	/* Semaphore creation */
	status = sem_init(&rtTaskSem1, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore1 : %d\n", status);

	status = sem_init(&rtTaskSem2, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore2 : %d\n", status);

	printf("fin\n");
	/* Thread 1 */
	status = pthread_attr_getschedparam(&attribut, &parametre);
	if (status != 0)
		printf("Error while getting scheduler parameters : code <%d>\n", status);

	parametre.sched_priority = MAX_PRIO - 1 - 1;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);

#ifdef _PDB_threads_
	status = pthread_create(&my_thread1_th, NULL, my_thread1, NULL);
#else
	status = pthread_create(&tSubRate2Desc, &attribut, tSubRate_1, NULL);
#endif
	if (status != 0)
			printf ("Error while setting new scheduler parameters %d\n", status);

	/* Thread 2 */
	parametre.sched_priority = MAX_PRIO - 1 - 2;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);

#ifdef _PDB_threads_
	status = pthread_create(&my_thread1_th, NULL, my_thread2, NULL);
#else
	status = pthread_create(&tSubRate2Desc, &attribut, tSubRate_1, NULL);
#endif
	if (status != 0)
			printf ("Error while setting new scheduler parameters %d\n", status);

	/* TimerAlarm */
	status = TimerAlarm_r(CLOCK_REALTIME, &itime, NULL);	/* PDB */
	//status = TimerAlarm_r(CLOCK_REALTIME, ipoint, opoint);	/* PDB */
	if (status != 0)
		printf("Error stetting TimerAlarm : %d\n", status);
	/*
	printf("itime duration: %u [nsec], Interval= %u [nsec]\n",
		itime.nsec, itime.interval_nsec);
	 printf("otime duration: %u [nsec], Interval= %u [nsec]\n",
		otime.nsec, otime.interval_nsec); //*/

#ifndef _PDB_threads_
	sem_wait(&startStopSem);
#endif
	pthread_join(my_thread1_th, NULL);
	pthread_join(my_thread2_th, NULL);
	/* Disable rt_OneStep() here */
	printf("fin\n");
	return erreur;
}

