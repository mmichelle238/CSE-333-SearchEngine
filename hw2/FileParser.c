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

// Feature test macro enabling strdup (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include "./FileParser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "libhw1/CSE333.h"
#include "./MemIndex.h"


///////////////////////////////////////////////////////////////////////////////
// Constants and declarations of internal helper functions

#define ASCII_UPPER_BOUND 0x7F

// Frees a WordPositions.positions's payload, which is just a
// DocPositionOffset_t.
static void NoOpFree(LLPayload_t payload) { }

// Frees a WordPositions struct.
static void FreeWordPositions(HTValue_t payload) {
  WordPositions *pos = (WordPositions *) payload;
  LinkedList_Free(pos->positions, &NoOpFree);
  free(pos->word);
  free(pos);
}

// Add a normalized word and its byte offset into the WordPositions HashTable.
static void AddWordPosition(HashTable *tab, char *word,
                            DocPositionOffset_t pos);

// Parse the passed-in string into normalized words and insert into a HashTable
// of WordPositions structures.
static void InsertContent(HashTable *tab, char *content);


///////////////////////////////////////////////////////////////////////////////
// Publically-exported functions

char *ReadFileToString(const char *file_name, int *size) {
  struct stat file_stat;
  char *buf;
  int result, fd;
  ssize_t num_read;
  size_t left_to_read;

  // STEP 1.
  // Use the stat system call to fetch a "struct stat" that describes
  // properties of the file. ("man 2 stat"). You can assume we're on a 64-bit
  // system, with a 64-bit off_t field.
  result = stat(file_name, &file_stat);
  if (result != 0) {
    return NULL;
  }


  // STEP 2.
  // Make sure this is a "regular file" and not a directory or something else
  // (use the S_ISREG macro described in "man 2 stat").
  if (!S_ISREG(file_stat.st_mode)) {
    return NULL;
  }


  // STEP 3.
  // Attempt to open the file for reading (see also "man 2 open").
  fd = open(file_name, O_RDONLY);
  if (fd == -1) {
    return NULL;
  }


  // STEP 4.
  // Allocate space for the file, plus 1 extra byte to
  // '\0'-terminate the string.
  buf = (char*) malloc(file_stat.st_size * sizeof(char) + sizeof(char));


  // STEP 5.
  // Read in the file contents using the read() system call (see also
  // "man 2 read"), being sure to handle the case that read() returns -1 and
  // errno is either EAGAIN or EINTR.  Also, note that it is not an error for
  // read to return fewer bytes than what you asked for; you'll need to put
  // read() inside a while loop, looping until you've read to the end of file
  // or a non-recoverable error.  Read the man page for read() carefully, in
  // particular what the return values -1 and 0 imply.
  left_to_read = file_stat.st_size;
  num_read = 0;
  while (left_to_read > 0) {
    // read into buffer
    num_read = read(fd, buf, left_to_read);
    if (num_read == -1) {
      if (errno != EINTR && errno != EAGAIN) {
        perror("read failed");
        return NULL;
      }
      continue;
    }
    left_to_read -= num_read;
  }

  // Great, we're done!  We hit the end of the file and we read
  // filestat.st_size - left_to_read bytes. Close the file descriptor returned
  // by open() and return through the "size" output parameter how many bytes
  // we read.
  close(fd);
  *size = file_stat.st_size - left_to_read;

  // Null-terminate the string.
  buf[*size] = '\0';
  return buf;
}

HashTable *ParseIntoWordPositionsTable(char *file_contents) {
  HashTable *tab;
  int i, file_len;

  if (file_contents == NULL) {
    return NULL;
  }

  file_len = strlen(file_contents);
  if (file_len == 0) {
    return NULL;
  }

  // Verify that the file contains only ASCII text.  We won't try to index any
  // files that contain non-ASCII text; unfortunately, this means we aren't
  // Unicode friendly.
  for (i = 0; i < file_len; i++) {
    if (file_contents[i] == '\0' ||
        (unsigned char) file_contents[i] > ASCII_UPPER_BOUND) {
      free(file_contents);
      return NULL;
    }
  }

  // Great!  Let's split the file up into words.  We'll allocate the hash
  // table that will store the WordPositions structures associated with each
  // word.  Since our hash table dynamically grows, we'll start with a small
  // number of buckets.
  tab = HashTable_Allocate(32);
  Verify333(tab != NULL);

  // Loop through the file, splitting it into words and inserting a record for
  // each word.
  InsertContent(tab, file_contents);

  // If we found no words, return NULL instead of a zero-sized hashtable.
  if (HashTable_NumElements(tab) == 0) {
    HashTable_Free(tab, &FreeWordPositions);
    tab = NULL;
  }

  // Now that we've finished parsing the document, we can free up the
  // filecontents buffer and return our built-up table.
  free(file_contents);
  return tab;
}

