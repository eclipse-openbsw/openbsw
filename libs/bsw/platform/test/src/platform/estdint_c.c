// Copyright 2024 Accenture.

/*
 * This file is a C-File to make sure, estdint.h can be included in C-sources as well.
 */
#include "platform/estdint.h"

// clang-format off
size_t get_size_uint8_t()  { return sizeof(uint8_t);  }
size_t get_size_uint16_t() { return sizeof(uint16_t); }
size_t get_size_uint32_t() { return sizeof(uint32_t); }
size_t get_size_uint64_t() { return sizeof(uint64_t); }

size_t get_size_int8_t()  { return sizeof(int8_t);  }
size_t get_size_int16_t() { return sizeof(int16_t); }
size_t get_size_int32_t() { return sizeof(int32_t); }
size_t get_size_int64_t() { return sizeof(int64_t); }

size_t get_size_size_t()    { return sizeof(size_t); }
size_t get_size_ptrdiff_t() { return sizeof(ptrdiff_t); }

// clang-format on
