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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtest/gtest.h"

extern "C" {
  #include "./MemIndex.h"
  #include "libhw1/LinkedList.h"
  #include "./CrawlFileTree.h"
  #include "./DocTable.h"
}

#include "./test_suite.h"

char *MakeCopy(const char* s) {
  int len = strlen(s);
  char *copy = static_cast<char*>(malloc(len + 1));
  strncpy(copy, s, len);
  copy[len] = '\0';
  return copy;
}

namespace hw2 {

// Tests that MemIndex and CrawlFileTree
// are properly integrated to work with each other.
TEST(Test_MemIndex, Integration) {
  HW2Environment::OpenTestCase();
  int res;
  DocTable *dt;
  MemIndex *idx;

  LinkedList *llres;
  LLIterator *lit;
  int i;

  const char *q1[] = {"equations"};
  const char *q2[] = {"report", "normal"};
  const char *q3[] = {"report", "suggestions", "normal"};
  const char *q4[] = {"report", "normal", "foobarbaz"};

  // Crawl the test tree.
  res = CrawlFileTree(const_cast<char *>("./test_tree/bash-4.2/support"),
                      &dt,
                      &idx);
  ASSERT_EQ(1, res);
  ASSERT_EQ(3852, MemIndex_NumWords(idx));

  // Process query 1, check results.
  llres = MemIndex_Search(idx, const_cast<char **>(q1), 1);
  ASSERT_NE((LinkedList *) NULL, llres);
  ASSERT_EQ(LinkedList_NumElements(llres), 2);
  lit = LLIterator_Allocate(llres);
  for (i = 0; i < LinkedList_NumElements(llres); i++) {
    SearchResult *res;

    LLIterator_Get(lit, reinterpret_cast<LLPayload_t*>(&res));
    if (i == 0) {
      char *docname = DocTable_GetDocName(dt, res->doc_id);
      ASSERT_EQ(
         strcmp(docname,
                "./test_tree/bash-4.2/support/texi2html"), 0);
    } else if (i == 1) {
      char *docname = DocTable_GetDocName(dt, res->doc_id);
      ASSERT_EQ(
         strcmp(docname,
                "./test_tree/bash-4.2/support/man2html.c"), 0);
    }
    LLIterator_Next(lit);
  }
  LLIterator_Free(lit);
  LinkedList_Free(llres, reinterpret_cast<LLPayloadFreeFnPtr>(&free));
  HW2Environment::AddPoints(20);

  // Process query 2, check results.
  llres = MemIndex_Search(idx, const_cast<char **>(q2), 2);
  ASSERT_NE((LinkedList *) NULL, llres);
  ASSERT_EQ(LinkedList_NumElements(llres), 2);
  lit = LLIterator_Allocate(llres);
  for (i = 0; i < LinkedList_NumElements(llres); i++) {
    SearchResult *res;

    LLIterator_Get(lit, reinterpret_cast<LLPayload_t*>(&res));
    if (i == 0) {
      char *docname = DocTable_GetDocName(dt, res->doc_id);
      ASSERT_EQ(
         strcmp(docname,
                "./test_tree/bash-4.2/support/texi2html"), 0);
    } else if (i == 1) {
      char *docname = DocTable_GetDocName(dt, res->doc_id);
      ASSERT_EQ(
         strcmp(docname,
                "./test_tree/bash-4.2/support/man2html.c"), 0);
    }
    LLIterator_Next(lit);
  }
  LLIterator_Free(lit);
  LinkedList_Free(llres, reinterpret_cast<LLPayloadFreeFnPtr>(&free));
  HW2Environment::AddPoints(20);

  // Process query 3, check results.
  llres = MemIndex_Search(idx, const_cast<char **>(q3), 3);
  ASSERT_NE((LinkedList *) NULL, llres);
  ASSERT_EQ(LinkedList_NumElements(llres), 1);
  lit = LLIterator_Allocate(llres);
  for (i = 0; i < LinkedList_NumElements(llres); i++) {
    SearchResult *res;

    LLIterator_Get(lit, reinterpret_cast<LLPayload_t*>(&res));
    if (i == 0) {
      char *docname = DocTable_GetDocName(dt, res->doc_id);
      ASSERT_EQ(
         strcmp(docname,
                "./test_tree/bash-4.2/support/texi2html"), 0);
    }
    LLIterator_Next(lit);
  }
  LLIterator_Free(lit);
  LinkedList_Free(llres, reinterpret_cast<LLPayloadFreeFnPtr>(&free));
  HW2Environment::AddPoints(20);

  // Process query 4, check results.
  llres = MemIndex_Search(idx, const_cast<char **>(q4), 3);
  ASSERT_EQ((LinkedList *) NULL, llres);
  HW2Environment::AddPoints(20);

  // Free up everything.
  DocTable_Free(dt);
  MemIndex_Free(idx);
  HW2Environment::AddPoints(10);
}

TEST(Test_MemIndex, Simple) {
  HW2Environment::OpenTestCase();
  constexpr DocID_t kDocID1 = 1234;
  constexpr DocID_t kDocID2 = 5678;

  // We need to assign these logically-constant strings into a non-const
  // pointers because the compiler won't let me cast away the const
  // qualifier on a string literal.
  const char *kBananas = "bananas";
  const char *kPears = "pears";
  const char *kApples = "apples";
  const char *kGrapes = "grapes";

  LinkedList *ll1 = LinkedList_Allocate();
  LinkedList_Push(ll1, (LLPayload_t)100);
  LinkedList_Push(ll1, (LLPayload_t)200);

  LinkedList *ll2 = LinkedList_Allocate();
  LinkedList_Push(ll2, (LLPayload_t)300);

  LinkedList *ll3 = LinkedList_Allocate();
  LinkedList_Push(ll3, (LLPayload_t)400);
  LinkedList_Push(ll3, (LLPayload_t)500);
  LinkedList_Push(ll3, (LLPayload_t)600);

  LinkedList *ll4 = LinkedList_Allocate();
  LinkedList_Push(ll4, (LLPayload_t)700);

  LinkedList *ll5 = LinkedList_Allocate();
  LinkedList_Push(ll5, (LLPayload_t)800);

  MemIndex *idx = MemIndex_Allocate();

  // Document 1 has bananas, pears, and apples.
  MemIndex_AddPostingList(idx, MakeCopy(kBananas), kDocID1, ll1);
  MemIndex_AddPostingList(idx, MakeCopy(kPears), kDocID1, ll2);
  MemIndex_AddPostingList(idx, MakeCopy(kApples), kDocID1, ll3);

  // Document 2 only has apples and bananas.
  MemIndex_AddPostingList(idx, MakeCopy(kApples), kDocID2, ll4);
  MemIndex_AddPostingList(idx, MakeCopy(kBananas), kDocID2, ll5);

  ASSERT_EQ(3, MemIndex_NumWords(idx));

  // No results.
  char *query1[] = {const_cast<char *>(kGrapes)};
  LinkedList *results = MemIndex_Search(idx, query1, 1);
  ASSERT_EQ(0, (LinkedList *)results);

  HW2Environment::AddPoints(5);

  // One resultant document.
  char *query2[] = {const_cast<char *>(kPears)};
  results = MemIndex_Search(idx, query2, 1);
  ASSERT_EQ(1, LinkedList_NumElements(results));
  LLIterator *itr = LLIterator_Allocate(results);
  SearchResult *res;
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID1, res->doc_id);
  LLIterator_Free(itr);
  LinkedList_Free(results, (LLPayloadFreeFnPtr)free);

