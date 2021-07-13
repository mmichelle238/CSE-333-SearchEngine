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
#include <pthread.h>  // for the pthread threading/mutex functions
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

#include "./HttpConnection.h"

#include "gtest/gtest.h"
#include "./HttpRequest.h"
#include "./HttpResponse.h"
#include "./HttpUtils.h"
#include "./test_suite.h"

using std::string;

namespace hw4 {

static pthread_mutex_t rw_lock;
static int num_read = 0;
static int num_write = 0;

static void WritePartialRequests(void* args);

static void *WriteWrapper(void* args) {
  WritePartialRequests(args);
  return nullptr;
}

TEST(Test_HttpConnection, TestHttpConnectionBasic) {
  HW4Environment::OpenTestCase();

  // Create a socketpair; we'll hand one end of the socket to the
  // HttpConnection object, and use the other end of the socket
  // ourselves for testing.
  int spair[2] = {-1, -1};
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, spair));

  // Create the HttpConnection object we'll test.
  HttpConnection hc(spair[0]);

  // Write three requests on the socket.
  string req1 = "GET /foo HTTP/1.1\r\n";
  req1 += "Host: somehost.foo.bar\r\n";
  req1 += "Connection: close\r\n";
  req1 += "\r\n";
  // req2: check header order doesn't matter
  string req2 = "GET /bar HTTP/1.1\r\n";
  req2 += "Connection: close\r\n";
  req2 += "Host: somehost.foo.bar\r\n";
  req2 +=  "\r\n";
  // req3: check different values
  string req3 = "GET /baz HTTP/1.1\r\n";
  req3 += "connection: keep-alive\r\n";
  req3 += "host: somehost.foo.bar\r\n";
  req3 += "OTHER: some_value\r\n";
  req3 +=  "\r\n";
  ASSERT_EQ(static_cast<int>(req1.size()),
            WrappedWrite(spair[1],
                         (unsigned char *) req1.c_str(),
                         static_cast<int>(req1.size())));
  ASSERT_EQ(static_cast<int>(req2.size()),
            WrappedWrite(spair[1],
                         (unsigned char *) req2.c_str(),
                         static_cast<int>(req2.size())));
  ASSERT_EQ(static_cast<int>(req3.size()),
            WrappedWrite(spair[1],
                         (unsigned char *) req3.c_str(),
                         static_cast<int>(req3.size())));

  // Do the GetNextRequests.
  HttpRequest htreq1, htreq2, htreq3;
  ASSERT_TRUE(hc.GetNextRequest(&htreq1));
  ASSERT_TRUE(hc.GetNextRequest(&htreq2));
  ASSERT_TRUE(hc.GetNextRequest(&htreq3));
  HW4Environment::AddPoints(20);

  // Make sure the request parsing worked.
  ASSERT_EQ("/foo", htreq1.uri());
  ASSERT_EQ("somehost.foo.bar", htreq1.GetHeaderValue("host"));
  ASSERT_EQ("close", htreq1.GetHeaderValue("connection"));
  ASSERT_EQ(2, htreq1.GetHeaderCount());
  HW4Environment::AddPoints(10);

  ASSERT_EQ("/bar", htreq2.uri());
  ASSERT_EQ("somehost.foo.bar", htreq2.GetHeaderValue("host"));
  ASSERT_EQ("close", htreq2.GetHeaderValue("connection"));
  ASSERT_EQ(2, htreq2.GetHeaderCount());
  HW4Environment::AddPoints(10);

  ASSERT_EQ("/baz", htreq3.uri());
  ASSERT_EQ("somehost.foo.bar", htreq3.GetHeaderValue("host"));
  ASSERT_EQ("keep-alive", htreq3.GetHeaderValue("connection"));
  ASSERT_EQ("some_value", htreq3.GetHeaderValue("other"));
  ASSERT_EQ(3, htreq3.GetHeaderCount());
  HW4Environment::AddPoints(10);

  // Test a "split" response being written
  string req4 = "GET /foo HTTP/1.1\r\n";
  req1 += "Host: somehost.foo.bar\r\n";
  req1 += "Connection: close\r\n";
  req1 += "\r\n";
  // req2: check header order doesn't matter
  string req5 = "GET /bar HTTP/1.1\r\n";
  req2 += "Connection: close\r\n";
  req2 += "Host: somehost.foo.bar\r\n";
  req2 +=  "\r\n";

  // Prepare the responses.
  HttpResponse rep1;
  rep1.set_protocol("HTTP/1.1");
  rep1.set_response_code(200);
  rep1.set_message("OK");
  rep1.AppendToBody("This is the body of the response.");
  string expectedrep1 = "HTTP/1.1 200 OK\r\n";
  expectedrep1 += "Content-length: 33\r\n\r\n";
  expectedrep1 += "This is the body of the response.";

  HttpResponse rep2;
  rep2.set_protocol("HTTP/1.1");
  rep2.set_response_code(200);
  rep2.set_message("OK");
  rep2.set_content_type("text/html");
  rep2.AppendToBody("This is the second response.");
  string expectedrep2 = "HTTP/1.1 200 OK\r\n";
  expectedrep2 += "Content-type: text/html\r\n";
  expectedrep2 += "Content-length: 28\r\n\r\n";
  expectedrep2 += "This is the second response.";

  // Generate the responses, test them.
  unsigned char buf1[1024] = { 0 };
  ASSERT_TRUE(hc.WriteResponse(rep1));
  ASSERT_EQ(static_cast<int>(72), WrappedRead(spair[1], buf1, 1024));
  ASSERT_EQ(expectedrep1, (const char *) buf1);

  unsigned char buf2[1024] = { 0 };
  ASSERT_TRUE(hc.WriteResponse(rep2));
  ASSERT_EQ(static_cast<int>(92), WrappedRead(spair[1], buf2, 1024));
  ASSERT_EQ(expectedrep2, (const char *) buf2);

  // Clean up.
  close(spair[0]);
  close(spair[1]);
  HW4Environment::AddPoints(10);
}

