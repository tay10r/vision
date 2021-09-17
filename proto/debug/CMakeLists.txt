cmake_minimum_required(VERSION 3.9.6)

add_executable(vision_exec_file exec_file.cpp)

target_link_libraries(vision_exec_file
  PRIVATE
    vision::proto)

set_target_properties(vision_exec_file
  PROPERTIES
    OUTPUT_NAME exec_file
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
