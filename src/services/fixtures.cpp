#include "services/fixtures.hpp"
#include <crow/json.h>
#include <openssl/sha.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

FixtureData load_fixture(const std::string &directory) {
  std::string evidence;
  auto read = [&](const std::string &name) {
    std::ifstream file(directory + "/" + name + ".json");
    if (!file) throw std::invalid_argument("Cannot read fixture " + name);
    std::string body((std::istreambuf_iterator<char>(file)), {});
    if (body.size() > 1024 * 1024) throw std::invalid_argument("Fixture exceeds size limit");
    evidence += name + ":" + std::to_string(body.size()) + ":" + body;
    auto json = crow::json::load(body);
    if (!json || json.t() != crow::json::type::List)
      throw std::invalid_argument("Fixture must contain a JSON array: " + name);
    return json;
  };
  auto s = [](const auto &j, const char *key) -> std::string { return j[key].s(); };
  auto n = [](const auto &j, const char *key) -> std::optional<int> {
    if (!j.has(key) || j[key].t() == crow::json::type::Null) return std::nullopt;
    auto value = j[key].i();
    if (value < 0 || value > 1000000000) throw std::invalid_argument("Invalid fixture integer");
    return static_cast<int>(value);
  };
  FixtureData fixture;
  for (const auto &j : read("fencers"))
    fixture.fencers.push_back({n(j,"id").value(), s(j,"name"), s(j,"club"),
      s(j,"state"), s(j,"country"), s(j,"alias"), s(j,"external_id"), j["guardian_required"].b()});
  for (const auto &j : read("events"))
    fixture.events.push_back({s(j,"id"), s(j,"tournament"), s(j,"name"), s(j,"date"),
      s(j,"weapon"), s(j,"category"), s(j,"coverage"), s(j,"timezone")});
  for (const auto &j : read("entries"))
    fixture.entries.push_back({s(j,"event"), n(j,"fencer").value(), s(j,"source_name"),
      s(j,"pool"), n(j,"seed").value(), n(j,"place").value(), j["bye"].b()});
  for (const auto &j : read("bouts"))
    fixture.bouts.push_back({s(j,"id"), s(j,"event"), s(j,"stage"), s(j,"pool"),
      n(j,"left").value(), n(j,"right").value(), n(j,"left_score"), n(j,"right_score"),
      n(j,"winner"), s(j,"status"), s(j,"trust")});
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const unsigned char *>(evidence.data()), evidence.size(), digest);
  std::ostringstream checksum;
  for (auto byte : digest) checksum << std::hex << std::setw(2) << std::setfill('0') << int(byte);
  fixture.checksum = checksum.str();
  return fixture;
}
