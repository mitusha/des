/*
 * Implementation of queue.
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
#include <stdbool.h>
#include "process.h"
#include "queue.h"
#include "system.h"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
//#define debug(fmt, ...) ((void)0)

/* Return length of pqueue */
size_t pq_size(struct pq_t **queue)
{
	size_t size = 0;
	struct pq_t *tmp = *queue;

	for (; tmp; size++, tmp = tmp->next) ;

	return size;
}

/* Remove first item in pqueue */
void pq_pop(struct pq_t **queue)
{
	if (!pq_empty(queue)) {
		struct pq_t *tmp = *queue;
		*queue = (*queue)->next;
		free(tmp);
	}
}

/* Clear the linked list */
void pq_clear(struct pq_t **queue)
{
	while (!pq_empty(queue))
		pq_pop(queue);

	/* Prevent dangling pointer */
	*queue = NULL;
}

/*
 * Insert new process structure into the linked list considering
 * it's priority.
 */
void pq_push(struct pq_t **queue, size_t idx)
{
	pq_push_attr(queue, idx, 0);
}

/*
 * Insert new process structure into the linked list considering
 * it's priority and attribute.
 */
void pq_push_attr(struct pq_t **queue, size_t idx, unsigned int attr)
{
	struct pq_t *tmp = *queue;

	/* Create new item */
	struct pq_t *new = xmalloc(sizeof(*new));

	/* Initialize it */
	new->idx = idx;
	new->attr = attr;	// we care about attribute

#define this process_list[idx]
	/*
	 * Queue is empty or priority of first item in the queue is (only)
	 * lower than priority of new item.
	 */
	if (pq_empty(queue) || this.prio > process_list[tmp->idx].prio) {
		new->next = tmp;
		*queue = new;
	} else {
		/* Insert item into the linked list according to its priority */
		while (tmp->next && process_list[tmp->next->idx].prio >= this.prio)
			tmp = tmp->next;
		new->next = tmp->next;
		tmp->next = new;
	}
#undef this
}

/*
 * Print debug dump of the priority queue
 */
void pq_debug(struct pq_t **queue)
{
	if (!pq_empty(queue)) {
		struct pq_t *tmp = *queue;

		debug("Priority queue (length = %zu)", pq_size(queue));
		debug("head ->");

		while (tmp) {
			debug("\t[item: %zu prio: %d attr: %u]", tmp->idx, process_list[tmp->idx].prio, tmp->attr);
			tmp = tmp->next;
		}

		debug("tail ->\t[item: %p]", tmp);
	}
}
