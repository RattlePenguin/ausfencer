#include "db_manager.hpp"
#include <iostream>
#include <stdexcept>

DbManager::DbManager(const std::string& dbPath) : dbPath_(dbPath) {}

DbManager::~DbManager() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DbManager::init() {
    std::lock_guard<std::mutex> lock(dbMutex_);

    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[DbManager] Failed to open SQLite database (" << dbPath_ << "): "
                  << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Enable WAL mode for concurrent multi-phone access performance
    char* errMsgs = nullptr;
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &errMsgs);
    if (errMsgs) {
        sqlite3_free(errMsgs);
    }

    // Create table if it doesn't exist
    const char* createTableSql = R"(
        CREATE TABLE IF NOT EXISTS bouts (
            id TEXT PRIMARY KEY,
            left_fencer_name TEXT NOT NULL,
            right_fencer_name TEXT NOT NULL,
            left_score INTEGER NOT NULL DEFAULT 0,
            right_score INTEGER NOT NULL DEFAULT 0,
            time_remaining_seconds INTEGER NOT NULL DEFAULT 180,
            time_running INTEGER NOT NULL DEFAULT 0,
            left_yellow_cards INTEGER NOT NULL DEFAULT 0,
            right_yellow_cards INTEGER NOT NULL DEFAULT 0,
            left_red_cards INTEGER NOT NULL DEFAULT 0,
            right_red_cards INTEGER NOT NULL DEFAULT 0,
            weapon TEXT NOT NULL DEFAULT 'Foil',
            status TEXT NOT NULL DEFAULT 'NotStarted',
            created_at TEXT NOT NULL,
            finished_at TEXT
        );
    )";

    rc = sqlite3_exec(db_, createTableSql, nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        std::cerr << "[DbManager] Table creation error: " << (errMsgs ? errMsgs : "Unknown") << std::endl;
        if (errMsgs) sqlite3_free(errMsgs);
        return false;
    }

    return true;
}

bool DbManager::createBout(Bout& bout) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    if (bout.id.empty()) {
        bout.id = generateUUID();
    }
    if (bout.createdAt.empty()) {
        bout.createdAt = currentISOTimestamp();
    }

    const char* sql = R"(
        INSERT INTO bouts (
            id, left_fencer_name, right_fencer_name, left_score, right_score,
            time_remaining_seconds, time_running, left_yellow_cards, right_yellow_cards,
            left_red_cards, right_red_cards, weapon, status, created_at, finished_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DbManager] Insert prepare error: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    std::string weaponStr = weaponToString(bout.weapon);
    std::string statusStr = boutStatusToString(bout.status);

    sqlite3_bind_text(stmt, 1, bout.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, bout.leftFencerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, bout.rightFencerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, bout.leftScore);
    sqlite3_bind_int(stmt, 5, bout.rightScore);
    sqlite3_bind_int(stmt, 6, bout.timeRemainingSeconds);
    sqlite3_bind_int(stmt, 7, bout.timeRunning ? 1 : 0);
    sqlite3_bind_int(stmt, 8, bout.leftYellowCards);
    sqlite3_bind_int(stmt, 9, bout.rightYellowCards);
    sqlite3_bind_int(stmt, 10, bout.leftRedCards);
    sqlite3_bind_int(stmt, 11, bout.rightRedCards);
    sqlite3_bind_text(stmt, 12, weaponStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 13, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 14, bout.createdAt.c_str(), -1, SQLITE_TRANSIENT);

    if (bout.finishedAt.has_value()) {
        sqlite3_bind_text(stmt, 15, bout.finishedAt.value().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 15);
    }

    int stepResult = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return stepResult == SQLITE_DONE;
}

