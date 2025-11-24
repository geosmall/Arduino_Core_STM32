# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(MPU9250 INTERFACE)
add_library(MPU9250_usage INTERFACE)

target_include_directories(MPU9250_usage INTERFACE
  src
)

target_link_libraries(MPU9250_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(MPU9250 INTERFACE MPU9250_usage)

add_library(MPU9250_bin OBJECT EXCLUDE_FROM_ALL
  src/bf_bus.cpp
  src/mpu9250_bf.c
  src/MPU9250.cpp
)
target_link_libraries(MPU9250_bin PUBLIC MPU9250_usage)

target_link_libraries(MPU9250 INTERFACE
  MPU9250_bin
  $<TARGET_OBJECTS:MPU9250_bin>
)
