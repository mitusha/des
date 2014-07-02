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

/* stores */
struct store_t store;

/* Number of sewers in the facility */
static const unsigned int sewers = 10;

static void *foo(void *arg __unused__)
{
	double _time = cur_time;
	debug("< Enter(store)   PID: %d >", CURRENT());
	Enter(&store, CURRENT(), 20);
	debug("VE FRONTE %f", cur_time - _time);
	save_time(store.stats, cur_time - _time);
	debug("< Wait(store)    PID: %d >", CURRENT());
	Wait(Exponential(4.50));
	debug("< Leave(store)   PID: %d >", CURRENT());
	Leave(&store, CURRENT(), 20);

	debug("< Quit()          PID: %d >", CURRENT());
	Quit();

	return NULL;
}

int main(void)
{
	/* Store initialization */
	store_constructor(&store);
	store_set_capacity(&store, 30);
	store_set_name(&store, "Store");

	/* Simulation initialization */
	if (Init(0.0, 100.0) == -1)
		psimerr("init");

	/* Generating processes */
	for (unsigned i = 0; i < sewers; ++i)
		create_process(foo, i % 2);

	/* Runnig the simulation */
	Run();

	/* printing statistics */
	printf("\033[1;32mStats for %s\033[0m\n", store_get_name(&store));

	print_stats(store.stats, sewers / 2 - 1, 1);

	/* Store destruction */
	store_destructor(&store);

	return EXIT_SUCCESS;
}
