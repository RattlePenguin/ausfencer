#include "db/db_manager.hpp"
#include "services/fixtures.hpp"
#include "db/repos/fixtures.hpp"
#include "db/repos/tracker.hpp"
#include "db/repos/fencers.hpp"
#include "services/statistics.hpp"
#include "db/repos/accounts.hpp"
#include "db/repos/profiles.hpp"
#include "db/repos/results.hpp"
#include "handlers/tracker.hpp"
#include <gtest/gtest.h>

class TrackerTest : public ::testing::Test {};

TEST_F(TrackerTest, LoadsVersionedSyntheticFixture) {
  auto data = load_fixture(FIXTURE_DIR);
  EXPECT_EQ(data.fencers.size(), 12u);
  EXPECT_EQ(data.events.size(), 4u);
  EXPECT_EQ(data.entries.size(), 18u);
  EXPECT_EQ(data.bouts.size(), 44u);
  EXPECT_EQ(data.checksum.size(), 64u);
  EXPECT_EQ(load_fixture(FIXTURE_DIR).checksum, data.checksum);
  EXPECT_THROW(load_fixture("missing-fixture-directory"), std::invalid_argument);
}

TEST_F(TrackerTest, RejectsInvalidReferencesScoresAndDates) {
  auto data = load_fixture(FIXTURE_DIR);
  EXPECT_NO_THROW(validate_fixture(data));
  data.bouts[0].right = 999;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.bouts[0].left_score = 100;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.events[0].date = "2026-02-30";
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
  data = load_fixture(FIXTURE_DIR); data.fencers[1].id = data.fencers[0].id;
  EXPECT_THROW(validate_fixture(data), std::invalid_argument);
}

TEST_F(TrackerTest, FixtureContextPreviewIsReversibleAndRepeatable) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo repo(db);
  auto data = load_fixture(FIXTURE_DIR);
  auto preview=repo.import_fixture(data);
  EXPECT_EQ(preview.profiles_created,12); EXPECT_EQ(preview.events_created,4); EXPECT_EQ(preview.bouts_created,44);
  EXPECT_TRUE(db->query("SELECT * FROM profiles").empty());
  repo.import_fixture(data, true);
  auto repeat=repo.import_fixture(data, true);
  EXPECT_EQ(repeat.bouts_created,0); EXPECT_EQ(repeat.repeated_observations,44u);
  EXPECT_EQ(db->query("SELECT * FROM profiles").size(), 12u);
  EXPECT_EQ(db->query("SELECT * FROM entries").size(), 18u);
  EXPECT_EQ(db->query("SELECT visible FROM profiles WHERE fencer_id=112")[0].at("visible"), "0");
}

TEST_F(TrackerTest, ImportsBothPerspectivesOnceAndRollsBackConflicts) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo repo(db);
  auto data = load_fixture(FIXTURE_DIR);
  auto mirror = data.bouts[0];
  std::swap(mirror.left,mirror.right); std::swap(mirror.left_score,mirror.right_score);
  data.bouts.push_back(mirror);
  repo.import_fixture(data, true); repo.import_fixture(data, true);
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(), 44u);
  EXPECT_EQ(db->query("SELECT * FROM observations").size(), 44u);
  auto changed = load_fixture(FIXTURE_DIR);
  changed.bouts[0].left_score = 4;
  EXPECT_THROW(repo.import_fixture(changed, true), std::invalid_argument);
  EXPECT_EQ(db->query("SELECT left_score FROM bouts WHERE id=?",
    {"ausfencer-demo-v1:epee-2026:A-101-102"})[0].at("left_score"), "5");
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(), 44u);
}

TEST_F(TrackerTest, SearchesUnicodeAliasesDuplicateNamesAndBoundedPages) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.search("VALE mira").size(), 2u);
  EXPECT_EQ(repo.search("M. Vale").size(), 1u);
  EXPECT_EQ(repo.search("ESMÉ").size(), 1u);
  EXPECT_TRUE(repo.search("%").empty());
  EXPECT_EQ(repo.search("",2,10).size(), 1u);
  EXPECT_THROW(repo.search("",0,10), std::invalid_argument);
  EXPECT_THROW(repo.search("",1,101), std::invalid_argument);
  EXPECT_TRUE(repo.profile(112).empty());
}

