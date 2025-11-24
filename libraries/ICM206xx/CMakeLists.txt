# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(ICM206xx INTERFACE)
add_library(ICM206xx_usage INTERFACE)

target_include_directories(ICM206xx_usage INTERFACE
  src
)

target_link_libraries(ICM206xx_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(ICM206xx INTERFACE ICM206xx_usage)

add_library(ICM206xx_bin OBJECT EXCLUDE_FROM_ALL
  src/bf_bus.cpp
  src/icm20689_bf.c
  src/ICM206xx.cpp
)
target_link_libraries(ICM206xx_bin PUBLIC ICM206xx_usage)

target_link_libraries(ICM206xx INTERFACE
  ICM206xx_bin
  $<TARGET_OBJECTS:ICM206xx_bin>
)