void FreeWordPositionsTable(HashTable *table) {
  HashTable_Free(table, &FreeWordPositions);
}


///////////////////////////////////////////////////////////////////////////////
// Internal helper functions

static void InsertContent(HashTable *tab, char *content) {
  char *cur_ptr = content, *word_start = content;

  // STEP 6.
  // This is the interesting part of Part A!
  //
  // "content" contains a C string with the full contents of the file.  You
  // need to implement a loop that steps through the file content one character
  // at a time, testing to see whether a character is an alphabetic character.
  // If a character is alphabetic, it's part of a word.  If a character is not
  // alphabetic, it's part of the boundary between words.  You can use
  // string.h's "isalpha()" macro to test whether a character is alphabetic or
  // not (see also "man isalpha").
  //
  // For example, here's a string with its words underlined with "=" and
  // boundary characters underlined with "+":
  //
  // The  Fox  Can't   CATCH the  Chicken.
  // ===++===++===+=+++=====+===++=======+
  //
  // Any time you detect the start of a word, you should use the "wordstart"
  // pointer to remember where the word started.  You should also use the
  // "tolower()" macro to convert alphabetic characters to lowercase (ie,
  // *curptr = tolower(*curptr)).  Finally, as a hint, you can overwrite
  // boundary characters with '\0' (null terminators) in the buffer to create
  // valid C strings out of each parsed word.
  //
  // Each time you find a word that you want to record in the hashtable, call
  // AddWordPosition() helper with appropriate arguments, e.g.,
  //    AddWordPosition(tab, wordstart, pos);

  char* beginning = content;

  bool in_word = false;
  while (true) {
    char cur_char = *cur_ptr;
    if (isalpha(cur_char)) {
      // set letter to lowercase
      *cur_ptr = tolower(*cur_ptr);
      if (!in_word) {
        word_start = cur_ptr;
      }
      in_word = true;
    } else {
      bool end = false;
      // have we reached end of file?
      if (*cur_ptr == '\0') {
        end = true;
      }
      if (in_word) {
        // otherwise, add null-terminator
        *cur_ptr = '\0';
        AddWordPosition(tab, word_start, word_start - beginning);
      }
      // we've reached a boundary character
      in_word = false;
      if (end) {
        break;
      }
    }
    cur_ptr++;
  }  // end while-loop
}

static void AddWordPosition(HashTable *tab, char *word,
                            DocPositionOffset_t pos) {
  HTKey_t hash_key;
  HTKeyValue_t kv;
  WordPositions *wp;

  // Hash the string.
  hash_key = FNVHash64((unsigned char *) word, strlen(word));

  // Have we already encountered this word within this file?  If so, it's
  // already in the hashtable.
  if (HashTable_Find(tab, hash_key, &kv)) {
    // Yes; we just need to add a position in using LinkedList_Append(). Note
    // how we're casting the DocPositionOffset_t position variable to an
    // LLPayload_t to store it in the linked list payload without needing to
    // malloc space for it.  Ugly, but it works!
    wp = (WordPositions *) kv.value;

    // Ensure we don't have hash collisions (two different words that hash to
    // the same key, which is very unlikely).
    Verify333(strcmp(wp->word, word) == 0);

    LinkedList_Append(wp->positions, (LLPayload_t) (int64_t) pos);
  } else {
    // STEP 7.
    // No; this is the first time we've seen this word.  Allocate and prepare
    // a new WordPositions structure, and append the new position to its list
    // using a similar ugly hack as right above.
    HTKeyValue_t new_val;
    new_val.key = hash_key;

    wp = (WordPositions*)malloc(sizeof(WordPositions));
    Verify333(wp != NULL);
    // make copy of word
    char* word_copy = calloc(strlen(word) + 1, sizeof(char));
    snprintf(word_copy, strlen(word) + 1, "%s", word);
    wp->word = word_copy;
    Verify333(strcmp(word, "") != 0);

    LinkedList *list = LinkedList_Allocate();
    Verify333(list != NULL);
    // add structure to LinkedList
    LinkedList_Append(list, (LLPayload_t) (int64_t) pos);
    wp->positions = list;
    new_val.value = wp;

    HTKeyValue_t *garbage = NULL;
    Verify333(!HashTable_Insert(tab, new_val, garbage));
  }
}
