# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(SEGGER_RTT INTERFACE)
add_library(SEGGER_RTT_usage INTERFACE)

target_include_directories(SEGGER_RTT_usage INTERFACE
  src
)

target_link_libraries(SEGGER_RTT_usage INTERFACE
  base_config
)

target_link_libraries(SEGGER_RTT INTERFACE SEGGER_RTT_usage)

add_library(SEGGER_RTT_bin OBJECT EXCLUDE_FROM_ALL
  src/SEGGER_RTT.c
  src/SEGGER_RTT_printf.c
  src/SEGGER_RTT_Syscalls_GCC.c
)
target_link_libraries(SEGGER_RTT_bin PUBLIC SEGGER_RTT_usage)

target_link_libraries(SEGGER_RTT INTERFACE
  SEGGER_RTT_bin
  $<TARGET_OBJECTS:SEGGER_RTT_bin>
)
