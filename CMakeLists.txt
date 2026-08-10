cmake_minimum_required(VERSION 3.10)
project(my_crow_app)

set(CMAKE_CXX_STANDARD 17)

find_package(Crow)
add_executable(my_app src/crowapp.cpp)
target_link_libraries(my_app PUBLIC Crow::Crow)
