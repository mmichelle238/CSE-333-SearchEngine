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

#ifndef HW4_HTTPSERVER_H_
#define HW4_HTTPSERVER_H_

#include <stdint.h>
#include <string>
#include <list>

#include "./ThreadPool.h"
#include "./ServerSocket.h"

namespace hw4 {

// The HttpServer class contains the main logic for the web server.
class HttpServer {
 public:
  // Creates a new HttpServer object for port "port" and serving
  // files out of path "staticfile_dirpath".  The indices for
  // query processing are located in the "indices" list. The constructor
  // does not do anything except memorize these variables.
  explicit HttpServer(uint16_t port,
                      const std::string &static_file_dir_path,
                      const std::list<std::string> &indices)
    : socket_(port), static_file_dir_path_(static_file_dir_path),
      indices_(indices) { }

  // The destructor closes the listening socket if it is open and
  // also kills off any threads in the threadpool.
  virtual ~HttpServer() { }

  // Creates a listening socket for the server and launches it, accepting
  // connections and dispatching them to worker threads.  Returns
  // "true" if the server was able to start and run, "false" otherwise.
  // The server continues to run until a kill command is used to send
  // a SIGTERM signal to the server process (i.e., kill pid).
  bool Run();

 private:
  ServerSocket socket_;
  std::string static_file_dir_path_;
  std::list<std::string> indices_;
  static const int kNumThreads;
};

class HttpServerTask : public ThreadPool::Task {
 public:
  explicit HttpServerTask(ThreadPool::thread_task_fn f)
    : ThreadPool::Task(f) { }

  int client_fd;
  uint16_t c_port;
  std::string c_addr, c_dns, s_addr, s_dns;
  std::string base_dir;
  std::list<std::string> *indices;
};

}  // namespace hw4

#endif  // HW4_HTTPSERVER_H_
