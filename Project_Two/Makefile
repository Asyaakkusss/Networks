#Name: Asya Akkus 
#Case Network ID: aya29
#File Name: Makefile 
#Date Created: 9/24/2024
#Description: This is the makefile for proj2

CC=gcc
CXX=g++
LD=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=

TARGETS=proj2

all: $(TARGETS)

# Define the proj1 target
proj1: proj2.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(TARGETS)

distclean: clean
	rm -f $(TARGETS)
