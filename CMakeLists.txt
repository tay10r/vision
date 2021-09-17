cmake_minimum_required(VERSION 3.14.7)

project(vision)

option(VISION_TESTS "Whether or not to build the tests." OFF)

option(VISION_EXAMPLE "Whether or not to build the example." OFF)

option(VISION_SERVERLIB "Whether or not to build the server library." OFF)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

add_executable(vision
  main.hpp
  main.cpp)

set_target_properties(vision
  PROPERTIES
    AUTOMOC ON
    AUTORCC ON)

target_link_libraries(vision PUBLIC Qt5::Widgets Qt5::Network)

add_subdirectory(proto)

if(VISION_TESTS)
  add_subdirectory(tests)
endif(VISION_TESTS)

if(VISION_EXAMPLE OR VISION_SERVERLIB)
  add_subdirectory(server)
endif(VISION_EXAMPLE OR VISION_SERVERLIB)

if(VISION_EXAMPLE)
  add_subdirectory(example)
endif(VISION_EXAMPLE)
