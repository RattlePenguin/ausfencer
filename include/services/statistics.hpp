#pragma once

#include "db/db_manager.hpp"
#include "models/Tracker.hpp"

TrackerStats calculate_statistics(const DbRows &history);
