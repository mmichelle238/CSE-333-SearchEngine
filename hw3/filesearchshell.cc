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

#include <cstdlib>    // for EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>   // for std::cout, std::cerr, etc.
#include <sstream>    // for stringstream
#include <algorithm>  // to transform tolower

#include "./QueryProcessor.h"

using std::cerr;
using std::endl;
using std::list;
using std::string;
using std::cout;
using std::cin;
using hw3::QueryProcessor;
using std::stringstream;
using std::ios_base;
using std::tolower;
using std::transform;


/* Prints a message to cerr in the case where the program is started
 * incorrectly.
 */
static void Usage(char *prog_name);

/* grabs the file names of indices passed through as arguments
from the console and returns the list of file names by using
the number of arguments and an array of arguments passed through
into main */
static list<string> ListOfFiles(int argc, char **argv);

/* asks the user to enter their query in the console, and returns
the typed in query through the output parameter. If "control-D" is pressed,
return false. If there is a valid query, returns true.
If an error while reading occurs, exits with EXIT_FAILURE */
static bool GetQuery(string *ret_str);

/* with the passed in string containing the entire query, break it up
into words (broken up at the spaces) and insert each individual word
into a vector which is returned */
static vector<string> ProcessQuery(string entire_query);

/* given a vector of the query results, prints out all of the results
to the console with the file name and the number of times the query
occurs in each file. if there are no results, prints out that there
are no results to the console */
static void PrintQueryResults(vector<QueryProcessor::QueryResult> res);

// Your job is to implement the entire filesearchshell.cc
// functionality. We're essentially giving you a blank screen to work
// with; you need to figure out an appropriate design, to decompose
// the problem into multiple functions or classes if that will help,
// to pick good interfaces to those functions/classes, and to make
// sure that you don't leak any memory.
//
// Here are the requirements for a working solution:
//
// The user must be able to run the program using a command like:
//
//   ./filesearchshell ./foo.idx ./bar/baz.idx /tmp/blah.idx [etc]
//
// i.e., to pass a set of filenames of indices as command line
// arguments. Then, your program needs to implement a loop where
// each loop iteration it:
//
//  (a) prints to the console a prompt telling the user to input the
//      next query.
//
//  (b) reads a white-space separated list of query words from
//      std::cin, converts them to lowercase, and constructs
//      a vector of c++ strings out of them.
//
//  (c) uses QueryProcessor.cc/.h's QueryProcessor class to
//      process the query against the indices and get back a set of
//      query results.  Note that you should instantiate a single
//      QueryProcessor  object for the lifetime of the program, rather
//      than  instantiating a new one for every query.
//
//  (d) print the query results to std::cout in the format shown in
//      the transcript on the hw3 web page.
//
// Also, you're required to quit out of the loop when std::cin
// experiences EOF, which a user passes by pressing "control-D"
// on the console.  As well, users should be able to type in an
// arbitrarily long query -- you shouldn't assume anything about
// a maximum line length.  Finally, when you break out of the
// loop and quit the program, you need to make sure you deallocate
// all dynamically allocated memory.  We will be running valgrind
// on your filesearchshell implementation to verify there are no
// leaks or errors.
//
// You might find the following technique useful, but you aren't
// required to use it if you have a different way of getting the
// job done.  To split a std::string into a vector of words, you
// can use a std::stringstream to get the job done and the ">>"
// operator. See, for example, "gnomed"'s post on stackoverflow for
// his example on how to do this:
//
//   http://stackoverflow.com/questions/236129/c-how-to-split-a-string
//
// (Search for "gnomed" on that page.  He uses an istringstream, but
// a stringstream gets the job done too.)
//
// Good luck, and write beautiful code!

int main(int argc, char **argv) {
  if (argc < 2) {
    Usage(argv[0]);
  }

  // STEP 1:
  // Implement filesearchshell!
  // Probably want to write some helper methods ...

  list<string> idx_list = ListOfFiles(argc, argv);
  // Construct the QueryProcessor.
  QueryProcessor qp(idx_list);
  // keep going until you exit it out of get query
  while (true) {
    string entire_query;
    bool status = GetQuery(&entire_query);
    if (!status) {  // we've reached EOF
      break;
    }

    // if an empty query was passed through, continue
    // so that the user is reprompted
    if (entire_query.length() == 0) {
      continue;
    }
    vector<string> query = ProcessQuery(entire_query);
    vector<QueryProcessor::QueryResult> res = qp.ProcessQuery(query);
    PrintQueryResults(res);
  }

  return EXIT_SUCCESS;
}

static list<string> ListOfFiles(int argc, char **argv) {
  list<string> idx_list;
  for (int i = 1; i < argc; i++) {
    idx_list.push_back(argv[i]);
  }
  return idx_list;
}

static bool GetQuery(string *ret_str) {
  cout << "Enter query:" << endl;
  string entire_query;
  getline(cin, entire_query);
  if (cin.bad()) {
    cerr << "Something went wrong!" << endl;
    exit(EXIT_FAILURE);
  } else if (cin.eof()) {
    return false;
  }
  *ret_str = entire_query;
  return true;
}

static vector<string> ProcessQuery(string entire_query) {
  string word;
  stringstream ss(entire_query, ios_base::in);
  vector<string> query;
  while (ss >> word) {
    for (size_t i = 0; i < word.length(); i++) {
      word[i] = tolower(word[i]);
    }
    query.push_back(word);
  }
  return query;
}

static void PrintQueryResults(vector<QueryProcessor::QueryResult> res) {
  if (res.size() == 0) {
    cout << "  [no results]" << endl;
  } else {
    for (size_t i = 0; i < res.size(); i++) {
      cout << "  " << res[i].document_name;
      cout << " (" << res[i].rank << ")" << endl;
    }
  }
}

static void Usage(char *prog_name) {
  cerr << "Usage: " << prog_name << " [index files+]" << endl;
  exit(EXIT_FAILURE);
}
