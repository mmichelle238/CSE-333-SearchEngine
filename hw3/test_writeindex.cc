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
#include <stdint.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
extern "C" {
#include "libhw2/CrawlFileTree.h"
#include "libhw2/DocTable.h"
#include "libhw2/MemIndex.h"
}
#include "./test_suite.h"
#include "./WriteIndex.h"
#include "./hw3fsck/FileIndexChecker.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

namespace hw3 {

class Test_WriteIndex : public ::testing::Test {
 protected:
  // Code here will be called once for the entire test fixture
  // (ie, before all TEST_Fs).  Note it is a static member function
  // (ie, a class method, not an object instance method).
  static void SetUpTestCase() {
    cout << "             Crawling ./test_tree/enron_mail..."
         << std::endl;
    int res = CrawlFileTree(const_cast<char *>("./test_tree/enron_email"),
                            &Test_WriteIndex::dt_,
                            &Test_WriteIndex::mi_);
    std::cout << "               ...done crawling." << std::endl;
    ASSERT_NE(0, res);
  }

  // Code here will be called once for the entire test fixture
  // (ie, after all TEST_Fs).  Note it is a static member function
  // (ie, a class method, not an object instance method).
  static void TearDownTestCase() {
    DocTable_Free(dt_);
    MemIndex_Free(mi_);
  }

  // We'll reuse the MemIndex and DocTable across tests so that we don't have
  // to recrawl/reparse the test tree in each TEST_F.
  static DocTable *dt_;
  static MemIndex *mi_;
};

// Statics:
DocTable *Test_WriteIndex::dt_;
MemIndex *Test_WriteIndex::mi_;


// Test our ability to write to a file.  Does not verify file contents.
TEST_F(Test_WriteIndex, Basic) {
  int points = 0;
  RecordProperty("points", points);

  stringstream ss;
  ss << "/tmp/test." << (uint32_t) getpid() << ".index";
  string f_name = ss.str();

  cout << "             " <<
    "Writing index " << mi_ << " out to " << f_name << "..." << endl;
  int res = WriteIndex(mi_, dt_, f_name.c_str());
  cout << "             " << "...done writing." << endl;
  ASSERT_LT(100000, res);

  // Use hw3fsck to check the file.
  FileIndexChecker fileIndexChecker(f_name);

  ASSERT_TRUE(fileIndexChecker.CheckHeader());
  points += 20;
  RecordProperty("points", points);
  HW3Environment::AddPoints(20);

  ASSERT_TRUE(fileIndexChecker.CheckDocTable());
  points += 35;
  RecordProperty("points", points);
  HW3Environment::AddPoints(35);

  ASSERT_TRUE(fileIndexChecker.CheckIndexTable());
  points += 45;
  RecordProperty("points", points);
  HW3Environment::AddPoints(45);

  ASSERT_EQ(0, unlink(f_name.c_str()));
}

}  // namespace hw3
