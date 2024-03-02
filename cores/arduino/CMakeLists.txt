# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

add_library(core INTERFACE)
add_library(core_usage INTERFACE)

target_include_directories(core_usage INTERFACE
  .
)


target_link_libraries(core_usage INTERFACE
  base_config
)

target_link_libraries(core INTERFACE core_usage)



add_library(core_bin STATIC EXCLUDE_FROM_ALL
  board.c
  main.cpp
  stm32/clock.c
  stm32/dwt.c
  stm32/startup_stm32yyxx.S
  stm32/syscalls.c
  stm32/system_stm32yyxx.c
  stm32/usb/usb_device.c
  stm32/usb/usb_device_cdc.c
  stm32/usb/usb_device_core.c
  stm32/usb/usb_device_ctlreq.c
  stm32/usb/usb_device_ioreq.c
  stm32/usb/usbd_cdc_if.c
  stm32/usb/usbd_desc.c
  stm32/HAL/stm32yyxx_hal.c
  stm32/HAL/stm32yyxx_hal_cortex.c
  stm32/HAL/stm32yyxx_hal_pcd.c
  stm32/HAL/stm32yyxx_hal_pcd_ex.c
  stm32/HAL/stm32yyxx_hal_pwr.c
  stm32/HAL/stm32yyxx_hal_pwr_ex.c
  stm32/HAL/stm32yyxx_hal_rcc.c
  stm32/HAL/stm32yyxx_hal_rcc_ex.c
  stm32/LL/stm32yyxx_ll_adc.c
  stm32/LL/stm32yyxx_ll_crc.c
  stm32/LL/stm32yyxx_ll_dma.c
  stm32/LL/stm32yyxx_ll_exti.c
  stm32/LL/stm32yyxx_ll_gpio.c
  stm32/LL/stm32yyxx_ll_hrtim.c
  stm32/LL/stm32yyxx_ll_i2c.c
  stm32/LL/stm32yyxx_ll_pwr.c
  stm32/LL/stm32yyxx_ll_rcc.c
  stm32/LL/stm32yyxx_ll_rtc.c
  stm32/LL/stm32yyxx_ll_sdmmc.c
  stm32/LL/stm32yyxx_ll_spi.c
  stm32/LL/stm32yyxx_ll_tim.c
  stm32/LL/stm32yyxx_ll_usart.c
  stm32/LL/stm32yyxx_ll_usb.c
  stm32/LL/stm32yyxx_ll_utils.c
)
target_link_libraries(core_bin PUBLIC core_usage)

target_link_libraries(core INTERFACE
  core_bin
)

