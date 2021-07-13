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

extern "C" {
  #include "./FileParser.h"
}

#include "gtest/gtest.h"
#include "./test_suite.h"

namespace hw2 {

TEST(Test_FileParser, ReadFileToString) {
  HW2Environment::OpenTestCase();
  char *filebuf = NULL;
  int len = 0;

  // Read a non-trivial file.
  filebuf = ReadFileToString(
      const_cast<char *>("./test_tree/books/aliceinwonderland.txt"), &len);
  ASSERT_NE(static_cast<char *>(NULL), filebuf);
  ASSERT_EQ(163763, len);
  ASSERT_EQ('P', filebuf[0]);
  ASSERT_EQ('s', filebuf[163760]);
  free(filebuf);
  HW2Environment::AddPoints(10);

  // Try to read a non-existent file.
  filebuf = ReadFileToString(const_cast<char *>("bogus/file"), &len);
  ASSERT_EQ(static_cast<char *>(NULL), filebuf);
  HW2Environment::AddPoints(10);

  // Read a small file.
  filebuf = ReadFileToString(const_cast<char *>("./test_tree/bash-4.2/trap.h"),
                             &len);
  ASSERT_NE(static_cast<char *>(NULL), filebuf);
  ASSERT_EQ(3213, len);
  ASSERT_EQ('/', filebuf[0]);
  ASSERT_EQ('\n', filebuf[3212]);
  free(filebuf);
  HW2Environment::AddPoints(10);

  // Try to read a file that contains non-ASCII/non-UTF-8 bytes.
  filebuf = ReadFileToString(const_cast<char *>("./test_tree/libhw1.a"), &len);
  ASSERT_EQ(static_cast<char *>(NULL), filebuf);
  HW2Environment::AddPoints(10);
}

TEST(Test_FileParser, ParseIntoWordPositionsTable) {
  HW2Environment::OpenTestCase();
  HashTable *tab;
  int numbytes;
  char *filebuf = ReadFileToString(
      const_cast<char *>("./test_tree/bash-4.2/doc/README"),
      &numbytes);
  ASSERT_NE(static_cast<char*>(NULL), filebuf);
  ASSERT_LT(0, numbytes);

  tab = ParseIntoWordPositionsTable(filebuf);
  ASSERT_NE(static_cast<HashTable *>(NULL), tab);

  ASSERT_EQ(91, HashTable_NumElements(tab));
  HW2Environment::AddPoints(10);

  HTKeyValue_t kv;
  WordPositions *wp;
  LLPayload_t pos;

  static const char *kW1 = "article";  // 154, 170
  ASSERT_TRUE(HashTable_Find(tab,
                             FNVHash64((unsigned char *) kW1, strlen(kW1)),
                             &kv));
  wp = static_cast<WordPositions *>(kv.value);
  ASSERT_EQ(0, strcmp(kW1, wp->word));
  ASSERT_EQ(2, LinkedList_NumElements(wp->positions));
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(154U), pos);
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(170U), pos);
  HW2Environment::AddPoints(5);

  static const char *kW2 = "identical";  // 918
  ASSERT_TRUE(HashTable_Find(tab,
                             FNVHash64((unsigned char *) kW2, strlen(kW2)),
                             &kv));
  wp = static_cast<WordPositions *>(kv.value);
  ASSERT_EQ(0, strcmp(kW2, wp->word));
  ASSERT_EQ(1, LinkedList_NumElements(wp->positions));
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(918U), pos);
  HW2Environment::AddPoints(5);

  static const char *kW3 = "versions";  // 499, 550, 653
  ASSERT_TRUE(HashTable_Find(tab,
                             FNVHash64((unsigned char *) kW3, strlen(kW3)),
                             &kv));
  wp = static_cast<WordPositions *>(kv.value);
  ASSERT_EQ(0, strcmp(kW3, wp->word));
  ASSERT_EQ(3, LinkedList_NumElements(wp->positions));
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(499U), pos);
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(550U), pos);
  ASSERT_TRUE(LinkedList_Pop(wp->positions, &pos));
  ASSERT_EQ(reinterpret_cast<LLPayload_t>(653U), pos);
  HW2Environment::AddPoints(5);

  ASSERT_FALSE(HashTable_Find(
                   tab,
                   FNVHash64((unsigned char *) "nonexistantword", 4),
                   &kv));
  HW2Environment::AddPoints(10);

  FreeWordPositionsTable(tab);
  HW2Environment::AddPoints(10);
}

TEST(Test_FileParser, ParseIntoWordPositionsTable_Large) {
  HW2Environment::OpenTestCase();
  HashTable *tab;
  int numbytes;
  char *filebuf = ReadFileToString(
      const_cast<char *>("./test_tree/books/lesmiserables.txt"),
      &numbytes);
  ASSERT_NE(static_cast<char*>(NULL), filebuf);
  ASSERT_LT(0, numbytes);

  tab = ParseIntoWordPositionsTable(filebuf);
  ASSERT_NE(static_cast<HashTable *>(NULL), tab);

  ASSERT_EQ(22876, HashTable_NumElements(tab));
  FreeWordPositionsTable(tab);
  HW2Environment::AddPoints(10);
}

TEST(Test_FileParser, ParseIntoWordPositionsTable_NotAscii) {
  HashTable *tab;
  int numbytes;
  char *filebuf = ReadFileToString(
      const_cast<char *>("./test_tree/bash-4.2/doc/article.pdf"),
      &numbytes);
  ASSERT_NE(static_cast<char*>(NULL), filebuf);
  ASSERT_LT(0, numbytes);

  tab = ParseIntoWordPositionsTable(filebuf);
  ASSERT_EQ(static_cast<HashTable *>(NULL), tab);
}

}  // namespace hw2
