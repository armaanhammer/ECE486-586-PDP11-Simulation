IDIR =../include
CC=g++
CFLAGS=-I$(IDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
CFLAGS+=-DLINUX
endif


PDPSim: pdp11simcontroller.cpp mem.cpp register.cpp octalword.cpp main.cpp
	make clean
	$(CC) -o main main.cpp pdp11simcontroller.cpp mem.cpp register.cpp octalword.cpp $(CFLAGS)

.PHONY: clean
clean:
	rm -f main
