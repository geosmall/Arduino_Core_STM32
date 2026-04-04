cmake_minimum_required(VERSION 3.21)

add_library(Scheduler INTERFACE)
add_library(Scheduler_usage INTERFACE)

target_include_directories(Scheduler_usage INTERFACE
  src
)

target_link_libraries(Scheduler_usage INTERFACE
  base_config
)

target_link_libraries(Scheduler INTERFACE Scheduler_usage)

add_library(Scheduler_bin OBJECT EXCLUDE_FROM_ALL
  src/scheduler.c
)
target_link_libraries(Scheduler_bin PUBLIC Scheduler_usage)

target_link_libraries(Scheduler INTERFACE
  Scheduler_bin
  $<TARGET_OBJECTS:Scheduler_bin>
)
