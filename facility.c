/*
 * Facility class.
 *
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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#ifdef __linux__
#include <mcheck.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stats.h"
#include "system.h"
#include "facility.h"
#include "process.h"
#include "cal.h"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
//#define debug(fmt, ...) ((void)0)

#define INTERNAL_ERROR(errstr)	\
	errx(EXIT_FAILURE, _("%s(): INTERNAL ERROR at line %d (%s-%s): %s"),	\
	__func__, __LINE__, VERSION, __DATE__, errstr)

/*
 * Initialization of the facility
 */
void fac_constructor(struct facility_t *fac)
{
	fac->name = NULL;
	fac->busy = false;
	fac->queue = NULL;
	fac->idx = (ssize_t) - 1;
	fac->stats = xcalloc(1, sizeof(struct stat_t));
	/* Initialize lock and cond */
	if (pthread_mutex_init(&fac->flock, NULL)
	    || pthread_cond_init(&fac->fcond, NULL))
		err(EXIT_FAILURE, _("pthread init failed"));
}

void fac_clear(struct facility_t *fac)
{
	free(fac->name);
	fac->name = NULL;
	fac->busy = false;
	free(fac->queue);
	fac->queue = NULL;
	fac->idx = (ssize_t) - 1;
}

/*
 * Free all allocated memory for the facility
 */
void fac_destructor(struct facility_t *fac)
{
	free(fac->name);
	free(fac->queue);
	free(fac->stats);
	/* Destroy lock and cond */
	pthread_cond_destroy(&fac->fcond);
	pthread_mutex_destroy(&fac->flock);
}

/*
 * Set facility name
 */
void fac_set_name(struct facility_t *fac, const char *name)
{
	/*
	 * if fac->name is NULL, xrealloc is equivalent
	 * to xmalloc, otherwise xrealloc changes allocated
	 * memory, new memory will be uninitialized -> memset
	 */
	fac->name = xrealloc(fac->name, strlen(name) + 1);
	memset(fac->name, 0, strlen(name) + 1);
	strncpy(fac->name, name, strlen(name));
}

/*
 * Get facility name
 */
char *fac_get_name(struct facility_t *fac)
{
	return fac->name;
}

void Seize(struct facility_t *fac, size_t idx)
{
	if (!fac_busy(fac)) {
		// zamkni
		pthread_mutex_lock(&fac->flock);
		// obsad
		fac->idx = idx;
		fac->busy = true;
	} else {
		// musime jit do fronty
		fac_queue_in(fac, idx);
		// rekneme kalndari at spusti dalsi proces
		pthread_cond_signal(&process_list[idx].cond);
		// cekame dokud nas zarizeni samo nenatahne dovnitr
		if (fac->idx == (ssize_t) idx)
			return;
		do {
			pthread_cond_wait(&fac->fcond, &fac->flock);
		} while (fac->idx != (ssize_t) idx);
	}
}

/*
 * Process releases the facility and seizes the facility
 * with the first process in queue (if any)
 */
void Release(struct facility_t *fac)
{
	// uvolneni
	fac->idx = (ssize_t) - 1;
	fac->busy = false;
	// odemceni
	pthread_mutex_unlock(&fac->flock);

	if (!pq_empty(&fac->queue)) {
		pthread_mutex_lock(&fac->flock);
		// vybrat dalsi prvek
		// pustit ho
		fac->idx = pq_top(&fac->queue);
		pq_pop(&fac->queue);
		fac->busy = true;
		process_list[fac->idx].atime = cur_time;
		add_elem(fac->idx);

		pthread_cond_broadcast(&fac->fcond);

		pthread_mutex_unlock(&fac->flock);
	}
}

/*
 * True if the facility is busy, false otherwise
 */
bool fac_busy(struct facility_t *fac)
{
	return fac->busy;
}

/*
 * Length of the priority queue
 */
size_t fac_queue_len(struct facility_t * fac)
{
	return pq_size(&fac->queue);
}

void fac_queue_in(struct facility_t *fac, size_t idx)
{
	pq_push(&fac->queue, idx);
}
