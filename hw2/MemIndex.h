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

#ifndef HW2_MEMINDEX_H_
#define HW2_MEMINDEX_H_

#include <stdbool.h>
#include "libhw1/HashTable.h"
#include "libhw1/LinkedList.h"
#include "./DocTable.h"

// A word's byte offset from the start of a document.
typedef uint32_t DocPositionOffset_t;

// A MemIndex is an in-memory inverted index.
//
// An inverted index is a unidirectional mapping of word to a logical set
// of (document, document offset) tuples, indicating all the occurances
// of that word within the document corpus.
//
// Although C is a procedural language, HW1 demonstrated that we could
// implement some aspects of object-oriented programming; namely, data
// encapsulation (ie, a struct), access restrictions (ie, hiding the struct
// definition in the .c), and information hiding (ie, hiding functionality
// in the .c).  MemIndex is a primitive example of how to do another OOP
// feature: inheritance.  Specifically, it is a simple example of
// implementation inheritance (ie, we reuse methods but add no new fields).
// More features, such as dynamic dispatch (also known as "late binding")
// and adding new fields would require the creation of a bookkeeping struct
// that contained function pointers (to enable dynamic dispatch) and field
// offsets (to enable adding fields) -- essentially, a hand-implemented
// vtable.  See also DocTable for an example of class composition.
typedef HashTable MemIndex;

// Allocate and return a new MemIndex.  The caller takes responsibility for
// eventually calling MemIndex_Free to free memory associated with the table.
//
// Arguments: none.
//
// Returns:
// - the newly-allocated table (never NULL).
MemIndex *MemIndex_Allocate(void);

// Frees a MemIndex that was previously allocated by MemIndex_Allocate,
// including all the structures stored within it.
//
// Arguments:
// - index: a previously-allocated MemIndex.
void MemIndex_Free(MemIndex *index);

// Returns the number of words contained within the index.
//
// Arguments:
// - index: the MemIndex
//
// Returns:
// - the number of unique words in the index.
int MemIndex_NumWords(MemIndex *index);

// Adds a "posting list" to the MemIndex.
//
// A "posting list" is the list of positions within a specific document where
// a particular word appears.  The postings (ie, positions) are specified as
// byte offsets from the beginning of the document (ie, DocPositionOffset_t),
// and the passed-in list must be in ascending order.  The passed-in word must
// not have been previously added to this document.
//
// Arguments:
// - index: the MemIndex to add these postings to
// - word: the word that these postings refer to.  MemIndex takes ownership
//   of this argument.
// - docid: the document containing these postings
// - postings: a non-empty list of byte offsets, in ascending order.
//   MemIndex takes ownership of this list.
void MemIndex_AddPostingList(MemIndex *index, char *word, DocID_t doc_id,
                             LinkedList *postings);

// A document that matches a search query.
typedef struct {
  DocID_t doc_id;  // a document that matches a search query
  int rank;   // an indicator of the quality of the match
} SearchResult;


// Processes a query against the MemIndex, returning a list SearchResults.
//
// The results are a list of SearchResult's, containing all the documents
// containing every query word at least once.  The results will be returned
// in decreasing quality (i.e. sorted by the rank field in decreasing order).
// The client is responsible for freeing the returned list of SearchResults;
// the appropriate deallocator to pass into LinkedList_Free is stdlib's free().
//
// Arguments:
// - index: the MemIndex to query
// - query: a non-empty array of strings; each string is a single
//          null-terminated query word, all lower-case.
// - querylen: the number of words in the query array.
//
// Returns:
// - NULL: if no matching documents were found
// - a non-NULL LinkedList of SearchResult's
LinkedList *MemIndex_Search(MemIndex *index, char *query[], int query_len);


//////////////////////////////////////////////////////////////////////////////

// The struct stored within our hash table as the HTValue_t.
//
// For HW3, clients will need to break the MemIndex abstraction by directly
// accessing the contents of the MemIndex HashTable.  This struct is how
// we implement the "logical set of (document, document offset) tuples"
// which is discussed at the top of the file; the 'word' field is the logical
// key to the inverted index and the 'postings' is its associated value,
// represented as a mapping from DocID_t -> LinkedList(DocPositionOffset_t).
//
// The WordPostings struct owns the memory allocated to both the word and
// postings fields.
typedef struct {
  char        *word;
  HashTable   *postings;
} WordPostings;

#endif  // HW2_MEMINDEX_H_
