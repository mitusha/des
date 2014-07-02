/*
 * Error handling module.
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

#include <stdio.h>
#include "error.h"

/*
 * Global error number variable
 * This differs from plain `int errno;' in that it doesn't create
 * a common definition, but a plain symbol that resides in .bss,
 * which can have an alias.
 */
int simerr __attribute__ ((nocommon));

/* Error strings */
static const char *const strs[] = {
	[GLOB_NOTINIT] = "simulation not initialized",
	[GLOB_INVAL] = "invalid arguments",
};

void psimerr(const char *s)
{
	const char *colon;
	const char *errstring;

	if (!s || *s == '\0')
		s = colon = "";
	else
		colon = ": ";

	errstring = errstr(simerr);

	fprintf(stderr, "%s%s%s\n", s, colon, errstring);
}

const char *errstr(int val)
{
#define nstrs (sizeof(strs) / sizeof(strs[0]))
	static char buf[100];

	if (val < 0 || val >= (int)nstrs || !strs[val]) {
		snprintf(buf, sizeof(buf), "GLOB_??? (%d)", val);
		return buf;
	}

	return strs[val];
#undef nstrs
}
