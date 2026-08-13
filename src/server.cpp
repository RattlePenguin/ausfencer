#include "server.hpp"
#include "handlers/interface.hpp"
#include <crow/logging.h>
#include <crow/middlewares/cors.h>
#include <memory>

Server::Server(const ServerConfig &config) : config_{config} {
  // Gives Warning, Error and Critical logs, but not Debug and Info
  // tmpl uses a switch statement but I defaulted to Warning
  crow::logger::setLogLevel(crow::LogLevel::Warning);

  app_ = std::make_unique<App>();

  // TODO write comment about this part
  auto &cors = app_->get_middleware<crow::CORSHandler>();
  cors.global()
      .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method,
               "OPTIONS"_method)
      .headers("Content-type", "Authorization")
      .origin("*")
      .prefix("/api")
      .max_age(3600);
}

void Server::setup() {
  // add handlers for each type here
  // referee : POST
  // fencers : CRUD
  // bouts : CRUD
  // tournaments : CRUD
};

void Server::start() {
  this->setup();
  std::cout << "Server started on port " << config_.port << std::endl;
  app_->port(config_.port).multithreaded().run(); // default run command
}

void Server::add_handler(std::shared_ptr<IHandler> handler) {
  handlers_.push_back(handler);
}
