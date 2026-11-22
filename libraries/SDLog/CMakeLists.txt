# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(SDLog INTERFACE)
add_library(SDLog_usage INTERFACE)

target_include_directories(SDLog_usage INTERFACE
  src
)

target_link_libraries(SDLog_usage INTERFACE
  base_config
  SDFS
)

target_link_libraries(SDLog INTERFACE SDLog_usage)

add_library(SDLog_bin OBJECT EXCLUDE_FROM_ALL
  src/SDLog.cpp
)
target_link_libraries(SDLog_bin PUBLIC SDLog_usage)

target_link_libraries(SDLog INTERFACE
  SDLog_bin
  $<TARGET_OBJECTS:SDLog_bin>
)
