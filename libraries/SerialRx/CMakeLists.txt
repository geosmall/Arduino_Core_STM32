# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(SerialRx INTERFACE)
add_library(SerialRx_usage INTERFACE)

target_include_directories(SerialRx_usage INTERFACE
  src
  src/parsers
)

target_link_libraries(SerialRx_usage INTERFACE
  base_config
)

target_link_libraries(SerialRx INTERFACE SerialRx_usage)

add_library(SerialRx_bin OBJECT EXCLUDE_FROM_ALL
  src/SerialRx.cpp
  src/parsers/IBusParser.cpp
  src/parsers/SBusParser.cpp
  src/parsers/CRSFParser.cpp
)
target_link_libraries(SerialRx_bin PUBLIC SerialRx_usage)

target_link_libraries(SerialRx INTERFACE
  SerialRx_bin
  $<TARGET_OBJECTS:SerialRx_bin>
)
