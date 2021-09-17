cmake_minimum_required(VERSION 3.9.6)

add_library(vision_proto
  src/lexer.cpp
  src/parser.cpp
  src/line_buffer.cpp
  src/interpreter.cpp
  src/server.cpp)

target_include_directories(vision_proto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(NOT MSVC)
  target_compile_options(vision_proto PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

add_library(vision::proto ALIAS vision_proto)

if(VISION_TESTS)
  add_subdirectory(tests)
  add_subdirectory(debug)
endif(VISION_TESTS)
