CC=gcc
CXX=g++
LD=gcc
CFLAGS=-Wall -Werror -g
LDFLAGS=$(CFLAGS)

TARGETS=parser

all: $(TARGETS)

# Define the proj1 target
proj1: proj1.c
	gcc -o proj1 proj1.c
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule for building .o files from .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Pattern rule for building .o files from .cc files
%.o: %.cc
	$(CXX) $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGETS)
