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
  # Top-level Arduino core sources
  abi.cpp
  avr/dtostrf.c
  board.c
  core_debug.c
  HardwareSerial.cpp
  HardwareTimer.cpp
  hooks.c
  IPAddress.cpp
  itoa.c
  main.cpp
  new.cpp
  pins_arduino.c
  Print.cpp
  RingBuffer.cpp
  Stream.cpp
  syscalls.c
  Time.cpp
  Tone.cpp
  USBSerial.cpp
  WInterrupts.cpp
  wiring_analog.cpp
  wiring_digital.c
  wiring_pulse.cpp
  wiring_shift.c
  wiring_time.c
  WMath.cpp
  WSerial.cpp
  WString.cpp

  # STM32 core support
  stm32/analog.cpp
  stm32/bootloader.c
  stm32/clock.c
  stm32/core_callback.c
  stm32/dwt.c
  stm32/hw_config.c
  stm32/interrupt.cpp
  stm32/otp.c
  stm32/pinmap.c
  stm32/PortNames.c
  stm32/startup_stm32yyxx.S
  stm32/stm32_def.c
  stm32/system_stm32yyxx.c
  stm32/timer.c
  stm32/uart.c

  # STM32 USB support
  stm32/usb/cdc/cdc_queue.c
  stm32/usb/cdc/usbd_cdc.c
  stm32/usb/cdc/usbd_cdc_if.c
  stm32/usb/hid/usbd_hid_composite.c
  stm32/usb/hid/usbd_hid_composite_if.c
  stm32/usb/usb_device_core.c
  stm32/usb/usb_device_ctlreq.c
  stm32/usb/usb_device_ioreq.c
  stm32/usb/usbd_conf.c
  stm32/usb/usbd_desc.c
  stm32/usb/usbd_ep_conf.c
  stm32/usb/usbd_if.c

  # STM32 HAL drivers
  stm32/HAL/stm32yyxx_hal.c
  stm32/HAL/stm32yyxx_hal_adc.c
  stm32/HAL/stm32yyxx_hal_adc_ex.c
  stm32/HAL/stm32yyxx_hal_cortex.c
  stm32/HAL/stm32yyxx_hal_crc.c
  stm32/HAL/stm32yyxx_hal_crc_ex.c
  stm32/HAL/stm32yyxx_hal_dac.c
  stm32/HAL/stm32yyxx_hal_dma.c
  stm32/HAL/stm32yyxx_hal_dma_ex.c
  stm32/HAL/stm32yyxx_hal_dma2d.c
  stm32/HAL/stm32yyxx_hal_exti.c
  stm32/HAL/stm32yyxx_hal_flash.c
  stm32/HAL/stm32yyxx_hal_flash_ex.c
  stm32/HAL/stm32yyxx_hal_gpio.c
  stm32/HAL/stm32yyxx_hal_gpio_ex.c
  stm32/HAL/stm32yyxx_hal_i2c.c
  stm32/HAL/stm32yyxx_hal_i2c_ex.c
  stm32/HAL/stm32yyxx_hal_pcd.c
  stm32/HAL/stm32yyxx_hal_pcd_ex.c
  stm32/HAL/stm32yyxx_hal_pwr.c
  stm32/HAL/stm32yyxx_hal_pwr_ex.c
  stm32/HAL/stm32yyxx_hal_rcc.c
  stm32/HAL/stm32yyxx_hal_rcc_ex.c
  stm32/HAL/stm32yyxx_hal_rtc.c
  stm32/HAL/stm32yyxx_hal_rtc_ex.c
  stm32/HAL/stm32yyxx_hal_sd.c
  stm32/HAL/stm32yyxx_hal_sd_ex.c
  stm32/HAL/stm32yyxx_hal_sdram.c
  stm32/HAL/stm32yyxx_hal_spi.c
  stm32/HAL/stm32yyxx_hal_spi_ex.c
  stm32/HAL/stm32yyxx_hal_tim.c
  stm32/HAL/stm32yyxx_hal_tim_ex.c
  stm32/HAL/stm32yyxx_hal_uart.c
  stm32/HAL/stm32yyxx_hal_uart_ex.c
  stm32/HAL/stm32yyxx_hal_usart.c
  stm32/HAL/stm32yyxx_hal_usart_ex.c

  # STM32 LL drivers
  stm32/LL/stm32yyxx_ll_adc.c
  stm32/LL/stm32yyxx_ll_bdma.c
  stm32/LL/stm32yyxx_ll_comp.c
  stm32/LL/stm32yyxx_ll_cordic.c
  stm32/LL/stm32yyxx_ll_crc.c
  stm32/LL/stm32yyxx_ll_crs.c
  stm32/LL/stm32yyxx_ll_dac.c
  stm32/LL/stm32yyxx_ll_delayblock.c
  stm32/LL/stm32yyxx_ll_dlyb.c
  stm32/LL/stm32yyxx_ll_dma.c
  stm32/LL/stm32yyxx_ll_dma2d.c
  stm32/LL/stm32yyxx_ll_exti.c
  stm32/LL/stm32yyxx_ll_fmac.c
  stm32/LL/stm32yyxx_ll_fmc.c
  stm32/LL/stm32yyxx_ll_fmpi2c.c
  stm32/LL/stm32yyxx_ll_fsmc.c
  stm32/LL/stm32yyxx_ll_gpio.c
  stm32/LL/stm32yyxx_ll_hrtim.c
  stm32/LL/stm32yyxx_ll_i2c.c
  stm32/LL/stm32yyxx_ll_i3c.c
  stm32/LL/stm32yyxx_ll_icache.c
  stm32/LL/stm32yyxx_ll_lpgpio.c
  stm32/LL/stm32yyxx_ll_lptim.c
  stm32/LL/stm32yyxx_ll_lpuart.c
  stm32/LL/stm32yyxx_ll_mdma.c
  stm32/LL/stm32yyxx_ll_opamp.c
  stm32/LL/stm32yyxx_ll_pka.c
  stm32/LL/stm32yyxx_ll_pwr.c
  stm32/LL/stm32yyxx_ll_rcc.c
  stm32/LL/stm32yyxx_ll_rng.c
  stm32/LL/stm32yyxx_ll_rtc.c
  stm32/LL/stm32yyxx_ll_sdmmc.c
  stm32/LL/stm32yyxx_ll_spi.c
  stm32/LL/stm32yyxx_ll_swpmi.c
  stm32/LL/stm32yyxx_ll_tim.c
  stm32/LL/stm32yyxx_ll_ucpd.c
  stm32/LL/stm32yyxx_ll_usart.c
  stm32/LL/stm32yyxx_ll_usb.c
  stm32/LL/stm32yyxx_ll_utils.c
)
target_link_libraries(core_bin PUBLIC core_usage)

target_link_libraries(core INTERFACE
  core_bin
)

