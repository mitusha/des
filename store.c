/*
 * Implementation of store.
 *
 * Copyright (C) 2010, Marek Polacek <xpolac06@stud.fit.vutbr.cz>
 * Copyright (C) 2010, Jiri Mikulka <xmikul39@stud.fit.vutbr.cz>
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "process.h"
#include "stats.h"
#include "system.h"
#include "store.h"
#include "cal.h"

#define debug(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
//#define debug(fmt, ...) ((void)0)

/*
 * Constructor of the store.
 * Everything is set to NULL/0.
 */
void store_constructor(struct store_t *store)
{
	store->name = NULL;
	store->capacity = (unsigned int)0;
	store->free_capacity = (unsigned int)0;
	store->queue = NULL;
	store->stats = xcalloc(1, sizeof(struct stat_t));
	log_init(&store->log);
	store->first_available = -1;
	if (pthread_mutex_init(&store->slock, NULL)
	    || pthread_cond_init(&store->scond, NULL))
		err(EXIT_FAILURE, _("pthread init failed"));
}

/*
 * Destructor of the store.
 * All allocated memory is freed.
 */
void store_destructor(struct store_t *store)
{
	free(store->name);
	free(store->queue);
	free(store->stats);
	log_clear(&store->log);
	free(store->log);
	pthread_cond_destroy(&store->scond);
	pthread_mutex_destroy(&store->slock);
}

/*
 * Set store name (for stats).
 */
void store_set_name(struct store_t *store, const char *name)
{
	store->name = xrealloc(store->name, strlen(name) + 1);
	memset(store->name, 0, strlen(name) + 1);
	strncpy(store->name, name, strlen(name));
}

/*
 * Return store name
 */
char *store_get_name(struct store_t *store)
{
	return store->name;
}

/*
 * Clear all allocated memory a set store into default state
 */
void store_clear(struct store_t *store)
{
	free(store->name);
	store->name = NULL;
	store->capacity = (unsigned int)0;
	store->free_capacity = (unsigned int)0;
	pq_clear(&store->queue);
	store->queue = NULL;
	log_clear(&store->log);
	free(store->log);
	store->log = NULL;
}

/*
 * Set capacity of the store
 */
void store_set_capacity(struct store_t *store, unsigned int capacity)
{
	store->capacity = capacity;
	store->free_capacity = capacity;
}

/*
 * Return capacity of the store
 */
unsigned int store_get_capacity(struct store_t *store)
{
	return store->capacity;
}

/*
 * Return free capacity
 */
unsigned int store_free(struct store_t *store)
{
	return store->free_capacity;
}

/*
 * Return used capacity
 */
unsigned int store_used(struct store_t *store)
{
	return store->capacity - store->free_capacity;
}

/*
 * True if all the capacity is free, false otherwise
 */
bool store_empty(struct store_t * store)
{
	return store->free_capacity == store->capacity ? true : false;
}

/*
 * True if there is no free capacity in the store,
 * false otherwise
 */
bool store_full(struct store_t * store)
{
	return store->free_capacity == 0 ? true : false;
}

/*
 * Process idx blocks capacity of the store.
 * If there isn't enough free capacity, process is queued in
 * the priority queue.
 */
void Enter(struct store_t *store, size_t idx, unsigned int capacity)
{
	assert(capacity <= store->capacity);

	/*
	 * if queue is empty and there is enough capacity
	 * process blockes capacity and make a record in log
	 */
	if (pq_empty(&store->queue) && store_free(store) >= capacity) {
		pthread_mutex_lock(&store->slock);
		store->free_capacity -= capacity;
		log_add_capacity(&store->log, idx, capacity);
	} else {	/* no free capacity -> process in queue */
		store_queue_in(store, idx, capacity);
		pthread_cond_signal(&process_list[idx].cond);
	
		if (store->first_available == (ssize_t) idx)			
			return;

		do
			pthread_cond_wait(&store->scond, &store->slock);
		while (store->first_available != (ssize_t) idx);
	}
}

/*
 * Process leaves capacity to the store (removes capacity from log).
 * Process which can be satistified (there is enough capacity in
 * the store) is removed from the queue and served (add note into
 * the log).
 */
