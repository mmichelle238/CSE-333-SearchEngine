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

#include "./Utils.h"

#include <stdio.h>   // for fprintf()
#include <string.h>  // for strcmp()

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw3 {

const uint32_t kMagicNumber = 0xCAFEF00D;

// Initialize the "CRC32::table_is_initialized" static member variable.
bool CRC32::table_is_initialized_ = false;

// We need this declaration here so we can refer to the table_ below.
uint32_t CRC32::table_[256];

static uint32_t CRC32Reflect(uint32_t reflect_me, const char c) {
  uint32_t val = 0;

  // Swap bit 0 for bit 7, bit 1 for bit 6, etc....
  for (int position = 1; position < (c + 1); position++) {
    if (reflect_me & 1) {
      val |= (1 << (c - position));
    }
    reflect_me >>= 1;
  }
  return val;
}

CRC32::CRC32(void) {
  if (!table_is_initialized_) {
    // Yes, there is a potential race condition if multiple threads
    // construct the very first CRC32() objects simultaneously.  We'll
    // live with it. :(
    Initialize();
    table_is_initialized_ = true;
  }

  // Prep the CRC32 state.
  finalized_ = false;
  crc_state_ = 0xFFFFFFFF;
}

void CRC32::FoldByteIntoCRC(uint8_t next_byte) {
  Verify333(finalized_ != true);
  crc_state_ = (crc_state_ >> 8) ^ table_[(crc_state_ & 0xFF) ^ next_byte];
}

uint32_t CRC32::GetFinalCRC(void) {
  if (!finalized_) {
    finalized_ = true;
    crc_state_ ^= 0xffffffff;
  }
  return crc_state_;
}

void CRC32::Initialize(void) {
  static const uint32_t kPolynomial = 0x04C11DB7;

  // Initialize the CRC32 lookup table; the table's 256 values
  // represent ASCII character codes.
  for (uint32_t code = 0; code <= 0xFF; code++) {
    CRC32::table_[code] = CRC32Reflect(code, 8) << 24;
    for (int position = 0; position < 8; position++) {
      table_[code] =
        (table_[code] << 1) ^ ((table_[code] & (1 << 31)) ? kPolynomial : 0);
    }
    table_[code] = CRC32Reflect(table_[code], 32);
  }
}

FILE* FileDup(FILE *f) {
  // Duplicate the underlying file descriptor using dup().
  int new_fd = dup(fileno(f));
  Verify333(new_fd != -1);

  // Use fdopen() to open a new (FILE *) for the dup()'ed descriptor.
  FILE *retfile = fdopen(new_fd, "rb");
  Verify333(retfile != nullptr);

  // Return the new (FILE *).
  return retfile;
}

}  // namespace hw3
