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

#include <iostream>

#include "gtest/gtest.h"
#include "./test_suite.h"

using std::cout;
using std::endl;

// static
int HW1Environment::total_points_ = 0;
int HW1Environment::curr_test_points_ = 0;

// static
void HW1Environment::AddPoints(int points) {
  total_points_ += points;
  curr_test_points_ += points;
  ::testing::Test::RecordProperty("points", curr_test_points_);
  cout << " (" << total_points_ << "/" << HW1_MAXPOINTS << ")"<< endl;
}

void HW1Environment::OpenTestCase() {
  // Should be called at the beginning of each test case
  curr_test_points_ = 0;
  ::testing::Test::RecordProperty("points", curr_test_points_);
}

void HW1Environment::SetUp() {
  cout << "HW1: there are " << HW1_MAXPOINTS;
  cout << " points available." << endl;
}

void HW1Environment::TearDown() {
  // Code here is run once for the entire test environment.
  cout << endl;
  cout << "You earned " << total_points_ << " out of ";
  cout << HW1_MAXPOINTS << " points available (";
  cout << ((100.0 * total_points_) / HW1_MAXPOINTS) << "%)" << endl;
  cout << endl;
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new HW1Environment);
  return RUN_ALL_TESTS();
}
