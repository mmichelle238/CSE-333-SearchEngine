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

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

// From Ed for getline
#define _GNU_SOURCE

#define MAX_BUF_LENGTH 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
/* prints out a usage error when user doesn't pass in the
expected number of arguments*/
static void Usage(void);

/*using the given DocTable and LinkedList, prints out the document name
and rank for each search result stored in the linked list*/
static void ProcessQueries(DocTable *table, LinkedList *results);

/*seperates the given query into the string array output parameter and
returns the query length*/
static int ProcessNextLine(char **ret_str, char *query);


//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char **argv) {
  if (argc != 2) {
    Usage();
  }

  printf("Indexing \'%s\'\n", argv[1]);
  DocTable *table;
  MemIndex *index;
  if (!CrawlFileTree(argv[1], &table, &index)) {
    fprintf(stderr, "Bad news\n");
    return EXIT_FAILURE;
  }

  printf("enter query:\n");
  char *query = NULL;
  size_t len = 0;

  /* loops through all of the different query entries until the user presses
  cntrl-D from the keyboard*/
  while (getline(&query, &len, stdin) != -1) {
    query[strlen(query) - 1] = '\0';
    char **words = (char**)(malloc(sizeof(char*) * MAX_BUF_LENGTH));
    int query_len = ProcessNextLine(words, query);
    LinkedList *results = MemIndex_Search(index, words, query_len);
    ProcessQueries(table, results);
    free(words);
    printf("enter query:\n");
    free(query);
    query = NULL;
    len = 0;
  }

  free(query);
  DocTable_Free(table);
  MemIndex_Free(index);
  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable *table, LinkedList *results) {
  if (results != NULL) {
    LLIterator *iterator = LLIterator_Allocate(results);
    while (LLIterator_IsValid(iterator)) {
      LLPayload_t result;
      LLIterator_Get(iterator, (LLPayload_t*)(&result));
      SearchResult search_result = *(SearchResult*)(result);
      // parse the search result
      char *doc_name = DocTable_GetDocName(table, search_result.doc_id);
      int rank = search_result.rank;
      printf("  %s (%d)\n", doc_name, rank);
      LLIterator_Next(iterator);
    }
    LLIterator_Free(iterator);
    LinkedList_Free(results, free);
  }
}

static int ProcessNextLine(char **ret_str, char *query) {
  char *curr_word;
  char *context;
  int query_len = 0;
  for (int i = 0; i < MAX_BUF_LENGTH; i++) {
    if (i == 0) {
      curr_word = strtok_r(query, " ", &context);
    } else {
      curr_word = strtok_r(NULL, " ", &context);
    }
    if (curr_word == NULL) {
      break;
    }
    for (int j = 0; j < strlen(curr_word); j++) {
      curr_word[j] = tolower(curr_word[j]);
    }
    query_len++;
    ret_str[i] = curr_word;
  }
  return query_len;
}