TEST(Test_HttpConnection, TestHttpConnectionPartialRead) {
  HW4Environment::OpenTestCase();

  // Create a socketpair; we'll hand one end of the socket to the
  // HttpConnection object, and use the other end of the socket
  // ourselves for testing.
  int spair[2] = {-1, -1};
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, spair));

  // Note that we use sleep() a lot to try and "motivate"
  // the threads to swap. This is beacuse valgrind isn't
  // the happiest when we have multiple threads so
  // sleep is needed to make it cooperate.

  // Create the HttpConnection object we'll test.
  HttpConnection hc(spair[0]);

  // create a thread to write on the other end of the socket
  // so that we can test our http connection class
  ASSERT_EQ(0, pthread_mutex_init(&rw_lock, nullptr));
  pthread_t thread_id;
  ASSERT_EQ(0, pthread_create(&thread_id, nullptr, WriteWrapper, &(spair[1])));
  // sleep so that the write thread can start writing to us
  sleep(1);

  // Do the GetNextRequests.
  HttpRequest htreq1, htreq2, htreq3;

  // loop until we know that the write socket has started to
  // write the first request.
  while (1) {
    ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
    if (num_write == 1) {
      num_read += 1;
      ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
      ASSERT_TRUE(hc.GetNextRequest(&htreq1));
      break;
    }
    ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
    sleep(1);
  }

  // Make sure the request parsing worked.
  ASSERT_EQ("/foo", htreq1.uri());
  ASSERT_EQ("somehost.foo.bar", htreq1.GetHeaderValue("host"));
  ASSERT_EQ("close", htreq1.GetHeaderValue("connection"));
  ASSERT_EQ(2, htreq1.GetHeaderCount());
  HW4Environment::AddPoints(10);
  sleep(1);  // sleep so that req2 can be sent

  // start parsing req2
  ASSERT_TRUE(hc.GetNextRequest(&htreq2));
  ASSERT_EQ("/bar", htreq2.uri());
  ASSERT_EQ("somehost.foo.bar", htreq2.GetHeaderValue("host"));
  ASSERT_EQ("close", htreq2.GetHeaderValue("connection"));
  ASSERT_EQ(2, htreq2.GetHeaderCount());
  HW4Environment::AddPoints(10);

  // loop until we know that the write socket has started to
  // write the third request.
  while (1) {
    ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
    if (num_write == 2) {
      num_read += 1;
      ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
      break;
    }
    ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
    sleep(1);
  }

  // verify that req3 is correct
  ASSERT_TRUE(hc.GetNextRequest(&htreq3));
  ASSERT_EQ("/baz", htreq3.uri());
  ASSERT_EQ("somehost.foo.bar", htreq3.GetHeaderValue("host"));
  ASSERT_EQ("keep-alive", htreq3.GetHeaderValue("connection"));
  ASSERT_EQ("some_value", htreq3.GetHeaderValue("other"));
  ASSERT_EQ(3, htreq3.GetHeaderCount());

  // make sure that we read/wrote the correct
  // number of times and clean up
  ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
  num_read += 1;
  ASSERT_EQ(3, num_write);
  ASSERT_EQ(3, num_read);
  ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
  ASSERT_EQ(0, pthread_mutex_destroy(&rw_lock));
  ASSERT_EQ(0, pthread_join(thread_id, nullptr));

  HW4Environment::AddPoints(20);

  // Clean up.
  close(spair[0]);
  close(spair[1]);
}

