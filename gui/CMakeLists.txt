cmake_minimum_required(VERSION 3.14.7)

project(vision)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Charts)

set(controller_sources
  fake_controller.hpp
  fake_controller.cpp)

add_executable(vision
  address_bar.hpp
  address_bar.cpp
  central_widget.hpp
  central_widget.cpp
  view_page.hpp
  view_page.cpp
  view.hpp
  view.cpp
  monitor.hpp
  monitor.cpp
  ${controller_sources}
  main.cpp)

if(NOT MSVC)
  target_compile_options(vision PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_link_libraries(vision PUBLIC Qt5::Widgets Qt5::Network Qt5::Charts)

#############
# View Test #
#############

if(VISION_TESTS)

  add_executable(view_test view_test.cpp view.hpp view.cpp)

  target_link_libraries(view_test Qt5::Widgets Qt5::Charts)

endif(VISION_TESTS)
