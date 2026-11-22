# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(SDBlock INTERFACE)
add_library(SDBlock_usage INTERFACE)

target_include_directories(SDBlock_usage INTERFACE
  src
)

target_link_libraries(SDBlock_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(SDBlock INTERFACE SDBlock_usage)

add_library(SDBlock_bin OBJECT EXCLUDE_FROM_ALL
  src/SDBlock.cpp
)
target_link_libraries(SDBlock_bin PUBLIC SDBlock_usage)

target_link_libraries(SDBlock INTERFACE
  SDBlock_bin
  $<TARGET_OBJECTS:SDBlock_bin>
)
