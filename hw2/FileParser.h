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

#ifndef HW2_FILEPARSER_H_
#define HW2_FILEPARSER_H_

#include "libhw1/LinkedList.h"
#include "libhw1/HashTable.h"

// Reads the full contents of "file_name" into memory, malloc'ing space for
// its contents and returning a pointer to the allocated memory.  No special
// escaping/handling is provided if the file contains null (ie, zero-valued)
// bytes.
//
// Arguments:
// - file_name:  a string containing the pathname of the file to read
// - size: if successful, an output parameter through which we return the size
//         of the file in bytes, assuming we successfully read it in.
//
// Returns:
// - NULL, if the read fails
// - Otherwise, the file's content.  This function will add '\0' to the end
//   of the file; however, the number of bytes returned through 'size' doesn't
//   include the final '\0'.  The caller is responsible for freeing the
//   returned pointer.
char *ReadFileToString(const char *file_name, int *size);


// This is our HTKeyValue_t, which maps a word to all its positions within
// the file.
//
// We play a nasty trick with the linked list; instead of malloc'ing space for
// the linked list's payload, we instead cast the 64-bit DocPositionOffset_t
// position as a 64-bit LLPayload_t (ie, a void *); note we're assuming that
// pointers are 64 bits long in order to do this.
typedef struct WordPositions {
  char        *word;        // normalized word.  Owned.
  LinkedList  *positions;   // list of DocPositionOffset_t.  Owned.
} WordPositions;

// Parses the passed-in string into words, then builds a word->positions table.
//
// Words are defined as a string of whitespace-delimited alphabetic ASCII
// characters (eg, no punctuation, no numbers, etc.) and are normalized to
// its lowercase form.  Each unique word is mapped into a list of "positions"
// (ie, byte offset from the beginning of the string) using a malloc'ed
// WordPositions struct, above.
//
// ParseIntoWordPositions takes ownership of the passed-in string.  The caller
// is responsible for freeing the resultant HashTable.
//
// Arguments:
//  - file_contents: a null-terminated string of words.  Takes ownership.
//
// Returns:
// - NULL, on failure (e.g., empty content, contains non-ASCII characters, no
//   parseable content).
// - a HashTable of (FNVHash64(word), WordPositions(word)).  Caller is
//   responsible for freeing this structure using FreeWordPositions(), below.
HashTable *ParseIntoWordPositionsTable(char *file_contents);

// Frees memory allocated by ParseIntoWordPositions.
void FreeWordPositionsTable(HashTable *table);

#endif  // HW2_FILEPARSER_H_
