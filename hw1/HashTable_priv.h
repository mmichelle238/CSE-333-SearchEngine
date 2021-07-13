/*
 * Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2021 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#ifndef HW1_HASHTABLE_PRIV_H_
#define HW1_HASHTABLE_PRIV_H_

#include <stdint.h>  // for uint32_t, etc.

#include "./LinkedList.h"
#include "./HashTable.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Internal structures and helper functions for our HashTable implementation.
//
// These would typically be located in HashTable.c; however, we have broken
// them out into a "private .h" so that our unittests can access them.  This
// allows our test code to peek inside the implementation to verify correctness.
//
// Customers should not include this file or assume anything based on
// its contents.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


// The hash table implementation.
//
// A hash table is an array of buckets, where each bucket is a linked list
// of HTKeyValue structs.
typedef struct ht {
  int             num_buckets;   // # of buckets in this HT?
  int             num_elements;  // # of elements currently in this HT?
  LinkedList    **buckets;       // the array of buckets
} HashTable;

// The hash table iterator.
typedef struct ht_it {
  HashTable  *ht;          // the HT we're pointing into
  int         bucket_idx;  // which bucket are we in?
  LLIterator *bucket_it;   // iterator for the bucket, or NULL
} HTIterator;

// This is the internal hash function we use to map from HTKey_t keys to a
// bucket number.
int HashKeyToBucketNum(HashTable *ht, HTKey_t key);

#endif  // HW1_HASHTABLE_PRIV_H_
