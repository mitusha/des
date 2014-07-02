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

#ifndef _CAL_H_
#define _CAL_H_

#include "process.h"

extern double start_time;
extern double end_time;
extern double cur_time;

struct cal {
	/* Next element */
	struct cal *next;

	/*Index of the process in the process list */
	size_t idx;
};

extern int Init(double, double);
extern int Run(void);
extern int add_elem(size_t);
size_t get_head(void);
void del_head(void);

#endif /* _CAL_H_ */
