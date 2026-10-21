#include "db/repos/tracker.hpp"
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <sstream>
#include <stdexcept>

namespace {
std::string fold(const std::string &input) {
  UErrorCode status = U_ZERO_ERROR;
  auto normalizer = icu::Normalizer2::getNFKCCasefoldInstance(status);
  icu::UnicodeString output;
  if (U_FAILURE(status)) throw std::runtime_error("Unicode normalizer unavailable");
  normalizer->normalize(icu::UnicodeString::fromUTF8(input), output, status);
  if (U_FAILURE(status)) throw std::invalid_argument("Invalid Unicode name");
  std::string result;
  return output.toUTF8String(result);
}
}

DbRows TrackerRepo::search(const std::string &query, int page, int limit) {
  if (query.size() > 200 || page < 1 || page > 1000000 || limit < 1 || limit > 100)
    throw std::invalid_argument("Invalid search pagination or query length");
  std::vector<std::string> tokens;
  std::istringstream words(fold(query));
  for (std::string word; words >> word;) tokens.push_back(word);
  auto lock = db_->acquire_lock();
  auto rows = db_->query(R"(SELECT p.fencer_id AS id,p.name,p.club,p.state,p.country,
    COALESCE((SELECT group_concat(a.name,' ') FROM aliases a WHERE a.fencer_id=p.fencer_id),'') AS aliases
    FROM profiles p WHERE p.visible=1 ORDER BY p.name,p.fencer_id)");
  DbRows result;
  size_t skipped = 0, offset = static_cast<size_t>(page - 1) * limit;
  for (auto &row : rows) {
    auto names = fold(row.at("name") + " " + row.at("aliases"));
    bool matches = true;
    for (const auto &token : tokens) if (names.find(token) == std::string::npos) matches = false;
    if (!matches || skipped++ < offset) continue;
    row.erase("aliases"); result.push_back(std::move(row));
    if (result.size() == static_cast<size_t>(limit)) break;
  }
  return result;
}

DbRows TrackerRepo::profile(int id) {
  auto lock = db_->acquire_lock();
  return db_->query("SELECT fencer_id AS id,name,club,state,country FROM profiles WHERE fencer_id=? AND visible=1",
                     {std::to_string(id)});
}
