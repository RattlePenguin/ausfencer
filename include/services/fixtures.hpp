#pragma once

#include "models/Tracker.hpp"

FixtureData load_fixture(const std::string &directory);
void validate_fixture(const FixtureData &fixture);
bool valid_competition_date(const std::string &date);
