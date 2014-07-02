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

#ifndef LIB_SYSTEM_H
#define LIB_SYSTEM_H	1

#include <stddef.h>
#include <stdint.h>

#define VERSION "0.0.3"

/* min()/max() macros that also do strict type-checking */
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define abs(x) ({				\
		long __x = (x);			\
		(__x < 0) ? -__x : __x;	})

/* Swap value of @a and @b */
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(divisor) __divisor = divisor;		\
	(((x) + ((__divisor) / 2)) / (__divisor));	\
}							\
)

#ifndef __attribute__
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
#  define __attribute__(x) /* empty */
# endif
#endif

/* Conditionalize the use of __builtin_expect features using macros */
#if defined(__GNUC__) && __GNUC__ >= 3
# define likely(x) __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

# define __noreturn__		__attribute__((noreturn))
# define __noinline__		__attribute__((noinline))
# define __const__		__attribute__((__const__))
# define __pure__		__attribute__((pure))
# define __unused__		__attribute__((unused))
# define __possibly_unused	__attribute__((unused))
# define __must_check__		__attribute__((warn_unused_result))
# define attribute_hidden	__attribute__((visibility ("hidden")))

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0])
#endif

#if defined(i386)
# define internal_function_def __attribute__((regparm(3), stdcall))
#else
# define internal_function_def
#endif

/* A marker for the automated extraction of messages */
#ifndef _
# define _(str) str
#endif

/*
 *	prefetch(x)	- prefetches the cacheline at "x" for read
 *	prefetchw(x)	- prefetches the cacheline at "x" for write
 */
#define prefetch(x) __builtin_prefetch(x)
#define prefetchw(x) __builtin_prefetch(x,1)

/* Function prototypes */
extern void *xmalloc(size_t) __attribute__ ((__malloc__));
extern void *xcalloc(size_t, size_t) __attribute__ ((__malloc__));
extern void *xrealloc(void *, size_t) __attribute__ ((__malloc__));

#endif /* system.h */
