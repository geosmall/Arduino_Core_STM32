# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(Storage INTERFACE)
add_library(Storage_usage INTERFACE)

target_include_directories(Storage_usage INTERFACE
  src
)

target_link_libraries(Storage_usage INTERFACE
  base_config
)

target_link_libraries(Storage INTERFACE Storage_usage)

add_library(Storage_bin OBJECT EXCLUDE_FROM_ALL
  src/Storage.cpp
  src/BoardStorage.cpp
)
target_link_libraries(Storage_bin PUBLIC Storage_usage)

target_link_libraries(Storage INTERFACE
  Storage_bin
  $<TARGET_OBJECTS:Storage_bin>
)
