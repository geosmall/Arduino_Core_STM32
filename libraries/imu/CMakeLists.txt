# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(imu INTERFACE)
add_library(imu_usage INTERFACE)

target_include_directories(imu_usage INTERFACE
  src
  src/bus
  src/devices
  betaflight
  betaflight/drivers/accgyro
)

target_link_libraries(imu_usage INTERFACE
  base_config
  SPI
)

target_link_libraries(imu INTERFACE imu_usage)

add_library(imu_bin OBJECT EXCLUDE_FROM_ALL
  src/bf_bus.cpp
  src/IMU.cpp
  src/IMU_Driver.cpp
  src/devices/ICM206xx.cpp
  src/devices/ICM42688.cpp
  src/devices/MPU6000.cpp
  src/devices/MPU9250.cpp
  betaflight/drivers/accgyro/accgyro_mpu.c
  betaflight/drivers/accgyro/accgyro_spi_icm20689.c
  betaflight/drivers/accgyro/accgyro_spi_icm426xx.c
  betaflight/drivers/accgyro/accgyro_spi_icm456xx.c
  betaflight/drivers/accgyro/accgyro_spi_mpu6000.c
  betaflight/drivers/accgyro/accgyro_spi_mpu9250.c
)
target_link_libraries(imu_bin PUBLIC imu_usage)

target_link_libraries(imu INTERFACE
  imu_bin
  $<TARGET_OBJECTS:imu_bin>
)
