cmake_minimum_required(VERSION 3.21)

add_library(DShot INTERFACE)
add_library(DShot_usage INTERFACE)

target_include_directories(DShot_usage INTERFACE
  src
)

target_link_libraries(DShot_usage INTERFACE
  base_config
)

target_link_libraries(DShot INTERFACE DShot_usage)

add_library(DShot_bin OBJECT EXCLUDE_FROM_ALL
  src/DShot_ll.cpp
  src/DShotOutput.cpp
)
target_link_libraries(DShot_bin PUBLIC DShot_usage)

target_link_libraries(DShot INTERFACE
  DShot_bin
  $<TARGET_OBJECTS:DShot_bin>
)
