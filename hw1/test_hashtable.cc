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
  #include "./HashTable.h"
  #include "./HashTable_priv.h"
  #include "./LinkedList.h"
  #include "./LinkedList_priv.h"
}

#include "gtest/gtest.h"

#include "./test_suite.h"

namespace hw1 {

// Our payload structure
typedef struct payload_st {
  int magic_num;
  int payload_num;
} Payload;

class Test_HashTable : public ::testing::Test {
 protected:
  static const int kMagicNum = 0xDEADBEEF;

  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    freeInvocations_ = 0;
  }

  // A version of free() that verifies the payload before freeing it.
  static void VerifiedFree(HTValue_t payload) {
    ASSERT_EQ(kMagicNum, (static_cast<Payload *>(payload))->magic_num);
    free(payload);
  }

  // A version of VerifiedFree() which counts how many times it's been
  // invoked; this allows us to make assertions.  Note that the counter
  //  is reset in SetUp().
  static int freeInvocations_;
  static void InstrumentedFree(HTValue_t payload) {
    freeInvocations_++;
    VerifiedFree(payload);
  }
};  // class Test_HashTable

// statics:
int Test_HashTable::freeInvocations_;
const int Test_HashTable::kMagicNum;


TEST_F(Test_HashTable, AllocFree) {
  HashTable *ht = HashTable_Allocate(3);
  ASSERT_EQ(0, ht->num_elements);
  ASSERT_EQ(3, ht->num_buckets);

  ASSERT_TRUE(ht->buckets != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(ht->buckets[0]));
  ASSERT_EQ(0, LinkedList_NumElements(ht->buckets[1]));
  ASSERT_EQ(0, LinkedList_NumElements(ht->buckets[2]));
  HashTable_Free(ht, &Test_HashTable::VerifiedFree);
}

TEST_F(Test_HashTable, InsertFindRemove) {
  HW1Environment::OpenTestCase();
  HashTable *table = HashTable_Allocate(10);
  int i;

  // Allocate and insert a bunch of elements.
  for (i = 0; i < 25; i++) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);

    // Create an element and do the insert.  Note that we promptly overwrite
    // these elements in the next section, so we don't bother
    // allocating/freeing memory for these items.
    Payload *np;
    HTKeyValue_t oldkv, newkv;
    newkv.key = hashed_key;
    newkv.value = static_cast<HTValue_t>(&newkv);
    ASSERT_FALSE(HashTable_Insert(table, newkv, &oldkv));

    // Test the double-insert case, using a different dynamically-allocated
    // value.  We compare the returned "old" element with the just-inserted
    // stack-allocated element, above.
    np = static_cast<Payload *>(malloc(sizeof(Payload)));
    ASSERT_TRUE(np != NULL);
    np->magic_num = kMagicNum;
    np->payload_num = i;
    newkv.value = static_cast<HTValue_t>(np);
    ASSERT_TRUE(HashTable_Insert(table, newkv, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    ASSERT_EQ(static_cast<HTValue_t>(&newkv), oldkv.value);

    // Lookup the newly-inserted value.
    oldkv.key = -1;       // reinitialize "oldkv" so we can verify it was
    oldkv.value = NULL;   // set by Find.
    ASSERT_TRUE(HashTable_Find(table, hashed_key, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    ASSERT_EQ(static_cast<HTValue_t>(np), oldkv.value);

    // Lookup and remove a value that doesn't exist in the table.
    ASSERT_FALSE(HashTable_Find(table, hashed_key+1, &oldkv));
    ASSERT_FALSE(HashTable_Remove(table, hashed_key+1, &oldkv));

    // Remove the item we just inserted.
    oldkv.key = -1;
    oldkv.value = NULL;
    ASSERT_TRUE(HashTable_Remove(table, hashed_key, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    ASSERT_EQ(static_cast<HTValue_t>(np), oldkv.value);
    ASSERT_EQ(i, HashTable_NumElements(table));

    // Insert it again.
    ASSERT_FALSE(HashTable_Insert(table, newkv, &oldkv));
    ASSERT_TRUE(HashTable_Insert(table, newkv, &oldkv));
    ASSERT_EQ(i+1, HashTable_NumElements(table));
  }
  HW1Environment::AddPoints(20);

  // Delete every other key.
  for (i = 0; i < 25; i += 2) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);
    HTKeyValue_t oldkv;

    ASSERT_TRUE(HashTable_Remove(table, i, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    VerifiedFree(oldkv.value);

    // This second attempt fails, since the element was already removed.
    ASSERT_FALSE(HashTable_Remove(table, i, &oldkv));
  }
  ASSERT_EQ(table->num_elements, 12);

  // Delete the remaining keys.
  for (i = 1; i < 25; i += 2) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);
    HTKeyValue_t oldkv;

    ASSERT_TRUE(HashTable_Remove(table, i, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    VerifiedFree(oldkv.value);

    // As before, this second attempt should fail.
    ASSERT_FALSE(HashTable_Remove(table, i, &oldkv));
  }
  ASSERT_EQ(table->num_elements, 0);

  // One more pass, inserting elements.
  for (i = 0; i < 25; i++) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);

    // Do the insert.
    Payload *np = static_cast<Payload *>(malloc(sizeof(Payload)));
    HTKeyValue_t oldkv, newkv;
    ASSERT_TRUE(np != NULL);
    np->magic_num = kMagicNum;
    np->payload_num = i;
    newkv.key = hashed_key;
    newkv.value = np;
    ASSERT_FALSE(HashTable_Insert(table, newkv, &oldkv));

    // Ensure it's there.
    ASSERT_TRUE(HashTable_Find(table, hashed_key, &oldkv));
    ASSERT_EQ(oldkv.key, hashed_key);
    ASSERT_EQ(oldkv.value, static_cast<HTValue_t>(np));
  }
  ASSERT_EQ(25, table->num_elements);

  // Delete most of the remaining keys.
  for (i = 0; i < 23; i++) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);
    HTKeyValue_t oldkv;

    ASSERT_TRUE(HashTable_Remove(table, hashed_key, &oldkv));
    ASSERT_EQ(hashed_key, oldkv.key);
    VerifiedFree(oldkv.value);
    ASSERT_FALSE(HashTable_Remove(table, hashed_key, &oldkv));
  }
  ASSERT_EQ(2, table->num_elements);

  // Delete the HT and the final keys
  HashTable_Free(table, &Test_HashTable::InstrumentedFree);
  ASSERT_EQ(2, freeInvocations_);
  HW1Environment::AddPoints(20);
}