static void WritePartialRequests(void* args) {
  int socket = *static_cast<int*>(args);
  // Write three requests on the socket.
  // Note that the writes for these requests
  // are split up in such a way to test that
  // your http connection works properly if
  // some of the request that are read are not
  // read completely.
  string req1 = "GET /foo HTTP/1.1\r\n";
  req1 += "Host: somehost.foo.bar\r\n";
  req1 += "Connection: close\r\n";
  req1 += "\r\nGET /bar ";
  // req2: check that it is ok if the request
  // is split in the first line
  // across calls to WrappedRead().
  string req2 = "HTTP/1.1\r\n";
  req2 += "Connection: close\r\n";
  req2 += "Host: somehost.foo.bar\r\n";
  req2 += "\r\nGET /baz HTTP/1.1\r\n";
  req2 += "connection:";
  // req3: check that it is ok if the request
  // is split in the "headers" and during the
  // end \r\n\r\n sequence
  string req3 = " keep-alive\r\n";
  req3 += "host: somehost.foo.bar\r\n";
  req3 += "OTHER: some_value\r\n";
  string req3tail = "\r\n";

  // acquire the rw lock to make sure that
  // the socket doesn't read until we have
  // written req1
  ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
  ASSERT_EQ(0, num_read);
  ASSERT_EQ(0, num_write);

  // write req1
  ASSERT_EQ(static_cast<int>(req1.size()),
            WrappedWrite(socket,
                         (unsigned char *) req1.c_str(),
                         static_cast<int>(req1.size())));

  // Release lock and update num_write so that
  // the read socket thread knows it can proceed
  // to read req1
  num_write += 1;
  ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
  sleep(3);

  // Loop trying to acquire the lock but checking
  // to make sure that the client has had the chance
  // to start reading req1
  while (1) {
    ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
    if (num_read == 1) {
      num_write += 1;
      break;
    }
    ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
    sleep(1);
  }

  // now that we know req1 has started to have been read
  // (probably), we write req2.
  ASSERT_EQ(static_cast<int>(req2.size()),
            WrappedWrite(socket,
                         (unsigned char *) req2.c_str(),
                         static_cast<int>(req2.size())));
  ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
  sleep(3);

  // Loop trying to acquire the lock but checking
  // to make sure that the reading thread has had
  // the chance to start reading req2
  while (1) {
    ASSERT_EQ(0, pthread_mutex_lock(&rw_lock));
    if (num_read == 2) {
      num_write += 1;
      break;
    }
    ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));
    sleep(1);
  }

  // Write the first part of req3
  ASSERT_EQ(static_cast<int>(req3.size()),
            WrappedWrite(socket,
                         (unsigned char *) req3.c_str(),
                         static_cast<int>(req3.size())));
  ASSERT_EQ(0, pthread_mutex_unlock(&rw_lock));

  // sleep so that the read thread has a chance to
  // start reading req3.
  sleep(3);

  // write the last \r\n necessary to mark the
  // end of req3. If you are going infinite at
  // this point, you are not handling the reading
  // of http requests properly
  ASSERT_EQ(static_cast<int>(req3tail.size()),
            WrappedWrite(socket,
                         (unsigned char *) req3tail.c_str(),
                         static_cast<int>(req3tail.size())));
}

}  // namespace hw4
