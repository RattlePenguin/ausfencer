add_executable(ausfencer_tests
  test_main.cpp
  db/test_db_manager.cpp
  db/test_fencer_repo.cpp
  handlers/test_fencer_handler.cpp
  ${CMAKE_SOURCE_DIR}/src/db/db_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/db/repos/base.cpp
  ${CMAKE_SOURCE_DIR}/src/db/repos/fencers.cpp
  ${CMAKE_SOURCE_DIR}/src/handlers/base.cpp
  ${CMAKE_SOURCE_DIR}/src/handlers/fencers.cpp
)

target_include_directories(ausfencer_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ausfencer_tests PRIVATE
  gtest
  gtest_main
  Crow::Crow
  sqlite_orm::sqlite_orm
  SQLite3::SQLite3
)

gtest_discover_tests(ausfencer_tests)
