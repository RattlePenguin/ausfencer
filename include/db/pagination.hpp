#pragma once

#include <cstdint>
#include <stdexcept>

// Shared by repositories and HTTP parsing. SQLite offsets are signed 64-bit.
inline std::int64_t page_offset(int page, int limit) {
  if (page < 1 || limit < 1 || limit > 100)
    throw std::invalid_argument("page must be positive; limit must be 1-100");
  return (static_cast<std::int64_t>(page) - 1) * limit;
}
