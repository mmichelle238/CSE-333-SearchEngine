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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace hw4 {

static const char *kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest *request) {
  // Use "WrappedRead" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header. Be sure to try and read in
  // a large amount of bytes each time you call WrappedRead.
  //
  // Once you've seen the request header, use ParseRequest()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes GetNextRequest()!

  // STEP 1:
  bool full_request_sent = buffer_.find(kHeaderEnd) != string::npos;
  while (!full_request_sent) {
    unsigned char buf[1024] = { 0 };
    int bytes_read = WrappedRead(fd_,
                                 buf,
                                 1023);
    string temp((const char *) buf);

    buffer_ += temp;
    if (buffer_.find(kHeaderEnd) != string::npos) {
      full_request_sent = true;
    }

    if (bytes_read == 0) {  // the connection dropped somehow
      full_request_sent = true;
    } else if (bytes_read < 0) {
      return false;  // something bad happened
    }
  }

  size_t end_of_request = buffer_.find(kHeaderEnd);
  // there is AT LEAST ONE complete header inside here
  if (end_of_request != string::npos) {
    const string full_request =
      buffer_.substr(0, end_of_request + kHeaderEndLen);

    *request = ParseRequest(full_request);

    buffer_ = buffer_.substr(end_of_request + kHeaderEndLen,
                            buffer_.length() - full_request.length());
  }

  return true;  // You may want to change this.
}

bool HttpConnection::WriteResponse(const HttpResponse &response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char *>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string &request) const {
  HttpRequest req("/");  // by default, get "/".

  // Split the request into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
  // at HttpRequest.h for details about the HTTP header format that
  // you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.
  //
  // Note that you may assume the request you are parsing is correctly
  // formatted. If for some reason you encounter a header that is
  // malformed, you may skip that line.

  // STEP 2:

  // split the request into lines
  vector<string> lines;
  boost::split(lines, request,
               boost::is_any_of("\r\n"), boost::token_compress_on);

  string first_line = lines[0];
  boost::algorithm::trim(first_line);

  // we actually use this once for the start-line
  vector<string> header_tokens;
  boost::split(header_tokens, first_line, boost::is_any_of(" "),
               boost::token_compress_on);


  // can assume start-line correctly formatted
  if (header_tokens.size() < 2) {
    return req;
  }
  req.set_uri(header_tokens[1]);

  for (uint i = 1; i < lines.size(); i++) {
    string curr_line = lines[i];
    boost::algorithm::trim(curr_line);
    boost::split(header_tokens, curr_line, boost::is_any_of(":"),
        boost::token_compress_on);

    // second token (if it exists) will be fieldvalue
    if (header_tokens.size() < 2) {
      continue;
    }

    string header_name = header_tokens[0];
    boost::algorithm::trim(header_name);
    boost::algorithm::to_lower(header_name);

    string header_value = header_tokens[1];
    boost::algorithm::trim(header_value);
    req.AddHeader(header_name, header_value);
  }

  return req;
}

}  // namespace hw4
