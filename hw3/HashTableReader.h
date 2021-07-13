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

#ifndef HW3_HASHTABLEREADER_H_
#define HW3_HASHTABLEREADER_H_

#include <stdint.h>  // for uint32_t, etc.
#include <cstdio>    // for (FILE *).
#include <list>      // for std::list.

#include "./LayoutStructs.h"

namespace hw3 {

// A HashTableReader is the base class for the different kinds
// of hash table readers.  Its subclasses are the DocTableReader, the
// IndexTableReader, and the DocPositionsReader.
class HashTableReader {
 public:
  // Construct a HashTableReader reader.
  //
  // Arguments:
  // - f: an open (FILE *) for the index file to read.  Takes ownership of
  //   the passed-in file's memory, and will also fclose() it on destruction.
  // - offset: the hash table's byte offset within the file.
  HashTableReader(FILE *f, IndexFileOffset_t offset);
  virtual ~HashTableReader();

 protected:
  // Given a 64-bit hash key, this function navigates through
  // the on-disk hash table and returns a list of file offsets of
  // "element" fields within the bucket that the hash key maps to.
  // Only subclasses may invoke this.
  //
  // Arguments:
  //
  // - hashval: the 64-bit hash key to look up.
  //
  // Returns:
  //
  // - A list of offsets of "element" fields inside the bucket that
  //   the hash value maps to.  If no elements are in the  bucket,
  //   this returns an empty list.
  std::list<IndexFileOffset_t> LookupElementPositions(HTKey_t hash_val) const;

  // The open (FILE *) stream associated with this hash table.
  FILE *file_;

  // The byte offset within the file that this hash table starts at.
  IndexFileOffset_t offset_;

  // A cached copy of the total number of buckets in this hash table.
  BucketListHeader header_;

 private:
  // This friend declaration is here so that the Test_HashTableReader
  // unit test fixture can access protected member variables of
  // HashTableReader.  See test_hashtablereader.h for details.
  friend class Test_HashTableReader;

  DISALLOW_COPY_AND_ASSIGN(HashTableReader);
};

}  // namespace hw3

#endif  // HW3_HASHTABLEREADER_H_
