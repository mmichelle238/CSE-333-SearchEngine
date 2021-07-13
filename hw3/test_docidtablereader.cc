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

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>

#include "gtest/gtest.h"
extern "C" {
  #include "libhw1/CSE333.h"
}
#include "./DocIDTableReader.h"
#include "./IndexTableReader.h"
#include "./LayoutStructs.h"
#include "./test_suite.h"
#include "./Utils.h"

namespace hw3 {

TEST(Test_DocIDTableReader, TestDocIDTableReaderBasic) {
  HW3Environment::OpenTestCase();

  // This is a tough unit test to write, since we can't assume
  // much about where in an index file a DocIDTable lives.
  // So, we violate a little bit of the unit testing credo and
  // fold in a different module as a dependency: we make use of the
  // IndexTableReader class.

  // Open up the FILE * for ./unit_test_indices/enron.idx
  std::string idx("./unit_test_indices/enron.idx");
  FILE *f = fopen(idx.c_str(), "rb");
  ASSERT_NE(static_cast<FILE *>(nullptr), f);

  // Read in the size of the doctable.
  Verify333(fseek(f, DT_BYTES_OFFSET, SEEK_SET) == 0U);
  int32_t doctable_size;
  Verify333(fread(&doctable_size, sizeof(int32_t), 1, f) == 1);
  doctable_size = ntohl(doctable_size);

  // Prep an IndexTableReader; the word-->docid table is at offset
  // sizeof(IndexFileHeader) + doctable_size
  IndexTableReader itr(f, sizeof(IndexFileHeader) + doctable_size);

  // Use the IndexTableReader to manufacture a DocIDTableReader.
  DocIDTableReader *ditr = itr.LookupWord(std::string("happy"));
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);

  // Use the DocIDTableReader to look for matching docids.
  std::list<DocPositionOffset_t> match_list;
  bool success = ditr->LookupDocID(604, &match_list);
  auto it = match_list.begin();
  ASSERT_TRUE(success);
  ASSERT_EQ(2U, match_list.size());
  // can't make assumptions about the ordering:
  if (*it == 693U) {
    ASSERT_EQ(693U, *(it++));
    ASSERT_EQ(1433U, *it);
  } else {
    ASSERT_EQ(1433U, *(it++));
    ASSERT_EQ(693U, *it);
  }

  match_list.clear();
  success = ditr->LookupDocID(613, &match_list);
  it = match_list.begin();
  ASSERT_TRUE(success);
  ASSERT_EQ(1U, match_list.size());
  ASSERT_EQ(2201U, *it);

  match_list.clear();
  success = ditr->LookupDocID(37, &match_list);
  it = match_list.begin();
  ASSERT_TRUE(success);
  ASSERT_EQ(1U, match_list.size());
  ASSERT_EQ(790U, *it);

  // Lookup a doc_id that shouldn't exist.
  match_list.clear();
  success = ditr->LookupDocID(100000, &match_list);
  ASSERT_FALSE(success);
  ASSERT_EQ(0U, match_list.size());

  // Clean up.
  delete ditr;

  HW3Environment::AddPoints(20);

  // Test that DocIDTableReader::GetDocIDList() works
  //
  // Note that we have a map for the expected results since
  // DocIDTableReader::GetDocIDList doesn't make a garuntee on the ordering
  std::map<DocID_t, int32_t> terminal_results {
    {447U, 6},
    {733U, 1},
    {801U, 1}
  };
  std::map<DocID_t, int32_t> sun_results {
    {631U, 3},
    {635U, 2},
    {575U, 2},
    {630U, 1}
  };
  std::map<DocID_t, int32_t> careful_results {
    {203U, 1},
    {785U, 1}
  };


  // look up "terminal" and make sure we get the correct results
  ditr = itr.LookupWord(std::string("terminal"));
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  std::list<DocIDElementHeader> doc_id_list = ditr->GetDocIDList();
  ASSERT_EQ(terminal_results.size(), doc_id_list.size());
  for (auto iter = doc_id_list.begin(); iter != doc_id_list.end(); iter++) {
    ASSERT_EQ(1U, terminal_results.count(iter->doc_id));
    ASSERT_EQ(terminal_results[iter->doc_id], iter->num_positions);
  }
  // Clean up.
  delete ditr;

  // look up "sun" and make sure we get the correct results
  ditr = itr.LookupWord(std::string("sun"));
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  doc_id_list = ditr->GetDocIDList();
  ASSERT_EQ(sun_results.size(), doc_id_list.size());
  for (auto iter = doc_id_list.begin(); iter != doc_id_list.end(); iter++) {
    ASSERT_EQ(1U, sun_results.count(iter->doc_id));
    ASSERT_EQ(sun_results[iter->doc_id], iter->num_positions);
  }
  // Clean up.
  delete ditr;

  // look up "careful" and make sure we get the correct results
  ditr = itr.LookupWord(std::string("careful"));
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  doc_id_list = ditr->GetDocIDList();
  ASSERT_EQ(careful_results.size(), doc_id_list.size());
  for (auto iter = doc_id_list.begin(); iter != doc_id_list.end(); iter++) {
    ASSERT_EQ(1U, careful_results.count(iter->doc_id));
    ASSERT_EQ(careful_results[iter->doc_id], iter->num_positions);
  }
  // Clean up.
  delete ditr;

  // check for a word that does not exist
  ditr = itr.LookupWord(std::string("echoes"));
  ASSERT_EQ(static_cast<DocIDTableReader *>(nullptr), ditr);

  // Done!
  HW3Environment::AddPoints(20);
}

}  // namespace hw3
