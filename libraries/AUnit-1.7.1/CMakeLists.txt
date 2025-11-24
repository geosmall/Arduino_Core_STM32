# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(AUnit INTERFACE)
add_library(AUnit_usage INTERFACE)

target_include_directories(AUnit_usage INTERFACE
  src
)

target_link_libraries(AUnit_usage INTERFACE
  base_config
)

target_link_libraries(AUnit INTERFACE AUnit_usage)

add_library(AUnit_bin OBJECT EXCLUDE_FROM_ALL
  src/aunit/Assertion.cpp
  src/aunit/Compare.cpp
  src/aunit/FCString.cpp
  src/aunit/MetaAssertion.cpp
  src/aunit/print64.cpp
  src/aunit/Printer.cpp
  src/aunit/string_util.cpp
  src/aunit/Test.cpp
  src/aunit/TestAgain.cpp
  src/aunit/TestOnce.cpp
  src/aunit/TestRunner.cpp
)
target_link_libraries(AUnit_bin PUBLIC AUnit_usage)

target_link_libraries(AUnit INTERFACE
  AUnit_bin
  $<TARGET_OBJECTS:AUnit_bin>
)
