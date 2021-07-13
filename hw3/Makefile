# Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
# hereby granted to students registered for University of Washington
# CSE 333 for use solely during Spring Quarter 2021 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

AR = ar
ARFLAGS = rcs
CC = gcc
CXX = g++
CFLAGS = -g -Wall -Wpedantic -std=c++17 -I. -I..
LDFLAGS = -L. -L./libhw2 -lhw2 -L./libhw1 -lhw1
CPPUNITFLAGS = -L../gtest -lgtest -lpthread
HEADERS = DocIDTableReader.h \
          DocTableReader.h \
          FileIndexReader.h \
          HashTableReader.h \
          IndexTableReader.h \
          LayoutStructs.h \
          QueryProcessor.h \
          Utils.h \
          WriteIndex.h

TESTOBJS = test_suite.o test_utils.o test_writeindex.o \
           test_fileindexreader.o test_hashtablereader.o \
           test_doctablereader.o test_indextablereader.o \
           test_docidtablereader.o test_queryprocessor.o

OBJS = DocIDTableReader.o DocTableReader.o FileIndexReader.o \
       HashTableReader.o IndexTableReader.o QueryProcessor.o \
       Utils.o WriteIndex.o 

all: buildfileindex filesearchshell test_suite libhw3.a

filesearchshell: filesearchshell.o libhw3.a $(HEADERS)
	$(CXX) $(CFLAGS) -o filesearchshell filesearchshell.o \
	-L. -lhw3 $(LDFLAGS)

buildfileindex: buildfileindex.o libhw3.a $(HEADERS)
	$(CXX) $(CFLAGS) -o buildfileindex buildfileindex.o \
	-L. -lhw3 $(LDFLAGS)

libhw3.a: $(OBJS) $(HEADERS)
	$(AR) $(ARFLAGS) libhw3.a $(OBJS)

test_suite: $(TESTOBJS) $(OBJS) $(HEADERS) HW3FSCK
	$(CXX) $(CFLAGS) -o test_suite $(OBJS) $(TESTOBJS) \
	-L./hw3fsck -l:hw3fsck.a $(CPPUNITFLAGS) $(LDFLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CFLAGS) -c $<

HW3FSCK:
	make -C ./hw3fsck

clean:
	/bin/rm -f *.o *~ test_suite libhw3.a buildfileindex \
        filesearchshell
	make -C ./hw3fsck clean
