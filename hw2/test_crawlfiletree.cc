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

extern "C" {
  #include "./CrawlFileTree.h"
}

#include "gtest/gtest.h"
extern "C" {
  #include "./DocTable.h"
  #include "./MemIndex.h"
}

#include "./test_suite.h"

static char *AppendString(const char* prefix, const char* suffix) {
  int pre_len = strlen(prefix);
  int suf_len = strlen(suffix);
  char *copy = static_cast<char*>(malloc(pre_len + suf_len + 1));
  strncpy(copy, prefix, pre_len);
  strncpy(&copy[pre_len], suffix, suf_len);
  copy[pre_len + suf_len] = '\0';
  return copy;
}

namespace hw2 {

TEST(Test_CrawlFileTree, ReadsFromDisk) {
  HW2Environment::OpenTestCase();
  bool res;
  DocTable *dt;
  MemIndex *idx;

  // Test that it detects a valid directory.
  res = CrawlFileTree(const_cast<char *>("./test_tree/bash-4.2/support"),
                      &dt,
                      &idx);
  ASSERT_EQ(true, res);
  HW2Environment::AddPoints(10);
  DocTable_Free(dt);
  MemIndex_Free(idx);
  HW2Environment::AddPoints(10);

  // Test that it detects a non-existant directory.
  res = CrawlFileTree(const_cast<char *>("./nonexistent/"), &dt, &idx);
  ASSERT_EQ(false, res);
  HW2Environment::AddPoints(10);

  // Test that it rejects files (instead of directories).
  res = CrawlFileTree(const_cast<char *>("./test_suite.c"), &dt, &idx);
  ASSERT_EQ(false, res);
  HW2Environment::AddPoints(10);
}

TEST(Test_CrawlFileTree, ChecksArgs) {
  DocTable *dt;
  MemIndex *idx;

  // Test that CrawlFileTree can be called with any combination of NULLs
  ASSERT_EQ(false, CrawlFileTree(NULL, &dt, &idx));
  ASSERT_EQ(false, CrawlFileTree(const_cast<char *>(""), &dt, &idx));
  ASSERT_EQ(false,
            CrawlFileTree(const_cast<char *>("./test_tree"), NULL, &idx));
  ASSERT_EQ(false,
            CrawlFileTree(const_cast<char *>("./test_tree"), &dt, NULL));
}

TEST(Test_CrawlFileTree, CheckDocTable) {
  HW2Environment::OpenTestCase();
  int res;
  DocTable *dt;
  MemIndex *idx;

  int i;
  DocID_t id;
  char *path;
  const char *docname;

  const char *directory = "./test_tree/bash-4.2/doc/";

  // Note that this does not include some files since those files don't contain
  // ascii characters and should be skipped when handled.
  const char *files[] = {"FAQ", "INTRO", "Makefile.in", "README", "article.ms",
                          "article.ps", "article.txt", "bash.0", "bash.1",
                          "bash.html", "bash.ps", "bashbug.0", "bashbug.1",
                          "bashbug.ps", "bashref.html", "bashref.info",
                          "bashref.ps", "bashref.texi", "builtins.0",
                          "builtins.1", "builtins.ps", "fdl.texi", "fdl.txt",
                          "htmlpost.sh", "infopost.sh", "rbash.0", "rbash.1",
                          "rbash.ps", "rose94.ps", "texinfo.tex",
                          "version.texi"};

  // Crawl the test tree.
  res = CrawlFileTree(const_cast<char *>(directory),
                      &dt,
                      &idx);
  HW2Environment::AddPoints(10);

  ASSERT_EQ(1, res);
  ASSERT_EQ(31, DocTable_NumDocs(dt));
  HW2Environment::AddPoints(10);

  for (i = 0; i < 31; i++) {
    path = AppendString(directory, files[i]);
    id = DocTable_GetDocID(dt, path);
    ASSERT_NE(INVALID_DOCID, id);
    docname = DocTable_GetDocName(dt, id);
    ASSERT_NE((const char*) NULL, docname);
    ASSERT_EQ(0, strcmp(docname, path));
    free(path);
  }

  DocTable_Free(dt);
  MemIndex_Free(idx);
  HW2Environment::AddPoints(10);
}

}  // namespace hw2

