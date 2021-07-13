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

#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include "gtest/gtest.h"

extern "C" {
  #include "./LinkedList.h"
  #include "./LinkedList_priv.h"
}

#include "./test_suite.h"

namespace hw1 {

int TestLLPayloadComparator(LLPayload_t p1, LLPayload_t p2) {
  // A comparator used to test sort.
  if (p1 > p2)
    return 1;
  if (p1 < p2)
    return -1;
  return 0;
}

class Test_LinkedList : public ::testing::Test {
 protected:
  // Code here will be called before each test executes (ie, before
  // each TEST_F).
  virtual void SetUp() {
    freeInvocations_ = 0;
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Verify that none of the tests modifies any of the
    // testing constants.
    ASSERT_EQ((LLPayload_t)1U, kOne);
    ASSERT_EQ((LLPayload_t)2U, kTwo);
    ASSERT_EQ((LLPayload_t)3U, kThree);
    ASSERT_EQ((LLPayload_t)4U, kFour);
    ASSERT_EQ((LLPayload_t)5U, kFive);
  }

  // These values are used as payloads for the LinkedList tests.
  // They cannot be const, as stored value pointers are non-const.
  static LLPayload_t kOne, kTwo, kThree, kFour, kFive;

  // A stubbed and instrumented version of free() which counts how many
  // times it's been invoked; this allows us to make assertions without
  // actually freeing the payload (which had never been allocated in the
  // first place).  Note that the counter is reset in SetUp().
  static int freeInvocations_;
  static void StubbedFree(LLPayload_t payload) {
    // Do nothing but verify the payload is non-NULL and
    // increment the free count.
    ASSERT_TRUE(payload != NULL);
    freeInvocations_++;
  }
};  // class Test_LinkedList

// statics:
LLPayload_t Test_LinkedList::kOne = (LLPayload_t)1;
LLPayload_t Test_LinkedList::kTwo = (LLPayload_t)2;
LLPayload_t Test_LinkedList::kThree = (LLPayload_t)3;
LLPayload_t Test_LinkedList::kFour = (LLPayload_t)4;
LLPayload_t Test_LinkedList::kFive = (LLPayload_t)5;
int Test_LinkedList::freeInvocations_;

TEST_F(Test_LinkedList, Basic) {
  HW1Environment::OpenTestCase();
  // Try creating a list.
  LinkedList* llp = LinkedList_Allocate();
  ASSERT_TRUE(llp != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);
  HW1Environment::AddPoints(5);

  // Try deleting the (empty) list.
  LinkedList_Free(llp, &Test_LinkedList::StubbedFree);
  ASSERT_EQ(0, freeInvocations_);
  llp = NULL;
  HW1Environment::AddPoints(5);
}

TEST_F(Test_LinkedList, PushPop) {
  HW1Environment::OpenTestCase();
  // Creating a list.
  LinkedList *llp = LinkedList_Allocate();
  ASSERT_TRUE(llp != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);

  // Insert an element.
  LinkedList_Push(llp, kOne);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);
  HW1Environment::AddPoints(10);

  // Pop the element.
  LLPayload_t payload_ptr;
  ASSERT_TRUE(LinkedList_Pop(llp, &payload_ptr));
  ASSERT_EQ(kOne, payload_ptr);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  HW1Environment::AddPoints(10);

  // Try (and fail) to pop the element a second time.
  ASSERT_FALSE(LinkedList_Pop(llp, &payload_ptr));
  HW1Environment::AddPoints(5);

  // Insert two elements.
  LinkedList_Push(llp, kOne);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);

  LinkedList_Push(llp, kTwo);
  ASSERT_EQ(2, LinkedList_NumElements(llp));
  ASSERT_NE(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(llp->tail, llp->head->next);
  ASSERT_EQ(llp->head, llp->tail->prev);
  ASSERT_EQ(kTwo, llp->head->payload);
  ASSERT_EQ(kOne, llp->tail->payload);
  HW1Environment::AddPoints(10);

  // Pop the first element.
  ASSERT_TRUE(LinkedList_Pop(llp, &payload_ptr));
  ASSERT_EQ(kTwo, payload_ptr);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);
  HW1Environment::AddPoints(10);

  // Free the non-empty list.
  LinkedList_Free(llp, &Test_LinkedList::StubbedFree);
  ASSERT_EQ(1, freeInvocations_);
  llp = NULL;
}

