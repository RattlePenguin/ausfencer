#pragma once

#include "../middleware/logger.hpp"
#include <crow/app.h>
#include <crow/middlewares/cors.h>

using App = crow::App<crow::CORSHandler, LoggerMiddleware>;

class IHandler {
public:
  virtual ~IHandler() = default;
  virtual void register_routes(App &app) = 0;
  virtual std::string get_name() const = 0;
};
