# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

# PrecompLib is a precompiled library - headers + static archives
add_library(PrecompLib INTERFACE)
add_library(PrecompLib_usage INTERFACE)

target_include_directories(PrecompLib_usage INTERFACE
  src
)

target_link_libraries(PrecompLib_usage INTERFACE
  base_config
)

target_link_libraries(PrecompLib INTERFACE PrecompLib_usage)

# Link the appropriate precompiled archive based on target CPU
# Note: The actual .a file selection happens at link time based on the target
# For CMake builds, you may need to specify the correct archive path
# e.g., src/cortex-m4/fpv4-sp-d16-hard/libPrecompLib.a for F4/G4
#       src/cortex-m7/fpv4-sp-d16-hard/libPrecompLib.a for F7/H7
