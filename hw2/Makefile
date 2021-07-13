# Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Washington
# CSE 333 for use solely during Spring Quarter 2021 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

# define the commands we'll use for compilation and library building
AR = ar
ARFLAGS = rcs
CC = gcc
CXX = g++

# define useful flags to cc/ld/etc.
CFLAGS = -g -Wall -Wpedantic -I. -I.. -O0
LDFLAGS = -L. -L./libhw1/ -lhw2 -lhw1
CPPUNITFLAGS = -L../gtest -lgtest

#define common dependencies
OBJS = CrawlFileTree.o DocTable.o FileParser.o MemIndex.o
HEADERS = CrawlFileTree.h DocTable.h FileParser.h MemIndex.h
TESTOBJS = test_suite.o test_fileparser.o test_doctable.o \
           test_crawlfiletree.o test_memindex.o

# compile everything
all: test_suite searchshell libhw2.a

libhw2.a: $(OBJS) $(HEADERS)
	$(AR) $(ARFLAGS) libhw2.a $(OBJS)

searchshell: searchshell.o libhw2.a $(HEADERS)
	$(CC) $(CFLAGS) -o searchshell searchshell.o $(LDFLAGS)

test_suite: $(TESTOBJS) libhw2.a
	$(CXX) $(CFLAGS) -o test_suite $(TESTOBJS) \
	$(CPPUNITFLAGS) $(LDFLAGS) -lpthread $(LDFLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CFLAGS) -std=c++17 -c $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -std=c17 $<

clean:
	/bin/rm -f *.o *~ test_suite searchshell libhw2.a
