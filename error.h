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

#ifndef _ERROR_H_
#define _ERROR_H_

/* Error codes */
#define GLOB_NOTINIT	1	/* Simulation not initialized */
#define GLOB_INVAL	2	/* Invalid arguments */

extern int simerr;

extern void psimerr(const char *);
extern const char *errstr(int);

#endif /* _ERROR_H_ */
