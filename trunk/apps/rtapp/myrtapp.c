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


/*
	int TimerAlarm( clockid_t id,
					const struct _itimer * itime,
					struct _itimer * otime );

	int TimerAlarm_r( clockid_t id,
					  const struct _itimer * itime,
					  struct _itimer * otime );

	Arguments:

	id
		The timer type to use to implement the alarm; one of:

			* CLOCK_REALTIME -- This is the standard POSIX-defined clock. Timers based on this clock should will wake up the processor if it's in a power-saving mode.
			* CLOCK_SOFTTIME -- This clock is only active when the processor is not in a power-saving mode. For example, an application using a CLOCK_SOFTTIME timer to sleep wouldn't wake up the processor when the application was due to wake up. This will allow the processor to enter a power-saving mode.

			  While the processor isn't in a power-saving mode, CLOCK_SOFTTIME behaves the same as CLOCK_REALTIME.
			* CLOCK_MONOTONIC -- This clock always increases at a constant rate and can't be adjusted.

	itime
		NULL, or a pointer to a _itimer structure that specifies the length of time to wait.
	otime
		NULL, or a pointer to a _itimer structure where the function can store the old timer trigger time.
 */

//struct sigevent event;
const struct _itimer itime = {
	.nsec = 10 * 1000000,
	.interval_nsec= 50
};


void task_1(void)
{
	printf("task_1\n");




}

void task_2(void)
{
	printf("task_2\n");




}

int main(void)
{
	int ret;
	int task = 0;

	ret= TimerAlarm(CLOCK_REALTIME, &itime, NULL);
	switch (++task % 2) {
	case 0:
		task_1();
	case 1:
		task_2();
	}
	return ret;
}


