/*
 * Copyright (C) 2010, Jiri Mikulka <xmikul39@stud.fit.vutbr.cz>
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

#ifndef _FACILITY_H_
#define _FACILITY_H_

#include <pthread.h>
#include "queue.h"

struct facility_t {
	char *name;		/* name of facility */
	bool busy;		/* true if facility is busy */
	struct pq_t *queue;	/* priority queue for pending processes */
	struct stat_t *stats;  /* stats of facility */
	ssize_t idx;		/* index of serving process */
	pthread_cond_t fcond;
	pthread_mutex_t flock;
};

void fac_constructor(struct facility_t *);
void fac_clear(struct facility_t *);
void fac_destructor(struct facility_t *);

void fac_set_name(struct facility_t *, const char *);
char *fac_get_name(struct facility_t *);

bool fac_busy(struct facility_t *);

void Seize(struct facility_t *, size_t);
void Release(struct facility_t *);

size_t fac_queue_len(struct facility_t *);
void fac_queue_in(struct facility_t *, size_t);

#endif				/* _FACILITY_H_ */