TEST_F(TrackerTest, EventEntriesRedactSourceNamesAndHistoricalAffiliations) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db);
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.events().size(), 4u);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  auto entries = repo.event_entries("epee-2025");
  ASSERT_EQ(entries.size(), 2u);
  EXPECT_EQ(entries[0].at("name"), "Private fencer");
  EXPECT_EQ(entries[0].count("id"), 0u);
  EXPECT_TRUE(entries[0].at("source_name").empty());
  EXPECT_TRUE(entries[0].at("club").empty());
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  EXPECT_TRUE(repo.search("M. Vale").empty());
  EXPECT_TRUE(repo.profile(101).empty());
}

TEST_F(TrackerTest, HistoriesUseCanonicalScoresAndRedactHiddenOpponents) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  EXPECT_EQ(repo.history(101).size(), 9u);
  EXPECT_EQ(repo.history(107).size(), 9u);
  EXPECT_EQ(repo.event_bouts("epee-2026").size(), 41u);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  EXPECT_TRUE(repo.history(101).empty());
  auto history = repo.history(107);
  EXPECT_EQ(history.back().at("left_name"), "Private fencer");
  EXPECT_EQ(history.back().count("left_id"), 0u);
  EXPECT_EQ(history.back().at("received"), "5");
  EXPECT_TRUE(repo.bout(99999).empty());
}

TEST_F(TrackerTest, StatisticsMatchHandCalculationsAndAnnualRoundFilters) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  TrackerRepo repo(db);
  TrackerFilter filter; filter.year = "2026"; filter.weapon = "epee";
  auto stats = calculate_statistics(repo.history(101,filter));
  EXPECT_EQ(stats.wins, 7); EXPECT_EQ(stats.losses, 1);
  EXPECT_EQ(stats.scored, 67); EXPECT_EQ(stats.received, 45);
  EXPECT_DOUBLE_EQ(*stats.win_rate(), 7.0/8);
  EXPECT_DOUBLE_EQ(*stats.touch_share(), 67.0/112);
  stats = calculate_statistics(repo.history(107,filter));
  EXPECT_EQ(stats.wins, 8); EXPECT_EQ(stats.losses, 0);
  EXPECT_EQ(stats.scored, 70); EXPECT_EQ(stats.received, 44);
  stats = calculate_statistics(repo.history(101));
  EXPECT_EQ(stats.wins, 8); EXPECT_EQ(stats.scored, 72); EXPECT_EQ(stats.received, 48);
  filter.stage = "de";
  stats = calculate_statistics(repo.history(101,filter));
  EXPECT_EQ(stats.wins, 2); EXPECT_EQ(stats.losses, 1); EXPECT_EQ(stats.scored, 43);
  EXPECT_FALSE(calculate_statistics({}).win_rate());
}

TEST_F(TrackerTest, StatisticsSeparateDisputesAdministrativeAndUnknownScores) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  db->query("UPDATE bouts SET disputed=1 WHERE rowid=1");
  db->query("UPDATE bouts SET status='administrative' WHERE rowid=2");
  db->query("UPDATE bouts SET status='unfinished',winner_id=NULL WHERE rowid=3");
  db->query("UPDATE bouts SET left_score=NULL,right_score=NULL WHERE rowid=4");
  auto stats = calculate_statistics(TrackerRepo(db).history(101));
  EXPECT_EQ(stats.wins, 5); EXPECT_EQ(stats.losses, 1);
  EXPECT_EQ(stats.disputed, 1); EXPECT_EQ(stats.administrative, 1); EXPECT_EQ(stats.unfinished, 1);
  EXPECT_EQ(stats.score_known, 5);
}

TEST_F(TrackerTest, LegacyFencerReadsRespectProfileSuppression) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  FencerRepo repo(db);
  EXPECT_EQ(repo.get_all("",1,100).size(), 11u);
  EXPECT_EQ(repo.get(112), nullptr);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  EXPECT_EQ(repo.get(101), nullptr);
  EXPECT_EQ(repo.get_all("Mira Vale",1,10).size(), 1u);
  EXPECT_THROW(repo.get_all("",1,1000), std::invalid_argument);
}

TEST_F(TrackerTest, DemoSessionsValidateExpiryAndRevocation) {
  auto db = std::make_shared<DbManager>(":memory:");
  AccountRepo accounts(db);
  EXPECT_THROW(accounts.create_demo_session("Demo","member"), std::invalid_argument);
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  auto token = accounts.create_demo_session("Contributor","member");
  EXPECT_EQ(accounts.authenticate(token).at("role"), "member");
  EXPECT_NE(db->query("SELECT token_hash FROM sessions")[0].at("token_hash"), token);
  EXPECT_TRUE(accounts.authenticate(std::string(64,'0')).empty());
  db->query("UPDATE sessions SET expires_at=0");
  EXPECT_TRUE(accounts.authenticate(token).empty());
  auto moderator = accounts.create_demo_session("Reviewer","moderator");
  accounts.logout(moderator);
  EXPECT_TRUE(accounts.authenticate(moderator).empty());
}

