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

#ifndef HW4_HTTPRESPONSE_H_
#define HW4_HTTPRESPONSE_H_

#include <stdint.h>

#include <map>
#include <string>
#include <sstream>

namespace hw4 {

// This class represents the state of an HTTP response, including the
// headers and body.  Customers (primarily HttpServer.cc) create an
// instance of this class when preparing their response, and they
// use GenerateResponseString() to generate a string-representation
// ready for sending on a socket.
//
// A response has the following format:
//
// [http_protocol] [response_code] [message]\r\n
// [headername]: [headerval]\r\n
// [headername]: [headerval]\r\n
// ... more headers ...
// [headername]: [headerval]\r\n
// [body]
//
// e.g.:
//
// HTTP/1.1 200 OK\r\n
// Content-length: 10\r\n
// \r\n
// Hi there!!

class HttpResponse {
 public:
  HttpResponse() { }
  virtual ~HttpResponse() { }

  void set_protocol(const std::string &protocol) { protocol_ = protocol; }
  void set_response_code(uint16_t code) { response_code_ = code; }
  void set_message(const std::string &msg) { message_ = msg; }
  void set_content_type(const std::string &type) { content_type_ = type; }

  void AppendToBody(const std::string &body_fragment) {
    body_ += body_fragment;
  }

  // A method to generate a std::string of the HTTP response, suitable
  // for writing back to the client.  We automatically generate the
  // "Content-length:" header, and make that be the last header
  // in the block.  The value of the Content-length header is the
  // size of the response body (in bytes).
  std::string GenerateResponseString() const {
    std::stringstream resp;

    resp << protocol_ << " " << response_code_ << " " << message_ << "\r\n";
    if (!content_type_.empty()) {
      resp << "Content-type: " << content_type_ << "\r\n";
    }
    resp << "Content-length: " << body_.size() << "\r\n";
    resp << "\r\n";
    resp << body_;
    return resp.str();
  }

 private:
  // The HTTP protocol string to pass back in the header.
  std::string protocol_;

  // The HTTP response code to pass back in the header.
  uint16_t response_code_;

  // The HTTP response code message to pass back in the header.
  std::string message_;

  // The HTTP content type string to pass back in the header.  Optional .
  std::string content_type_;

  // The body of the response.
  std::string body_;
};

}  // namespace hw4

#endif  // HW4_HTTPRESPONSE_H_
