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

// This file contains a number of HTTP and HTML parsing routines
// that come in useful throughput the assignment.

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include "./HttpUtils.h"

using boost::algorithm::replace_all;
using std::cerr;
using std::endl;
using std::map;
using std::pair;
using std::string;
using std::vector;

namespace hw4 {

bool IsPathSafe(const string &root_dir, const string &test_file) {
  // rootdir is a directory path. testfile is a path to a file.
  // return whether or not testfile is within rootdir.
  // Be sure that your code handles the case when"." and ".."
  // are found in the path for testfile.
  //
  // (HINT: It can be tricky to handle all of the cases where "."
  // and ".." show up in the passed in file paths. It would be easier
  // to just handle the absolute paths for the files (no "." or "..").
  // You may want to see if there is a C function that gets the absolute
  // path of a file.)

  // STEP 1

  // get absolute path of root_dir
  char root_buf[1024];
  char *absolute_root_dir = realpath(root_dir.c_str(), root_buf);
  if (!absolute_root_dir) {
    return false;
  }
  string abs_root(absolute_root_dir);

  // get absolute path of test_file
  char buf[1024];
  char *absolute_path = realpath(test_file.c_str(), buf);
  if (!absolute_path) {
    return false;
  }

  string abs_path(absolute_path);

  // test if absolute path starts with root_dir
  if (abs_path.rfind(abs_root, 0) == 0) {
    // absolute_path starts with root_dir
    return true;
  }

  // free things from the C function

  return false;  // You may want to change this.
}

string EscapeHtml(const string &from) {
  string ret = from;
  // Read through the passed in string, and replace any unsafe
  // html tokens with the proper escape codes. The characters
  // that need to be escaped in HTML are the same five as those
  // that need to be escaped for XML documents. You can see an
  // example in the comment for this function in HttpUtils.h and
  // the rest of the characters that need to be replaced can be
  // looked up online.

  // STEP 2
  boost::replace_all(ret, "&", "&amp;");
  boost::replace_all(ret, "\"", "&quot;");
  boost::replace_all(ret, "\'", "&apos;");
  boost::replace_all(ret, "<", "&lt;");
  boost::replace_all(ret, ">", "&gt;");

  return ret;
}

// Look for a "%XY" token in the string, where XY is a
// hex number.  Replace the token with the appropriate ASCII
// character, but only if 32 <= dec(XY) <= 127.
string URIDecode(const string &from) {
  string retstr;

  // Loop through the characters in the string.
  for (unsigned int pos = 0; pos < from.length(); pos++) {
    // note: use pos+n<from.length() instead of pos<from.length-n
    // to avoid overflow problems with unsigned int values
    char c1 = from[pos];
    char c2 = (pos+1 < from.length()) ? toupper(from[pos+1]) : ' ';
    char c3 = (pos+2 < from.length()) ? toupper(from[pos+2]) : ' ';

    // Special case the '+' for old encoders.
    if (c1 == '+') {
      retstr.append(1, ' ');
      continue;
    }

    // Is this an escape sequence?
    if (c1 != '%') {
      retstr.append(1, c1);
      continue;
    }

    // Yes.  Are the next two characters hex digits?
    if (!((('0' <= c2) && (c2 <= '9')) ||
          (('A' <= c2) && (c2 <= 'F')))) {
      retstr.append(1, c1);
      continue;
    }
    if (!((('0' <= c3) && (c3 <= '9')) ||
           (('A' <= c3) && (c3 <= 'F')))) {
      retstr.append(1, c1);
      continue;
    }

    // Yes.  Convert to a code.
    uint8_t code = 0;
    if (c2 >= 'A') {
      code = 16 * (10 + (c2 - 'A'));
    } else {
      code = 16 * (c2 - '0');
    }
    if (c3 >= 'A') {
      code += 10 + (c3 - 'A');
    } else {
      code += (c3 - '0');
    }

    // Is the code reasonable?
    if (!((code >= 32) && (code <= 127))) {
      retstr.append(1, c1);
      continue;
    }

    // Great!  Convert and append.
    retstr.append(1, static_cast<char>(code));
    pos += 2;
  }
  return retstr;
}

void URLParser::Parse(const string &url) {
  url_ = url;

  // Split the URL into the path and the args components.
  vector<string> ps;
  boost::split(ps, url, boost::is_any_of("?"));
  if (ps.size() < 1)
    return;

  // Store the URI-decoded path.
  path_ = URIDecode(ps[0]);

  if (ps.size() < 2)
    return;

  // Split the args into each field=val; chunk.
  vector<string> vals;
  boost::split(vals, ps[1], boost::is_any_of("&"));

  // Iterate through the chunks.
  for (unsigned int i = 0; i < vals.size(); i++) {
    // Split the chunk into field, value.
    string val = vals[i];
    vector<string> fv;
    boost::split(fv, val, boost::is_any_of("="));
    if (fv.size() == 2) {
      // Add the field, value to the args_ map.
      args_[URIDecode(fv[0])] = URIDecode(fv[1]);
    }
  }
}

uint16_t GetRandPort() {
  uint16_t portnum = 10000;
  portnum += ((uint16_t) getpid()) % 25000;
  portnum += ((uint16_t) rand()) % 5000;  // NOLINT(runtime/threadsafe_fn)
  return portnum;
}

int WrappedRead(int fd, unsigned char *buf, int read_len) {
  int res;
  while (1) {
    res = read(fd, buf, read_len);
    if (res == -1) {
      if ((errno == EAGAIN) || (errno == EINTR))
        continue;
    }
    break;
  }
  return res;
}

int WrappedWrite(int fd, const unsigned char *buf, int write_len) {
  int res, written_so_far = 0;

  while (written_so_far < write_len) {
    res = write(fd, buf + written_so_far, write_len - written_so_far);
    if (res == -1) {
      if ((errno == EAGAIN) || (errno == EINTR))
        continue;
      break;
    }
    if (res == 0)
      break;
    written_so_far += res;
  }
  return written_so_far;
}

bool ConnectToServer(const string &host_name, uint16_t port_num,
                     int *client_fd) {
  struct addrinfo hints, *results, *r;
  int client_sock, ret_val;
  char port_str[10];

  // Convert the port number to a C-style string.
  snprintf(port_str, sizeof(port_str), "%hu", port_num);

  // Zero out the hints data structure using memset.
  memset(&hints, 0, sizeof(hints));

  // Indicate we're happy with either AF_INET or AF_INET6 addresses.
  hints.ai_family = AF_UNSPEC;

  // Constrain the answers to SOCK_STREAM addresses.
  hints.ai_socktype = SOCK_STREAM;

  // Do the lookup.
  if ((ret_val = getaddrinfo(host_name.c_str(),
                            port_str,
                            &hints,
                            &results)) != 0) {
    cerr << "getaddrinfo failed: ";
    cerr << gai_strerror(ret_val) << endl;
    return false;
  }

  // Loop through, trying each out until one succeeds.
  for (r = results; r != nullptr; r = r->ai_next) {
    // Try manufacturing the socket.
    if ((client_sock = socket(r->ai_family, SOCK_STREAM, 0)) == -1) {
      continue;
    }
    // Try connecting to the peer.
    if (connect(client_sock, r->ai_addr, r->ai_addrlen) == -1) {
      continue;
    }
    *client_fd = client_sock;
    freeaddrinfo(results);
    return true;
  }
  freeaddrinfo(results);
  return false;
}

}  // namespace hw4