TEST_F(Test_HashTable, Iterator) {
  HW1Environment::OpenTestCase();
  int i;
  HTKeyValue_t oldkv, newkv;
  HashTable *table = HashTable_Allocate(300);

  // Test using an iterator on an empty table.
  HTIterator *it = HTIterator_Allocate(table);
  ASSERT_FALSE(HTIterator_IsValid(it));
  ASSERT_FALSE(HTIterator_Get(it, &oldkv));
  HTIterator_Free(it);
  HW1Environment::AddPoints(5);

  // Allocate and insert a bunch of elements, then create an iterator for
  // the populated table.
  for (i = 0; i < 100; i++) {
    HTKey_t hashed_key = static_cast<HTKey_t>(i);

    // Create an element and do the insert.
    Payload *np = static_cast<Payload *>(malloc(sizeof(Payload)));
    ASSERT_TRUE(np != NULL);
    np->magic_num = kMagicNum;
    np->payload_num = static_cast<int>(i);
    newkv.key = hashed_key;
    newkv.value = np;
    ASSERT_FALSE(HashTable_Insert(table, newkv, &oldkv));
  }
  it = HTIterator_Allocate(table);
  ASSERT_TRUE(HTIterator_IsValid(it));
  HW1Environment::AddPoints(5);

  // Now iterate through the table, verifying each value is found exactly once.
  int num_times_seen[100] = { 0 };   // array of 100 0's
  for (i = 0; i < 100; i++) {
    Payload *op;
    int htkey;

    ASSERT_TRUE(HTIterator_IsValid(it));
    ASSERT_TRUE(HTIterator_Get(it, &oldkv));

    // Verify that we've never seen this key before, then increment the
    // number of times we've seen it.
    htkey = static_cast<int>(oldkv.key);
    ASSERT_EQ(0, num_times_seen[htkey]);
    num_times_seen[htkey]++;

    // Verify that this is the value we previously inserted.
    op = static_cast<Payload *>(oldkv.value);
    ASSERT_EQ(kMagicNum, op->magic_num);
    ASSERT_EQ(htkey, op->payload_num);

    // Increment the iterator.
    if (i == 99) {
      ASSERT_TRUE(HTIterator_IsValid(it));
      ASSERT_FALSE(HTIterator_Next(it));
      ASSERT_FALSE(HTIterator_IsValid(it));
    } else {
      ASSERT_TRUE(HTIterator_Next(it));
      ASSERT_TRUE(HTIterator_IsValid(it));
    }
  }
  for (i = 0; i < 100; i++) {
    ASSERT_EQ(1, num_times_seen[i]);  // verify each was seen exactly once.
  }

  ASSERT_FALSE(HTIterator_Next(it));
  HTIterator_Free(it);
  HW1Environment::AddPoints(10);

  // Iterate through again, removing every third element and resetting all
  // the "was seen" counters.
  it = HTIterator_Allocate(table);
  ASSERT_TRUE(HTIterator_IsValid(it));
  for (i = 0; i < 100; i++) {
    int htkey;

    ASSERT_TRUE(HTIterator_Get(it, &oldkv));
    htkey = static_cast<int>(oldkv.key);
    num_times_seen[htkey] = 0;

    if (i % 3 == 0) {
      int oldnumelements = HashTable_NumElements(table);
      Payload *op = static_cast<Payload *>(oldkv.value);
      ASSERT_EQ(htkey, op->payload_num);
      num_times_seen[htkey]++;

      // Remove the element.  Don't forget that HTIterator_Remove automatically
      // increments the iterator.
      ASSERT_TRUE(HTIterator_Remove(it, &oldkv));
      ASSERT_EQ(oldnumelements - 1, HashTable_NumElements(table));
      free(op);
    } else {
      // Manually increment the iterator.
      if (i == 99) {
        ASSERT_TRUE(HTIterator_IsValid(it));
        ASSERT_FALSE(HTIterator_Next(it));
        ASSERT_FALSE(HTIterator_IsValid(it));
      } else {
        ASSERT_TRUE(HTIterator_Next(it));
        ASSERT_TRUE(HTIterator_IsValid(it));
      }
    }
  }
  HTIterator_Free(it);
  HW1Environment::AddPoints(10);

  // Iterate through one last time, making sure we only retain elements whose
  // key is NOT a multiple of 3.
  it = HTIterator_Allocate(table);
  ASSERT_TRUE(HTIterator_IsValid(it));

  ASSERT_EQ(66, HashTable_NumElements(table));
  for (i = 0; i < 66; i++) {
    int htkey;
    ASSERT_TRUE(HTIterator_Get(it, &oldkv));
    htkey = static_cast<int>(oldkv.key);
    ASSERT_EQ(0, num_times_seen[htkey]);

    if (i == 65) {
      ASSERT_TRUE(HTIterator_IsValid(it));
      ASSERT_FALSE(HTIterator_Next(it));
      ASSERT_FALSE(HTIterator_IsValid(it));
    } else {
      ASSERT_TRUE(HTIterator_Next(it));
      ASSERT_TRUE(HTIterator_IsValid(it));
    }
  }
  HTIterator_Free(it);
  HW1Environment::AddPoints(5);

  // Delete the HT and the final remaining keys.
  HashTable_Free(table, &Test_HashTable::InstrumentedFree);
  ASSERT_EQ(66, freeInvocations_);
  HW1Environment::AddPoints(5);
}

