cmake_minimum_required(VERSION 3.10)
project(aus_fencer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

find_package(Crow REQUIRED)
find_package(SQLite3 REQUIRED)

FetchContent_Declare(
  sqlite_orm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  GIT_TAG        5f1a2ce84a3d72711b4f0a440fdaba977868ae67
)
FetchContent_MakeAvailable(sqlite_orm)

add_executable(my_app src/crowapp.cpp)
target_link_libraries(my_app PRIVATE
  Crow::Crow
  SQLite3::SQLite3
  sqlite_orm::sqlite_orm
)
