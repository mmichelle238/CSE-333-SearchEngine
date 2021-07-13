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

#ifndef HW3_WRITEINDEX_H_
#define HW3_WRITEINDEX_H_

#include <stdint.h>  // [C++ doesn't yet standardize <cstdint>.]

// Include the HW1 and HW2 headers.  We'll need these to access the
// routines in the libhw1.a and libhw2.a libraries.  Note that those
// libraries were compiled with gcc, and therefore have "C"-style
// linkage.  Since WriteIndex.cc is compiled by g++, we need
// do use 'extern "C"' to tell g++ that the routines accessed through
// these header files were compiled with gcc.
extern "C" {
  #include "libhw1/CSE333.h"
  #include "libhw2/DocTable.h"
  #include "libhw2/MemIndex.h"
}

namespace hw3 {

// Writes the contents of a MemIndex and the docid_to_docname mapping of a
// DocTable into an index file.  The on-disk representation is defined in
// detail on the hw3 web page.
//
// Arguments:
//   - mi: the MemIndex to write.
//   - dt: the DocTable to write.
//   - filename: a C-style string containing the name of the index
//     file that we should create.
//
// Returns:
//   - the resulting size of the index file, in bytes, or negative value
//     on error
int WriteIndex(MemIndex *mi, DocTable *dt, const char *file_name);

}  // namespace hw3

#endif  // HW3_WRITEINDEX_H_