static Bout rowToBout(sqlite3_stmt* stmt) {
    Bout b;
    b.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    b.leftFencerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    b.rightFencerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    b.leftScore = sqlite3_column_int(stmt, 3);
    b.rightScore = sqlite3_column_int(stmt, 4);
    b.timeRemainingSeconds = sqlite3_column_int(stmt, 5);
    b.timeRunning = sqlite3_column_int(stmt, 6) != 0;
    b.leftYellowCards = sqlite3_column_int(stmt, 7);
    b.rightYellowCards = sqlite3_column_int(stmt, 8);
    b.leftRedCards = sqlite3_column_int(stmt, 9);
    b.rightRedCards = sqlite3_column_int(stmt, 10);

    const char* weaponText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    if (weaponText) b.weapon = stringToWeapon(weaponText);

    const char* statusText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    if (statusText) b.status = stringToBoutStatus(statusText);

    const char* createdText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
    if (createdText) b.createdAt = createdText;

    const char* finishedText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
    if (finishedText) {
        b.finishedAt = std::string(finishedText);
    } else {
        b.finishedAt = std::nullopt;
    }

    return b;
}

std::vector<Bout> DbManager::getAllBouts() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Bout> bouts;

    const char* sql = "SELECT id, left_fencer_name, right_fencer_name, left_score, right_score, time_remaining_seconds, time_running, left_yellow_cards, right_yellow_cards, left_red_cards, right_red_cards, weapon, status, created_at, finished_at FROM bouts ORDER BY created_at DESC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            bouts.push_back(rowToBout(stmt));
        }
        sqlite3_finalize(stmt);
    }

    return bouts;
}

std::optional<Bout> DbManager::getBoutById(const std::string& id) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    const char* sql = "SELECT id, left_fencer_name, right_fencer_name, left_score, right_score, time_remaining_seconds, time_running, left_yellow_cards, right_yellow_cards, left_red_cards, right_red_cards, weapon, status, created_at, finished_at FROM bouts WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Bout> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToBout(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DbManager::updateBout(const std::string& id, const Bout& updatedBout) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    const char* sql = R"(
        UPDATE bouts SET
            left_fencer_name = ?,
            right_fencer_name = ?,
            left_score = ?,
            right_score = ?,
            time_remaining_seconds = ?,
            time_running = ?,
            left_yellow_cards = ?,
            right_yellow_cards = ?,
            left_red_cards = ?,
            right_red_cards = ?,
            weapon = ?,
            status = ?,
            finished_at = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string weaponStr = weaponToString(updatedBout.weapon);
    std::string statusStr = boutStatusToString(updatedBout.status);

    sqlite3_bind_text(stmt, 1, updatedBout.leftFencerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, updatedBout.rightFencerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, updatedBout.leftScore);
    sqlite3_bind_int(stmt, 4, updatedBout.rightScore);
    sqlite3_bind_int(stmt, 5, updatedBout.timeRemainingSeconds);
    sqlite3_bind_int(stmt, 6, updatedBout.timeRunning ? 1 : 0);
    sqlite3_bind_int(stmt, 7, updatedBout.leftYellowCards);
    sqlite3_bind_int(stmt, 8, updatedBout.rightYellowCards);
    sqlite3_bind_int(stmt, 9, updatedBout.leftRedCards);
    sqlite3_bind_int(stmt, 10, updatedBout.rightRedCards);
    sqlite3_bind_text(stmt, 11, weaponStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, statusStr.c_str(), -1, SQLITE_TRANSIENT);

    if (updatedBout.finishedAt.has_value()) {
        sqlite3_bind_text(stmt, 13, updatedBout.finishedAt.value().c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 13);
    }

    sqlite3_bind_text(stmt, 14, id.c_str(), -1, SQLITE_TRANSIENT);

    int stepResult = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return stepResult == SQLITE_DONE;
}

bool DbManager::deleteBout(const std::string& id) {
    std::lock_guard<std::mutex> lock(dbMutex_);

    const char* sql = "DELETE FROM bouts WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    int stepResult = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return stepResult == SQLITE_DONE;
}
