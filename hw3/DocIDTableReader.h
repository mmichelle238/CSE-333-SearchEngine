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

#ifndef HW3_DOCIDTABLEREADER_H_
#define HW3_DOCIDTABLEREADER_H_

#include <list>      // for std::list
#include <cstdio>    // for (FILE *)

#include "./HashTableReader.h"
#include "./LayoutStructs.h"

namespace hw3 {

// A DocIDTableReader (a subclass of HashTableReader) is used to
// read one of the many the embedded docid-->positions "docIDtable"
// tables within the index file.
class DocIDTableReader : protected HashTableReader {
 public:
  // Construct a new DocIDTableReader at a specific offset with an
  // index file.  Arguments:
  //
  // - f: an open (FILE *) for the underlying index file.  The
  //   constructed object takes ownership of the (FILE *) and will
  //   fclose() it  on destruction.
  //
  // - offset: the "docIDtable"'s byte offset within the file.
  DocIDTableReader(FILE *f, IndexFileOffset_t offset);
  ~DocIDTableReader() { }

  // Lookup a docid and get back a std::list<DocPositionOffset_t>
  // containing the positions listed for that docid.
  //
  // Arguments:
  //
  // - docid:  the docID to look for within the docIDtable.
  //
  // - ret_tr: the std::list<DocPositionOffset_t> containing the positions of
  //   the word in the document (an output parameter). Nothing is
  //   returned through this if the docID is not found.
  //
  // Returns:
  //
  // - true if the docID is found, false otherwise.
  bool LookupDocID(const DocID_t &doc_id,
                   std::list<DocPositionOffset_t> *ret_val) const;

  // Returns a list of docid_element structs where
  // the num_positions element is the number of matches
  // the word has within that docID for this docIDtable.
  std::list<DocIDElementHeader> GetDocIDList() const;

 private:
  // This friend declaration is here so that the Test_DocIDTableReader
  // unit test fixture can access protected member variables of
  // DocIDTableReader.  See test_docidtablereader.h for details.
  friend class Test_DocIDTableReader;

  DISALLOW_COPY_AND_ASSIGN(DocIDTableReader);
};
}  // namespace hw3

#endif  // HW3_DOCIDTABLEREADER_H_
