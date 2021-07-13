# Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Washington
# CSE 333 for use solely during Spring Quarter 2021 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

# define the commands we will use for compilation and library building
AR = ar
ARFLAGS = rcs
CC = gcc
CXX = g++

# define useful flags to cc/ld/etc.
CFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c17 -O0
CXXFLAGS += -g -Wall -Wpedantic -I. -I.. -std=c++17 -O0
LDFLAGS += -L. -lhw1
CPPUNITFLAGS = -L../gtest -lgtest

# define common dependencies
OBJS = LinkedList.o HashTable.o CSE333.o
HEADERS = LinkedList.h HashTable.h CSE333.h
TESTOBJS = test_linkedlist.o test_hashtable.o test_suite.o

# compile everything; this is the default rule that fires if a user
# just types "make" in the same directory as this Makefile
all: test_suite example_program_ll example_program_ht

example_program_ll: example_program_ll.o libhw1.a $(HEADERS)
	$(CC) $(CFLAGS) -o example_program_ll example_program_ll.o $(LDFLAGS)

example_program_ht: example_program_ht.o libhw1.a $(HEADERS)
	$(CC) $(CFLAGS) -o example_program_ht example_program_ht.o $(LDFLAGS)

libhw1.a: $(OBJS) $(HEADERS)
	$(AR) $(ARFLAGS) libhw1.a $(OBJS)

test_suite: $(TESTOBJS) libhw1.a
	$(CXX) $(CFLAGS) -o test_suite $(TESTOBJS) \
	$(CPPUNITFLAGS) $(LDFLAGS) -lpthread $(LDFLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o *~ *.gcno *.gcda *.gcov test_suite libhw1.a \
    example_program_ll example_program_ht
