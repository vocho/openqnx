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
//#define	_PDB_mask_
#ifdef _PDB_mask_
#ifndef STACK_SIZE
#define STACK_SIZE										 16384
#endif

#define MAX_PRIO											 255
#define CYCLE													1.0
#define IRQ0													 0
#define BASE_FREQ											1193181.666
#define BASE_ADDRESS_8254							0x40
#endif

sem_t rtClockSem;
sem_t startStopSem;
sem_t rtTaskSem1;
sem_t rtTaskSem2;
sched_param_t parametre;
pthread_attr_t attribut;
pthread_t tBaseRateDesc;
pthread_t tInt_thread;
pthread_t tSubRate1Desc;
pthread_t tSubRate2Desc;
pthread_t my_thread1_th, my_thread2_th;
volatile int id1;
volatile int erreur;
struct sigevent event;
#if 0
const struct _itimer itime = {
	.nsec = 2 * 1000000000,	/* '* 1000000000' makes seconds */
	.interval_nsec= 100 * 1000000	/* '* 1000000' makes miliseconds */
};
#else
struct _itimer itime = {
	2 * 1000000000,	/* '* 1000000000' makes seconds */
	100 * 1000000	/* '* 1000000' makes miliseconds */
};
#endif
const struct _itimer *ipoint= &itime;
struct _itimer otime;
struct _itimer *opoint= &otime;
int task = 0;

void my_thread1(void * cookie)	/* PDB */
{
	sem_wait(&rtTaskSem1);
	printf("my_thread1\n");
	sem_post(&rtTaskSem1);
	pthread_exit(NULL);
}
void my_thread2(void * cookie)	/* PDB */
{
	sem_wait(&rtTaskSem2);
	printf("my_thread2\n");
	sem_post(&rtTaskSem2);
	pthread_exit(NULL);
}

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
#ifdef _PDB_mask_
const struct sigevent *handler(void *area, int id)
{

	printf("handler: id= %u\n", id);

	switch (id) {
	case SIGALRM: {
		tSubRate_1(NULL);
		break;
	}
	default: {
		// evenevement keyboard?
		if (id == id1) {
			*((int*)area) = IRQ0;
			return(&event);
		} else	// not our IRQs
		return NULL;
	}
	}
}
#endif

void alarm_handler(int signum)	/* PDB */
{

	signal(SIGALRM, alarm_handler); /* PDB: it looks like
	the handler expires and
	must be reregistered each time */

	printf("alarm_handler: signum= %u", signum);
	if (signum == SIGALRM)
		printf(" (is alarm)");
	else
		printf(" OTHER SIGNAL");
	printf("\n");

	task= task % 2 + 1;
	printf("task is %u\n", task);	//*/
//*
	switch (task) {
	case 1: {
		//printf("task 1\n");
#if 0
		status = pthread_attr_getschedparam(&attribut, &parametre);
		if (status != 0)
			printf("Error while getting scheduler parameters : code <%d>\n", status);

		parametre.sched_priority = MAX_PRIO - 1 - 1;

		status = pthread_attr_setschedparam(&attribut, &parametre);
		if (status != 0)
			printf ("Error while setting new scheduler parameters %d\n", status);
#endif
		pthread_create(&my_thread1_th, NULL, my_thread1, NULL);
		//tSubRate_1(NULL);
		break;
	}
	case 2: {
		//printf("task 2\n");
#if 0
		status = pthread_attr_getschedparam(&attribut, &parametre);
		if (status != 0)
			printf("Error while getting scheduler parameters : code <%d>\n", status);

		parametre.sched_priority = MAX_PRIO - 1;

		status = pthread_attr_setschedparam(&attribut, &parametre);
		if (status != 0)
			printf ("Error while setting new scheduler parameters %d\n", status);
#endif
		pthread_create(&my_thread2_th, NULL, my_thread2, NULL);
		//tSubRate_2(NULL);
		break;
	}
	default: {
		printf("alarm_handler ERROR: task= %u\n", task);
		break;
	}
	}
	//*/
}

