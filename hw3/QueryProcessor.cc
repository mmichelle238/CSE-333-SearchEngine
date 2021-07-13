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

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

QueryProcessor::QueryProcessor(const list<string> &index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[array_len_];
  itr_array_ = new IndexTableReader *[array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int rank;       // The rank of the result so far.
  int index;      // the index of the corresponding IndexReader
} IdxQueryResult;

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;
  vector<IdxQueryResult> results;
  for (size_t start = 0; start < query.size(); start++) {
    string curr_word = query[start];
    vector<IdxQueryResult> good_list;
    for (int i = 0; i < array_len_; i++) {
      IndexTableReader *curr_index_table = itr_array_[i];
      DocIDTableReader *doc_id_table = curr_index_table->LookupWord(curr_word);
      if (doc_id_table != nullptr) {
        list<DocIDElementHeader> docid_header_list =
                                 doc_id_table->GetDocIDList();
        for (DocIDElementHeader docid_element : docid_header_list) {
          IdxQueryResult local;
          local.doc_id = docid_element.doc_id;
          local.rank = docid_element.num_positions;
          local.index = i;
          if (start == 0) {
            good_list.push_back(local);
          } else {
            for (IdxQueryResult iqr : results) {
              if (iqr.doc_id == local.doc_id && iqr.index == local.index) {
                local.rank += iqr.rank;
                good_list.push_back(local);
                break;
              }
            }
          }
        }
        delete doc_id_table;
      }
    }
    results = good_list;
  }

  for (IdxQueryResult result : results) {
    string ret_str;
    DocTableReader *doctable_reader = dtr_array_[result.index];
    if (doctable_reader->LookupDocID(result.doc_id, &ret_str)) {
        QueryProcessor::QueryResult curr_result;
        curr_result.document_name = ret_str;
        curr_result.rank = result.rank;
        final_result.push_back(curr_result);
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

}  // namespace hw3
