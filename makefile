IDIR =../include
CC=g++
CFLAGS=-I$(IDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
CFLAGS+=-DLINUX
endif


PDPSim: pdp11simcontroller.cpp memory.cpp register.cpp table.cpp list.cpp octalword.cpp main.cpp
	make clean
	$(CC) -o pdp11simcontroller.o memory.cpp register.cpp table.cpp list.cpp octalword.o PDPSim.o $(CFLAGS)

.PHONY: clean

clean:
	rm -f CacheSim.o
