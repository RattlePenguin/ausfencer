#include <iostream>

#include "server.hpp"
#include "db/repos/fixtures.hpp"
#include "services/fixtures.hpp"
#include "db/repos/accounts.hpp"

int main(int argc, char *argv[]) {
  try {
    ServerConfig config{};
    std::string directory, approval;
    std::string account_label, account_role{"member"};
    bool role_requested = false;
    bool seed = false, reset = false;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--help") {
        std::cout << "aus-fencer [--db PATH] [--preview DIR | --seed DIR | --reset-demo DIR] [--approve SHA256]\n";
        std::cout << "aus-fencer --db DEMO_PATH --demo-account LABEL [--role member|moderator]\n";
        return 0;
      }
      if (i + 1 >= argc) throw std::invalid_argument("Missing value for " + arg);
      std::string value = argv[++i];
      if (arg == "--db") config.database = value;
      else if (arg == "--preview" || arg == "--seed" || arg == "--reset-demo") {
        if (!directory.empty()) throw std::invalid_argument("Choose one fixture operation");
        directory = value; seed = arg != "--preview"; reset = arg == "--reset-demo";
      } else if (arg == "--approve") approval = value;
      else if (arg == "--demo-account") account_label = value;
      else if (arg == "--role") { account_role = value; role_requested = true; }
      else throw std::invalid_argument("Unknown option " + arg);
    }
    if (role_requested && account_label.empty()) throw std::invalid_argument("--role requires --demo-account");
    if (!account_label.empty()) {
      if (!directory.empty() || !approval.empty()) throw std::invalid_argument("Run account creation separately from fixture operations");
      AccountRepo accounts(std::make_shared<DbManager>(config.database));
      std::cout << accounts.create_demo_session(account_label,account_role) << '\n';
      return 0;
    }
    if (!directory.empty()) {
      auto data = load_fixture(directory);
      if (seed && approval != data.checksum)
        throw std::invalid_argument("Preview the fixture and supply its checksum with --approve");
      FixtureRepo repo(std::make_shared<DbManager>(config.database));
      if (!seed && !approval.empty()) throw std::invalid_argument("Approval requires --seed or --reset-demo");
      repo.import_fixture(data, seed, reset);
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
