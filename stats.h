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

#ifndef _STATS_H_
#define _STATS_H_

#include "system.h"
#include <stdbool.h>

/* A histogram structure */
struct histogram {
	size_t n;	/* Number of histogram bins */
	double range;	/* Range of the bins */
};

/* Structure holding stats */
struct times {
	double *arr;		/* Array of times */
	size_t nmemb;		/* Number of elements */
	size_t allocated;	/* Allocated elements */
};

/* The statistics structure */
struct stat_t {
	struct times times;
	bool sorted;
	char p[0];
};

#define HISTOGRAM_SYMBOL	'*'

extern void print_stats(struct stat_t *, size_t, bool);
extern void stats_foo(void);
extern double Exponential(double);
extern double Random(void);
extern double Uniform(double, double);
extern double Normal(double, double);
extern void save_time(struct stat_t *, double);
extern size_t internal_function_def times_cnt(struct stat_t *);
extern double times_sum(struct stat_t *);
extern double internal_function_def times_avg(struct stat_t *);
extern double internal_function_def times_min(struct stat_t *);
extern double internal_function_def times_max(struct stat_t *);
extern double times_dev(struct stat_t *);
extern void free_times(struct stat_t *s);
extern void print_histogram(struct stat_t *, FILE *, size_t);
extern void output_file(const char *);

#endif /* _STATS_H_ */
