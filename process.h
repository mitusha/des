/*
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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <pthread.h>

/* Process states */
#define TASK_RUNNING		0	/* Thread is running */
#define TASK_STOPPED		1	/* Thread is stopped */
#define TASK_DEAD		2	/* Thread should be joined */
#define TASK_WAKING		3	/* Thread wasn't created yet */

#define TASK_STATE_TO_CHAR_STR "RSDW"

/* Returns index in process_list of current thread */
#define CURRENT()							\
({									\
	size_t __i = 0;							\
	while (!pthread_equal(pthread_self(), process_list[__i].th))	\
		__i++;							\
	if (unlikely(__i > process_count))				\
		INTERNAL_ERROR("couldn't find process_struct");		\
	__i;								\
})


#define INTERNAL_ERROR(errstr)	\
	errx(EXIT_FAILURE, _("%s(): INTERNAL ERROR at line %d (%s-%s): %s"),	\
	__func__, __LINE__, VERSION, __DATE__, errstr)


/*
 * Usage:
 * state < (int)sizeof(TASK_STATE_TO_CHAR_STR) -1 ? TASK_STATE_TO_CHAR_STR[state] : '?'
 */

/* The process structure */
struct process_struct {
	volatile int state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	int prio;
	double atime;		/* Activate time */
	pthread_t th;		/* Thread ID */
	pthread_mutex_t lock;
	pthread_cond_t cond;
	void *(*behaviour) (void *);
};

extern struct process_struct *process_list;
extern size_t process_count;

extern int create_process(void *(*) (void *), int);
extern int destroy_process(size_t);
extern int Wait(double);
extern int Quit(void);

#endif				/* _PROCESS_H_ */