static void NoOpFree(HTValue_t freeme) { }

TEST_F(Test_HashTable, Resize) {
  HW1Environment::OpenTestCase();
  HashTable *table = HashTable_Allocate(2);
  ASSERT_EQ(2, table->num_buckets);

  HTKeyValue_t newval;
  HTKeyValue_t oldkv;

  // Add elements to the Table, expect the table to resize
  // which makes use of HashTable_Iterator.
  for (int i = 0; i < 7; ++i) {
    newval.key = i;
    newval.value = (HTValue_t)(int64_t)i;
    ASSERT_FALSE(HashTable_Insert(table, newval, &oldkv));
    ASSERT_TRUE(HashTable_Insert(table, newval, &oldkv));
    ASSERT_EQ(newval.key, oldkv.key);
    ASSERT_EQ(newval.value, oldkv.value);

    oldkv.key = -1;       // reinitialize "oldkv" so we can verify it was
    oldkv.value = NULL;   // set by HashTable_Find.
    ASSERT_TRUE(HashTable_Find(table, newval.key, &oldkv));
    ASSERT_EQ(newval.key, oldkv.key);
    ASSERT_EQ(newval.value, oldkv.value);
  }

  ASSERT_LT(2, table->num_buckets);
  int old_buckets = table->num_buckets;
  HW1Environment::AddPoints(10);

  // Make sure that all of the elements are still inside the
  // HashTable after Resizing, then ensure that num_buckets
  // stays the same.
  for (int i = 0; i < 7; ++i) {
    HTKey_t key = i;
    HTValue_t value = reinterpret_cast<HTValue_t>(key);

    oldkv.key = -1;       // reinitialize "oldkv" so we can verify it was
    oldkv.value = NULL;   // set by HashTable_Find.
    ASSERT_TRUE(HashTable_Find(table, key, &oldkv));
    ASSERT_EQ(key, oldkv.key);
    ASSERT_EQ(value, oldkv.value);

    oldkv.key = -1;       // reinitialize "oldkv" so we can verify it was
    oldkv.value = NULL;   // set by HashTable_Remove.
    ASSERT_TRUE(HashTable_Remove(table, key, &oldkv));
    ASSERT_EQ(key, oldkv.key);
    ASSERT_EQ(value, oldkv.value);

    // Assert that the KeyValue is no longer within the HashTable
    ASSERT_FALSE(HashTable_Find(table, key, &oldkv));
    ASSERT_FALSE(HashTable_Remove(table, key, &oldkv));
  }

  // Assert that the number of buckets has not changed
  ASSERT_LT(2, table->num_buckets);
  ASSERT_EQ(old_buckets, table->num_buckets);

  HashTable_Free(table, NoOpFree);
  HW1Environment::AddPoints(10);
}

}  // namespace hw1
