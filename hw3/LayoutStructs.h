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

#ifndef HW3_LAYOUTSTRUCTS_H_
#define HW3_LAYOUTSTRUCTS_H_

#include <stdint.h>
#include <cstddef>

extern "C" {
  #include "./libhw1/CSE333.h"
  #include "./libhw2/MemIndex.h"
}
#include "./Utils.h"

// This header file defines the in-memory structs representing hw3 on-disk
// data structures.  They are "dumb data carriers" that require no additional
// transformations; they can be written and read directly from disk.  For
// that reason, the field ordering and each field's types must correspond
// exactly to the file format described in the HW3 spec.


// C/C++ will add padding to structures to place the individual
// fields on their natural byte alignment.  That can lead to
// trouble: the file must be read using the same structures as
// were used to write it.  Because that's so fragile, we
// turn off padding on the structures defined here:
#pragma pack(push, 1)

namespace hw3 {

//---------------------------------------------
// General types for encoded data on disk
//---------------------------------------------

// An offset within an index file.
typedef int32_t IndexFileOffset_t;


//---------------------------------------------
// Index file header
//---------------------------------------------

struct IndexFileHeader {
  uint32_t   magic_number;    // the indicator of a well-formed header.
  uint32_t   checksum;       // the checksum for the entire file.
  int32_t    doctable_bytes;  // number of bytes in the contained doctable.
  int32_t    index_bytes;     // number of bytes in the contained index.

  IndexFileHeader() { }  // this constructor yields uninitialized fields!

  IndexFileHeader(uint32_t magic_number_arg,
                  uint32_t checksum_arg,
                  int32_t doctable_bytes_arg,
                  int32_t index_bytes_arg)
    : magic_number(magic_number_arg),
      checksum(checksum_arg),
      doctable_bytes(doctable_bytes_arg),
      index_bytes(index_bytes_arg) {
  }

  void ToDiskFormat() { magic_number = htonl(magic_number);
                        checksum = htonl(checksum);
                        doctable_bytes = htonl(doctable_bytes);
                        index_bytes = htonl(index_bytes);
                      }
  void ToHostFormat() { magic_number = ntohl(magic_number);
                        checksum = ntohl(checksum);
                        doctable_bytes = ntohl(doctable_bytes);
                        index_bytes = ntohl(index_bytes);
                      }
};

// Offset of magic number within an IndexFileHeader struct, and also within
// the file itself.
#define MAGIC_NUMBER_OFFSET offsetof(IndexFileHeader, magic_number)

// Offset of the doctable size field within an IndexFileHeader struct
#define DT_BYTES_OFFSET offsetof(IndexFileHeader, doctable_bytes)


//---------------------------------------------
// Bucket lists
//  (Used by doctable, index, and docID table)
//---------------------------------------------

struct BucketListHeader {
  int32_t   num_buckets;  // number of buckets in the hash table

  BucketListHeader() { }  // this constructor doesn't initialize numBuckets!

  explicit BucketListHeader(int32_t num_buckets_arg)
    : num_buckets(num_buckets_arg) {
  }

  void ToDiskFormat() { num_buckets = htonl(num_buckets); }
  void ToHostFormat() { num_buckets = ntohl(num_buckets); }
};

struct BucketRecord {
  int32_t            chain_num_elements;  // number of elements in this bucket's
                                        // chain.
  IndexFileOffset_t  position;          // byte offset from the start of the
                                        // index file, indicating where the
                                        //  bucket's chain begins.

  BucketRecord() { }  // this constructor doesn't initialize any fields!

  BucketRecord(int32_t num_elts, IndexFileOffset_t pos)
    : chain_num_elements(num_elts), position(pos) {
  }

  void ToDiskFormat() {
    chain_num_elements = htonl(chain_num_elements);
    position = htonl(position);
  }
  void ToHostFormat() {
    chain_num_elements = ntohl(chain_num_elements);
    position = ntohl(position);
  }
};

struct ElementPositionRecord {
  IndexFileOffset_t  position;  // byte offset from the start of the index file,
                                // indicating where the element begins.

  ElementPositionRecord() { }
  explicit ElementPositionRecord(IndexFileOffset_t pos)
    : position(pos) {
  }

  void ToDiskFormat() { position  = htonl(position); }
  void ToHostFormat() { position = ntohl(position); }
};

//---------------------------------------------
// DocTable
//---------------------------------------------

struct DoctableElementHeader {
  DocID_t   doc_id;
  int16_t  file_name_bytes;  // number of bytes needed to
                             // serialize the filename.
                             // note: 16-bit quantity!

  DoctableElementHeader() { }
  DoctableElementHeader(DocID_t id, int32_t num_bytes)
    : doc_id(id), file_name_bytes(num_bytes) {
  }

  void ToDiskFormat() {
    doc_id = htonll(doc_id);
    file_name_bytes = htons(file_name_bytes);
  }
  void ToHostFormat() {
    doc_id = ntohll(doc_id);
    file_name_bytes = ntohs(file_name_bytes);
  }
};

//---------------------------------------------
// IndexTable
//
// Each element in the index is a WordPostings
// (see also HW2).
//---------------------------------------------

struct WordPostingsHeader {
  int16_t   word_bytes;      // number of bytes needed to serialize the word.
  int32_t   postings_bytes;  // number of bytes for the doctable.

  WordPostingsHeader() { }
  WordPostingsHeader(int16_t word_bytes_arg, int32_t postings_bytes_arg)
    : word_bytes(word_bytes_arg), postings_bytes(postings_bytes_arg) {
  }

  void ToDiskFormat() {
    word_bytes = htons(word_bytes);
    postings_bytes = htonl(postings_bytes);
  }
  void ToHostFormat() {
    word_bytes = ntohs(word_bytes);
    postings_bytes = ntohl(postings_bytes); }
};

//---------------------------------------------
// DocIDTable
//---------------------------------------------

struct DocIDElementHeader {
  DocID_t   doc_id;
  int32_t   num_positions;  // number of word positions stored in this element

  DocIDElementHeader() { }
  DocIDElementHeader(DocID_t id, int32_t num_pos)
    : doc_id(id), num_positions(num_pos) {
  }

  void ToDiskFormat() {
    doc_id = htonll(doc_id);
    num_positions = htonl(num_positions);
  }
  void ToHostFormat() {
    doc_id = ntohll(doc_id);
    num_positions = ntohl(num_positions);
  }
};

struct DocIDElementPosition {
  DocPositionOffset_t position;  // byte offset into the document referenced by
                                 // docID, indicating where the word occurs

  void ToDiskFormat() { position = htonl(position); }
  void ToHostFormat() { position = ntohl(position); }
};


}  // namespace hw3

#pragma pack(pop)

#endif  // HW3_LAYOUTSTRUCTS_H_