void * int_thread(void * cookie)
{
	int i;
	int area;
	int cptVal;
	unsigned long clk;
	unsigned char msb;
	unsigned char lsb;

	printf("int_thread\n");	/* PDB */
#ifdef _PDB_mask_
	// Request I/O privity
	ThreadCtl(_NTO_TCTL_IO, 0);
	clk = mmap_device_io(4, BASE_ADDRESS_8254);
	cptVal = CYCLE * BASE_FREQ;
	lsb = cptVal & 0xFF;
	msb = cptVal >> 8;

	/*acces au control word*/
	out8(clk + 3, 0x34);

	/*affectation du lsb et du msb pour le timer 0*/
	out8(clk, lsb);
	out8(clk, msb);
	munmap_device_io(4, BASE_ADDRESS_8254);

	// Initialize event structure
	event.sigev_notify = SIGEV_INTR;
	id1= InterruptAttach(IRQ0, &handler, &area, 0, 0 );
	for (i = 0; i < 5000; i++ ) {
		InterruptWait( 0, NULL );
		if (area == IRQ0) {
			sem_post(&rtClockSem);
		}
	}

	// Disconnect the ISR handler
	InterruptDetach(id1);
	erreur = 0;
	sem_post(&startStopSem);
#endif
	return NULL;
}

void cleanExit(int dummy)
{
	printf("cleanup...\n");
	pthread_exit(&tBaseRateDesc);

	/* Task deletion */
	pthread_exit(&tSubRate2Desc);
	pthread_exit(&tSubRate1Desc);

	/* Semaphores deletion */
	sem_destroy(&rtTaskSem2);
	sem_destroy(&rtTaskSem1);
	sem_destroy(&rtClockSem);
	sem_destroy(&startStopSem);
	printf("quit..\n");
}

int main(void)
{
	int status;

#ifdef _PDB_mask_
	float freq = 0;
	freq = 1.0/CYCLE;
	printf("Actual sample rate in Hertz: %f\n", freq);
#endif
	itime.interval_nsec= 100 * 1000000;
	printf("itime duration: %u [nsec], Interval= %u [nsec]\n",
			itime.nsec, itime.interval_nsec);
	//printf("taille itime= %u\n", sizeof(itime));

	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	//signal(SIGALRM, handler);	/* PDB (doesn't work) */
	signal(SIGALRM, alarm_handler);	/* PDB */

	mlockall(MCL_CURRENT | MCL_FUTURE);

	status = sem_init(&rtClockSem, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore : %d\n", status);

	status = sem_init(&startStopSem, NULL, 0);
	if (status != 0)
		printf("Error creating semaphore startStopSem : %d\n", status);

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

#ifdef _PDB_mask_
	status = pthread_attr_getschedparam(&attribut, &parametre);
	if (status != 0)
		printf("Error while getting scheduler parameters : code <%d>\n", status);

	parametre.sched_priority = MAX_PRIO - 1 - 1;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);

	status = pthread_create(&tSubRate1Desc, &attribut, tSubRate_1, NULL);
	if (status != 0)
		printf ("Error creating tSubRate_1 : %d\n", status);

	parametre.sched_priority = MAX_PRIO - 1 - 2;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);

	status = pthread_create(&tSubRate2Desc, &attribut, tSubRate_2, NULL);
	if (status != 0)
		printf ("Error creating tSubRate_2 : %d\n", status);

	parametre.sched_priority = MAX_PRIO - 1;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);

	status = pthread_create(&tBaseRateDesc, &attribut, tBaseRate, NULL);
	if (status != 0)
		printf ("Error creating task	tBaseRate: %d\n", status);

	parametre.sched_priority = MAX_PRIO;

	status = pthread_attr_setschedparam(&attribut, &parametre);
	if (status != 0)
		printf ("Error while setting new scheduler parameters %d\n", status);
	status = pthread_create(&tInt_thread, &attribut, int_thread, NULL);
	if (status != 0)
		printf ("Error creating task interrupt manager : %d\n", status);
#endif
	//status = TimerAlarm_r(CLOCK_REALTIME, &itime, NULL);	/* PDB */
	status = TimerAlarm_r(CLOCK_REALTIME, ipoint, opoint);	/* PDB */
	if (status != 0)
		printf("Error stetting TimerAlarm : %d\n", status);
	printf("itime duration: %u [nsec], Interval= %u [nsec]\n",
		itime.nsec, itime.interval_nsec);
	printf("otime duration: %u [nsec], Interval= %u [nsec]\n",
		otime.nsec, otime.interval_nsec);

	sem_wait(&startStopSem);

	/* Disable rt_OneStep() here */

	return erreur;
}