  HW2Environment::AddPoints(5);

  // Multiple resultant documents.
  char *query3[] = {const_cast<char *>(kApples)};
  results = MemIndex_Search(idx, query3, 1);
  ASSERT_EQ(2, LinkedList_NumElements(results));
  itr = LLIterator_Allocate(results);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID1, res->doc_id);
  LLIterator_Next(itr);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID2, res->doc_id);
  LLIterator_Free(itr);
  LinkedList_Free(results, (LLPayloadFreeFnPtr)free);

  HW2Environment::AddPoints(5);

  // Multiple search terms.
  char *query4[] = {const_cast<char *>(kApples), const_cast<char *>(kBananas)};
  results = MemIndex_Search(idx, query4, 2);
  ASSERT_EQ(2, LinkedList_NumElements(results));
  itr = LLIterator_Allocate(results);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID1, res->doc_id);
  LLIterator_Next(itr);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID2, res->doc_id);
  LLIterator_Free(itr);
  LinkedList_Free(results, (LLPayloadFreeFnPtr)free);

  // Multiple search terms: testing different term order.
  char *query5[] = {const_cast<char *>(kBananas), const_cast<char *>(kApples)};
  results = MemIndex_Search(idx, query5, 2);
  ASSERT_EQ(2, LinkedList_NumElements(results));
  itr = LLIterator_Allocate(results);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID1, res->doc_id);
  LLIterator_Next(itr);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID2, res->doc_id);
  LLIterator_Free(itr);
  LinkedList_Free(results, (LLPayloadFreeFnPtr)free);

  // Multiple search terms: not all documents should be results.
  char *query6[] = {const_cast<char *>(kPears), const_cast<char *>(kBananas)};
  results = MemIndex_Search(idx, query6, 2);
  ASSERT_EQ(1, LinkedList_NumElements(results));
  itr = LLIterator_Allocate(results);
  LLIterator_Get(itr, reinterpret_cast<LLPayload_t*>(&res));
  ASSERT_EQ(kDocID1, res->doc_id);
  LLIterator_Free(itr);
  LinkedList_Free(results, (LLPayloadFreeFnPtr)free);

  HW2Environment::AddPoints(5);

  MemIndex_Free(idx);
}

}  // namespace hw2
