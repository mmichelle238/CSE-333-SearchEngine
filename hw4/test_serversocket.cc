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
#include <stdint.h>
#include <iostream>
#include <string>
#include <cstdlib>

#include "gtest/gtest.h"
#include "./ServerSocket.h"
#include "./HttpUtils.h"
#include "./ThreadPool.h"
#include "./test_suite.h"

using std::cout;
using std::endl;
using std::string;

namespace hw4 {

static uint16_t portnum;

// A task object created to hold the necessary information
// of what information is retrived from the server when
// Accept is run.
class TestServerTask : public ThreadPool::Task {
 public:
  explicit TestServerTask(ThreadPool::thread_task_fn f)
    : ThreadPool::Task(f), task_done(false) { }

  // If the client_fd is open, close it.
  virtual ~TestServerTask() {
    if (accept_fd >= 0) {
      close(accept_fd);
    }
  }

  // public fields to store conneciton information.
  int accept_fd;
  uint16_t cport;
  std::string caddr, cdns, saddr, sdns;
  bool task_done;
};

void TestSSThreadFn(ThreadPool::Task *t) {
  TestServerTask *task = static_cast<TestServerTask *>(t);

  // Create the server socket.
  int listen_fd;
  cout << "Creating ServerSocket on " << portnum << endl;
  ServerSocket ss(portnum);
  cout << "Doing BindAndListen" << endl;
  // note how we are always using AF_INET6
  ASSERT_TRUE(ss.BindAndListen(AF_INET6, &listen_fd));

  // Accept a connection.
  cout << "Doing accept..." << endl;
  ASSERT_TRUE(ss.Accept(&task->accept_fd, &task->caddr, &task->cport,
                        &task->cdns, &task->saddr, &task->sdns));

  // It worked!
  cout << "Accept succeeded." << endl;
  task->task_done = true;

  return;
}

TEST(Test_ServerSocket, TestServerSocketBasic) {
  // Create a threadpool, and dispatch a thread to go listen on a
  // server socket on a random port.
  HW4Environment::OpenTestCase();
  portnum = GetRandPort();
  ThreadPool tp(1);
  TestServerTask tsk(&TestSSThreadFn);
  tp.Dispatch(static_cast<ThreadPool::Task*>(&tsk));

  // Give the thread a chance to create the socket.
  sleep(1);

  // Connect to the socket
  cout << "Attempting to connect to 127.0.0.1 port "
       << portnum << endl;
  int cfd = -1;
  ASSERT_TRUE(ConnectToServer("127.0.0.1", portnum, &cfd));

  // Make sure that the server had a chance to get client & server info
  while (!tsk.task_done) {
    sleep(1);
  }

  // verify that the file descriptor is valid.
  ASSERT_LE(0, cfd);

  // record points
  HW4Environment::AddPoints(45);

  // Check that the output params
  // (caddr, cport, cdns, saddr, sdns)
  // are set correctly.
  cout << "Checking output params from Accept..." << endl;

  // check the port...
  ASSERT_LT(0, tsk.cport);

  // check the fd...
  ASSERT_LT(0, tsk.accept_fd);

  // check the dns results...

  // note that since the server & client are the same machine
  // both values should be localhost
  ASSERT_EQ("localhost", tsk.sdns);
  ASSERT_EQ(tsk.sdns, tsk.cdns);

  // check the addresses...

  // in this case, saddr should be ::ffff:127.0.0.1 since this is our
  // local machine and because TestSSThreadFn() sets up server to use AF_INET6
  ASSERT_EQ("::ffff:127.0.0.1", tsk.saddr);
  // client address could be in either ipv4 or ipv6,
  // depending on implementation and ConnectToServer()
  ASSERT_TRUE(tsk.caddr == "::ffff:127.0.0.1" || tsk.caddr == "127.0.0.1");

  close(cfd);
  HW4Environment::AddPoints(35);
}

}  // namespace hw4
