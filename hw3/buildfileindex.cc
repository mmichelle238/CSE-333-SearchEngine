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

#include <stdlib.h>
#include <unistd.h>
#include <iostream>

extern "C" {
  #include "./libhw2/CrawlFileTree.h"
  #include "./libhw2/DocTable.h"
  #include "./libhw2/MemIndex.h"
}
#include "./WriteIndex.h"

using std::cerr;
using std::cout;
using std::endl;

void Usage(char *filename) {
  cerr << "Usage: " << filename;
  cerr << " crawlrootdir indexfilename" << endl;
  cerr << "where:" << endl;
  cerr << "  crawlrootdir is the name of a directory to crawl";
  cerr << endl;
  cerr << "  indexfilename is the name of the index file to create";
  cerr << endl;
  exit(EXIT_FAILURE);
}


// Crawls the filesystem starting at the subtree specified by argv[1], builds
// an in-memory inverted index (see HW2 CrawlFileTree()), and then writes it
// out using WriteIndex().
int main(int argc, char **argv) {
  DocTable *dt;
  MemIndex *idx;

  // Make sure the user provided us the right command-line options.
  if (argc != 3)
    Usage(argv[0]);

  // Try to crawl.
  cout << "Crawling " << argv[1] << "..." << endl;
  if (CrawlFileTree(argv[1], &dt, &idx) != 1)
    Usage(argv[0]);

  // Try to write out the index file.
  cout << "Writing index to " << argv[2];
  cout << "..." << endl;
  int idxlen = hw3::WriteIndex(idx, dt, argv[2]);
  if (idxlen <= 0) {
    DocTable_Free(dt);
    MemIndex_Free(idx);
    return EXIT_FAILURE;
  }

  // All done!  Clean up.
  cout << "Done. Cleaning up memory." << endl;
  DocTable_Free(dt);
  MemIndex_Free(idx);
  return EXIT_SUCCESS;
}
