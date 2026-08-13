#include "handlers/base.hpp"
#include <crow/common.h>
#include <crow/http_response.h>
#include <crow/json.h>

BaseHandler::BaseHandler(const std::string &basePath)
    : basePath_{basePath}, name_{"BaseHandler"} {}

std::string BaseHandler::get_name() const { return this->name_; }

crow::response BaseHandler::bad_request(const std::string &message) {
  crow::json::wvalue res;
  res["status"] = "error";
  res["message"] = message;

  return crow::response(crow::BAD_REQUEST, res);
}

crow::response BaseHandler::not_found(const std::string &message) {
  crow::json::wvalue res;
  res["status"] = "error";
  res["message"] = message;

  return crow::response(crow::NOT_FOUND, res);
}

crow::response BaseHandler::internal(const std::string &message) {
  crow::json::wvalue res;
  res["status"] = "error";
  res["message"] = message;

  return crow::response(crow::INTERNAL_SERVER_ERROR, res);
}
