#pragma once

#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include <sqlite3.h>
#include "../models/bout.hpp"

class DbManager {
public:
    explicit DbManager(const std::string& dbPath = "fencing.db");
    ~DbManager();

    // Prevent copying
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    bool init();

    // CRUD operations
    bool createBout(Bout& bout);
    std::vector<Bout> getAllBouts();
    std::optional<Bout> getBoutById(const std::string& id);
    bool updateBout(const std::string& id, const Bout& updatedBout);
    bool deleteBout(const std::string& id);

private:
    std::string dbPath_;
    sqlite3* db_ = nullptr;
    std::mutex dbMutex_;

    void executeNonQuery(const std::string& sql);
};
