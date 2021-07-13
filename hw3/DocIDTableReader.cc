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

#include "./DocIDTableReader.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

using std::list;

namespace hw3 {

// The constructor for DocIDTableReader calls the constructor
// of HashTableReader(), its superclass. The superclass takes
// care of taking ownership of f and using it to extract and
// cache the number of buckets within the table.
DocIDTableReader::DocIDTableReader(FILE *f, IndexFileOffset_t offset)
  : HashTableReader(f, offset) { }

bool DocIDTableReader::LookupDocID(const DocID_t &doc_id,
                                   list<DocPositionOffset_t> *ret_val) const {
  // Use the superclass's "LookupElementPositions" function to
  // walk through the docIDtable and get back a list of offsets
  // to elements in the bucket for this docID.
  auto elements = LookupElementPositions(doc_id);

  // If the list of elements is empty, we're done.
  if (elements.empty())
    return false;

  // Iterate through the elements, looking for our docID.
  for (auto it = elements.begin(); it != elements.end(); it++) {
    IndexFileOffset_t curr = *it;

    // STEP 1.
    // Slurp the next docid out of the element.
    DocIDElementHeader curr_header;
    Verify333(fseek(file_, curr, SEEK_SET) == 0);
    Verify333(fread(&curr_header, sizeof(DocIDElementHeader), 1, file_) == 1);
    curr_header.ToHostFormat();

    list<DocPositionOffset_t> offsets;

    // Is it a match?
    if (curr_header.doc_id == doc_id) {
      // STEP 2.
      // Yes!  Extract the positions themselves, appending to
      // std::list<DocPositionOffset_t>.  Be sure to push in the right
      // order, adding to the end of the list as you extract
      // successive positions.
      for (int i = 0; i < curr_header.num_positions; i++) {
        DocIDElementPosition pos;
        Verify333(fread(&pos, sizeof(DocIDElementPosition), 1, file_) == 1);
        pos.ToHostFormat();
        offsets.push_back(pos.position);
      }

      // STEP 3.
      // Return the positions list through the output parameter,
      // and return true.
      *ret_val = offsets;
      return true;
    }
  }

  // We failed to find a matching docID, so return false.
  return false;
}

list<DocIDElementHeader> DocIDTableReader::GetDocIDList() const {
  // This will be our returned list of docIDs within this table.
  list<DocIDElementHeader> doc_id_list;

  // Go through *all* of the buckets of this hashtable, extracting
  // out the docids in each element and the number of word positions
  // for the each docid.
  for (int i = 0; i < header_.num_buckets; i++) {
    // STEP 4.
    // Seek to the next BucketRecord.  The "offset_" member
    // variable stores the offset of this docid table within
    // the index file.
    Verify333(fseek(file_, offset_ + sizeof(BucketListHeader) +
          i * sizeof(BucketRecord), SEEK_SET) == 0);

    // STEP 5.
    // Read in the chain length and bucket position fields from
    // the bucket_rec.
    BucketRecord bucket_rec;
    Verify333(fread(&bucket_rec, sizeof(BucketRecord), 1, file_) == 1);
    bucket_rec.ToHostFormat();

    // Sweep through the next bucket, iterating through each
    // chain element in the bucket.
    for (int j = 0; j < bucket_rec.chain_num_elements; j++) {
      // Seek to chain element's position field in the bucket header.
      Verify333(fseek(file_,   bucket_rec.position
                             + j*sizeof(ElementPositionRecord), SEEK_SET) == 0);

      // STEP 6.
      // Read the next element position from the bucket header.
      // and seek to the element itself.
      DocIDElementPosition element_pos;
      Verify333(fread(&element_pos,  sizeof(DocIDElementPosition),
                                     1, file_) == 1);
      element_pos.ToHostFormat();
      Verify333(fseek(file_, element_pos.position, SEEK_SET) == 0);


      // STEP 7.
      // Read in the docid and number of positions from the element.
      DocIDElementHeader element;
      Verify333(fread(&element, sizeof(DocIDElementHeader), 1, file_) == 1);
      element.ToHostFormat();

      // Append it to our result list.
      doc_id_list.push_back(element);
    }
  }

  // Done!  Return the result list.
  return doc_id_list;
}

}  // namespace hw3
