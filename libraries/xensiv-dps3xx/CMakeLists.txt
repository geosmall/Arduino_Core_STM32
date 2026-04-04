# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(xensiv-dps3xx INTERFACE)
add_library(xensiv-dps3xx_usage INTERFACE)

target_include_directories(xensiv-dps3xx_usage INTERFACE
  src
  src/util
)

target_link_libraries(xensiv-dps3xx_usage INTERFACE
  base_config
)

target_link_libraries(xensiv-dps3xx INTERFACE xensiv-dps3xx_usage)

add_library(xensiv-dps3xx_bin OBJECT EXCLUDE_FROM_ALL
  src/Dps3xx.cpp
  src/DpsClass.cpp
)
target_link_libraries(xensiv-dps3xx_bin PUBLIC xensiv-dps3xx_usage)

target_link_libraries(xensiv-dps3xx INTERFACE
  xensiv-dps3xx_bin
  $<TARGET_OBJECTS:xensiv-dps3xx_bin>
)
