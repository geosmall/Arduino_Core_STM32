cmake_minimum_required(VERSION 3.21)

add_library(EmbeddedCLI INTERFACE)
add_library(EmbeddedCLI_usage INTERFACE)

target_include_directories(EmbeddedCLI_usage INTERFACE
  src
)

target_link_libraries(EmbeddedCLI_usage INTERFACE
  base_config
)

target_link_libraries(EmbeddedCLI INTERFACE EmbeddedCLI_usage)

add_library(EmbeddedCLI_bin OBJECT EXCLUDE_FROM_ALL
  src/embedded_cli.c
)
target_link_libraries(EmbeddedCLI_bin PUBLIC EmbeddedCLI_usage)

target_link_libraries(EmbeddedCLI INTERFACE
  EmbeddedCLI_bin
  $<TARGET_OBJECTS:EmbeddedCLI_bin>
)
