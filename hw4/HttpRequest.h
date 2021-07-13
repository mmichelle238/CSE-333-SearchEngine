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

#ifndef HW4_HTTPREQUEST_H_
#define HW4_HTTPREQUEST_H_

#include <stdint.h>

#include <map>
#include <string>

namespace hw4 {

// This class represents the state of an HTTP request.  We'll
// only handle "GET"-style requests in this project, which means
// a request has the following format:
//
// GET [URI] [http_protocol]\r\n
// [headername]: [headerval]\r\n
// [headername]: [headerval]\r\n
// ... more headers ...
// [headername]: [headerval]\r\n
// \r\n
//
// e.g.:
//
// GET /foo/bar?baz=bam HTTP/1.1\r\n
// Host: www.news.com\r\n
//
class HttpRequest {
 public:
  HttpRequest() { }
  explicit HttpRequest(const std::string &uri)
    : uri_(uri) { }
  virtual ~HttpRequest() { }

  const std::string& uri() const { return uri_; }
  void set_uri(const std::string& uri) { uri_ = uri; }

  // Returns the value associated with the passed-in header name, or empty
  // string if it does not exist in the header map.  The passed-in name must
  // be entirely lowercase to comply with our implementation of RFC 2616:4.2.
  std::string GetHeaderValue(const std::string &name) const {
    std::map<std::string, std::string>::const_iterator it = headers_.find(name);
    if (it == headers_.end()) {
      return "";
    } else {
      return it->second;
    }
  }

  // Adds a name -> value mapping to the header map, over-writing any existing
  // previous mapping for name.
  void AddHeader(const std::string &name, const std::string &value) {
    headers_[name] = value;
  }

  // Returns the number of headers this HttpRequest contains
  int GetHeaderCount() {
    return headers_.size();
  }

 private:
  // Which URI did the client request?
  std::string uri_;

  // A map from header name to header value, representing
  // all of the headers that the client supplied to us.  The
  // header names are converted to all lower case since RFC
  // 2616:4.2 states that header names are case-insensitive;
  // the header values are retained verbatim.
  std::map<std::string, std::string> headers_;
};

}  // namespace hw4

#endif  // HW4_HTTPREQUEST_H_
