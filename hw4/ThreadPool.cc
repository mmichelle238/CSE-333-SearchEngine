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
#include <iostream>

#include "./ThreadPool.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

// This is the thread start routine, i.e., the function that threads
// are born into.
void *ThreadLoop(void *t_pool);

ThreadPool::ThreadPool(uint32_t num_threads) {
  // Initialize our member variables.
  num_threads_running_ = 0;
  killthreads_ = false;
  Verify333(pthread_mutex_init(&q_lock_, nullptr) == 0);
  Verify333(pthread_cond_init(&q_cond_, nullptr) == 0);

  // Allocate the array of pthread structures.
  thread_array_ = new pthread_t[num_threads];

  // Spawn the threads one by one, passing them a pointer to self
  // as the argument to the thread start routine.
  Verify333(pthread_mutex_lock(&q_lock_) == 0);
  for (uint32_t i = 0; i < num_threads; i++) {
    Verify333(pthread_create(&(thread_array_[i]),
                             nullptr,
                             &ThreadLoop,
                             static_cast<void *>(this)) == 0);
  }

  // Wait for all of the threads to be born and initialized.
  while (num_threads_running_ != num_threads) {
    Verify333(pthread_mutex_unlock(&q_lock_) == 0);
    sleep(1);  // give another thread the chance to acquire the lock
    Verify333(pthread_mutex_lock(&q_lock_) == 0);
  }
  Verify333(pthread_mutex_unlock(&q_lock_) == 0);

  // Done!  The thread pool is ready, and all of the worker threads
  // are initialized and waiting on q_cond_ to be notified of available
  // work.
}

ThreadPool:: ~ThreadPool() {
  Verify333(pthread_mutex_lock(&q_lock_) == 0);
  uint32_t num_threads = num_threads_running_;

  // Tell all of the worker threads to kill themselves.
  killthreads_ = true;

  // Join with the running threads 1-by-1 until they have all died.
  for (uint32_t i = 0; i < num_threads; i++) {
    // Use a sledgehammer and broadcast every loop iteration, just to
    // be extra-certain that worker threads wake up and see the "kill
    // yourself" flag.
    Verify333(pthread_cond_broadcast(&q_cond_) == 0);
    Verify333(pthread_mutex_unlock(&q_lock_) == 0);
    Verify333(pthread_join(thread_array_[i], nullptr) == 0);
    Verify333(pthread_mutex_lock(&q_lock_) == 0);
  }

  // All of the worker threads are dead, so clean up the thread
  // structures.
  Verify333(num_threads_running_ == 0);
  if (thread_array_ != nullptr) {
    delete[] thread_array_;
  }
  thread_array_ = nullptr;
  Verify333(pthread_mutex_unlock(&q_lock_) == 0);

  // Empty the task queue, serially issuing any remaining work.
  while (!work_queue_.empty()) {
    Task *nextTask = work_queue_.front();
    work_queue_.pop_front();
    nextTask->func_(nextTask);
  }
}

// Enqueue a Task for dispatch.
void ThreadPool::Dispatch(Task *t) {
  Verify333(pthread_mutex_lock(&q_lock_) == 0);
  Verify333(killthreads_ == false);
  work_queue_.push_back(t);
  Verify333(pthread_cond_signal(&q_cond_) == 0);
  Verify333(pthread_mutex_unlock(&q_lock_) == 0);
}

// This is the main loop that all worker threads are born into.  They
// wait for a signal on the work queue condition variable, then they
// grab work off the queue.  Threads return (i.e., kill themselves)
// when they notice that killthreads_ is true.
void *ThreadLoop(void *t_pool) {
  ThreadPool *pool = static_cast<ThreadPool *>(t_pool);

  // Grab the lock, increment the thread count so that the ThreadPool
  // constructor knows this new thread is alive.
  Verify333(pthread_mutex_lock(&(pool->q_lock_)) == 0);
  pool->num_threads_running_++;

  // This is our main thread work loop.
  while (pool->killthreads_ == false) {
    // Wait to be signaled that something has happened.
    Verify333(pthread_cond_wait(&(pool->q_cond_), &(pool->q_lock_)) == 0);

    // Keep trying to dequeue work until the work queue is empty.
    while (!pool->work_queue_.empty() && (pool->killthreads_ == false)) {
      ThreadPool::Task *nextTask = pool->work_queue_.front();
      pool->work_queue_.pop_front();

      // We picked up a Task, so invoke the task function with the
      // lock released, then check so see if more tasks are waiting to
      // be picked up.
      Verify333(pthread_mutex_unlock(&(pool->q_lock_)) == 0);
      nextTask->func_(nextTask);
      Verify333(pthread_mutex_lock(&(pool->q_lock_)) == 0);
    }
  }

  // All done, exit.
  pool->num_threads_running_--;
  Verify333(pthread_mutex_unlock(&(pool->q_lock_)) == 0);
  return nullptr;
}

}  // namespace hw4
