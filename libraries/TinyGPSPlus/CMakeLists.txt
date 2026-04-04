# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(TinyGPSPlus INTERFACE)
add_library(TinyGPSPlus_usage INTERFACE)

target_include_directories(TinyGPSPlus_usage INTERFACE
  src
)

target_link_libraries(TinyGPSPlus_usage INTERFACE
  base_config
)

target_link_libraries(TinyGPSPlus INTERFACE TinyGPSPlus_usage)

add_library(TinyGPSPlus_bin OBJECT EXCLUDE_FROM_ALL
  src/TinyGPS++.cpp
)
target_link_libraries(TinyGPSPlus_bin PUBLIC TinyGPSPlus_usage)

target_link_libraries(TinyGPSPlus INTERFACE
  TinyGPSPlus_bin
  $<TARGET_OBJECTS:TinyGPSPlus_bin>
)