TEST_F(TrackerTest, ClaimsRequireEvidenceAndIndependentReview) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  AccountRepo accounts(db); ProfileRepo profiles(db);
  auto member = accounts.create_demo_session("Fencer","member");
  auto reviewer = accounts.create_demo_session("Reviewer","moderator");
  EXPECT_THROW(profiles.request_claim("",101,"self","Evidence",""), std::domain_error);
  EXPECT_THROW(profiles.request_claim(member,112,"self","Evidence",""), std::domain_error);
  EXPECT_THROW(profiles.request_claim(member,112,"guardian","Evidence",""), std::invalid_argument);
  auto claim = profiles.request_claim(member,101,"self","Fictional identity evidence","");
  EXPECT_THROW(profiles.review_claim(member,claim,true), std::domain_error);
  EXPECT_NO_THROW(profiles.review_claim(reviewer,claim,true));
  auto own = profiles.request_claim(reviewer,107,"self","Evidence","");
  EXPECT_THROW(profiles.review_claim(reviewer,own,true), std::domain_error);
}

TEST_F(TrackerTest, GuardianPublicationAndWithdrawalPersistAcrossReimports) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db); fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  AccountRepo accounts(db); ProfileRepo profiles(db); TrackerRepo tracker(db);
  auto guardian = accounts.create_demo_session("Parent","member");
  auto reviewer = accounts.create_demo_session("Reviewer","moderator");
  EXPECT_THROW(profiles.set_visibility(guardian,112,true), std::domain_error);
  auto claim = profiles.request_claim(guardian,112,"guardian","Fictional authority declaration","demo-v1");
  profiles.review_claim(reviewer,claim,true); profiles.set_visibility(guardian,112,true);
  EXPECT_EQ(tracker.profile(112).size(), 1u);
  profiles.withdraw(guardian,112);
  EXPECT_TRUE(tracker.profile(112).empty());
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  EXPECT_TRUE(tracker.search("Noor").empty());
  EXPECT_THROW(profiles.set_visibility(guardian,112,true), std::domain_error);
  EXPECT_EQ(profiles.claims(guardian)[0].at("state"), "withdrawn");
  EXPECT_EQ(db->query("SELECT * FROM profile_audit").size(), 2u);
}

TEST_F(TrackerTest, UnregisteredPrivacyRequestsNeedAReviewedDecision) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  AccountRepo accounts(db); ProfileRepo profiles(db); TrackerRepo tracker(db);
  auto reviewer = accounts.create_demo_session("Reviewer","moderator");
  auto request = profiles.request_privacy(101,"fictional@example.invalid","Please hide this demo profile");
  EXPECT_EQ(tracker.profile(101).size(), 1u);
  EXPECT_THROW(profiles.review_privacy("",request,true), std::domain_error);
  EXPECT_EQ(profiles.review_queue(reviewer).size(), 1u);
  profiles.review_privacy(reviewer,request,true);
  EXPECT_TRUE(tracker.profile(101).empty());
  EXPECT_TRUE(profiles.review_queue(reviewer).empty());
}

TEST_F(TrackerTest, ResetRequiresADemoMarkerAndImportsRecordFailures) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo repo(db); auto data = load_fixture(FIXTURE_DIR);
  EXPECT_THROW(repo.import_fixture(data,true,true), std::invalid_argument);
  repo.import_fixture(data,true);
  auto changed = data; changed.bouts[0].left_score = 4;
  EXPECT_THROW(repo.import_fixture(changed,true), std::invalid_argument);
  EXPECT_EQ(db->query("SELECT * FROM imports WHERE status='failed'").size(), 2u);
  db->query("UPDATE bouts SET left_score=4 WHERE rowid=1");
  repo.import_fixture(data,true,true);
  EXPECT_EQ(db->query("SELECT left_score FROM bouts WHERE rowid=1")[0].at("left_score"), "5");
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(), 44u);
}

