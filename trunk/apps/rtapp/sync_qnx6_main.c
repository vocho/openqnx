/*
 * Auto generated QNX main program for model: slbenchmodel
 *
 * Real-Time Workshop version : 7.1  (R2008a)  23-Jan-2008
 * C source code generated on : Sun Dec 14 11:21:38 2008
 *
 */
/* ANSI C headers */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>

/*Posix and QNX headers*/
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/neutrino.h>
#include <hw/inout.h>

/* RTW Headers */
#ifdef _EXT_MODELS_	/* PDB */
#include "slbenchmodel.h"							/* Model's header file */
#include "rtwtypes.h"									/* MathWorks types */
#endif


/* this sets the standard stack size for spawned tasks used by the model.
 * this can be changed by compiling with '-DSTACK_SIZE=nnn' where nnn is
 * the stack size desired.
 */
#ifndef STACK_SIZE
#define STACK_SIZE										 16384
#endif

#define MAX_PRIO											 255
#define CYCLE													1.0
#define IRQ0													 0
#define BASE_FREQ											1193181.666
#define BASE_ADDRESS_8254							0x40

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
volatile int id1;
volatile int erreur;
struct sigevent event;
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
#ifdef _EXT_MODELS_	/* PDB */
		/* Set model inputs associated to subrate here */
		slbenchmodel_step1();
#endif
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
#ifdef _EXT_MODELS_
		/* Set model inputs associated to subrate here */
		slbenchmodel_step2();
#endif
		/* Write model outputs associated to subrate here */
	}
}

void * tBaseRate(void * cookie)
{
	int valeur_sem;
	printf("tBaseRate\n");	/* PDB */
	while (1) {
#ifdef _EXT_MODELS_	/* PDB */
		if (!
				rtmGetErrorStatus(slbenchmodel_M) == NULL) {
			fprintf(stderr,"Error status: %s \n", rtmGetErrorStatus(slbenchmodel_M));
			erreur = -1;
			sem_post(&startStopSem);
		}
#endif
		sem_getvalue(&rtClockSem, &valeur_sem);
		if (valeur_sem > 0) {
			printf("Rate for baserate too fast.\n");
			erreur = -1;
			sem_post(&startStopSem);
		}

		sem_wait(&rtClockSem);
#ifdef _EXT_MODELS_	/* PDB */
		if (rtmStepTask(slbenchmodel_M, 1))
			sem_post(&rtTaskSem1);
		if (rtmStepTask(slbenchmodel_M, 2))
			sem_post(&rtTaskSem2);

		/* Set model inputs associated with base rate here */
		slbenchmodel_step0();
#endif

		/* Get model outputs associated with base rate here */
	}
}

const struct sigevent *handler(void *area, int id)
{
	// evenevement keyboard?
	if (id == id1) {
		*((int*)area) = IRQ0;
		return(&event);
	}

	// not our IRQs
	else
		return NULL;
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
	float freq = 0;

#ifdef _EXT_MODELS_	/* PDB */
	/* Model initialisation */
	slbenchmodel_initialize(1);
#endif
	freq = 1.0/CYCLE;
	printf("Actual sample rate in Hertz: %f\n", freq);
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
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
	sem_wait(&startStopSem);

	/* Disable rt_OneStep() here */
#ifdef _EXT_MODELS_	/* PDB */
	/* Terminate model */
	slbenchmodel_terminate();
#endif
	return erreur;
}
