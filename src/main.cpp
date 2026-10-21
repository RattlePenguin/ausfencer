#include <iostream>

#include "server.hpp"
#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"

int main(int argc, char *argv[]) {
  try {
    ServerConfig config{};
    std::string directory, approval;
    bool seed = false;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--help") {
        std::cout << "aus-fencer [--db PATH] [--preview DIR | --seed DIR --approve SHA256]\n";
        return 0;
      }
      if (i + 1 >= argc) throw std::invalid_argument("Missing value for " + arg);
      std::string value = argv[++i];
      if (arg == "--db") config.database = value;
      else if (arg == "--preview" || arg == "--seed") {
        if (!directory.empty()) throw std::invalid_argument("Choose one fixture operation");
        directory = value; seed = arg == "--seed";
      } else if (arg == "--approve") approval = value;
      else throw std::invalid_argument("Unknown option " + arg);
    }
    if (!directory.empty()) {
      auto data = load_fixture(directory);
      if (seed && approval != data.checksum)
        throw std::invalid_argument("Preview the fixture and supply its checksum with --approve");
      FixtureRepo repo(std::make_shared<DbManager>(config.database));
      repo.import_fixture(data, seed);
      std::cout << (seed ? "Applied" : "Preview") << " synthetic fixture: " << data.fencers.size()
                << " fencers, " << data.events.size() << " events, " << data.bouts.size()
                << " observations\nSHA256 " << data.checksum << '\n';
      return 0;
    }
    if (!approval.empty()) throw std::invalid_argument("--approve requires --seed");
    Server server(config);
    server.start();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