void Leave(struct store_t *store, size_t idx, unsigned int capacity)
{
	/* process tries to return more capacity than it has blocked */
	assert((int) capacity <= log_process_capacity(&store->log, idx));

	/* leave capacity (add to the free capacity, remove from log */
	store->free_capacity += capacity;
	log_del_capacity(&store->log, idx, capacity);
	store->first_available = (ssize_t) -1;
	pthread_mutex_unlock(&store->slock);

	/* is no process is pending in the queue, no one is served */
	if (pq_empty(&store->queue))
		return;

	pthread_mutex_lock(&store->slock);

	/*
	 * firstly we find process which can be served (it demands less
	 * or equal capacity as is free capacity)
	 */
	struct pq_t *del = store->queue;
	while (del) {
		if (pq_top_attr(&del) <= store_free(store))
			break;
		del = del->next;
	}

	/* is del is NULL - no process can be served */
	if (!del)
		return;

	/* otherwise del is process which is about to be served */
	struct pq_t *tmp = store->queue;
	if (tmp != del) {
		/* we find previous item */
		while (tmp->next != del)
			tmp = tmp->next;

		/* new process blockes the capacity */
		store->free_capacity -= pq_top_attr(&del);

		store->first_available = pq_top(&del);

		log_add_capacity(&store->log, pq_top(&del), pq_top_attr(&del));
		/* and it is removed from the queue */
		tmp->next = del->next;
		free(del);
	} else {
		/* is del is the very first item in the queue */
		store->free_capacity -= pq_top_attr(&tmp);
		
		store->first_available = pq_top(&del);
		
		log_add_capacity(&store->log, pq_top(&tmp), pq_top_attr(&tmp));
		pq_pop(&store->queue);
	}

	process_list[store->first_available].atime = cur_time;
	add_elem(store->first_available);

	pthread_cond_broadcast(&store->scond);
	pthread_mutex_unlock(&store->slock);
}

/*
 * Return length of the queue
 */
size_t store_queue_len(struct store_t *store)
{
	return pq_size(&store->queue);
}

/*
 * Queue process which can't be server into the queue
 */
void store_queue_in(struct store_t *store, size_t idx, unsigned int capacity)
{
	pq_push_attr(&store->queue, idx, capacity);
}

/*
 * Initialization of the log linked list
 */
void log_init(struct log_t **log)
{
	*log = NULL;
}

/*
 * Clear log list and free all allocated memory (if any)
 */
void log_clear(struct log_t **log)
{
	struct log_t *tmp = *log;
	while (tmp) {
		*log = (*log)->next;
		free(tmp);
		tmp = *log;
	}
}

/*
 * Find process in the log list and return pointer to this item
 */
struct log_t *log_find_process(struct log_t **log, size_t idx)
{
	struct log_t *tmp = *log;
	while (tmp) {
		if (tmp->idx == idx)
			break;
		tmp = tmp->next;
	}

	return tmp;
}

/*
 * Return blocked capacity of process
 */
int log_process_capacity(struct log_t **log, size_t idx)
{
	// prochazim seznamem dokud nenarazim na prvek se
	// stejnym cislem procesu -> vratim zabranou kapacitu
	// pokud neni nalezen, vratim 0
	struct log_t *tmp = log_find_process(log, idx);
	if (tmp)
		return tmp->capacity;

	return -1;
}

/*
 * Add capacity to given process
 */
void log_add_capacity(struct log_t **log, size_t idx, unsigned int capacity)
{
	/* process is already in the list */
	struct log_t *tmp = log_find_process(log, idx);
	if (tmp) {
		tmp->capacity += capacity;
		return;
	}

	/* process isn't in the list -> new item is allocated */
	struct log_t *new = xmalloc(sizeof(*new));
	new->idx = idx;
	new->capacity = capacity;
	tmp = *log;
	if (tmp) {
		/* list isn't empty */
		while (tmp->next)
			tmp = tmp->next;
		new->next = tmp->next;
		tmp->next = new;
	} else {
		/* list is empty */
		new->next = tmp;
		*log = new;
	}
}

/*
 * Remove capacity of process in the log list
 */
void log_del_capacity(struct log_t **log, size_t idx, unsigned int capacity)
{
	/* pointer to the process in log list */
	struct log_t *tmp = log_find_process(log, idx);
	/* it's capacity */
	int idx_cap = log_process_capacity(log, idx);

	/*
	 * if process isn't in the list or capacity supposed to release
	 * is more than actually blocked -> return
	 */
	if (!tmp || (int)capacity > idx_cap)
		return;

	/* if some capacity will remain in the list, just substract */
	if ((int)capacity < idx_cap) {
		tmp->capacity -= capacity;
		return;
	}

	/* remove item from the log list (if process is the first item) */
	if (tmp == *log) {
		*log = (*log)->next;
		free(tmp);
		return;
	}

	/* remove process inside the linked list */
	struct log_t *del = tmp;
	tmp = *log;
	/* firstly we find previous item */
	while (tmp->next != del)
		tmp = tmp->next;

	/* then we free the item */
	tmp->next = del->next;
	free(del);
}
