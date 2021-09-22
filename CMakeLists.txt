cmake_minimum_required(VERSION 3.14.7)

project(vision)

option(VISION_TESTS "Whether or not to build the tests." OFF)

option(VISION_EXAMPLE "Whether or not to build the example." OFF)

add_subdirectory(gui)
