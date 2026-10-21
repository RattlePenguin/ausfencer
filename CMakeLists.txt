cmake_minimum_required(VERSION 3.10)
project(aus-fencer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(Crow REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)

FetchContent_Declare(
  sqlite_orm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  GIT_TAG        5f1a2ce84a3d72711b4f0a440fdaba977868ae67
)
FetchContent_MakeAvailable(sqlite_orm)

add_library(tracker_domain src/services/fixtures.cpp)
target_sources(tracker_domain PRIVATE src/services/fixture_validation.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/fixtures.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/fixture_bouts.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker_events.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker_bouts.cpp)
target_sources(tracker_domain PRIVATE src/services/statistics.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/accounts.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/profiles.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/privacy.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/results.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/corrections.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/contributions.cpp)
target_sources(tracker_domain PRIVATE src/handlers/tracker.cpp)
target_link_libraries(tracker_domain PUBLIC ICU::uc)
target_include_directories(tracker_domain PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tracker_domain PUBLIC Crow::Crow sqlite_orm::sqlite_orm SQLite3::SQLite3 OpenSSL::Crypto)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        063de7e9578f82b369302001269680b4b1553359
)
FetchContent_MakeAvailable(gtest)
else()
  add_library(gtest ALIAS GTest::gtest)
  add_library(gtest_main ALIAS GTest::gtest_main)
endif()

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/server.cpp
  src/handlers/base.cpp
  src/handlers/fencers.cpp
  src/db/db_manager.cpp
  src/db/migrations.cpp
  src/db/repos/base.cpp
  src/db/repos/fencers.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  tracker_domain
  Crow::Crow
  sqlite_orm::sqlite_orm
  SQLite3::SQLite3
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

enable_testing()
include(GoogleTest)
add_subdirectory(tests)
