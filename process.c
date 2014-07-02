/*
 * Implementation of process.
 *
 * Copyright (C) 2010, Marek Polacek <xpolac06@stud.fit.vutbr.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <assert.h>
#include <err.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "system.h"
#include "cal.h"
#include "process.h"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
//#define debug(fmt, ...) ((void)0)

/* Array of processes in the system */
struct process_struct *process_list attribute_hidden;

/* Number of processes in the system */
size_t process_count attribute_hidden;

/* Lock for process_list */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Allocates and initializes a new process_struct.
 * The actual kick-off is left to the calendar.
 * Returns index of the process in the process_list or -1 when error.
 */
int create_process(void *(*tf) (void *), int prio)
{
	/* Get the mutex */
	pthread_mutex_lock(&lock);

	/* Allocate space for process */
	const size_t sz = (process_count + 1) * sizeof(struct process_struct);
	process_list = xrealloc(process_list, sz);


#define this process_list[process_count]
	/* Initialize this new process */
	this.state = TASK_WAKING;
	this.prio = prio;
	// XXX Toto je jen pro testovani funkcnosti vkladani na spravne misto
	// XXX aby nemely vsechny prvky stejny atime
	this.atime = cur_time;
	//this.atime = process_count % 2 == 0 ? prio ^ 3 : prio | 3;
	this.th = (pthread_t)0;
	this.behaviour = tf;

	/* Initialize lock and cond */
	if (pthread_mutex_init(&this.lock, NULL)
	    || pthread_cond_init(&this.cond, NULL))
		return -1;

	/* Now the thread is ready to run */

	/* Add this process into calendar */
	add_elem(process_count);

	/* We have a new process */
	process_count++;

	/* Release the mutex */
	pthread_mutex_unlock(&lock);

	/* We have to return current index, not the new */
	return process_count - 1;
#undef this
}

/* Suspends thread until we receive a signal */
int Wait(double t)
{
#define this process_list[i]
	const size_t i = CURRENT();

	/* Get the mutex */
	pthread_mutex_lock(&this.lock);

	/* Mark process as stopped */
	this.state = TASK_STOPPED;

	/* Re-schedule */
	this.atime = t + cur_time;
	//this.atime = t + cur_time * i;

	/* Add entry into the calendar */
	add_elem(i);

	/* Tell calendar we're done */
	int e = pthread_cond_signal(&this.cond);
	if (unlikely(e))
		printf("pthread_cond_signal: %s\n", strerror(e));

	e = pthread_cond_wait(&this.cond, &this.lock);
	if (unlikely(e))
		printf("pthread_cond_wait: %s\n", strerror(e));

	/* Release the mutex */
	pthread_mutex_unlock(&this.lock);

	return 0;
#undef this
}

/* Mark process as terminated */
int Quit(void)
{
#define this process_list[i]
	const size_t i = CURRENT();

	/* Get the mutex */
	pthread_mutex_lock(&this.lock);

	/* Mark process as dead */
	this.state = TASK_DEAD;

	/* Tell calendar we're done */
	int e = pthread_cond_signal(&this.cond);
	if (unlikely(e))
		printf("pthread_cond_signal: %s\n", strerror(e));

	/* Release the mutex */
	e = pthread_mutex_unlock(&this.lock);
	if (unlikely(e))
		printf("pthread_mutex_unlock: %s\n", strerror(e));

	return e;
#undef this
}

/* Invalidate entry in process_list */
int destroy_process(size_t i)
{
#define this process_list[i]
	int e;

	/* Invalidate values */
	this.prio = -1;
	this.atime = 0.0;
	this.th = (pthread_t)0;

	/* Destroy lock and cond */
	e = pthread_cond_destroy(&this.cond);
	if (unlikely(e))
		printf("PROCESS pthread_cond_destroy: %s\n", strerror(e));
	
	/*
	e = pthread_mutex_destroy(&this.lock);
	if (unlikely(e))
		printf("PROCESS pthread_mutex_destroy: %s\n", strerror(e));
	*/

	return e;
#undef this
}

static void __attribute__((destructor)) process_cleanup(void)
{
	/* Free whole process_list array */
	free(process_list);
}
