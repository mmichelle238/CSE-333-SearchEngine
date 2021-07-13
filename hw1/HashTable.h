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

#ifndef HW1_HASHTABLE_H_
#define HW1_HASHTABLE_H_

#include <stdbool.h>    // for bool type (true, false)
#include <stdint.h>     // for uint64_t, etc.

///////////////////////////////////////////////////////////////////////////////
// A HashTable is a automatically-resizing chained hash table.
//
// We provide the interface; your job is to provide the implementation.
//
// This hash table allows the caller to pass in an arbitrary value type
// (via the void*).  However, we require that the caller hash the key before
// providing it to the table for storage.  It's up to the customer to
// figure out how to produce an appropriate hash key, but below we provide
// an implementation of FNV hashing to help them out.
//
// As the load factor approaches 1, linked lists hanging off of each bucket
// will start to grow.  This implementation will dynamically resize the
// hashtable when the load factor exceeds 3.  It will multiple the number
// of buckets in the hashtable by 9, so that post-resize load factor is 1/3.
//
// To hide the implementation of HashTable, we declare the "struct ht"
// structure and its associated typedef here, but we *define* the structure
// in the internal header HashTable_priv.h.  This lets us define a pointer
// to HashTable as a new type, while leaving the implementation details
// opaque to the customer.
typedef struct ht HashTable;

// Key and value type definitions:
// For generality, a value must be large enough to hold a pointer.
// If the client's data is no bigger than a pointer, a copy of that
// data can be stored in the HashTable, by casting it to the HTValue_t
// type.  Otherwise, a pointer to the client's data is maintained in
// the table.
typedef uint64_t HTKey_t;      // hash table key type
typedef void*    HTValue_t;    // hash table value type
typedef struct {
  HTKey_t   key;      // the key in the (key,value) pair
  HTValue_t value;    // the value in the (key,value) pair
} HTKeyValue_t;

// When freeing a HashTable, customers need to pass a pointer to a function
// that frees the payload.  The pointed-to function is invoked once for each
// value in the HashTable.
typedef void(*ValueFreeFnPtr)(HTValue_t value);

// FNV hash implementation.
//
// Customers can use this to hash an arbitrary sequence of bytes into
// a 64-bit key suitable for using as a hash key.  If you're curious, you
// can read about FNV hashing here:
//     http://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
//
// Arguments:
// - buffer: a pointer to a len-size buffer of unsigned chars.
// - len: how many bytes are in the buffer.
//
// Returns:
// - a nicely distributed 64-bit hash value suitable for
//   use in a HTKeyValue_t.
HTKey_t FNVHash64(unsigned char *buffer, int len);


// Allocate and return a new HashTable.
//
// Arguments:
// - num_buckets: the number of buckets the hash table should
//   initially contain; MUST be greater than zero.
//
// Returns NULL on error, non-NULL on success.
HashTable* HashTable_Allocate(int num_buckets);

// Free a HashTable and its entries.
//
// Arguments:
// - table:  the HashTable to free.  It is unsafe to use table
//   after this function returns.
//
// - value_free_function:  this argument is a pointer to a value
//   freeing function; see above for details.
void HashTable_Free(HashTable *table, ValueFreeFnPtr value_free_function);

// Figure out the number of elements in the hash table.
//
// Arguments:
//
// - table:  the table to query
//
// Returns:
//
// - table size (>=0); note that this is an unsigned 64-bit integer.
int HashTable_NumElements(HashTable *table);

// Inserts a (key,value) pair into the HashTable.
//
// Arguments:
// - table: the HashTable to insert into.
// - newkeyvalue: the HTKeyValue_t to insert into the table.
// - oldkeyval: if the key in newkeyvalue is already present
//   in the HashTable, that old (key,value) is replaced with
//   newkeyvalue.  In that case, the old (key,value) is returned via
//   this return parameter to the caller.  It's up to the caller
//   to free any allocated memory associated with oldkeyvalue->value.
//
// Returns:
//  - false: if the newkeyvalue was inserted and there was no
//    existing (key,value) with that key.
//  - true: if the newkeyvalue was inserted and an old (key,value)
//    with the same key was replaced and returned through
//    the oldkeyval return parameter.  In this case, the caller assumes
//    ownership of oldkeyvalue.
bool HashTable_Insert(HashTable *table,
                      HTKeyValue_t newkeyvalue,
                      HTKeyValue_t *oldkeyvalue);

