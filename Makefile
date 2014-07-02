# Makefile for discrete simulator.

# Copyright (C) 2010, Marek Polacek <xpolac06@stud.fit.vutbr.cz>

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.


VERSION := 1
SUBVERSION := 0
MINORVERSION := 0
TARVERSION := $(VERSION).$(SUBVERSION).$(MINORVERSION)

CC = gcc
AR = ar
OPTFLAGS = -O2
CDEBUG = -g
LDFLAGS = -Wl,-O1 -lm -lpthread
DEFS = -D_GNU_SOURCE # -D_FORTIFY_SOURCE=2 ## Merlin is fucked up.
CFLAGS = -Wwrite-strings \
	-Winline \
	-Wshadow \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wredundant-decls \
	-Wcast-qual \
	-Wcast-align \
	-std=gnu99 \
	-Wall \
	-fomit-frame-pointer \
	-Wextra \
	-pipe \
	-march=native \
	$(OPTFLAGS) $(CDEBUG) $(DEFS) $(LDFLAGS)
SRC1 = main.c
SRC2 = facility.c stats.c cal.c queue.c store.c error.c process.c
SRC3 = xmalloc.c 
SRCS = $(SRC1) main2.c  $(SRC2) $(SRC3)
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)
OBJ3 = $(SRC3:.c=.o)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3)
AUX = Makefile facility.h stats.h system.h cal.h queue.h store.h error.h process.h
FILE = doc
LOGIN = xmikul39_xpolac06

.PHONY: all
all:	$(OBJS) dsim.a main main2

debug: CFLAGS += -ggdb3 -O0
debug: clean all

mudflap: CFLAGS += -fmudflap
mudflap: LDFLAGS += -lmudflap
mudflap: debug

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

.PHONY:	main
main: $(OBJ1) dsim.a
	$(CC) $(CFLAGS) -o $@ $^ 

.PHONY:	main2
main2: main2.c dsim.a
	$(CC) $(CFLAGS) -o $@ $^ 

.PHONY: dsim.a
## We don't have to use ranlib here.
dsim.a: $(OBJ2) $(OBJ3)
	$(AR) crs $@ $?

.PHONY: clean
clean:
	-rm -f main main2 $(LOGIN).tar.gz *.o *~ *.core core dsim.a \
	$(FILE).log $(FILE).aux $(FILE).dvi $(FILE).ps $(FILE).out

.PHONY: mostlyclean
mostlyclean:
	-rm -f main $(LOGIN).tar.gz *.o *~ *.core core

.PHONY: dist
dist:	
	tar -czhvf $(LOGIN).tar.gz $(SRCS) $(AUX) $(FILE).pdf
	@echo $(LOGIN)".tar.gz created"

.PHONY: doc
doc:
#	latex $(FILE).tex
#	bibtex $(FILE).aux
	latex $(FILE).tex
	latex $(FILE).tex
	dvips $(FILE).dvi
	ps2pdf -sPAPERSIZE=a4 $(FILE).ps

.PHONY: run
run: 
	./main

.PHONY: run2
run2: 
	./main2
