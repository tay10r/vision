cmake_minimum_required(VERSION 3.9.6)

find_package(GTest REQUIRED)

add_executable(vision_proto_tests
  lexer_tests.cpp
  parser_tests.cpp
  line_buffer_tests.cpp
  interpreter_tests.cpp)

target_link_libraries(vision_proto_tests
  PRIVATE
    vision::proto
    GTest::GTest
    GTest::Main)

set_target_properties(vision_proto_tests
  PROPERTIES
    OUTPUT_NAME run_proto_tests
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
