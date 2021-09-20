cmake_minimum_required(VERSION 3.14.7)

project(vision)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Charts)

set(controller_sources
  fake_controller.hpp
  fake_controller.cpp)

add_library(vision_gui SHARED
  address_bar.hpp
  address_bar.cpp
  central_widget.hpp
  central_widget.cpp
  view_page.hpp
  view_page.cpp
  view.hpp
  view.cpp
  schedule.hpp
  schedule.cpp
  monitor.hpp
  monitor.cpp
  vertex.hpp
  vertex.cpp
  ${controller_sources})

target_compile_features(vision_gui PUBLIC cxx_std_17)

target_link_libraries(vision_gui PUBLIC Qt5::Widgets Qt5::Network Qt5::Charts)

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

#############
# View Test #
#############

if(VISION_TESTS)

  find_package(GTest REQUIRED)

  add_executable(vision_gui_tests
    schedule_tests.cpp)

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
