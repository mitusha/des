/*
 * Check malloc routines.
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

#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include "system.h"

/* Allocate N bytes of memory dynamically, with error checking */
void *xmalloc(size_t n)
{
	void *p;

	p = malloc(n);
	if (!p)
		errx(EXIT_FAILURE, "memory exhausted");
	return p;
}

/* Allocate memory for N elements of S bytes, with error checking */
void *xcalloc(size_t n, size_t s)
{
	void *p;

	p = calloc(n, s);
	if (!p)
		errx(EXIT_FAILURE, "memory exhausted");
	return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking */
void *xrealloc(void *p, size_t n)
{
	p = realloc(p, n);
	if (!p)
		errx(EXIT_FAILURE, "memory exhausted");
	return p;
}
