#include "handlers/fencers.hpp"
#include "models/Fencer.hpp"

FencerHandler::FencerHandler(const std::string &basePath)
    : BaseHandler(basePath) {
  this->name_ = "FencerHandler";
  this->lastID_ = 0;

  // mock data to work with
  this->fencers_[++this->lastID_] =
      Fencer{this->lastID_, "Allan", "Goodman", 2002};
  this->fencers_[++this->lastID_] =
      Fencer{this->lastID_, "Bennett", "Chan", 2003};
};
