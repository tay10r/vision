cmake_minimum_required(VERSION 3.9.6)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_library(vision_server
  include/vision/server.hpp
  src/server.cpp)

target_include_directories(vision_server PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(vision_server PUBLIC Boost::boost Threads::Threads)

add_library(vision::server ALIAS vision_server)
