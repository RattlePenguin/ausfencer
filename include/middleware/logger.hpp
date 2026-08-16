#pragma once

#include <crow.h>
#include <crow/logging.h>

struct LoggerMiddleware {
  struct context {};

  void before_handle(crow::request &req, crow::response &res, context &ctx) {
    CROW_LOG_INFO << req.url << '\n';
  }

  void after_handle(crow::request &req, crow::response &res, context &ctx) {
    CROW_LOG_INFO << req.url << " -> " << res.code << '\n';
  }
};
