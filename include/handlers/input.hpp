#pragma once

#include "db/pagination.hpp"
#include <crow/json.h>
#include <crow/http_request.h>
#include <charconv>
#include <cctype>
#include <limits>
#include <stdexcept>
#include <string>

namespace input {
inline crow::json::rvalue object(const std::string &body) {
  auto json = crow::json::load(body);
  if (!json || json.t() != crow::json::type::Object)
    throw std::invalid_argument("Expected a JSON object");
  return json;
}

inline const crow::json::rvalue &field(const crow::json::rvalue &json,
                                       const std::string &key) {
  if (!json.has(key)) throw std::invalid_argument("Missing field: " + key);
  return json[key];
}

inline std::int64_t integer(const crow::json::rvalue &json, const std::string &key,
                            std::int64_t min, std::int64_t max) {
  const auto &v = field(json, key);
  using N = crow::json::num_type;
  if (v.t() != crow::json::type::Number ||
      (v.nt() != N::Signed_integer && v.nt() != N::Unsigned_integer))
    throw std::invalid_argument("Expected integer: " + key);
  std::int64_t value;
  try {
    if (v.nt() == N::Unsigned_integer &&
        v.u() > static_cast<std::uint64_t>(max))
      throw std::invalid_argument("Out of range: " + key);
    value = v.i();
  } catch (const std::runtime_error &) {
    throw std::invalid_argument("Out of range: " + key);
  }
  if (value < min || value > max)
    throw std::invalid_argument("Out of range: " + key);
  return value;
}

inline std::string name(const crow::json::rvalue &json, const std::string &key) {
  const auto &v = field(json, key);
  if (v.t() != crow::json::type::String)
    throw std::invalid_argument("Expected string: " + key);
  std::string s = v.s();
  if (s.empty() || s.size() > 100 || s.find_first_not_of(" \t\r\n\f\v") == s.npos)
    throw std::invalid_argument("Name must contain 1-100 bytes: " + key);
  return s;
}

inline int query(const crow::request &req, const char *key, int fallback) {
  const char *raw = req.url_params.get(key);
  if (!raw) return fallback;
  std::string s(raw);
  int value = 0;
  const auto result = std::from_chars(s.data(), s.data() + s.size(), value);
  if (s.empty() || s[0] < '0' || s[0] > '9' ||
      result.ec != std::errc{} || result.ptr != s.data() + s.size() || value < 1)
    throw std::invalid_argument(std::string("Invalid query: ") + key);
  return value;
}
} // namespace input
