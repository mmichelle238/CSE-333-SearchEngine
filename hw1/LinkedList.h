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

#ifndef HW1_LINKEDLIST_H_
#define HW1_LINKEDLIST_H_

#include <stdbool.h>    // for bool type (true, false)
#include <stdint.h>     // for uint64_t, etc.


///////////////////////////////////////////////////////////////////////////////
// A LinkedList is a doubly-linked list.
//
// We provide the interface to the LinkedList here; your job is to fill
// in the implementation holes that we left in LinkedList.c.
//
// To hide the implementation of LinkedList, we declare the "struct ll"
// structure and its associated typedef here, but we *define* the structure
// in the internal header LinkedList_priv.h. This lets us define a pointer
// to LinkedList as a new type while leaving the implementation details
// opaque to the customer.
typedef struct ll LinkedList;

// LLPayload type definition:
// For generality, a payload must be large enough to hold a pointer.
// If the client's data is no bigger than a pointer, a copy of that
// data can be stored in the LinkedList, by casting it to the LLPayload
// type.  Otherwise, a pointer to the client's data is maintained in
// the list.
typedef void* LLPayload_t;

// When a customer frees a linked list or a contained node, they need
// to pass in a pointer to a function which does any necessary freeing
// of the payload. We invoke the pointed-to function once for each node
// requiring freeing.
typedef void(*LLPayloadFreeFnPtr)(LLPayload_t payload);

// Allocate and return a new linked list.  The caller takes responsibility for
// eventually calling LinkedList_Free to free memory associated with the list.
//
// Arguments: none.
//
// Returns:
// - the newly-allocated linked list (never NULL).
LinkedList* LinkedList_Allocate(void);

// Free a linked list that was previously allocated by LinkedList_Allocate.
//
// Arguments:
// - list: the linked list to free.  It is unsafe to use "list" after this
//   function returns.
// - payload_free_function: a pointer to a payload freeing function; see above
//   for details on what this is.
void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function);

// Return the number of elements in the linked list.
//
// Arguments:
// - list:  the list to query.
//
// Returns:
// - list length.
int LinkedList_NumElements(LinkedList *list);

// Adds a new element to the head of the linked list.
//
// Arguments:
// - list: the LinkedList to push onto.
// - payload: the payload to push; it's up to the caller to interpret and
//   manage the memory of the payload.
void LinkedList_Push(LinkedList *list, LLPayload_t payload);

// Pop an element from the head of the linked list.
//
// Arguments:
// - list: the LinkedList to pop from.
// - payload_ptr: a return parameter; on success, the popped node's payload
//   is returned through this parameter.
//
// Returns:
// - false on failure (eg, the list is empty).
// - true on success.
bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr);

// Adds a new element to the tail of the linked list.
//
// This is the "tail" version of LinkedList_Push.
//
// Arguments:
// - list: the LinkedList to push onto.
// - payload: the payload to push; it's up to the caller to interpret and
//   manage the memory of the payload.
void LinkedList_Append(LinkedList *list, LLPayload_t payload);

// When sorting a linked list or comparing two elements of a linked list,
// customers must pass in a comparator function.  The function accepts two
// payloads as arguments and returns an integer that is:
//    -1  if payload_a < payload_b
//     0  if payload_a == payload_b
//    +1  if payload_a > payload_b
typedef int(*LLPayloadComparatorFnPtr)(LLPayload_t payload_a,
                                       LLPayload_t payload_b);

// Sorts a LinkedList in place.
//
// Arguments:
// - list: the list to sort.
// - ascending: if false, sorts descending; else sorts ascending.
// - comparator_function:  this argument is a pointer to a payload comparator
//   function; see above.
void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function);


///////////////////////////////////////////////////////////////////////////////
// Linked list iterator.
//
// Linked lists support the notion of an iterator, similar to Java iterators.
// You use an iterator to navigate back and forth through the linked list and
// to insert/remove elements from the list.  You use LLIterator_Allocate() to
// manufacture a new iterator and LLIterator_Free() to free an iterator when
// you're done with it.
//
// If you use a LinkedList*() function to mutate a linked list, any iterators
// you have on that list become undefined (ie, dangerous to use; arbitrary
// memory corruption can occur). Thus, you should only use LLIterator*()
// functions in between the manufacturing and freeing of an iterator.
typedef struct ll_iter LLIterator;  // same trick to hide implementation.

// Manufacture an iterator for the list.  Caller is responsible for
// eventually calling LLIterator_Free to free memory associated with
// the iterator.
//
// Arguments:
// - list: the list from which we'll return an iterator.
//
// Returns:
// - a newly-allocated iterator, which may be invalid or "past the end" if
//   the list cannot be iterated through (eg, empty).
LLIterator* LLIterator_Allocate(LinkedList *list);

// When you're done with an iterator, you must free it by calling this
// function.
//
// Arguments:
// - iter: the iterator to free. Don't use it after freeing it.
void LLIterator_Free(LLIterator *iter);

// Tests to see whether the iterator is pointing at a valid element.
//
// Arguments:
// - iter: the iterator to test.
//
// Returns:
// - true: if iter is not past the end of the list.
// - false: if iter is past the end of the list.
bool LLIterator_IsValid(LLIterator *iter);

// Advance the iterator, i.e. move to the next node in the list.  The
// passed-in iterator must be valid (eg, not "past the end").
//
// Arguments:
// - iter: the iterator.
//
// Returns:
// - true: if the iterator has been advanced to the next node.
// - false: if the iterator is no longer valid after the
//   advancing has completed (eg, it's now "past the end").
bool LLIterator_Next(LLIterator *iter);

// Returns the payload of the list node that the iterator currently points
// at.  The passed-in iterator must be valid (eg, not "past the end").
//
// Arguments:
// - iter: the iterator to fetch the payload from.
// - payload: a "return parameter" through which the payload is returned.
void LLIterator_Get(LLIterator *iter, LLPayload_t *payload);

// Remove the node the iterator is pointing to.  After deletion, the iterator
// may be in one of the following three states:
// - if there was only one element in the list, the iterator is now invalid
//   and cannot be used.  In this case, the caller is recommended to free
//   the now-invalid iterator.
// - if the deleted node had a successor (ie, it was pointing at the tail),
//   the iterator is now pointing at the successor.
// - if the deleted node was the tail, the iterator is now pointing at the
//    predecessor.
//
// The passed-in iterator must be valid (eg, not "past the end").
//
// Arguments:
// - iter:  the iterator to delete from.
// - payload_free_function: invoked to free the payload.
//
// Returns:
// - false if the deletion succeeded, but the list is now empty.
// - true if the deletion succeeded, and the list is still non-empty.
bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function);

#endif  // HW1_LINKEDLIST_H_