TEST_F(TrackerTest, ReportsPreserveScoresAndExcludeDisputesFromStatistics) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR), true);
  auto token = AccountRepo(db).create_demo_session("Reporter","member");
  ResultRepo results(db);
  EXPECT_THROW(results.report("",1,"score","Wrong score"), std::domain_error);
  results.report(token,1,"score","Demo evidence for a correction");
  EXPECT_EQ(db->query("SELECT left_score FROM bouts WHERE rowid=1")[0].at("left_score"), "5");
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).wins, 7);
  EXPECT_EQ(results.reports(token)[0].at("state"), "pending");
  EXPECT_THROW(results.reports(token,true), std::domain_error);
}

TEST_F(TrackerTest, CorrectionsAreAuditedRejectStaleWritesAndSurviveReimports) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db); fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  AccountRepo accounts(db); ResultRepo results(db);
  auto member = accounts.create_demo_session("Reporter","member");
  auto reviewer = accounts.create_demo_session("Reviewer","moderator");
  results.report(member,1,"score","Fictional score evidence");
  EXPECT_THROW(results.correct(member,1,0,4,2,"left","completed","Correction"), std::domain_error);
  results.correct(reviewer,1,0,4,2,"left","completed","Reviewed fictional evidence");
  EXPECT_THROW(results.correct(reviewer,1,0,5,2,"left","completed","Stale"), std::domain_error);
  fixtures.import_fixture(load_fixture(FIXTURE_DIR), true);
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).scored, 71);
  EXPECT_EQ(results.reports(member)[0].at("state"), "resolved");
  EXPECT_EQ(db->query("SELECT * FROM revisions").size(), 1u);
  results.correct(reviewer,1,1,5,2,"left","completed","Reverted after second review");
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).scored, 72);
}

TEST_F(TrackerTest, CommunitySubmissionsPublishOnceAndCannotOverwriteAcceptedScores) {
  auto db=std::make_shared<DbManager>(":memory:");
  auto data=load_fixture(FIXTURE_DIR); auto bout=data.bouts[0]; data.bouts.erase(data.bouts.begin());
  FixtureRepo(db).import_fixture(data,true);
  auto token=AccountRepo(db).create_demo_session("Contributor","member"); ResultRepo results(db);
  auto id=results.submit(token,"demo-operation",bout,"Fictional supporting record");
  EXPECT_EQ(results.submit(token,"demo-operation",bout,"Fictional supporting record"),id);
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(),44u);
  bout.left_score=4;
  EXPECT_THROW(results.submit(token,"demo-operation",bout,"Fictional supporting record"),std::domain_error);
  results.submit(token,"conflicting-operation",bout,"Conflicting evidence");
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(),44u);
  EXPECT_EQ(db->query("SELECT state FROM contributions ORDER BY id DESC LIMIT 1")[0].at("state"),"conflict");
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).wins,7);
  auto reviewer=AccountRepo(db).create_demo_session("Reviewer","moderator");
  results.correct(reviewer,44,0,5,2,"left","completed","Rejected conflicting evidence");
  EXPECT_EQ(results.contributions(token)[0].at("state"),"rejected");
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).wins,8);
}

TEST_F(TrackerTest, PublicTrackerRoutesValidatePaginationAndDisableCaching) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR),true);
  App app; TrackerHandler handler(std::make_shared<TrackerRepo>(db));
  handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/tracker/fencers"; req.method=crow::HTTPMethod::GET;
  req.url_params=crow::query_string("?q=Mira%20Vale");
  crow::response response; app.handle_full(req,response);
  EXPECT_EQ(response.code,200);
  EXPECT_EQ(crow::json::load(response.body)["fencers"].size(),2u);
  EXPECT_EQ(response.get_header_value("Cache-Control"),"no-store");
  req.url_params=crow::query_string("?limit=101"); response=crow::response();
  app.handle_full(req,response); EXPECT_EQ(response.code,400);
}

TEST_F(TrackerTest, ProfileRouteReturnsKnownTotalsAndHidesPrivateRecords) {
  auto db = std::make_shared<DbManager>(":memory:");
  FixtureRepo(db).import_fixture(load_fixture(FIXTURE_DIR),true);
  App app; TrackerHandler handler(std::make_shared<TrackerRepo>(db)); handler.register_routes(app); app.validate();
  crow::request req; req.url="/api/tracker/fencers/101"; req.method=crow::HTTPMethod::GET;
  req.url_params=crow::query_string("?year=2026");
  crow::response response; app.handle_full(req,response);
  EXPECT_EQ(crow::json::load(response.body)["statistics"]["scored"].i(),67);
  req.url="/api/tracker/fencers/112"; response=crow::response();
  app.handle_full(req,response); EXPECT_EQ(response.code,404);
}

