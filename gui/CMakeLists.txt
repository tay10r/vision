cmake_minimum_required(VERSION 3.14.7)

project(vision)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Charts)

find_package(OpenMP REQUIRED COMPONENTS CXX)

add_library(vision_gui
  address_bar.hpp
  address_bar.cpp
  command_stream.hpp
  command_stream.cpp
  connection_view_proxy.hpp
  connection_view_proxy.cpp
  content_view.hpp
  content_view.cpp
  process_view.hpp
  process_view.cpp
  debug_connection.hpp
  debug_connection.cpp
  debug_render.hpp
  debug_render.cpp
  debug_input.hpp
  debug_input.cpp
  page.hpp
  page.cpp
  view.hpp
  view.cpp
  schedule.hpp
  schedule.cpp
  monitor.hpp
  monitor.cpp
  response.hpp
  response.cpp
  response_signal_emitter.hpp
  response_signal_emitter.cpp
  vertex.hpp
  vertex.cpp
  lexer.hpp
  lexer.cpp
  token.hpp
  token.cpp
  shaders.qrc)

set_target_properties(vision_gui
  PROPERTIES
    AUTOMOC ON
    AUTORCC ON)

target_compile_features(vision_gui PUBLIC cxx_std_17)

target_link_libraries(vision_gui PUBLIC Qt5::Widgets Qt5::Network Qt5::Charts OpenMP::OpenMP_CXX)

if(NOT MSVC)
  target_compile_options(vision_gui PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

add_library(vision::gui ALIAS vision_gui)


add_executable(vision
  main.cpp)

if(NOT MSVC)
  target_compile_options(vision PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_link_libraries(vision PUBLIC vision::gui)

set_target_properties(vision
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

#############
# View Test #
#############

if(VISION_TESTS)

  find_package(GTest REQUIRED)

  add_executable(vision_gui_tests
    response_tests.cpp
    schedule_tests.cpp
    lexer_tests.cpp)

  target_link_libraries(vision_gui_tests
    PUBLIC
      vision::gui
      GTest::GTest
      GTest::Main)

  set_target_properties(vision_gui_tests
    PROPERTIES
      OUTPUT_NAME run_tests
      RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

  add_test(NAME vision_gui_tests COMMAND $<TARGET_FILE:vision_gui_tests>)

  add_executable(view_test view_test.cpp)

  target_link_libraries(view_test Qt5::Widgets Qt5::Charts vision::gui)

endif(VISION_TESTS)
