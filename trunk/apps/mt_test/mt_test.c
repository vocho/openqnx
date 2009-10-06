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

#include <sys/neutrino.h>
#include <sys/mt_trace.h>

void * thread( void *arg ) {

	pthread_mutex_t* pMutex = arg;

	while (1)
	{
/*		SyncMutexLock((sync_t *) &pMutex[1]);
		usleep(random() % 200000);
		SyncMutexLock((sync_t *) &pMutex[0]);
		usleep(random() % 200000);
		SyncMutexUnlock((sync_t *) &pMutex[0]);
		usleep(random() % 200000);
		SyncMutexUnlock((sync_t *) &pMutex[1]);
		usleep(random() % 200000); */

		pthread_mutex_lock(&pMutex[1]);
		usleep(random() % 200000);
		pthread_mutex_lock(&pMutex[0]);
		usleep(random() % 200000);
		pthread_mutex_unlock(&pMutex[0]);
		usleep(random() % 200000);
		pthread_mutex_unlock(&pMutex[1]);
		usleep(random() % 200000);
	}
}

int main () {

//	MtCtl(_MT_CTL_DUMMY, NULL);

	int hThread;
	pthread_mutex_t mutex[2];
	pthread_mutex_init(&mutex[0], NULL);
	pthread_mutex_init(&mutex[1], NULL);

	if (pthread_create(&hThread, NULL, thread, mutex))
	{
		perror("Creating thread");
		return -1;
	}

	while (1)
	{
/*		SyncMutexLock((sync_t *) &mutex[0]);
		usleep(random() % 200000);
		SyncMutexLock((sync_t *) &mutex[1]);
		usleep(random() % 200000);
		SyncMutexUnlock((sync_t *) &mutex[1]);
		usleep(random() % 200000);
		SyncMutexUnlock((sync_t *) &mutex[0]);
		usleep(random() % 200000); */

		pthread_mutex_lock(&mutex[0]);
		usleep(random() % 200000);
		pthread_mutex_lock(&mutex[1]);
		usleep(random() % 200000);
		pthread_mutex_unlock(&mutex[1]);
		usleep(random() % 200000);
		pthread_mutex_unlock(&mutex[0]);
		usleep(random() % 200000);
	}

	return 0;
}
