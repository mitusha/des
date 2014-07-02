/*
 * Implementation of statistic routines and RNG.
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

#include <alloca.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "stats.h"
#include "system.h"

//#define debug(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define debug(fmt, ...) ((void)0)

const __possibly_unused unsigned int seed[3] =
    { 0x12344321U, 0xEE11DD22U, 0xFEDCBA98 };

/* Structure holding stats */
static struct {
	double *arr;		/* Array of times */
	size_t nmemb;		/* Number of elements */
	size_t allocated;	/* Allocated elements */
} times;

/* True if array of times was sorted */
//static bool sorted;

/* Output file.  NULL means stdout */
static const char *outfile;

/* Prototypes of local functions */
static void test_print_times(void) __unused__;

/* Set output file */
void output_file(const char *fname)
{
	outfile = fname;
}

/* Print stats and histogram w or w/o header */
void print_stats(struct stat_t *s, size_t nbins, bool print_header)
{
	FILE *fp = outfile ? fopen(outfile, "w") : stdout;
	if (!fp)
		err(EXIT_FAILURE, "cannot open output file");

#define FMT "\033[1m"
#define END "\033[0m"

	fputs(print_header ? "\033[1;32m\
Statistics\n\
----------\n\033[0m" : "", fp);
	fprintf(fp, FMT"Sum:"END"                \033[1;34m%6.2f min\033[0m\n", times_sum(s));
	fprintf(fp, FMT"Average:"END"            \033[1;34m%6.2f min\033[0m\n", times_avg(s));
	fprintf(fp, FMT"Maximum:"END"            \033[1;34m%6.2f min\033[0m\n", times_max(s));
	fprintf(fp, FMT"Minimum:"END"            \033[1;34m%6.2f min\033[0m\n", times_min(s));
	fprintf(fp, FMT"Standard deviation:"END" \033[1;34m%6.2f min\033[0m\n", times_dev(s));

	fputs(print_header ? "\033[1;32m\n\
Histogram\n\
---------\n\033[0m" : "", fp);
	print_histogram(s, fp, nbins);

#undef FMT
#undef END
}

/* From the least to the biggest */
static int compare(const void *a1, const void *b1)
{
	double a = *(const double *)a1;
	double b = *(const double *)b1;

	return (a < b ? -1 : (a == b ? 0 : 1));
}

static void sort_times(struct stat_t *s)
{
	qsort(s->times.arr, s->times.nmemb, sizeof(double), compare);

	/* The array is now sorted */
	s->sorted = true;
}

//static __attribute__ ((destructor))
void free_times(struct stat_t *s)
{
	free(s->times.arr);
}

/* Return random number from <0; 1) */
double Random(void)
{
	return (double)(random() / (double)RAND_MAX);
}

/* Returns number from <M; N) */
double Uniform(double M, double N)
{
	return M + random() / (RAND_MAX / (N - M + 1.0) + 1.0);
}

/*
 * Normal distribution:
 * x = mean +/- std_dev * sqrt((-2.0) * log(y)), 0 < y <= 1
 */
double Normal(double mean, double std_dev)
{
	double y;
	unsigned int bin;

	errno = 0;

	/* std_dev must be greater than 0.0 (or machine epsilon) */
	if (!(std_dev > DBL_EPSILON)) {
		printf("%s*(: std_dev too small or zero: %s", __func__,
		       strerror(EDOM));
		return mean;
	}

	/* 0.0 <= y < 1.0 */
	y = (double) (random() / (double) (RAND_MAX + 1.0));
	bin  = (y < 0.5) ? 0 : 1;
	y = fabs(y - 1.0);                        /* 0.0 < y <= 1.0 */
	y = std_dev * sqrt((-2.0) * log(y));

	return bin ? (mean + y) : (mean - y);
}


/*
 * The exponential distribution has the form
 *	p(x) dx = exp(-x/mu) dx/mu
 */
double Exponential(double mu)
{
	/* `u' in <0; 1) */
	double u = Random();
	debug("<%s> From <0; 1): %f\n", __FILE__, u);

	return -mu * log(u);
}

static void __attribute__ ((constructor)) set_seed(void)
{
	debug("<%s> Setting seed...\n", __FILE__);

	/* Set seed */
	srandom(time(NULL));
}

size_t internal_function_def times_cnt(struct stat_t *s)
{
	return s->times.nmemb;
}

double internal_function_def times_avg(struct stat_t *s)
{
	return times_sum(s) / times_cnt(s);
}

double internal_function_def times_max(struct stat_t *s)
{
	if (!s->sorted)
		sort_times(s);

	return s->times.arr[s->times.nmemb - 1];
}

double internal_function_def times_min(struct stat_t *s)
{
	if (!s->sorted)
		sort_times(s);

	return s->times.arr[0];
}

double times_sum(struct stat_t *s)
{
	size_t i;
	double sum = 0.0;

	for (i = 0; i < s->times.nmemb; i++)
		sum += s->times.arr[i];

	return sum;
}

/* See <http://en.wikipedia.org/wiki/Standard_deviation> */
double times_dev(struct stat_t *s)
{
	const double mean = times_avg(s);
	double total = 0.0;
	size_t i;

	/*
	 * Compute the difference of each data point from the mean, and square
	 * the result of each:
	 */
	for (i = 0; i < s->times.nmemb; i++) {
		const double diff = s->times.arr[i] - mean;
		total += (diff * diff);
	}

	/* Compute the average of these values, and take the square root */
	total /= s->times.nmemb;
	return sqrt(total);
}

/* Print HISTOGRAM_SYMBOL for every number in interval (from; to>? */
static void print_syms(struct stat_t *s, FILE *fp, double from, double to)
{
	size_t i;
	static size_t last_stop;

	/* It'll be more effective, if we operate on sorted array */
	if (s->sorted)
		sort_times(s);

	/* Go through remaining elements */
	for (i = last_stop; i < s->times.nmemb; i++) {
		if (s->times.arr[i] > to) {
			/*
			 * No point in searching further -- all the
			 * remaining elements are bigger.
			 */
			last_stop = i;
			break;
		}
		if (s->times.arr[i] >= from && s->times.arr[i] < to)
			/* Found it */
			fputc(HISTOGRAM_SYMBOL, fp);
	}
}

/* Print a histogram */
void print_histogram(struct stat_t *s, FILE *fp, size_t nbins)
{
	struct histogram h;
	size_t i;
	const double min = times_min(s);
	double step = min;

	/* Fill the histogram struct */
	h.n = nbins ?: 1;
	h.range = ((times_max(s) - min) / h.n);

	/* Go through all the bins */
	for (i = 0; i <= h.n; i++) {
		fprintf(fp, "< %8.2f; %8.2f ): ", step, step + h.range);
		print_syms(s, fp, step, step + h.range);
		step += h.range;
		fputc('\n', fp);
	}
}

/* Add new time into times array */
void save_time(struct stat_t *stats, double t)
{
	/* Do we need to allocate more space? */
	if (stats->times.nmemb + 1 > stats->times.allocated) {
		/* Add space for ten elements */
		stats->times.allocated += 10;
		size_t newsize = stats->times.allocated * sizeof(double);
		stats->times.arr = (double *)xrealloc(stats->times.arr, newsize);
	}

	/* Save new value */
	stats->times.arr[stats->times.nmemb] = t;

	/* We added an element */
	stats->times.nmemb++;

	/* The array has changed and may not be sorted */
	stats->sorted = false;
}

static void test_print_times(void)
{
	size_t i;

	debug("[");
	for (i = 0; i < times.nmemb; i++)
		debug("%g ", times.arr[i]);
	debug("]\n");
}
