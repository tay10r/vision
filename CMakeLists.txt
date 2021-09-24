cmake_minimum_required(VERSION 3.14.7)

project(vision)

option(VISION_TESTS "Whether or not to build the tests." OFF)

option(VISION_EXAMPLES "Whether or not to build the examples." OFF)

add_subdirectory(gui)

if(VISION_EXAMPLES)
  add_subdirectory(examples)
endif(VISION_EXAMPLES)
