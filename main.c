#if _POSIX_THREAD_SAFE_FUNCTIONS == -1
#error Thread safe functions not available
#endif

#include <assert.h>
#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "facility.h"
#include "cal.h"
#include "queue.h"
#include "store.h"
#include "error.h"
#include "process.h"
#include "system.h"
#include "stats.h"

//#define debug(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define debug(fmt, ...) ((void)0)

/* Facilities and stores */
struct facility_t fac;

/* Number of sewers in the facility */
static const unsigned int sewers = 10;

static void *foo(void *arg __unused__)
{
	/* actual time */
	double _time = cur_time;

	/* seize the facility */
	debug("< Seize(fac)   PID: %d >", CURRENT());
	Seize(&fac, CURRENT());

	/* after succesfull seize, save time into stats */
	save_time(fac.stats, cur_time - _time);

	/* wait for time */
	debug("< Wait(fac)    PID: %d >", CURRENT());
	Wait(Exponential(1.25));

	/* release the facility */
	debug("< Release(fac) PID: %d >", CURRENT());
	Release(&fac);

	/* terminating the process */
	debug("< Quit()       PID: %d >", CURRENT());
	Quit();

	return NULL;
}

int main(void)
{
	/* Facility initialization */
	fac_constructor(&fac);
	fac_set_name(&fac, "Facility");

	/* Simulation initialization */
	if (Init(0.0, 100.0) == -1)
		psimerr("init");

	/* Generating processes */
	for (unsigned i = 0; i < sewers; ++i)
		/* creating process with priority */
		create_process(foo, i % 2);

	/* Runnig the simulation */
	Run();

	/* printing statistics */
	printf("\033[1;32mStats for %s\033[0m\n", fac_get_name(&fac));
	print_stats(fac.stats, sewers / 2 - 1, 1);

	/* Facility destruction */
	fac_destructor(&fac);

	return EXIT_SUCCESS;
}
