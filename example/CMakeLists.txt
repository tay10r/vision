cmake_minimum_required(VERSION 3.9.6)

add_executable(vision_example
  main.cpp)

set_target_properties(vision_example
  PROPERTIES
    OUTPUT_NAME run_example
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

target_link_libraries(vision_example PRIVATE vision::server)
