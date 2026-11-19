cmake_minimum_required(VERSION 3.16)
project(aus-fencer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(Boost CONFIG QUIET)
if(NOT Boost_FOUND)
  find_package(Boost REQUIRED)
endif()
find_package(Threads REQUIRED)
# Crow is header-only; its installed CMake package may require standalone Asio
# even when using the supported Boost networking backend.
find_path(CROW_INCLUDE_DIR crow.h)
if(NOT CROW_INCLUDE_DIR)
  message(FATAL_ERROR "Crow headers (crow.h) are required")
endif()
include(cmake/crow-compat.cmake)
add_library(ausfencer_http INTERFACE)
target_include_directories(ausfencer_http SYSTEM INTERFACE
  ${CROW_COMPAT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_compile_definitions(ausfencer_http INTERFACE CROW_USE_BOOST)
target_link_libraries(ausfencer_http INTERFACE Threads::Threads)
find_package(SQLite3 REQUIRED)
if(TARGET SQLite3::SQLite3)
  set(AUSFENCER_SQLITE_TARGET SQLite3::SQLite3)
else()
  set(AUSFENCER_SQLITE_TARGET SQLite::SQLite3)
endif()
find_package(OpenSSL REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)

FetchContent_Declare(
  sqlite_orm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  GIT_TAG        5f1a2ce84a3d72711b4f0a440fdaba977868ae67
)
FetchContent_MakeAvailable(sqlite_orm)
# Treat sqlite_orm as system headers, like Crow.
target_include_directories(sqlite_orm SYSTEM INTERFACE
  "$<BUILD_INTERFACE:${sqlite_orm_SOURCE_DIR}/include>")
# The pinned sqlite_orm uses the target name deprecated in CMake 4.3.
if(TARGET SQLite3::SQLite3)
  get_target_property(sqlite_orm_links sqlite_orm INTERFACE_LINK_LIBRARIES)
  list(TRANSFORM sqlite_orm_links REPLACE "^SQLite::SQLite3$" "SQLite3::SQLite3")
  set_property(TARGET sqlite_orm PROPERTY INTERFACE_LINK_LIBRARIES "${sqlite_orm_links}")
endif()

add_library(tracker_domain src/services/fixtures.cpp)
target_sources(tracker_domain PRIVATE src/services/fixture_validation.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/fixtures.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/fixture_bouts.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker_events.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/tracker_bouts.cpp)
target_sources(tracker_domain PRIVATE src/services/statistics.cpp)
target_sources(tracker_domain PRIVATE src/services/names.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/accounts.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/profiles.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/privacy.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/results.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/report_reviews.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/corrections.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/result_identities.cpp)
target_sources(tracker_domain PRIVATE src/db/repos/contributions.cpp)
target_sources(tracker_domain PRIVATE src/handlers/tracker.cpp)
target_sources(tracker_domain PRIVATE src/handlers/tracker_profiles.cpp)
target_sources(tracker_domain PRIVATE src/handlers/workflows.cpp)
target_link_libraries(tracker_domain PUBLIC ICU::uc)
target_include_directories(tracker_domain PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(tracker_domain PUBLIC ausfencer_http sqlite_orm::sqlite_orm ${AUSFENCER_SQLITE_TARGET} OpenSSL::Crypto)

find_package(GTest QUIET)
if(TARGET GTest::gtest)
  set(AUSFENCER_GTEST_TARGET GTest::gtest)
elseif(TARGET GTest::GTest)
  set(AUSFENCER_GTEST_TARGET GTest::GTest)
else()
  FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        063de7e9578f82b369302001269680b4b1553359
  )
  FetchContent_MakeAvailable(gtest)
  set(AUSFENCER_GTEST_TARGET gtest)
endif()

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/server.cpp
  src/handlers/base.cpp
  src/handlers/fencers.cpp
  src/handlers/bouts.cpp
  src/db/db_manager.cpp
  src/db/migrations.cpp
  src/db/repos/base.cpp
  src/db/repos/fencers.cpp
  src/db/repos/bouts.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  tracker_domain
  ausfencer_http
  sqlite_orm::sqlite_orm
  ${AUSFENCER_SQLITE_TARGET}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

enable_testing()
include(GoogleTest)
add_subdirectory(tests)
