#include "server.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

TEST(ServerLifecycle, ValidatesThreadConfiguration) {
  ServerConfig config;
  config.database_path = ":memory:";
  for (int threads : {0, 1}) {
    config.threads = threads;
    EXPECT_THROW(Server{config}, std::invalid_argument);
  }
  config.threads = 2;
  EXPECT_NO_THROW(Server{config});
}

TEST(ServerLifecycle, SetupAndConstructionDoNotSeedPersistentData) {
  char path[] = "/tmp/ausfencer-server-XXXXXX";
  int fd = mkstemp(path);
  ASSERT_GE(fd, 0);
  close(fd);
  struct Cleanup {
    const char *path;
    ~Cleanup() { std::remove(path); }
  } cleanup{path};
  ServerConfig config;
  config.database_path = path;
  for (int restart : {0, 1}) {
    (void)restart;
    Server server(config);
    EXPECT_NO_THROW(server.setup());
    EXPECT_NO_THROW(server.setup());
  }
  DbManager db(path);
  EXPECT_EQ(db.get_storage().count<Fencer>(), 0);
  EXPECT_EQ(db.get_storage().count<Bout>(), 0);
}

TEST(CrowCompatibility, DefaultRequestInitializesConnectionState) {
  crow::request request;
  EXPECT_EQ(request.http_ver_major, 0);
  EXPECT_EQ(request.http_ver_minor, 0);
  EXPECT_FALSE(request.keep_alive);
  EXPECT_FALSE(request.close_connection);
  EXPECT_FALSE(request.upgrade);
}