// Looks up a key in the HashTable, and if it is present, returns the
// (key,value) associated with it.
//
// Arguments:
// - table: the HashTable to look in.
// - key: the key to look up.
// - keyvalue: if the key is present, a copy of the (key,value) is
//   returned to the caller via this return parameter.  Note that the
//   (key,value) is left in the HashTable, so it is not safe for the
//   caller to free keyvalue->value.
//
// Returns:
//  - false: if the key wasn't found in the HashTable.
//  - true: if the key was found, and therefore the associated (key,value)
//    was returned to the caller via that keyvalue return parameter.
bool HashTable_Find(HashTable *table,
                    HTKey_t key,
                    HTKeyValue_t *keyvalue);

// Removes a (key,value) from the HashTable and returns it to the
// caller.
//
// Arguments:
// - table: the HashTable to look in.
// - key: the key to look up.
// - keyvalue: if the key is present, a copy of (key,value) is returned
//   to the caller via this return parameter and the (key,value) is
//   removed from the HashTable.  Note that the caller is responsible
//   for managing the memory associated with keyvalue->value from
//   this point on.
//
// Returns:
//  - false: if the key wasn't found in the HashTable.
//  - true: if the key was found, and therefore (a) the associated
//    (key,value) was returned to the caller via that keyvalue return
//    parameter, and (b) that (key,value) was removed from the
//    HashTable.
bool HashTable_Remove(HashTable *table,
                      HTKey_t key,
                      HTKeyValue_t *keyvalue);


///////////////////////////////////////////////////////////////////////////////
// HashTable iterator
//
// HashTables support the notion of an iterator, similar to Java iterators.
// You use an iterator to iterate forward through the HashTable.   The order
// in which the iterator goes through the HashTable is undefined, and not
// necessarily deterministic; all that is promised is that each (key,value)
// is visited exactly once.  Also, if the customer uses a HashTable function
// to mutate the hash table, any existing iterators become undefined (ie,
// dangerous to use; arbitrary memory corruption can occur).
typedef struct ht_it HTIterator;  // same trick to hide implementation.

// Manufacture an iterator for the table.  If there are
// elements in the hash table, the iterator is initialized
// to point at the "first" one.  The caller is responsible
// for eventually calling HTIterator_Free.
//
// Arguments:
// - table:  the table from which to return an iterator.
//
// Returns:
// - the newly-allocated iterator, which may be invalid or "past the end"
//   if the table cannot be iterated through (eg, empty).
HTIterator* HTIterator_Allocate(HashTable *table);

// When you're done with a hash table iterator, you must free it
// by calling this function.
//
// Arguments:
// - iter: the iterator to free.  Don't use it after freeing it.
void HTIterator_Free(HTIterator *iter);

// Tests to see whether the iterator is pointing at a valid element.
//
// Arguments:
// - iter: the iterator to test.
//
// Returns:
// - true: if iter is not at the end of the table (implying that
//   the table is non-empty).
// - false: if iter is past the end of the table.
//
bool HTIterator_IsValid(HTIterator *iter);

// Advance the iterator to the next element of the table.
//
// Arguments:
// - iter: the iterator to move.  Must be non-NULL.
//
// Returns:
// - true: if the iterator has been advanced to the next element.
// - false: if the iterator cannot be advanced (eg, it's "past the
//   end").  The iterator is no longer valid at this point.
bool HTIterator_Next(HTIterator *iter);

// Returns a copy of the (key,value) that the iterator is currently
// pointing at.
//
// Arguments:
// - iter: the iterator to fetch the (key,value) from.  Must be non-NULL.
// - keyvalue: a return parameter through which the (key,value)
//   is returned.
//
// Returns:
// - false: if the iterator is not valid or the table is empty.
// - true: success.
bool HTIterator_Get(HTIterator *iter, HTKeyValue_t *keyvalue);

// Returns a copy of (key,value) that the iterator is currently
// pointing at, and removes that (key,value) from the
// hashtable.  The caller assumes ownership of any memory
// pointed to by the value.  As well, this advances
// the iterator to the next element in the hashtable.
//
// Arguments:
// - iter: the iterator to fetch the (key,value) from.  Must be non-NULL.
// - keyvalue: a return parameter through which the (key,value)
//   is returned.
//
// Returns:
// - false: if the iterator is not valid or the table is empty.
// - true: successful deletion.  If there was a subsequent
//   element, the iterator has been advanced to it.  If the
//   iterator is past the end of the table, the iterator is
//   now invalid.
bool HTIterator_Remove(HTIterator *iter, HTKeyValue_t *keyvalue);

#endif  // HW1_HASHTABLE_H_
