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

#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <list>
#include <set>

#include "gtest/gtest.h"
extern "C" {
  #include "libhw1/HashTable.h"
  #include "libhw1/CSE333.h"
}
#include "./LayoutStructs.h"
#include "./IndexTableReader.h"

#include "./test_suite.h"
#include "./Utils.h"
#include "./WriteIndex.h"

namespace hw3 {

class Test_IndexTableReader : public ::testing::Test {
 protected:
  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    // Open up the (FILE *) for ./unit_test_indices/enron.idx
    FILE *f = fopen("./unit_test_indices/enron.idx", "rb");
    ASSERT_NE(static_cast<FILE *>(nullptr), f);

    // Read in the size of the doctable.
    ASSERT_EQ(0, fseek(f, DT_BYTES_OFFSET, SEEK_SET));
    int32_t doctable_size;
    ASSERT_EQ(1U, fread(&doctable_size, sizeof(int32_t), 1, f));
    doctable_size = ntohl(doctable_size);

    // Prep the IndexTableReader; the word-->docid_table table is at
    // offset sizeof(IndexFileHeader) + doctable_size.
    itr_ = new IndexTableReader(f, sizeof(IndexFileHeader) + doctable_size);
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    delete itr_;
  }

  // This method proxies our tests' calls to IndexTableReader,
  // allowing tests to access its protected members.
  std::list<IndexFileOffset_t> LookupElementPositions(HTKey_t hash_val) {
    return itr_->LookupElementPositions(hash_val);
  }

  // Objects declared here can be used by all tests in
  // the test case.
  IndexTableReader *itr_;
};  // class Test_IndexTableReader


TEST_F(Test_IndexTableReader, TestIndexTableReaderBasic) {
  HW3Environment::OpenTestCase();
  // Do a couple of bucket lookups to ensure we got data back.
  HTKey_t h1 = FNVHash64((unsigned char *) "anyway", 6);
  auto res = LookupElementPositions(h1);
  ASSERT_GT(res.size(), 0U);

  HTKey_t h2 = FNVHash64((unsigned char *) "attachment", 10);
  res = LookupElementPositions(h2);
  ASSERT_GT(res.size(), 0U);

  // The unit test test_docidtablereader.cc exercises the
  // IndexTableReader's LookupWord() method, so we won't replicate most
  // of it here.

  DocIDTableReader *ditr = itr_->LookupWord("anyway");
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  delete ditr;

  ditr = itr_->LookupWord("attachment");
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  delete ditr;

  ditr = itr_->LookupWord("slide");
  ASSERT_NE(static_cast<DocIDTableReader *>(nullptr), ditr);
  delete ditr;

  // Test that it does return nullptr for a word not found
  ditr = itr_->LookupWord("animals");
  ASSERT_EQ(static_cast<DocIDTableReader *>(nullptr), ditr);

  // Done!
  HW3Environment::AddPoints(20);
}

}  // namespace hw3
