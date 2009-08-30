/*
 * mt_logger.h
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

#ifndef MT_LOGGER_H_
#define MT_LOGGER_H_

#include <sys/siginfo.h>
#include <sys/types.h>

void * signal_catcher_thread( void *arg );
void* pulse_catcher_thread(void* arg);
void * mt_trace_to_file(int sig, siginfo_t *extra, void *cruft);
void * mt_new_trace_to_file(int ts_num);

#endif /* MT_LOGGER_H_ */
