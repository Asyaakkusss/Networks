#Name: Asya Akkus 
#Case Network ID: aya29
#File Name: Makefile 
#Date Created: 9/4/2024
#Description: This is the makefile for proj1 

CC=gcc
CXX=g++
LD=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=

TARGETS=proj1

all: $(TARGETS)

# Define the proj1 target
proj1: proj1.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(TARGETS)

distclean: clean
	rm -f $(TARGETS)
