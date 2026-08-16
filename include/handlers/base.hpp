#pragma once

#include "interface.hpp"

class BaseHandler : public IHandler {
protected:
  std::string basePath_;
  std::string name_;

  // Utility functions
  static crow::response bad_request(const std::string &message);
  static crow::response not_found(const std::string &message);
  static crow::response internal(const std::string &message);

public:
  explicit BaseHandler(const std::string &basePath);
  virtual ~BaseHandler() = default;
  std::string get_name() const override;
};
