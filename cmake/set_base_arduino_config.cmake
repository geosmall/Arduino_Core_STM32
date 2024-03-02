cmake_minimum_required(VERSION 3.21)

# note: the doc say these must be called _at file scope_, not in a function
enable_language(C)
enable_language(CXX)
enable_language(ASM)

add_library(base_config INTERFACE)

# better than an if/else because these may be defined later
target_link_libraries(base_config INTERFACE
  $<TARGET_NAME_IF_EXISTS:user_settings>
  $<TARGET_NAME_IF_EXISTS:board>
)

# generic compilation options
target_link_libraries(base_config INTERFACE
	board
	m
	stdc++
	c
	gcc
)
target_compile_definitions(base_config INTERFACE
	USE_FULL_LL_DRIVER
	ARDUINO_ARCH_STM32
)
target_compile_options(base_config INTERFACE
  -mthumb
	--param max-inline-insns-single=500
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
	-ffunction-sections
	-fdata-sections
)

target_link_options(base_config INTERFACE
  -mthumb
  LINKER:--cref
  LINKER:--check-sections
  LINKER:--gc-sections
  LINKER:--entry=Reset_Handler
  LINKER:--unresolved-symbols=report-all
  LINKER:--warn-common
	LINKER:--script=${BUILD_SYSTEM_PATH}/ldscript.ld
)
target_link_directories(base_config INTERFACE
	"${CMSIS5_PATH}/CMSIS/DSP/Lib/GCC"
)

target_include_directories(base_config INTERFACE
	"${BUILD_CORE_PATH}"
	"${BUILD_CORE_PATH}/stm32"
	"${BUILD_CORE_PATH}/stm32/LL"
	"${BUILD_CORE_PATH}/stm32/usb"
	"${BUILD_SYSTEM_PATH}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc"
	"${BUILD_SYSTEM_PATH}/Middlewares/ST/STM32_USB_Device_Library/Core/Src"
	"${BUILD_SYSTEM_PATH}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc"
	"${BUILD_SYSTEM_PATH}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src"
	"${CMSIS5_PATH}/CMSIS/DSP/Include"
	"${CMSIS5_PATH}/CMSIS/DSP/PrivateInclude"
	"${CMSIS5_PATH}/CMSIS/Core/Include/"
	"${CMSIS5_PATH}/CMSIS"
)

add_library(stm32_runtime INTERFACE)
target_link_libraries(stm32_runtime INTERFACE
  base_config

  core
)