TEST_F(Test_LinkedList, TestLinkedListAppendSlice) {
  HW1Environment::OpenTestCase();
  // Creating a list.
  LinkedList *llp = LinkedList_Allocate();
  ASSERT_TRUE(llp != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);

  // Insert an element.
  LinkedList_Append(llp, kOne);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);
  HW1Environment::AddPoints(5);

  // Delete the element.
  LLPayload_t payload_ptr;
  ASSERT_TRUE(LinkedList_Slice(llp, &payload_ptr));
  ASSERT_EQ(kOne, payload_ptr);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  HW1Environment::AddPoints(5);

  // Delete the element a second time.
  ASSERT_FALSE(LinkedList_Slice(llp, &payload_ptr));
  HW1Environment::AddPoints(5);

  // Insert two elements.
  LinkedList_Append(llp, kOne);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);

  LinkedList_Append(llp, kTwo);
  ASSERT_EQ(2, LinkedList_NumElements(llp));
  ASSERT_NE(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(llp->tail, llp->head->next);
  ASSERT_EQ(llp->head, llp->tail->prev);
  ASSERT_EQ(kOne, llp->head->payload);
  ASSERT_EQ(kTwo, llp->tail->payload);
  HW1Environment::AddPoints(5);

  // Delete the first element.
  ASSERT_TRUE(LinkedList_Slice(llp, &payload_ptr));
  ASSERT_EQ(kTwo, payload_ptr);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  ASSERT_EQ(llp->head, llp->tail);
  ASSERT_EQ(NULL, llp->head->prev);
  ASSERT_EQ(NULL, llp->tail->next);
  ASSERT_EQ(kOne, llp->head->payload);
  HW1Environment::AddPoints(5);

  // Delete the non-empty list.
  LinkedList_Free(llp, &Test_LinkedList::StubbedFree);
  ASSERT_EQ(1, freeInvocations_);
  llp = NULL;
}

TEST_F(Test_LinkedList, Sort) {
  HW1Environment::OpenTestCase();
  // Creating a list.
  LinkedList *llp = LinkedList_Allocate();
  ASSERT_TRUE(llp != NULL);
  ASSERT_EQ(0, LinkedList_NumElements(llp));
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);

  // Insert some elements.
  LinkedList_Append(llp, kThree);
  ASSERT_EQ(1, LinkedList_NumElements(llp));
  LinkedList_Append(llp, kTwo);
  ASSERT_EQ(2, LinkedList_NumElements(llp));
  LinkedList_Append(llp, kOne);
  ASSERT_EQ(3, LinkedList_NumElements(llp));

  // Sort ascending.
  LinkedList_Sort(llp, true, &TestLLPayloadComparator);

  // Verify the sort.
  ASSERT_EQ(kOne, llp->head->payload);
  ASSERT_EQ(kTwo, llp->head->next->payload);
  ASSERT_EQ(kThree, llp->head->next->next->payload);
  ASSERT_EQ(NULL, llp->head->next->next->next);

  // Resort descending.
  LinkedList_Sort(llp, false, &TestLLPayloadComparator);

  // Verify the sort.
  ASSERT_EQ(kThree, llp->head->payload);
  ASSERT_EQ(kTwo, llp->head->next->payload);
  ASSERT_EQ(kOne, llp->head->next->next->payload);
  ASSERT_EQ(NULL, llp->head->next->next->next);
  HW1Environment::AddPoints(5);

  // Delete the non-empty list.
  LinkedList_Free(llp, &Test_LinkedList::StubbedFree);
  ASSERT_EQ(3, freeInvocations_);
  llp = NULL;
}

