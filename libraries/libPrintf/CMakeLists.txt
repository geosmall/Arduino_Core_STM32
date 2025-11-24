# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(libPrintf INTERFACE)
add_library(libPrintf_usage INTERFACE)

target_include_directories(libPrintf_usage INTERFACE
  src
)

target_link_libraries(libPrintf_usage INTERFACE
  base_config
)

target_link_libraries(libPrintf INTERFACE libPrintf_usage)

add_library(libPrintf_bin OBJECT EXCLUDE_FROM_ALL
  src/printf.c
)
target_link_libraries(libPrintf_bin PUBLIC libPrintf_usage)

target_link_libraries(libPrintf INTERFACE
  libPrintf_bin
  $<TARGET_OBJECTS:libPrintf_bin>
)
