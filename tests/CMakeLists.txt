add_executable(test_main src/test_main.cpp)
target_link_libraries(test_main PRIVATE
  Crow::Crow
  SQLite3::SQLite3
  sqlite_orm::sqlite_orm
)
