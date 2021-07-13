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

#include "gtest/gtest.h"
extern "C" {
#include "libhw1/HashTable.h"
}
#include "./DocTableReader.h"
#include "./LayoutStructs.h"
#include "./test_suite.h"
#include "./Utils.h"
#include "./WriteIndex.h"

using std::string;

namespace hw3 {

class Test_DocTableReader : public ::testing::Test {
 protected:
  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    // Open up the FILE * for ./unit_test_indices/enron.idx
    FILE* f = fopen("./unit_test_indices/enron.idx", "rb");
    ASSERT_NE(static_cast<FILE *>(nullptr), f);

    // Prep the DocTableReader; the docid-->docname table is at
    // offset sizeof(IndexFileHeader).
    dtr_ = new DocTableReader(f, sizeof(IndexFileHeader));
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    delete dtr_;
  }

  // This method proxies our tests' calls to DocTableReader,
  // allowing tests to access its protected members.
  std::list<IndexFileOffset_t> LookupElementPositions(DocID_t hash_val) {
    return dtr_->LookupElementPositions(hash_val);
  }

  // Objects declared here can be used by all tests in
  // the test case.
  DocTableReader * dtr_;
};  // class Test_DocTableReader


TEST_F(Test_DocTableReader, TestDocTableReaderBasic) {
  HW3Environment::OpenTestCase();

  // Do a couple of bucket lookups, just to make sure we're
  // inheriting LookupElementPositions correctly.
  auto res = LookupElementPositions(5);
  ASSERT_GT(res.size(), 0U);

  res = LookupElementPositions(6);
  ASSERT_GT(res.size(), 0U);

  // Try some docid-->string lookups.  Start by trying two that
  // should exist.
  string str;
  bool success = dtr_->LookupDocID(5, &str);
  ASSERT_TRUE(success);
  ASSERT_EQ(std::string("test_tree/enron_email/102."),
            str);
  success = dtr_->LookupDocID(55, &str);
  ASSERT_TRUE(success);
  ASSERT_EQ(std::string("test_tree/enron_email/149."),
            str);

  // Lookup a docid that shouldn't exist.
  success = dtr_->LookupDocID(100000, &str);
  ASSERT_FALSE(success);

  // Done!
  HW3Environment::AddPoints(20);
}

}  // namespace hw3
