#include <mutex>
#include <shared_mutex>
#include <sqlite3.h>

#include "../models/Fencer.hpp"

class DbManager {
  std::string db_name_;
  sqlite3 *db_;

  // Expose shared lock to repositories that want to read
  std::shared_lock<std::shared_mutex> acquire_read_lock() const;

  // Unique lock for writes
  std::unique_lock<std::shared_mutex> acquire_write_lock() const;

public:
  explicit DbManager(const std::string &db_name);
  ~DbManager();
};
