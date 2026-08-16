#pragma once

#include <string>

class IRepo {
public:
  virtual ~IRepo() = default;
  virtual std::string get_name() const = 0;
};
