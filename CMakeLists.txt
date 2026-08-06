cmake_minimum_required(VERSION 3.20)
project(FencingTracker VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch nlohmann_json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Fetch standalone Asio (header-only library required by Crow)
FetchContent_Declare(
    asio
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
)
FetchContent_MakeAvailable(asio)
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "Path to Asio headers" FORCE)

# Fetch Crow Framework
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(Crow)

# Find SQLite3
find_package(SQLite3 REQUIRED)

add_executable(fencing_tracker
    src/main.cpp
    src/db/db_manager.cpp
)

target_include_directories(fencing_tracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(fencing_tracker PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    SQLite3::SQLite3
    pthread
)
