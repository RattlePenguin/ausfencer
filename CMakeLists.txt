cmake_minimum_required(VERSION 3.10)
project(aus_fencer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(Crow REQUIRED)
find_package(Boost REQUIRED)
find_package(SQLite3 REQUIRED)

FetchContent_Declare(
  sqlite_orm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  GIT_TAG        5f1a2ce84a3d72711b4f0a440fdaba977868ae67
)
FetchContent_MakeAvailable(sqlite_orm)

FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        063de7e9578f82b369302001269680b4b1553359
)
FetchContent_MakeAvailable(gtest)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/server.cpp
  src/handlers/base.cpp
  src/handlers/fencers.cpp
  src/db/db_manager.cpp
  src/db/repos/base.cpp
  src/db/repos/fencers.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Crow::Crow
  sqlite_orm::sqlite_orm
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)
