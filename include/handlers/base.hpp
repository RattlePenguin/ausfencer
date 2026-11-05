#pragma once

#include "interface.hpp"
#include <sqlite3.h>
#include <system_error>
#include <sqlite_orm/sqlite_orm.h>

class BaseHandler : public IHandler {
protected:
  std::string basePath_;
  std::string name_;

  // Utility functions
  static crow::response bad_request(const std::string &message);
  static crow::response not_found(const std::string &message);
  static crow::response internal(const std::string &message);

  template <typename Action> static crow::response checked(Action action) {
    try {
      return action();
    } catch (const std::invalid_argument &e) {
      return bad_request(e.what());
    } catch (const std::system_error &e) {
      if (e.code().category() == sqlite_orm::get_sqlite_error_category() &&
          (e.code().value() & 0xff) == SQLITE_CONSTRAINT) {
        crow::json::wvalue body;
        body["status"] = "error";
        body["message"] = "Participant reference conflicts with stored history";
        return crow::response(409, body);
      }
      return internal("Database operation failed");
    } catch (const std::exception &) {
      return internal("Request failed");
    }
  }

public:
  explicit BaseHandler(const std::string &basePath);
  virtual ~BaseHandler() = default;
  std::string get_name() const override;
};
