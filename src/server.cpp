#include "server.hpp"
#include "handlers/fencers.hpp"
#include "handlers/tracker.hpp"
#include "handlers/workflows.hpp"
#include "handlers/interface.hpp"
#include <crow/logging.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include <cstdlib>
#include <fstream>

Server::Server(const ServerConfig &config) : config_{config} {
  // Gives logs for given class and above
  // defaulting to Info
  crow::logger::setLogLevel(crow::LogLevel::Info);

  app_ = std::make_unique<App>();
  db_mgr_ = std::make_shared<DbManager>(config_.database);

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
  for (const auto &asset : std::vector<std::pair<std::string,std::string>>{
       {"/","index.html"},{"/styles.css","styles.css"},{"/app.js","app.js"},{"/ui.js","ui.js"},
       {"/profiles.js","profiles.js"},{"/events.js","events.js"},
       {"/forms.js","forms.js"},{"/account.js","account.js"},{"/contribute.js","contribute.js"},
       {"/moderation.js","moderation.js"}}) {
    app_->route_dynamic(asset.first)([name=asset.second] {
      std::ifstream file("web/" + name);
      if (!file) return crow::response(404);
      crow::response response(std::string((std::istreambuf_iterator<char>(file)),{}));
      response.set_header("Content-Type",name == "index.html" ? "text/html; charset=utf-8" :
        name == "styles.css" ? "text/css" : "text/javascript");
      response.set_header("Cache-Control","no-store");
      response.set_header("Content-Security-Policy","default-src 'self'; base-uri 'none'; frame-ancestors 'none'");
      return response;
    });
  }
  add_handler(std::make_shared<TrackerHandler>(std::make_shared<TrackerRepo>(db_mgr_)));
  add_handler(std::make_shared<WorkflowHandler>(db_mgr_));
  const char *token = std::getenv("AUSFENCER_ADMIN_TOKEN");
  this->add_handler(std::make_shared<FencerHandler>(
      "/api/fencers", std::make_shared<FencerRepo>(this->db_mgr_), token ? token : ""));
}

void Server::start() {
  this->setup();
  std::cout << "Server started on port " << config_.port << std::endl;
  app_->bindaddr(config_.bind_address).port(config_.port).concurrency(config_.threads).run();
}

void Server::add_handler(std::shared_ptr<IHandler> handler) {
  handlers_.push_back(handler);
  handler->register_routes(*this->app_);
}
