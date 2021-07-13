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

#ifndef HW3_UTILS_H_
#define HW3_UTILS_H_

#include <stdint.h>     // [C++ doesn't yet standardize <cstdint>.]
#include <arpa/inet.h>  // For htonl(), etc.
#include <unistd.h>     // for dup().
#include <cstdio>       // for fdopen(), (FILE *).

// Useful #defines, macros, utility functions, and utility classes.

namespace hw3 {

// The first four bytes of a valid index file.
//
// This magic number is the last thing written to an index file, and thus
// plays the role of a commit record.
extern const uint32_t kMagicNumber;


// Macros to convert 64-bit integers between "host order" and "network order".
//
// The on-disk format is big-endian, sometimes called "network order" since
// it's the commonly-used endianness in network protocols.  However, x86 is
// a little endian architecture.  Although <arpa/inet.h> header declares
// macros for converting 16-bit and 32-bit numbers back and forth (see also
// "man htonl"), we need one for 64-bit numbers.
//
// The following (crazy looking!) macros implement the 64-bit version of
// the arpa macros.  So, htonll() -- read aloud as "Host TO Network, Long
// Long" -- means converting from host order to big endian.  On a big-endian
// architecture, this is a no-op, but on a little-endian machine it swaps the
// byte order.
#define ntohll(x) \
  ( ((uint64_t) (ntohl((uint32_t)((x << 32) >> 32))) << 32) |   \
    ntohl(((uint32_t)(x >> 32))) )
#define htonll(x) (ntohll(x))


// A CRC32 object, useful for calculating a checksum.
//
// A checksum is a mathematical operation that calculates a "signature" of
// of some byte array; if the byte array gets corrupted later, a
// newly-calculated checksum won't match.  Checksums are therefore useful for
// validate the integrity of a byte array.
//
// To calculate a checksum, instantiate a CRC32 object and invoke
// FoldByteIntoCRC() repeatedly, once for each byte in the sequence.  Lastly,
// invoke GetFinalCRC() to retrieve the checksum for that byte sequence.  After
// you've called GetFinalCRC(), you cannot fold any additional bytes into
// that CRC32 instance.
//
// If you're curious, you can read about CRCs on wikipedia:
//   http://en.wikipedia.org/wiki/Cyclic_redundancy_check
class CRC32 {
 public:
  CRC32(void);

  // Use this function to fold the next byte into the CRC.
  void FoldByteIntoCRC(uint8_t nextbyte);

  // Once you're done folding bytes into the CRC, use this function to
  // get the final 32-bit CRC value.
  uint32_t GetFinalCRC(void);

 private:
  // Initialize the table_ to the appropriate values according to the
  // CRC32 algorithm.  Needs to be called once per program execution.
  void Initialize(void);

  // This private member variable holds the CRC calculation state.
  uint32_t crc_state_;

  // This bool indicates whether or not the CRC has been finalized.
  bool finalized_;

  // Here, the "static" specifier indicates that the variable table_,
  // which is an array of 256 uint32_t's, is associated  with the
  // CRC32 *class* rather than with each CRC32 object instance.  CRC32
  // object instances can access it, but there is a single copy of
  // this table_, no matter how many object instances exist.  C++
  // initializes the table to all zeroes.
  static uint32_t table_[256];

  // This indicates whether the static table_ has been initialized.
  static bool table_is_initialized_;
};


// Macro for disabling copy constructors and assignment operators.
//
// The idea and name are lifted from Google's C++ style guide but the
// implementation has been updated to C++ 11.  (Thanks, Google!)  The Google
// C++ style has more details on how this works and why it is a good idea:
//    https://google.github.io/styleguide/cppguide.html#Copyable_Movable_Types
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete


// Copies an existing FILE*, so that it can be shared by threads.
//
// Having separate FILE*s is helpful for avoiding potential race conditions
// when multiple threads are accessing the same logical file.
FILE *FileDup(FILE *f);

}  // namespace hw3

#endif  // HW3_UTILS_H_
