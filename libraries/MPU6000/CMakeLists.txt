# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(MPU6000 INTERFACE)
add_library(MPU6000_usage INTERFACE)

target_include_directories(MPU6000_usage INTERFACE
  src
)

target_link_libraries(MPU6000_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(MPU6000 INTERFACE MPU6000_usage)

add_library(MPU6000_bin OBJECT EXCLUDE_FROM_ALL
  src/bf_bus.cpp
  src/mpu6000_bf.c
  src/MPU6000.cpp
)
target_link_libraries(MPU6000_bin PUBLIC MPU6000_usage)

target_link_libraries(MPU6000 INTERFACE
  MPU6000_bin
  $<TARGET_OBJECTS:MPU6000_bin>
)
