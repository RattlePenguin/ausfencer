#pragma once

#include "middleware/logger.hpp"
#include <crow/app.h>
#include <crow/middlewares/cors.h>

using App = crow::App<crow::CORSHandler, LoggerMiddleware>;

class IHandler {
  virtual ~IHandler() = default;
  virtual void registerRoutes(App &app);
};
