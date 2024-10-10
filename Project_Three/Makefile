#Name: Asya Akkus 
#Case Network ID: aya29
#File Name: Makefile 
#Date Created: 10/10/2024
#Description: This is the makefile for proj3

CC=gcc
CXX=g++
LD=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=

TARGETS=proj3

all: $(TARGETS)

# Define the proj1 target
proj1: proj3.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(TARGETS)

distclean: clean
	rm -f $(TARGETS)
