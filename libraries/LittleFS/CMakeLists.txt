# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(LittleFS INTERFACE)
add_library(LittleFS_usage INTERFACE)

target_include_directories(LittleFS_usage INTERFACE
  src
  src/littlefs
)

target_link_libraries(LittleFS_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(LittleFS INTERFACE LittleFS_usage)

add_library(LittleFS_bin OBJECT EXCLUDE_FROM_ALL
  src/LittleFS.cpp
  src/littlefs/lfs.c
  src/littlefs/lfs_util.c
)
target_link_libraries(LittleFS_bin PUBLIC LittleFS_usage)

target_link_libraries(LittleFS INTERFACE
  LittleFS_bin
  $<TARGET_OBJECTS:LittleFS_bin>
)
