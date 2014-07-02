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

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdbool.h>

struct pq_t {
	struct pq_t *next;
	size_t idx;
	unsigned int attr;
};

extern size_t pq_size(struct pq_t **) __attribute__ ((nonnull));
extern void pq_pop(struct pq_t **) __attribute__ ((nonnull));
extern void pq_push(struct pq_t **, size_t) __attribute__ ((nonnull(1)));
extern void pq_push_attr(struct pq_t **, size_t, unsigned int) /* __attribute__((nonnull(1))) */ ;
extern void pq_clear(struct pq_t **) __attribute__ ((nonnull));
extern void pq_debug(struct pq_t **) __attribute__ ((nonnull));

/* True if linked list is empty */
static inline bool __attribute__ ((nonnull)) pq_empty(struct pq_t **queue)
{
	return pq_size(queue) == 0;
}

/* Initialization of the priority queue */
static inline void __attribute__ ((nonnull)) pq_init(struct pq_t **queue)
{
	*queue = NULL;
}

/* Return index of head */
static inline ssize_t __attribute__ ((nonnull)) pq_top(struct pq_t **queue)
{
	return pq_empty(queue) == false ? (ssize_t) (*queue)->idx : -1;
}

/* Return attribute of head */
static inline unsigned int __attribute__ ((nonnull)) pq_top_attr(struct pq_t **queue)
{
	return pq_empty(queue) == false ? (unsigned int)(*queue)->attr : 0;
}
#endif				/* _QUEUE_H_ */
