#pragma once

#include "../db_manager.hpp"

class IRepo {
public:
  virtual ~IRepo() = default;
  virtual std::string get_name() const = 0;
};