TEST_F(Test_LinkedList, TestLLIteratorBasic) {
  HW1Environment::OpenTestCase();
  // Create a linked list.
  LinkedList *llp = LinkedList_Allocate();

  // Add some data to the list.
  LinkedList_Append(llp, kFive);
  LinkedList_Append(llp, kFour);
  LinkedList_Append(llp, kThree);
  LinkedList_Append(llp, kTwo);
  LinkedList_Append(llp, kOne);

  // Create the iterator.
  LLIterator *lli = LLIterator_Allocate(llp);
  ASSERT_TRUE(lli != NULL);
  ASSERT_EQ(llp, lli->list);
  ASSERT_EQ(llp->head, lli->node);
  HW1Environment::AddPoints(5);

  // Navigate using the iterator.
  LLPayload_t payload;
  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kFive, payload);
  ASSERT_TRUE(LLIterator_Next(lli));
  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kFour, payload);
  ASSERT_TRUE(LLIterator_Next(lli));
  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kThree, payload);
  ASSERT_TRUE(LLIterator_Next(lli));
  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kTwo, payload);
  ASSERT_TRUE(LLIterator_Next(lli));
  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kOne, payload);
  ASSERT_FALSE(LLIterator_Next(lli));
  ASSERT_FALSE(LLIterator_IsValid(lli));
  HW1Environment::AddPoints(10);

  // The list contains 5 elements; try a delete from the front of the list.
  LLIterator_Rewind(lli);
  LinkedListNode *prev,
    *next = lli->node->next,
    *nextnext = lli->node->next->next;
  ASSERT_TRUE(LLIterator_Remove(lli, &Test_LinkedList::StubbedFree));
  ASSERT_EQ(next, llp->head);
  ASSERT_EQ(next, lli->node);
  ASSERT_EQ(NULL, lli->node->prev);
  ASSERT_EQ(nextnext, lli->node->next);
  ASSERT_EQ(4, LinkedList_NumElements(lli->list));
  ASSERT_EQ(1, freeInvocations_);

  LLIterator_Get(lli, &payload);
  ASSERT_EQ(kFour, payload);
  HW1Environment::AddPoints(10);

  // Delete the rest, but move the iterator forward by one to test removing
  // from the middle (ie, the 2nd element).
  ASSERT_TRUE(LLIterator_Next(lli));
  prev = lli->node->prev;
  next = lli->node->next;
  nextnext = lli->node->next->next;
  ASSERT_TRUE(LLIterator_Remove(lli, &Test_LinkedList::StubbedFree));
  ASSERT_EQ(3, LinkedList_NumElements(lli->list));
  ASSERT_EQ(next, lli->node);
  ASSERT_EQ(prev, lli->node->prev);
  ASSERT_EQ(nextnext, lli->node->next);
  ASSERT_EQ(prev->next, lli->node);
  ASSERT_EQ(lli->node, nextnext->prev);

  // We're still deleting from the middle (ie, the 2nd element).
  next = lli->node->next;
  prev = lli->node->prev;
  ASSERT_TRUE(LLIterator_Remove(lli, &Test_LinkedList::StubbedFree));
  ASSERT_EQ(2, LinkedList_NumElements(lli->list));
  ASSERT_EQ(next, lli->node);
  ASSERT_EQ(prev, lli->node->prev);
  ASSERT_EQ(NULL, lli->node->next);
  ASSERT_EQ(prev->next, lli->node);

  // This deletes from the tail position, since we are at the second element of
  // a two-element list.
  prev = lli->node->prev;
  ASSERT_TRUE(LLIterator_Remove(lli, &Test_LinkedList::StubbedFree));
  ASSERT_EQ(1, LinkedList_NumElements(lli->list));
  ASSERT_EQ(NULL, lli->node->next);
  ASSERT_EQ(prev, lli->node);
  ASSERT_EQ(NULL, lli->node->prev);
  ASSERT_EQ(prev, llp->tail);  // edge case found 17sp

  // Remove the remaining node from the list.
  ASSERT_FALSE(LLIterator_Remove(lli, &Test_LinkedList::StubbedFree));
  ASSERT_EQ(0, LinkedList_NumElements(lli->list));
  ASSERT_EQ(NULL, lli->node);
  ASSERT_EQ(NULL, llp->head);
  ASSERT_EQ(NULL, llp->tail);
  ASSERT_EQ(5, freeInvocations_);

  // Free the iterator.
  LLIterator_Free(lli);
  HW1Environment::AddPoints(5);

  // Free the list.
  LinkedList_Free(llp, &Test_LinkedList::StubbedFree);
}

}  // namespace hw1
