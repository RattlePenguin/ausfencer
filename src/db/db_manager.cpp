#include "db/db_manager.hpp"
#include "db/migrations.hpp"
#include <stdexcept>

DbManager::DbManager(const std::string &db_name)
    : db_name_{db_name}, storage_{create_db_storage(db_name)} {

  storage_.open_forever();
  query("PRAGMA foreign_keys = ON");
  query("PRAGMA busy_timeout = 5000");
  // Keep legacy records while adding versioned domain tables separately.
  check_tracker_schema(*this);
  storage_.sync_schema(true);
  migrate_tracker(*this);
}

DbRows DbManager::query(const std::string &statement,
                         const std::vector<std::string> &parameters) {
  auto *connection = storage_.connection();
  sqlite3_stmt *raw{nullptr};
  if (sqlite3_prepare_v2(connection, statement.c_str(), -1, &raw, nullptr) != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(connection));
  std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> prepared(raw, sqlite3_finalize);
  if (sqlite3_bind_parameter_count(raw) != static_cast<int>(parameters.size()))
    throw std::invalid_argument("SQL parameter count mismatch");
  for (size_t i = 0; i < parameters.size(); ++i) {
    if (sqlite3_bind_text(raw, i + 1, parameters[i].data(), parameters[i].size(), SQLITE_TRANSIENT) != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(connection));
  }
  DbRows rows;
  int result;
  while ((result = sqlite3_step(raw)) == SQLITE_ROW) {
    DbRow row;
    for (int i = 0; i < sqlite3_column_count(raw); ++i) {
      const auto *value = sqlite3_column_text(raw, i);
      if (value) row[sqlite3_column_name(raw, i)] =
        std::string(reinterpret_cast<const char *>(value),sqlite3_column_bytes(raw,i));
    }
    rows.push_back(std::move(row));
  }
  if (result != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(connection));
  return rows;
}
