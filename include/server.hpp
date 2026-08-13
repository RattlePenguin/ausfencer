#pragma once

#include "middleware/logger.hpp"
#include <crow.h>
#include <crow/app.h>
#include <crow/middlewares/cors.h>
#include <cstdint>
#include <handlers/interface.hpp>
#include <memory>

struct ServerConfig {
  uint16_t port{8080};
  uint16_t threads{2};
};

using App = crow::App<crow::CORSHandler, LoggerMiddleware>;

class Server {
  ServerConfig config_;
  std::unique_ptr<App> app_;
  std::unique_ptr<DbManager> db_;
  std::vector<std::shared_ptr<IHandler>> handlers_;

  bool is_running_{false};

public:
  explicit Server(const ServerConfig &config = ServerConfig());
  ~Server() = default;

  void setup();
  void start();
  void add_handler(std::shared_ptr<IHandler> handler);
};
