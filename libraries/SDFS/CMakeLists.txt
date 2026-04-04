# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(SDFS INTERFACE)
add_library(SDFS_usage INTERFACE)

target_include_directories(SDFS_usage INTERFACE
  src
  src/fatfs
)

target_link_libraries(SDFS_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(SDFS INTERFACE SDFS_usage)

add_library(SDFS_bin OBJECT EXCLUDE_FROM_ALL
  src/SDFS.cpp
  src/sd_spi_diskio.cpp
  src/fatfs/ff.c
)
target_link_libraries(SDFS_bin PUBLIC SDFS_usage)

target_link_libraries(SDFS INTERFACE
  SDFS_bin
  $<TARGET_OBJECTS:SDFS_bin>
)
