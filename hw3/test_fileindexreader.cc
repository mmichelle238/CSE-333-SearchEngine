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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "./FileIndexReader.h"
#include "./LayoutStructs.h"
#include "./test_suite.h"
#include "./Utils.h"

namespace hw3 {

class Test_FileIndexReader : public ::testing::Test {
 protected:
  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    // Open up the FILE * for ./unit_test_indices/enron.idx
    fir_ = new FileIndexReader(kIndexName);
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    delete fir_;
  }

  // These methods proxy our tests' calls to FileIndexReader,
  // allowing tests to access its protected members.
  int32_t get_doctable_size() {
    return fir_->header_.doctable_bytes;
  }
  int32_t get_index_size() {
    return fir_->header_.index_bytes;
  }

  // Objects declared here can be used by all tests in
  // the test case.
  const char* kIndexName = "./unit_test_indices/enron.idx";

  FileIndexReader *fir_;
};  // class Test_FileIndexReader


TEST_F(Test_FileIndexReader, TestFileIndexReaderBasic) {
  HW3Environment::OpenTestCase();

  // Make sure the header fields line up correctly with the file size.
  struct stat f_stat;
  ASSERT_EQ(stat(kIndexName, &f_stat), 0);
  ASSERT_EQ(f_stat.st_size, (unsigned int)(get_doctable_size() +
                                           get_index_size() +
                                           sizeof(IndexFileHeader)));
  FILE *f = fopen(kIndexName, "rb");
  ASSERT_NE(static_cast<FILE *>(nullptr), f);

  ASSERT_EQ(0, fseek(f, DT_BYTES_OFFSET, SEEK_SET));
  int32_t doctable_size;
  ASSERT_EQ(1U, fread(&doctable_size, sizeof(int32_t), 1, f));
  doctable_size = ntohl(doctable_size);

  ASSERT_EQ(doctable_size, get_doctable_size());

  fclose(f);

  // Done!
  HW3Environment::AddPoints(10);
}

}  // namespace hw3
