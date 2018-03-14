IDIR =../include
CC=g++
CFLAGS=-I$(IDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
CFLAGS+=-DLINUX
endif


PDPSim: pdp11simcontroller.cpp memory.cpp register.cpp table.cpp list.cpp octalword.cpp main.cpp
	g++ -std=c++17 -o PDPSim.o pdp11simcontroller.cpp memory.cpp register.cpp list.cpp table.cpp main.cpp octalword.cpp -I ../include/

.PHONY: clean
clean:
	rm -f pdp11simcontroller.o
