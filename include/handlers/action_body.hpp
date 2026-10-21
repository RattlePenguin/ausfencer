#pragma once

#include <crow/json.h>
#include <optional>
#include <stdexcept>

class ActionBody {
  crow::json::rvalue json_;
public:
  explicit ActionBody(const std::string &body) {
    if (body.size() > 8192) throw std::invalid_argument("Request body exceeds limit");
    json_ = crow::json::load(body);
    if (!json_ || json_.t() != crow::json::type::Object)
      throw std::invalid_argument("A JSON object is required");
  }
  std::string text(const char *key) const {
    if (!json_.has(key) || json_[key].t() != crow::json::type::String)
      throw std::invalid_argument(std::string("Missing text field: ") + key);
    return json_[key].s();
  }
  std::optional<int> number(const char *key) const {
    if (!json_.has(key) || json_[key].t() == crow::json::type::Null) return std::nullopt;
    if (json_[key].t() != crow::json::type::Number ||
        (json_[key].nt() != crow::json::num_type::Signed_integer &&
         json_[key].nt() != crow::json::num_type::Unsigned_integer))
      throw std::invalid_argument("Whole numbers are required");
    if (json_[key].nt() == crow::json::num_type::Unsigned_integer) {
      auto value = json_[key].u();
      if (value > 1000000000) throw std::invalid_argument("Number outside allowed range");
      return static_cast<int>(value);
    }
    auto value = json_[key].i();
    if (value < 0 || value > 1000000000) throw std::invalid_argument("Number outside allowed range");
    return static_cast<int>(value);
  }
  int integer(const char *key) const {
    auto value = number(key);
    if (!value) throw std::invalid_argument(std::string("Missing integer: ") + key);
    return *value;
  }
  bool boolean(const char *key) const {
    if (!json_.has(key) || (json_[key].t() != crow::json::type::True && json_[key].t() != crow::json::type::False))
      throw std::invalid_argument(std::string("Missing boolean: ") + key);
    return json_[key].b();
  }
};
