IDIR =../include
CC=g++
CFLAGS=-I$(IDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
CFLAGS+=-DLINUX
endif


PDPSim: pdp11simcontroller.cpp memory.cpp register.cpp table.cpp list.cpp octalword.cpp main.cpp
	make clean
	$(CC) -std=c++17 -c list.cpp -I $(CFLAGS)
	$(CC) -std=c++17 -c table.cpp -I $(CFLAGS)
	$(CC) -std=c++17 -c octalword.cpp -I $(CFLAGS)
	$(CC) -std=c++17 -c register.cpp -I $(CFLAGS)
	$(CC) -std=c++17 -c memory.cpp -I $(CFLAGS)
	$(CC) -std=c++17 -c pdp11simcontroller.cpp $(CFLAGS)
	$(CC) -std=c++17 -c main.cpp $(CFLAGS)
	$(CC) -std=c++17 -o PDPSim.o pdp11simcontroller.o memory.o register.o list.o table.o main.o octalword.o $(CFLAGS)

.PHONY: clean
clean:
	rm -f pdp11simcontroller.o