TEST_F(TrackerTest, ReviewedIdentityCorrectionsUpdateBothHistoriesAndPreserveEvidence) {
  auto db=std::make_shared<DbManager>(":memory:");
  auto data=load_fixture(FIXTURE_DIR); data.bouts.resize(1);
  FixtureRepo fixtures(db); fixtures.import_fixture(data,true);
  auto token=AccountRepo(db).create_demo_session("Reviewer","moderator");
  ResultRepo results(db); results.report(token,1,"identity","Fictional wrong participant");
  results.correct_identity(token,1,0,"left",103,"Reviewed fictional identity evidence");
  EXPECT_TRUE(TrackerRepo(db).history(101).empty());
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(103)).wins,1);
  fixtures.import_fixture(data,true);
  EXPECT_TRUE(TrackerRepo(db).history(101).empty());
  EXPECT_EQ(results.reports(token)[0].at("state"),"resolved");
  EXPECT_THROW(results.correct_identity(token,1,0,"right",101,"Stale"),std::domain_error);
}

TEST_F(TrackerTest, ChangedSourceIdsCannotRecreateSuppressedIdentities) {
  auto db=std::make_shared<DbManager>(":memory:");
  FixtureRepo fixtures(db); auto data=load_fixture(FIXTURE_DIR); fixtures.import_fixture(data,true);
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  auto replacement=data.fencers[0]; replacement.id=113;
  data.fencers={replacement}; data.entries.clear(); data.bouts.clear();
  EXPECT_THROW(fixtures.import_fixture(data,true),std::invalid_argument);
  data.fencers[0].external_id="DEMO-113"; data.fencers[0].name="ＭＩＲＡ　ＶＡＬＥ";
  EXPECT_THROW(fixtures.import_fixture(data,true),std::invalid_argument);
  EXPECT_TRUE(TrackerRepo(db).profile(113).empty());
  EXPECT_TRUE(TrackerRepo(db).search("M. Vale").empty());
}

TEST_F(TrackerTest, PlacingsRemainAvailableWhenBoutCoverageIsMissing) {
  auto db=std::make_shared<DbManager>(":memory:");
  auto data=load_fixture(FIXTURE_DIR);
  data.bouts.erase(data.bouts.begin()+41);
  data.events[1].coverage="results-only";
  FixtureRepo(db).import_fixture(data,true); TrackerRepo tracker(db);
  EXPECT_EQ(tracker.profile_events(101).size(),2u);
  TrackerFilter filter; filter.coverage="results-only";
  EXPECT_TRUE(tracker.history(101,filter).empty());
  auto events=tracker.profile_events(101,filter);
  ASSERT_EQ(events.size(),1u); EXPECT_EQ(events[0].at("place"),"1");
  db->query("UPDATE profiles SET visible=0 WHERE fencer_id=101");
  EXPECT_TRUE(tracker.profile_events(101).empty());
}

TEST_F(TrackerTest, FixtureImportsLinkMatchingCommunityResultsWithoutDoubleCounting) {
  auto db=std::make_shared<DbManager>(":memory:");
  auto complete=load_fixture(FIXTURE_DIR), partial=complete;
  partial.bouts.erase(partial.bouts.begin());
  FixtureRepo fixtures(db); fixtures.import_fixture(partial,true);
  auto token=AccountRepo(db).create_demo_session("Contributor","member");
  ResultRepo(db).submit(token,"missing-poule-bout",complete.bouts[0],"Fictional evidence");
  fixtures.import_fixture(complete,true); fixtures.import_fixture(complete,true);
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(),44u);
  EXPECT_EQ(calculate_statistics(TrackerRepo(db).history(101)).scored,72);
  EXPECT_EQ(db->query("SELECT * FROM bouts WHERE trust='community'").size(),1u);
}

TEST_F(TrackerTest, ContributionRateLimitsAllowSafeRetries) {
  auto db=std::make_shared<DbManager>(":memory:");
  auto data=load_fixture(FIXTURE_DIR); FixtureRepo(db).import_fixture(data,true);
  auto token=AccountRepo(db).create_demo_session("Contributor","member"); ResultRepo results(db);
  for (int i=0;i<30;++i) results.submit(token,"operation-"+std::to_string(i),data.bouts[0],"");
  EXPECT_THROW(results.submit(token,"excess",data.bouts[0],""),std::domain_error);
  EXPECT_NO_THROW(results.submit(token,"operation-0",data.bouts[0],""));
  EXPECT_EQ(db->query("SELECT * FROM bouts").size(),44u);
}
