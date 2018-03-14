IDIR =../include
CC=g++
CFLAGS=-I$(IDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
CFLAGS+=-DLINUX
endif


PDPSim: pdp11simcontroller.cpp memory.cpp register.cpp table.cpp list.cpp octalword.cpp main.cpp
	make clean
	$(CC) -o main main.cpp pdp11simcontroller.cpp memory.cpp register.cpp list.cpp table.cpp octalword.cpp $(CFLAGS)

.PHONY: clean
clean:
	rm -f pdp11simcontroller.o
