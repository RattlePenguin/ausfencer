add_executable(ausfencer_tests
  test_main.cpp
  test_server.cpp
  ${CMAKE_SOURCE_DIR}/src/server.cpp
  db/test_db_manager.cpp
  db/test_schema_upgrade.cpp
  db/test_weapon_persistence.cpp
  db/repos/test_fencer_repo.cpp
  db/repos/test_bout_repo.cpp
  handlers/test_fencer_handler.cpp
  handlers/test_bout_handler.cpp
  ${CMAKE_SOURCE_DIR}/src/db/db_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/db/repos/base.cpp
  ${CMAKE_SOURCE_DIR}/src/db/repos/fencers.cpp
  ${CMAKE_SOURCE_DIR}/src/db/repos/bouts.cpp
  ${CMAKE_SOURCE_DIR}/src/handlers/base.cpp
  ${CMAKE_SOURCE_DIR}/src/handlers/fencers.cpp
  ${CMAKE_SOURCE_DIR}/src/handlers/bouts.cpp
)

target_include_directories(ausfencer_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ausfencer_tests PRIVATE
  gtest
  gtest_main
  ausfencer_http
  sqlite_orm::sqlite_orm
  SQLite3::SQLite3
)

gtest_discover_tests(ausfencer_tests)

find_package(Python3 3.7 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME ProductionHttpSmoke COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/smoke_test.py $<TARGET_FILE:aus-fencer>)
  set_tests_properties(ProductionHttpSmoke PROPERTIES RUN_SERIAL TRUE TIMEOUT 30)
endif()
