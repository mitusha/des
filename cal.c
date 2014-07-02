/*
 * Implementation of calendar.
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

#if _POSIX_THREAD_SAFE_FUNCTIONS == -1
# error Thread safe functions not available
#endif

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <inttypes.h>
#include <mcheck.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "cal.h"
#include "error.h"
#include "system.h"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
//#define debug(fmt, ...) (void) 0

#define INTERNAL_ERROR(errstr)	\
	errx(EXIT_FAILURE, _("%s(): INTERNAL ERROR at line %d (%s-%s): %s"),	\
	__func__, __LINE__, VERSION, __DATE__, errstr)

/* Calendar times */
#pragma GCC visibility push(hidden)
double start_time;
double end_time;
double cur_time;
#pragma GCC visibility pop

/* State of the simulation */
static enum {
	SIM_START,
	SIM_INITIALIZED,
	SIM_IN_PROGRESS,
	SIM_TERMINATED,
} state;

/* This is the calendar itself */
static struct cal *cal;

static bool new_element_added;

/* Mutex protecting the calendar */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Iterate over a calendar with prefetching */
#define cal_for_each(pos) \
	for (pos = cal; pos != NULL; prefetch(pos->next), pos = pos->next)

/* Remove head of the calendar */
#define cal_remove_head()			\
do {						\
	pthread_mutex_lock(&lock);		\
	if (cal) {				\
		struct cal *__tmp = cal;	\
		cal = cal->next;		\
		free(__tmp);			\
	}					\
	pthread_mutex_unlock(&lock);		\
} while (0)

size_t get_head(void)
{
	return cal->idx;
}

void del_head(void)
{
	cal_remove_head();
}

/* Compare two processes according to time and priority */
static bool process_compare(size_t new, size_t old)
{
	/* new process is later than old
	 * or thay are in the same time but old process has
	 * higher priority */
	if (isgreater(process_list[new].atime, process_list[old].atime)
	    || (!islessgreater(process_list[new].atime, process_list[old].atime)
		&& process_list[new].prio <= process_list[old].prio))
		return true;

	return false;
}

/* Add new element into calendar */
int add_elem(size_t idx)
{
	/* Get the mutex */
	pthread_mutex_lock(&lock);

#define this process_list[idx]

	new_element_added = true;

	/* Create a new cal entry */
	struct cal *new = xcalloc(1, sizeof(*new));

	/* Set up index of the process */
	new->idx = idx;

	struct cal *prev = NULL;
	struct cal *act = cal;

	/* while act is in cal and new process should be after act */
	while (act != NULL && process_compare(new->idx, act->idx)) {
		prev = act;
		act = act->next;
		/* skip process with same PID */
		if (act && new->idx == act->idx)
			act = act->next;
	}

	if (prev == NULL) {
		/* cal is empty, create new head */
		new->next = act;
		cal = new;
	} else {
		/* insert item between prev and act */
		prev->next = new;
		new->next = act;
	}

#undef this

	/* Release the mutex */
	pthread_mutex_unlock(&lock);

	return 0;
}

/* Initialize the simulation */
int Init(double t0, double t1)
{
	if (t0 > t1 || t0 < 0.0 || t1 < 0.0) {
		/* Invalid arguments */
		simerr = GLOB_INVAL;
		return -1;
	}

	/* Initialize times */
	start_time = cur_time = t0;
	end_time = t1;

	/* Now the initialization's over */
	state = SIM_INITIALIZED;

	return 0;
}

/* Run the simulation until finished */
int Run(void)
{
	/* Sanity check */
	if (state != SIM_INITIALIZED) {
		simerr = GLOB_NOTINIT;
		return -1;
	}
#define this process_list[cp->idx]
	struct cal *cp = cal;

	puts("Initial state of the calendar");
	printf("top ->\n");
	cal_for_each(cp) {
		printf("        [ idx:%d atime:%g prio:%d state:%c ]\n", cp->idx,
		       this.atime, this.prio, TASK_STATE_TO_CHAR_STR[this.state]);
	}
	fputc_unlocked('\n', stdout);

	puts("<< START OF SIMULATION >>");

	cp = cal;

	/* The main loop */
	while (cp) {
		int res;

		printf("        [ idx:%d atime:%f prio:%d state:%c ]\n", cp->idx,
		       this.atime, this.prio, TASK_STATE_TO_CHAR_STR[this.state]);

		/* Get the mutex */
		res = pthread_mutex_lock(&this.lock);
		if (unlikely(res))
			printf("pthread_mutex_lock: %s\n", strerror(res));

		/* Update current simulation time */
		cur_time = this.atime;

		/* Did we reach end time? */
		if (cur_time >= end_time)
			/* Yes, end simulation */
			goto out;

		if (this.state == TASK_WAKING) {;
			/* Mark thread as running */
			this.state = TASK_RUNNING;

			/* This thread wasn't created, create it now */
			res =
			    pthread_create(&this.th, NULL, this.behaviour,
					   NULL);
			if (unlikely(res))
				printf("pthread_create: %s\n", strerror(res));
		} else if (this.state == TASK_STOPPED) {
			/* Mark thread as running */
			this.state = TASK_RUNNING;

			/* Thread is sleeping, wake it up now */
			res = pthread_cond_signal(&this.cond);
			if (unlikely(res))
				printf("pthread_cond_signal: %s\n",
				       strerror(res));
		} else if (this.state == TASK_DEAD)
			goto out;

		/* Wait for signal from child */
		do
			pthread_cond_wait(&this.cond, &this.lock);
		while (0 && this.state == TASK_RUNNING);

		/* Only join if the state is TASK_DEAD */
		if (this.state == TASK_DEAD) {
			res = pthread_join(this.th, NULL);
			if (unlikely(res))
				printf("pthread_join: %s\n", strerror(res));
		}

		/* Release the mutex */
		res = pthread_mutex_unlock(&this.lock);
		if (unlikely(res))
			printf("pthread_mutex_unlock: %s\n", strerror(res));

		if (this.state == TASK_DEAD)
			/* Invalidate data in process_list */
			destroy_process(cp->idx);

		/* Move to next entry */
		if (new_element_added || this.state == TASK_DEAD) {
			cp = cp->next;

			/* Remove processed element */
			cal_remove_head();
		}
	}
 out:
	puts("<< END OF SIMULATION >>\n");

	return 0;
}
