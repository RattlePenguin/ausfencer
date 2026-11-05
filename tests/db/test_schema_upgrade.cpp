#include "db/repos/bouts.hpp"
#include "db/repos/fencers.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

TEST(SchemaUpgrade, PreservesLegacyBoutsGuestsAndForeignKeysAcrossReopen) {
  char path[] = "/tmp/ausfencer-upgrade-XXXXXX";
  int fd = mkstemp(path);
  ASSERT_GE(fd, 0);
  close(fd);
  struct Cleanup {
    const char *path;
    ~Cleanup() { std::remove(path); }
  } cleanup{path};
  sqlite3 *raw = nullptr;
  ASSERT_EQ(sqlite3_open(path, &raw), SQLITE_OK);
  const char *legacy = R"(
    CREATE TABLE fencers (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      first_name TEXT NOT NULL, last_name TEXT NOT NULL, birth_year INTEGER NOT NULL);
    CREATE TABLE bouts (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
      left_fencer_id INTEGER NOT NULL, right_fencer_id INTEGER NOT NULL,
      timestamp INTEGER NOT NULL, weapon INTEGER NOT NULL, time INTEGER NOT NULL,
      left_score INTEGER NOT NULL, right_score INTEGER NOT NULL,
      left_yellow INTEGER NOT NULL, right_yellow INTEGER NOT NULL,
      left_red INTEGER NOT NULL, right_red INTEGER NOT NULL,
      FOREIGN KEY(left_fencer_id) REFERENCES fencers(id),
      FOREIGN KEY(right_fencer_id) REFERENCES fencers(id));
    INSERT INTO fencers VALUES (1,'Left','Fencer',2000),(2,'Right','Fencer',2001);
    INSERT INTO bouts VALUES (7,1,2,1700000000,2,180,15,12,1,0,0,1);
  )";
  int result = sqlite3_exec(raw, legacy, nullptr, nullptr, nullptr);
  sqlite3_close(raw);
  ASSERT_EQ(result, SQLITE_OK);
  int guest_id;
  {
    auto db = std::make_shared<DbManager>(path);
    BoutRepo bouts(db);
    FencerRepo fencers(db);
    auto old = bouts.get(7);
    ASSERT_NE(old, nullptr);
    EXPECT_EQ(old->left_fencer_id, 1);
    EXPECT_EQ(old->right_fencer_id, 2);
    EXPECT_EQ(old->timestamp, 1700000000);
    EXPECT_EQ(old->weapon, Sabre);
    EXPECT_EQ(old->time, 180);
    EXPECT_EQ(old->left_score, 15);
    EXPECT_EQ(old->right_score, 12);
    EXPECT_EQ(old->left_yellow, 1);
    EXPECT_EQ(old->right_yellow, 0);
    EXPECT_EQ(old->left_red, 0);
    EXPECT_EQ(old->right_red, 1);
    EXPECT_THROW(fencers.remove(1), std::system_error);
    old->id = 0;
    old->left_fencer_id.reset();
    old->right_fencer_id.reset();
    guest_id = bouts.create(*old);
    EXPECT_GT(guest_id, 7);
    old->left_fencer_id = 999;
    EXPECT_THROW(bouts.create(*old), std::system_error);
  }
  auto db = std::make_shared<DbManager>(path);
  BoutRepo bouts(db);
  ASSERT_NE(bouts.get(7), nullptr);
  auto guest = bouts.get(guest_id);
  ASSERT_NE(guest, nullptr);
  EXPECT_FALSE(guest->left_fencer_id);
  EXPECT_FALSE(guest->right_fencer_id);
  EXPECT_EQ(bouts.get_all_by_fencer(1, 1, 10).size(), 1u);
  EXPECT_EQ(db->get_storage().count<Fencer>(), 2);
  ASSERT_EQ(sqlite3_open(path, &raw), SQLITE_OK);
  sqlite3_stmt *check = nullptr;
  ASSERT_EQ(sqlite3_prepare_v2(raw, "PRAGMA foreign_key_check", -1, &check, nullptr), SQLITE_OK);
  EXPECT_EQ(sqlite3_step(check), SQLITE_DONE); // no orphaned references
  sqlite3_finalize(check);
  sqlite3_close(raw);
}
