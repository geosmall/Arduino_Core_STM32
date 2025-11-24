# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(ICM42688P INTERFACE)
add_library(ICM42688P_usage INTERFACE)

target_include_directories(ICM42688P_usage INTERFACE
  src
  src/Invn/Drivers/Icm426xx
  src/Invn/EmbUtils
  src/Invn/Helpers/Icm426xx
)

target_link_libraries(ICM42688P_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(ICM42688P INTERFACE ICM42688P_usage)

add_library(ICM42688P_bin OBJECT EXCLUDE_FROM_ALL
  src/Invn/Drivers/Icm426xx/Icm426xxDriver_HL.c
  src/Invn/Drivers/Icm426xx/Icm426xxSelfTest.c
  src/Invn/Drivers/Icm426xx/Icm426xxTransport.c
  src/Invn/EmbUtils/ErrorHelper.c
  src/Invn/EmbUtils/Message.c
  src/Invn/Helpers/Icm426xx/helperClockCalib.c
)
target_link_libraries(ICM42688P_bin PUBLIC ICM42688P_usage)

target_link_libraries(ICM42688P INTERFACE
  ICM42688P_bin
  $<TARGET_OBJECTS:ICM42688P_bin>
)
