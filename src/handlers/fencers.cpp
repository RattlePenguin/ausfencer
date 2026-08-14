#include "handlers/fencers.hpp"
#include "models/Fencer.hpp"
#include <crow/common.h>
#include <crow/http_request.h>
#include <crow/json.h>
#include <exception>

FencerHandler::FencerHandler(const std::string &basePath)
    : BaseHandler(basePath) {
  this->name_ = "FencerHandler";
};

      };
