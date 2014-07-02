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

#ifndef _STORE_H_
#define _STORE_H_

#include <stdbool.h>
#include <pthread.h>
#include "queue.h"

struct log_t {
	struct log_t *next;	/* next item */
	size_t idx;		/* process index */
	unsigned int capacity;	/* capacity blocked by one process */
};

struct store_t {
	char *name;		/* name of the store */
	unsigned int capacity;	/* capacity of the store */
	unsigned int free_capacity;
	struct pq_t *queue;	/* priority queue for pending processes */
	struct log_t *log;	/* log of occupied capacity */
	ssize_t first_available;
	pthread_cond_t scond;
	pthread_mutex_t slock;
	struct stat_t *stats;  /* stats of store */
};

void store_constructor(struct store_t *);
void store_destructor(struct store_t *);
void store_set_name(struct store_t *, const char *);
char *store_get_name(struct store_t *);
void store_clear(struct store_t *);
void store_set_capacity(struct store_t *, unsigned int);
unsigned int store_get_capacity(struct store_t *);
unsigned int store_free(struct store_t *);
unsigned int store_used(struct store_t *);
bool store_empty(struct store_t *);
bool store_full(struct store_t *);
void Enter(struct store_t *, size_t, unsigned int);
void Leave(struct store_t *, size_t, unsigned int);
size_t store_queue_len(struct store_t *);
void store_queue_in(struct store_t *, size_t, unsigned int);

/* work with log */
void log_init(struct log_t **);
void log_clear(struct log_t **);
struct log_t *log_find_process(struct log_t **, size_t);
int log_process_capacity(struct log_t **, size_t);
void log_add_capacity(struct log_t **, size_t, unsigned int);
void log_del_capacity(struct log_t **, size_t, unsigned int);

#endif				/* _STORE_H_ */
