# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(TimerPWM INTERFACE)
add_library(TimerPWM_usage INTERFACE)

target_include_directories(TimerPWM_usage INTERFACE
  .
  src
)

target_link_libraries(TimerPWM_usage INTERFACE
  base_config
)

target_link_libraries(TimerPWM INTERFACE TimerPWM_usage)

add_library(TimerPWM_bin OBJECT EXCLUDE_FROM_ALL
  TimerPWM.cpp
  src/PWMOutputBank.cpp
)
target_link_libraries(TimerPWM_bin PUBLIC TimerPWM_usage)

target_link_libraries(TimerPWM INTERFACE
  TimerPWM_bin
  $<TARGET_OBJECTS:TimerPWM_bin>
)
