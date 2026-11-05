#include "server.hpp"
#include "handlers/fencers.hpp"
#include "handlers/bouts.hpp"
#include "handlers/interface.hpp"
#include <crow/logging.h>
#include <crow/middlewares/cors.h>
#include <memory>

Server::Server(const ServerConfig &config) : config_{config} {
  if (config_.threads < 2)
    throw std::invalid_argument("Server threads must be at least 2");
  // Gives logs for given class and above
  // defaulting to Info
  crow::logger::setLogLevel(crow::LogLevel::Info);

  app_ = std::make_unique<App>();
  db_mgr_ = std::make_shared<DbManager>(config_.database_path);

  // TODO Taken from AsaHero, write documentation
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
  if (setup_complete_) return;
  this->add_handler(std::make_shared<FencerHandler>(
      "/api/fencers", std::make_shared<FencerRepo>(this->db_mgr_)));
  this->add_handler(std::make_shared<BoutHandler>(
      "/api/bouts", std::make_shared<BoutRepo>(this->db_mgr_)));
  app_->validate();
  setup_complete_ = true;
}

void Server::start() {
  this->setup();
  std::cout << "Server started on port " << config_.port << std::endl;
  app_->port(config_.port).concurrency(config_.threads).run();
}

void Server::add_handler(std::shared_ptr<IHandler> handler) {
  handlers_.push_back(handler);
  handler->register_routes(*this->app_);
}
