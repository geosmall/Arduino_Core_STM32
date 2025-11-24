# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(minIniStorage INTERFACE)
add_library(minIniStorage_usage INTERFACE)

target_include_directories(minIniStorage_usage INTERFACE
  src
)

target_link_libraries(minIniStorage_usage INTERFACE
  base_config
  Storage
)

target_link_libraries(minIniStorage INTERFACE minIniStorage_usage)

add_library(minIniStorage_bin OBJECT EXCLUDE_FROM_ALL
  src/minIni.cpp
)
target_link_libraries(minIniStorage_bin PUBLIC minIniStorage_usage)

target_link_libraries(minIniStorage INTERFACE
  minIniStorage_bin
  $<TARGET_OBJECTS:minIniStorage_bin>
)
