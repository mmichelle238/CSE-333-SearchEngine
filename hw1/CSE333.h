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

#ifndef HW1_CSE333_H_
#define HW1_CSE333_H_

#include <stdint.h>  // for uint32_t, etc.

// A wrapper for assert that permits side-effects within the
// Verify333() statement.  Borrowed from:
//
//   http://www.acm.uiuc.edu/sigops/roll_your_own/2.a.html

void VerificationFailure(const char *exp, const char *file,
                         const char *basefile, int line);

#define Verify333(exp) if (exp) ; \
  else VerificationFailure(#exp, __FILE__, __BASE_FILE__, __LINE__)

#endif  // HW1_CSE333_H_
