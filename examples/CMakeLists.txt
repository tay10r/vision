cmake_minimum_required(VERSION 3.9.6)

find_package(Qt5 REQUIRED COMPONENTS Gui) # Only used for the math library.

find_package(OpenMP REQUIRED COMPONENTS CXX)

function(add_example name)

  set(target vision_example_${name})

  add_executable(${target} ${name}.cpp)

  target_link_libraries(${target} PRIVATE Qt5::Gui OpenMP::OpenMP_CXX)

  set_target_properties(${target}
    PROPERTIES
      OUTPUT_NAME ${name}
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

endfunction(add_example name)

add_example(minimal)

add_example(path_tracer)
