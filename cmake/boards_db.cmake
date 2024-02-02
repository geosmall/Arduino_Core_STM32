# BLACK_F407VE
# -----------------------------------------------------------------------------

set(BLACK_F407VE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLACK_F407VE_MAXSIZE 524288)
set(BLACK_F407VE_MAXDATASIZE 131072)
set(BLACK_F407VE_MCU cortex-m4)
set(BLACK_F407VE_FPCONF "-")
add_library(BLACK_F407VE INTERFACE)
target_compile_options(BLACK_F407VE INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VE_MCU}
)
target_compile_definitions(BLACK_F407VE INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407VE"
	"BOARD_NAME=\"BLACK_F407VE\""
	"BOARD_ID=BLACK_F407VE"
	"VARIANT_H=\"variant_BLACK_F407VX.h\""
)
target_include_directories(BLACK_F407VE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407VE_VARIANT_PATH}
)

target_link_options(BLACK_F407VE INTERFACE
  "LINKER:--default-script=${BLACK_F407VE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VE_MCU}
)
target_link_libraries(BLACK_F407VE INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACK_F407VE_serial_disabled INTERFACE)
target_compile_options(BLACK_F407VE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACK_F407VE_serial_generic INTERFACE)
target_compile_options(BLACK_F407VE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACK_F407VE_serial_none INTERFACE)
target_compile_options(BLACK_F407VE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACK_F407VE_hid
# -----------------------------------------------------------------------------

set(BLACK_F407VE_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLACK_F407VE_hid_MAXSIZE 524288)
set(BLACK_F407VE_hid_MAXDATASIZE 131072)
set(BLACK_F407VE_hid_MCU cortex-m4)
set(BLACK_F407VE_hid_FPCONF "-")
add_library(BLACK_F407VE_hid INTERFACE)
target_compile_options(BLACK_F407VE_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VE_hid_MCU}
)
target_compile_definitions(BLACK_F407VE_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407VE"
	"BOARD_NAME=\"BLACK_F407VE\""
	"BOARD_ID=BLACK_F407VE"
	"VARIANT_H=\"variant_BLACK_F407VX.h\""
)
target_include_directories(BLACK_F407VE_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407VE_hid_VARIANT_PATH}
)

target_link_options(BLACK_F407VE_hid INTERFACE
  "LINKER:--default-script=${BLACK_F407VE_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VE_hid_MCU}
)
target_link_libraries(BLACK_F407VE_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACK_F407VG
# -----------------------------------------------------------------------------

set(BLACK_F407VG_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLACK_F407VG_MAXSIZE 1048576)
set(BLACK_F407VG_MAXDATASIZE 131072)
set(BLACK_F407VG_MCU cortex-m4)
set(BLACK_F407VG_FPCONF "-")
add_library(BLACK_F407VG INTERFACE)
target_compile_options(BLACK_F407VG INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VG_MCU}
)
target_compile_definitions(BLACK_F407VG INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407VG"
	"BOARD_NAME=\"BLACK_F407VG\""
	"BOARD_ID=BLACK_F407VG"
	"VARIANT_H=\"variant_BLACK_F407VX.h\""
)
target_include_directories(BLACK_F407VG INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407VG_VARIANT_PATH}
)

target_link_options(BLACK_F407VG INTERFACE
  "LINKER:--default-script=${BLACK_F407VG_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VG_MCU}
)
target_link_libraries(BLACK_F407VG INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACK_F407VG_serial_disabled INTERFACE)
target_compile_options(BLACK_F407VG_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACK_F407VG_serial_generic INTERFACE)
target_compile_options(BLACK_F407VG_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACK_F407VG_serial_none INTERFACE)
target_compile_options(BLACK_F407VG_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACK_F407VG_hid
# -----------------------------------------------------------------------------

set(BLACK_F407VG_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLACK_F407VG_hid_MAXSIZE 1048576)
set(BLACK_F407VG_hid_MAXDATASIZE 131072)
set(BLACK_F407VG_hid_MCU cortex-m4)
set(BLACK_F407VG_hid_FPCONF "-")
add_library(BLACK_F407VG_hid INTERFACE)
target_compile_options(BLACK_F407VG_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VG_hid_MCU}
)
target_compile_definitions(BLACK_F407VG_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407VG"
	"BOARD_NAME=\"BLACK_F407VG\""
	"BOARD_ID=BLACK_F407VG"
	"VARIANT_H=\"variant_BLACK_F407VX.h\""
)
target_include_directories(BLACK_F407VG_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407VG_hid_VARIANT_PATH}
)

target_link_options(BLACK_F407VG_hid INTERFACE
  "LINKER:--default-script=${BLACK_F407VG_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407VG_hid_MCU}
)
target_link_libraries(BLACK_F407VG_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACK_F407ZE
# -----------------------------------------------------------------------------

set(BLACK_F407ZE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(BLACK_F407ZE_MAXSIZE 524288)
set(BLACK_F407ZE_MAXDATASIZE 131072)
set(BLACK_F407ZE_MCU cortex-m4)
set(BLACK_F407ZE_FPCONF "-")
add_library(BLACK_F407ZE INTERFACE)
target_compile_options(BLACK_F407ZE INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZE_MCU}
)
target_compile_definitions(BLACK_F407ZE INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407ZE"
	"BOARD_NAME=\"BLACK_F407ZE\""
	"BOARD_ID=BLACK_F407ZE"
	"VARIANT_H=\"variant_BLACK_F407ZX.h\""
)
target_include_directories(BLACK_F407ZE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407ZE_VARIANT_PATH}
)

target_link_options(BLACK_F407ZE INTERFACE
  "LINKER:--default-script=${BLACK_F407ZE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZE_MCU}
)
target_link_libraries(BLACK_F407ZE INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACK_F407ZE_serial_disabled INTERFACE)
target_compile_options(BLACK_F407ZE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACK_F407ZE_serial_generic INTERFACE)
target_compile_options(BLACK_F407ZE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACK_F407ZE_serial_none INTERFACE)
target_compile_options(BLACK_F407ZE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACK_F407ZE_hid
# -----------------------------------------------------------------------------

set(BLACK_F407ZE_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(BLACK_F407ZE_hid_MAXSIZE 524288)
set(BLACK_F407ZE_hid_MAXDATASIZE 131072)
set(BLACK_F407ZE_hid_MCU cortex-m4)
set(BLACK_F407ZE_hid_FPCONF "-")
add_library(BLACK_F407ZE_hid INTERFACE)
target_compile_options(BLACK_F407ZE_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZE_hid_MCU}
)
target_compile_definitions(BLACK_F407ZE_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407ZE"
	"BOARD_NAME=\"BLACK_F407ZE\""
	"BOARD_ID=BLACK_F407ZE"
	"VARIANT_H=\"variant_BLACK_F407ZX.h\""
)
target_include_directories(BLACK_F407ZE_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407ZE_hid_VARIANT_PATH}
)

target_link_options(BLACK_F407ZE_hid INTERFACE
  "LINKER:--default-script=${BLACK_F407ZE_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZE_hid_MCU}
)
target_link_libraries(BLACK_F407ZE_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACK_F407ZG
# -----------------------------------------------------------------------------

set(BLACK_F407ZG_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(BLACK_F407ZG_MAXSIZE 1048576)
set(BLACK_F407ZG_MAXDATASIZE 131072)
set(BLACK_F407ZG_MCU cortex-m4)
set(BLACK_F407ZG_FPCONF "-")
add_library(BLACK_F407ZG INTERFACE)
target_compile_options(BLACK_F407ZG INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZG_MCU}
)
target_compile_definitions(BLACK_F407ZG INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407ZG"
	"BOARD_NAME=\"BLACK_F407ZG\""
	"BOARD_ID=BLACK_F407ZG"
	"VARIANT_H=\"variant_BLACK_F407ZX.h\""
)
target_include_directories(BLACK_F407ZG INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407ZG_VARIANT_PATH}
)

target_link_options(BLACK_F407ZG INTERFACE
  "LINKER:--default-script=${BLACK_F407ZG_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZG_MCU}
)
target_link_libraries(BLACK_F407ZG INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACK_F407ZG_serial_disabled INTERFACE)
target_compile_options(BLACK_F407ZG_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACK_F407ZG_serial_generic INTERFACE)
target_compile_options(BLACK_F407ZG_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACK_F407ZG_serial_none INTERFACE)
target_compile_options(BLACK_F407ZG_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACK_F407ZG_hid
# -----------------------------------------------------------------------------

set(BLACK_F407ZG_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(BLACK_F407ZG_hid_MAXSIZE 1048576)
set(BLACK_F407ZG_hid_MAXDATASIZE 131072)
set(BLACK_F407ZG_hid_MCU cortex-m4)
set(BLACK_F407ZG_hid_FPCONF "-")
add_library(BLACK_F407ZG_hid INTERFACE)
target_compile_options(BLACK_F407ZG_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZG_hid_MCU}
)
target_compile_definitions(BLACK_F407ZG_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACK_F407ZG"
	"BOARD_NAME=\"BLACK_F407ZG\""
	"BOARD_ID=BLACK_F407ZG"
	"VARIANT_H=\"variant_BLACK_F407ZX.h\""
)
target_include_directories(BLACK_F407ZG_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACK_F407ZG_hid_VARIANT_PATH}
)

target_link_options(BLACK_F407ZG_hid INTERFACE
  "LINKER:--default-script=${BLACK_F407ZG_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACK_F407ZG_hid_MCU}
)
target_link_libraries(BLACK_F407ZG_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACKPILL_F401CC
# -----------------------------------------------------------------------------

set(BLACKPILL_F401CC_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(BLACKPILL_F401CC_MAXSIZE 262144)
set(BLACKPILL_F401CC_MAXDATASIZE 65536)
set(BLACKPILL_F401CC_MCU cortex-m4)
set(BLACKPILL_F401CC_FPCONF "-")
add_library(BLACKPILL_F401CC INTERFACE)
target_compile_options(BLACKPILL_F401CC INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CC_MCU}
)
target_compile_definitions(BLACKPILL_F401CC INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F401CC"
	"BOARD_NAME=\"BLACKPILL_F401CC\""
	"BOARD_ID=BLACKPILL_F401CC"
	"VARIANT_H=\"variant_BLACKPILL_F401Cx.h\""
)
target_include_directories(BLACKPILL_F401CC INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F401CC_VARIANT_PATH}
)

target_link_options(BLACKPILL_F401CC INTERFACE
  "LINKER:--default-script=${BLACKPILL_F401CC_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CC_MCU}
)
target_link_libraries(BLACKPILL_F401CC INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACKPILL_F401CC_serial_disabled INTERFACE)
target_compile_options(BLACKPILL_F401CC_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACKPILL_F401CC_serial_generic INTERFACE)
target_compile_options(BLACKPILL_F401CC_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACKPILL_F401CC_serial_none INTERFACE)
target_compile_options(BLACKPILL_F401CC_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACKPILL_F401CC_hid
# -----------------------------------------------------------------------------

set(BLACKPILL_F401CC_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(BLACKPILL_F401CC_hid_MAXSIZE 262144)
set(BLACKPILL_F401CC_hid_MAXDATASIZE 65536)
set(BLACKPILL_F401CC_hid_MCU cortex-m4)
set(BLACKPILL_F401CC_hid_FPCONF "-")
add_library(BLACKPILL_F401CC_hid INTERFACE)
target_compile_options(BLACKPILL_F401CC_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CC_hid_MCU}
)
target_compile_definitions(BLACKPILL_F401CC_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F401CC"
	"BOARD_NAME=\"BLACKPILL_F401CC\""
	"BOARD_ID=BLACKPILL_F401CC"
	"VARIANT_H=\"variant_BLACKPILL_F401Cx.h\""
)
target_include_directories(BLACKPILL_F401CC_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F401CC_hid_VARIANT_PATH}
)

target_link_options(BLACKPILL_F401CC_hid INTERFACE
  "LINKER:--default-script=${BLACKPILL_F401CC_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CC_hid_MCU}
)
target_link_libraries(BLACKPILL_F401CC_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACKPILL_F401CE
# -----------------------------------------------------------------------------

set(BLACKPILL_F401CE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(BLACKPILL_F401CE_MAXSIZE 524288)
set(BLACKPILL_F401CE_MAXDATASIZE 98304)
set(BLACKPILL_F401CE_MCU cortex-m4)
set(BLACKPILL_F401CE_FPCONF "-")
add_library(BLACKPILL_F401CE INTERFACE)
target_compile_options(BLACKPILL_F401CE INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CE_MCU}
)
target_compile_definitions(BLACKPILL_F401CE INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F401CE"
	"BOARD_NAME=\"BLACKPILL_F401CE\""
	"BOARD_ID=BLACKPILL_F401CE"
	"VARIANT_H=\"variant_BLACKPILL_F401Cx.h\""
)
target_include_directories(BLACKPILL_F401CE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F401CE_VARIANT_PATH}
)

target_link_options(BLACKPILL_F401CE INTERFACE
  "LINKER:--default-script=${BLACKPILL_F401CE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CE_MCU}
)
target_link_libraries(BLACKPILL_F401CE INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACKPILL_F401CE_serial_disabled INTERFACE)
target_compile_options(BLACKPILL_F401CE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACKPILL_F401CE_serial_generic INTERFACE)
target_compile_options(BLACKPILL_F401CE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACKPILL_F401CE_serial_none INTERFACE)
target_compile_options(BLACKPILL_F401CE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACKPILL_F401CE_hid
# -----------------------------------------------------------------------------

set(BLACKPILL_F401CE_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(BLACKPILL_F401CE_hid_MAXSIZE 524288)
set(BLACKPILL_F401CE_hid_MAXDATASIZE 98304)
set(BLACKPILL_F401CE_hid_MCU cortex-m4)
set(BLACKPILL_F401CE_hid_FPCONF "-")
add_library(BLACKPILL_F401CE_hid INTERFACE)
target_compile_options(BLACKPILL_F401CE_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CE_hid_MCU}
)
target_compile_definitions(BLACKPILL_F401CE_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F401CE"
	"BOARD_NAME=\"BLACKPILL_F401CE\""
	"BOARD_ID=BLACKPILL_F401CE"
	"VARIANT_H=\"variant_BLACKPILL_F401Cx.h\""
)
target_include_directories(BLACKPILL_F401CE_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F401CE_hid_VARIANT_PATH}
)

target_link_options(BLACKPILL_F401CE_hid INTERFACE
  "LINKER:--default-script=${BLACKPILL_F401CE_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F401CE_hid_MCU}
)
target_link_libraries(BLACKPILL_F401CE_hid INTERFACE
  arm_cortexM4lf_math
)


# BLACKPILL_F411CE
# -----------------------------------------------------------------------------

set(BLACKPILL_F411CE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(BLACKPILL_F411CE_MAXSIZE 524288)
set(BLACKPILL_F411CE_MAXDATASIZE 131072)
set(BLACKPILL_F411CE_MCU cortex-m4)
set(BLACKPILL_F411CE_FPCONF "-")
add_library(BLACKPILL_F411CE INTERFACE)
target_compile_options(BLACKPILL_F411CE INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F411CE_MCU}
)
target_compile_definitions(BLACKPILL_F411CE INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F411CE"
	"BOARD_NAME=\"BLACKPILL_F411CE\""
	"BOARD_ID=BLACKPILL_F411CE"
	"VARIANT_H=\"variant_BLACKPILL_F411CE.h\""
)
target_include_directories(BLACKPILL_F411CE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F411CE_VARIANT_PATH}
)

target_link_options(BLACKPILL_F411CE INTERFACE
  "LINKER:--default-script=${BLACKPILL_F411CE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F411CE_MCU}
)
target_link_libraries(BLACKPILL_F411CE INTERFACE
  arm_cortexM4lf_math
)

add_library(BLACKPILL_F411CE_serial_disabled INTERFACE)
target_compile_options(BLACKPILL_F411CE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLACKPILL_F411CE_serial_generic INTERFACE)
target_compile_options(BLACKPILL_F411CE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLACKPILL_F411CE_serial_none INTERFACE)
target_compile_options(BLACKPILL_F411CE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLACKPILL_F411CE_hid
# -----------------------------------------------------------------------------

set(BLACKPILL_F411CE_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(BLACKPILL_F411CE_hid_MAXSIZE 524288)
set(BLACKPILL_F411CE_hid_MAXDATASIZE 131072)
set(BLACKPILL_F411CE_hid_MCU cortex-m4)
set(BLACKPILL_F411CE_hid_FPCONF "-")
add_library(BLACKPILL_F411CE_hid INTERFACE)
target_compile_options(BLACKPILL_F411CE_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F411CE_hid_MCU}
)
target_compile_definitions(BLACKPILL_F411CE_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLACKPILL_F411CE"
	"BOARD_NAME=\"BLACKPILL_F411CE\""
	"BOARD_ID=BLACKPILL_F411CE"
	"VARIANT_H=\"variant_BLACKPILL_F411CE.h\""
)
target_include_directories(BLACKPILL_F411CE_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLACKPILL_F411CE_hid_VARIANT_PATH}
)

target_link_options(BLACKPILL_F411CE_hid INTERFACE
  "LINKER:--default-script=${BLACKPILL_F411CE_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLACKPILL_F411CE_hid_MCU}
)
target_link_libraries(BLACKPILL_F411CE_hid INTERFACE
  arm_cortexM4lf_math
)


# BLUE_F407VE_MINI
# -----------------------------------------------------------------------------

set(BLUE_F407VE_MINI_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLUE_F407VE_MINI_MAXSIZE 524288)
set(BLUE_F407VE_MINI_MAXDATASIZE 131072)
set(BLUE_F407VE_MINI_MCU cortex-m4)
set(BLUE_F407VE_MINI_FPCONF "-")
add_library(BLUE_F407VE_MINI INTERFACE)
target_compile_options(BLUE_F407VE_MINI INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLUE_F407VE_MINI_MCU}
)
target_compile_definitions(BLUE_F407VE_MINI INTERFACE
  "STM32F4xx"
	"ARDUINO_BLUE_F407VE_MINI"
	"BOARD_NAME=\"BLUE_F407VE_MINI\""
	"BOARD_ID=BLUE_F407VE_MINI"
	"VARIANT_H=\"variant_BLUE_F407VE_MINI.h\""
)
target_include_directories(BLUE_F407VE_MINI INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLUE_F407VE_MINI_VARIANT_PATH}
)

target_link_options(BLUE_F407VE_MINI INTERFACE
  "LINKER:--default-script=${BLUE_F407VE_MINI_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLUE_F407VE_MINI_MCU}
)
target_link_libraries(BLUE_F407VE_MINI INTERFACE
  arm_cortexM4lf_math
)

add_library(BLUE_F407VE_MINI_serial_disabled INTERFACE)
target_compile_options(BLUE_F407VE_MINI_serial_disabled INTERFACE
  "SHELL:"
)
add_library(BLUE_F407VE_MINI_serial_generic INTERFACE)
target_compile_options(BLUE_F407VE_MINI_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(BLUE_F407VE_MINI_serial_none INTERFACE)
target_compile_options(BLUE_F407VE_MINI_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# BLUE_F407VE_MINI_hid
# -----------------------------------------------------------------------------

set(BLUE_F407VE_MINI_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(BLUE_F407VE_MINI_hid_MAXSIZE 524288)
set(BLUE_F407VE_MINI_hid_MAXDATASIZE 131072)
set(BLUE_F407VE_MINI_hid_MCU cortex-m4)
set(BLUE_F407VE_MINI_hid_FPCONF "-")
add_library(BLUE_F407VE_MINI_hid INTERFACE)
target_compile_options(BLUE_F407VE_MINI_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLUE_F407VE_MINI_hid_MCU}
)
target_compile_definitions(BLUE_F407VE_MINI_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_BLUE_F407VE_MINI"
	"BOARD_NAME=\"BLUE_F407VE_MINI\""
	"BOARD_ID=BLUE_F407VE_MINI"
	"VARIANT_H=\"variant_BLUE_F407VE_MINI.h\""
)
target_include_directories(BLUE_F407VE_MINI_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${BLUE_F407VE_MINI_hid_VARIANT_PATH}
)

target_link_options(BLUE_F407VE_MINI_hid INTERFACE
  "LINKER:--default-script=${BLUE_F407VE_MINI_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${BLUE_F407VE_MINI_hid_MCU}
)
target_link_libraries(BLUE_F407VE_MINI_hid INTERFACE
  arm_cortexM4lf_math
)


# CoreBoard_F401RC
# -----------------------------------------------------------------------------

set(CoreBoard_F401RC_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(CoreBoard_F401RC_MAXSIZE 262144)
set(CoreBoard_F401RC_MAXDATASIZE 65536)
set(CoreBoard_F401RC_MCU cortex-m4)
set(CoreBoard_F401RC_FPCONF "-")
add_library(CoreBoard_F401RC INTERFACE)
target_compile_options(CoreBoard_F401RC INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${CoreBoard_F401RC_MCU}
)
target_compile_definitions(CoreBoard_F401RC INTERFACE
  "STM32F4xx"
	"ARDUINO_CoreBoard_F401RC"
	"BOARD_NAME=\"CoreBoard_F401RC\""
	"BOARD_ID=CoreBoard_F401RC"
	"VARIANT_H=\"variant_CoreBoard_F401RC.h\""
)
target_include_directories(CoreBoard_F401RC INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${CoreBoard_F401RC_VARIANT_PATH}
)

target_link_options(CoreBoard_F401RC INTERFACE
  "LINKER:--default-script=${CoreBoard_F401RC_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${CoreBoard_F401RC_MCU}
)
target_link_libraries(CoreBoard_F401RC INTERFACE
  arm_cortexM4lf_math
)

add_library(CoreBoard_F401RC_serial_disabled INTERFACE)
target_compile_options(CoreBoard_F401RC_serial_disabled INTERFACE
  "SHELL:"
)
add_library(CoreBoard_F401RC_serial_generic INTERFACE)
target_compile_options(CoreBoard_F401RC_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(CoreBoard_F401RC_serial_none INTERFACE)
target_compile_options(CoreBoard_F401RC_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# CoreBoard_F401RC_hid
# -----------------------------------------------------------------------------

set(CoreBoard_F401RC_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(CoreBoard_F401RC_hid_MAXSIZE 262144)
set(CoreBoard_F401RC_hid_MAXDATASIZE 65536)
set(CoreBoard_F401RC_hid_MCU cortex-m4)
set(CoreBoard_F401RC_hid_FPCONF "-")
add_library(CoreBoard_F401RC_hid INTERFACE)
target_compile_options(CoreBoard_F401RC_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${CoreBoard_F401RC_hid_MCU}
)
target_compile_definitions(CoreBoard_F401RC_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_CoreBoard_F401RC"
	"BOARD_NAME=\"CoreBoard_F401RC\""
	"BOARD_ID=CoreBoard_F401RC"
	"VARIANT_H=\"variant_CoreBoard_F401RC.h\""
)
target_include_directories(CoreBoard_F401RC_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${CoreBoard_F401RC_hid_VARIANT_PATH}
)

target_link_options(CoreBoard_F401RC_hid INTERFACE
  "LINKER:--default-script=${CoreBoard_F401RC_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${CoreBoard_F401RC_hid_MCU}
)
target_link_libraries(CoreBoard_F401RC_hid INTERFACE
  arm_cortexM4lf_math
)


# DISCO_F407VG
# -----------------------------------------------------------------------------

set(DISCO_F407VG_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(DISCO_F407VG_MAXSIZE 1048576)
set(DISCO_F407VG_MAXDATASIZE 131072)
set(DISCO_F407VG_MCU cortex-m4)
set(DISCO_F407VG_FPCONF "fpv4-sp-d16-hard")
add_library(DISCO_F407VG INTERFACE)
target_compile_options(DISCO_F407VG INTERFACE
  "SHELL:-DSTM32F407xx  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DISCO_F407VG_MCU}
)
target_compile_definitions(DISCO_F407VG INTERFACE
  "STM32F4xx"
	"ARDUINO_DISCO_F407VG"
	"BOARD_NAME=\"DISCO_F407VG\""
	"BOARD_ID=DISCO_F407VG"
	"VARIANT_H=\"variant_DISCO_F407VG.h\""
)
target_include_directories(DISCO_F407VG INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${DISCO_F407VG_VARIANT_PATH}
)

target_link_options(DISCO_F407VG INTERFACE
  "LINKER:--default-script=${DISCO_F407VG_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DISCO_F407VG_MCU}
)
target_link_libraries(DISCO_F407VG INTERFACE
  arm_cortexM4lf_math
)

add_library(DISCO_F407VG_serial_disabled INTERFACE)
target_compile_options(DISCO_F407VG_serial_disabled INTERFACE
  "SHELL:"
)
add_library(DISCO_F407VG_serial_generic INTERFACE)
target_compile_options(DISCO_F407VG_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(DISCO_F407VG_serial_none INTERFACE)
target_compile_options(DISCO_F407VG_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(DISCO_F407VG_usb_CDC INTERFACE)
target_compile_options(DISCO_F407VG_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(DISCO_F407VG_usb_CDCgen INTERFACE)
target_compile_options(DISCO_F407VG_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(DISCO_F407VG_usb_HID INTERFACE)
target_compile_options(DISCO_F407VG_usb_HID INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE"
)
add_library(DISCO_F407VG_usb_none INTERFACE)
target_compile_options(DISCO_F407VG_usb_none INTERFACE
  "SHELL:"
)
add_library(DISCO_F407VG_xusb_FS INTERFACE)
target_compile_options(DISCO_F407VG_xusb_FS INTERFACE
  "SHELL:"
)
add_library(DISCO_F407VG_xusb_HS INTERFACE)
target_compile_options(DISCO_F407VG_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(DISCO_F407VG_xusb_HSFS INTERFACE)
target_compile_options(DISCO_F407VG_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
)

# DIYMORE_F407VGT
# -----------------------------------------------------------------------------

set(DIYMORE_F407VGT_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(DIYMORE_F407VGT_MAXSIZE 1048576)
set(DIYMORE_F407VGT_MAXDATASIZE 131072)
set(DIYMORE_F407VGT_MCU cortex-m4)
set(DIYMORE_F407VGT_FPCONF "-")
add_library(DIYMORE_F407VGT INTERFACE)
target_compile_options(DIYMORE_F407VGT INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DIYMORE_F407VGT_MCU}
)
target_compile_definitions(DIYMORE_F407VGT INTERFACE
  "STM32F4xx"
	"ARDUINO_DIYMORE_F407VGT"
	"BOARD_NAME=\"DIYMORE_F407VGT\""
	"BOARD_ID=DIYMORE_F407VGT"
	"VARIANT_H=\"variant_DIYMORE_F407VGT.h\""
)
target_include_directories(DIYMORE_F407VGT INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${DIYMORE_F407VGT_VARIANT_PATH}
)

target_link_options(DIYMORE_F407VGT INTERFACE
  "LINKER:--default-script=${DIYMORE_F407VGT_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DIYMORE_F407VGT_MCU}
)
target_link_libraries(DIYMORE_F407VGT INTERFACE
  arm_cortexM4lf_math
)

add_library(DIYMORE_F407VGT_serial_disabled INTERFACE)
target_compile_options(DIYMORE_F407VGT_serial_disabled INTERFACE
  "SHELL:"
)
add_library(DIYMORE_F407VGT_serial_generic INTERFACE)
target_compile_options(DIYMORE_F407VGT_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(DIYMORE_F407VGT_serial_none INTERFACE)
target_compile_options(DIYMORE_F407VGT_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# DIYMORE_F407VGT_hid
# -----------------------------------------------------------------------------

set(DIYMORE_F407VGT_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(DIYMORE_F407VGT_hid_MAXSIZE 1048576)
set(DIYMORE_F407VGT_hid_MAXDATASIZE 131072)
set(DIYMORE_F407VGT_hid_MCU cortex-m4)
set(DIYMORE_F407VGT_hid_FPCONF "-")
add_library(DIYMORE_F407VGT_hid INTERFACE)
target_compile_options(DIYMORE_F407VGT_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DIYMORE_F407VGT_hid_MCU}
)
target_compile_definitions(DIYMORE_F407VGT_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_DIYMORE_F407VGT"
	"BOARD_NAME=\"DIYMORE_F407VGT\""
	"BOARD_ID=DIYMORE_F407VGT"
	"VARIANT_H=\"variant_DIYMORE_F407VGT.h\""
)
target_include_directories(DIYMORE_F407VGT_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${DIYMORE_F407VGT_hid_VARIANT_PATH}
)

target_link_options(DIYMORE_F407VGT_hid INTERFACE
  "LINKER:--default-script=${DIYMORE_F407VGT_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DIYMORE_F407VGT_hid_MCU}
)
target_link_libraries(DIYMORE_F407VGT_hid INTERFACE
  arm_cortexM4lf_math
)


# FEATHER_F405
# -----------------------------------------------------------------------------

set(FEATHER_F405_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(FEATHER_F405_MAXSIZE 1048576)
set(FEATHER_F405_MAXDATASIZE 131072)
set(FEATHER_F405_MCU cortex-m4)
set(FEATHER_F405_FPCONF "-")
add_library(FEATHER_F405 INTERFACE)
target_compile_options(FEATHER_F405 INTERFACE
  "SHELL:-DSTM32F405xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FEATHER_F405_MCU}
)
target_compile_definitions(FEATHER_F405 INTERFACE
  "STM32F4xx"
	"ARDUINO_FEATHER_F405"
	"BOARD_NAME=\"FEATHER_F405\""
	"BOARD_ID=FEATHER_F405"
	"VARIANT_H=\"variant_FEATHER_F405.h\""
)
target_include_directories(FEATHER_F405 INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${FEATHER_F405_VARIANT_PATH}
)

target_link_options(FEATHER_F405 INTERFACE
  "LINKER:--default-script=${FEATHER_F405_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FEATHER_F405_MCU}
)
target_link_libraries(FEATHER_F405 INTERFACE
  arm_cortexM4lf_math
)

add_library(FEATHER_F405_serial_disabled INTERFACE)
target_compile_options(FEATHER_F405_serial_disabled INTERFACE
  "SHELL:"
)
add_library(FEATHER_F405_serial_generic INTERFACE)
target_compile_options(FEATHER_F405_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(FEATHER_F405_serial_none INTERFACE)
target_compile_options(FEATHER_F405_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# FEATHER_F405_hid
# -----------------------------------------------------------------------------

set(FEATHER_F405_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(FEATHER_F405_hid_MAXSIZE 1048576)
set(FEATHER_F405_hid_MAXDATASIZE 131072)
set(FEATHER_F405_hid_MCU cortex-m4)
set(FEATHER_F405_hid_FPCONF "-")
add_library(FEATHER_F405_hid INTERFACE)
target_compile_options(FEATHER_F405_hid INTERFACE
  "SHELL:-DSTM32F405xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FEATHER_F405_hid_MCU}
)
target_compile_definitions(FEATHER_F405_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_FEATHER_F405"
	"BOARD_NAME=\"FEATHER_F405\""
	"BOARD_ID=FEATHER_F405"
	"VARIANT_H=\"variant_FEATHER_F405.h\""
)
target_include_directories(FEATHER_F405_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${FEATHER_F405_hid_VARIANT_PATH}
)

target_link_options(FEATHER_F405_hid INTERFACE
  "LINKER:--default-script=${FEATHER_F405_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FEATHER_F405_hid_MCU}
)
target_link_libraries(FEATHER_F405_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CBUX
# -----------------------------------------------------------------------------

set(GENERIC_F401CBUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CBUX_MAXSIZE 131072)
set(GENERIC_F401CBUX_MAXDATASIZE 65536)
set(GENERIC_F401CBUX_MCU cortex-m4)
set(GENERIC_F401CBUX_FPCONF "-")
add_library(GENERIC_F401CBUX INTERFACE)
target_compile_options(GENERIC_F401CBUX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBUX_MCU}
)
target_compile_definitions(GENERIC_F401CBUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CBUX"
	"BOARD_NAME=\"GENERIC_F401CBUX\""
	"BOARD_ID=GENERIC_F401CBUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CBUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CBUX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CBUX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CBUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBUX_MCU}
)
target_link_libraries(GENERIC_F401CBUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CBUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CBUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CBUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CBUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CBUX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CBUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CBUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CBUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CBUX_hid_MAXSIZE 131072)
set(GENERIC_F401CBUX_hid_MAXDATASIZE 65536)
set(GENERIC_F401CBUX_hid_MCU cortex-m4)
set(GENERIC_F401CBUX_hid_FPCONF "-")
add_library(GENERIC_F401CBUX_hid INTERFACE)
target_compile_options(GENERIC_F401CBUX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBUX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CBUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CBUX"
	"BOARD_NAME=\"GENERIC_F401CBUX\""
	"BOARD_ID=GENERIC_F401CBUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CBUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CBUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CBUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CBUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBUX_hid_MCU}
)
target_link_libraries(GENERIC_F401CBUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CBYX
# -----------------------------------------------------------------------------

set(GENERIC_F401CBYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CBYX_MAXSIZE 131072)
set(GENERIC_F401CBYX_MAXDATASIZE 65536)
set(GENERIC_F401CBYX_MCU cortex-m4)
set(GENERIC_F401CBYX_FPCONF "-")
add_library(GENERIC_F401CBYX INTERFACE)
target_compile_options(GENERIC_F401CBYX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBYX_MCU}
)
target_compile_definitions(GENERIC_F401CBYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CBYX"
	"BOARD_NAME=\"GENERIC_F401CBYX\""
	"BOARD_ID=GENERIC_F401CBYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CBYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CBYX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CBYX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CBYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBYX_MCU}
)
target_link_libraries(GENERIC_F401CBYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CBYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CBYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CBYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CBYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CBYX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CBYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CBYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CBYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CBYX_hid_MAXSIZE 131072)
set(GENERIC_F401CBYX_hid_MAXDATASIZE 65536)
set(GENERIC_F401CBYX_hid_MCU cortex-m4)
set(GENERIC_F401CBYX_hid_FPCONF "-")
add_library(GENERIC_F401CBYX_hid INTERFACE)
target_compile_options(GENERIC_F401CBYX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBYX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CBYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CBYX"
	"BOARD_NAME=\"GENERIC_F401CBYX\""
	"BOARD_ID=GENERIC_F401CBYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CBYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CBYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CBYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CBYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CBYX_hid_MCU}
)
target_link_libraries(GENERIC_F401CBYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CCFX
# -----------------------------------------------------------------------------

set(GENERIC_F401CCFX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCFX_MAXSIZE 262144)
set(GENERIC_F401CCFX_MAXDATASIZE 65536)
set(GENERIC_F401CCFX_MCU cortex-m4)
set(GENERIC_F401CCFX_FPCONF "-")
add_library(GENERIC_F401CCFX INTERFACE)
target_compile_options(GENERIC_F401CCFX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCFX_MCU}
)
target_compile_definitions(GENERIC_F401CCFX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCFX"
	"BOARD_NAME=\"GENERIC_F401CCFX\""
	"BOARD_ID=GENERIC_F401CCFX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCFX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCFX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCFX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCFX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCFX_MCU}
)
target_link_libraries(GENERIC_F401CCFX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CCFX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CCFX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CCFX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CCFX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CCFX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CCFX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CCFX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CCFX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCFX_hid_MAXSIZE 262144)
set(GENERIC_F401CCFX_hid_MAXDATASIZE 65536)
set(GENERIC_F401CCFX_hid_MCU cortex-m4)
set(GENERIC_F401CCFX_hid_FPCONF "-")
add_library(GENERIC_F401CCFX_hid INTERFACE)
target_compile_options(GENERIC_F401CCFX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCFX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CCFX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCFX"
	"BOARD_NAME=\"GENERIC_F401CCFX\""
	"BOARD_ID=GENERIC_F401CCFX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCFX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCFX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCFX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCFX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCFX_hid_MCU}
)
target_link_libraries(GENERIC_F401CCFX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CCUX
# -----------------------------------------------------------------------------

set(GENERIC_F401CCUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCUX_MAXSIZE 262144)
set(GENERIC_F401CCUX_MAXDATASIZE 65536)
set(GENERIC_F401CCUX_MCU cortex-m4)
set(GENERIC_F401CCUX_FPCONF "-")
add_library(GENERIC_F401CCUX INTERFACE)
target_compile_options(GENERIC_F401CCUX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCUX_MCU}
)
target_compile_definitions(GENERIC_F401CCUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCUX"
	"BOARD_NAME=\"GENERIC_F401CCUX\""
	"BOARD_ID=GENERIC_F401CCUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCUX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCUX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCUX_MCU}
)
target_link_libraries(GENERIC_F401CCUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CCUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CCUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CCUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CCUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CCUX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CCUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CCUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CCUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCUX_hid_MAXSIZE 262144)
set(GENERIC_F401CCUX_hid_MAXDATASIZE 65536)
set(GENERIC_F401CCUX_hid_MCU cortex-m4)
set(GENERIC_F401CCUX_hid_FPCONF "-")
add_library(GENERIC_F401CCUX_hid INTERFACE)
target_compile_options(GENERIC_F401CCUX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCUX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CCUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCUX"
	"BOARD_NAME=\"GENERIC_F401CCUX\""
	"BOARD_ID=GENERIC_F401CCUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCUX_hid_MCU}
)
target_link_libraries(GENERIC_F401CCUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CCYX
# -----------------------------------------------------------------------------

set(GENERIC_F401CCYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCYX_MAXSIZE 262144)
set(GENERIC_F401CCYX_MAXDATASIZE 65536)
set(GENERIC_F401CCYX_MCU cortex-m4)
set(GENERIC_F401CCYX_FPCONF "-")
add_library(GENERIC_F401CCYX INTERFACE)
target_compile_options(GENERIC_F401CCYX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCYX_MCU}
)
target_compile_definitions(GENERIC_F401CCYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCYX"
	"BOARD_NAME=\"GENERIC_F401CCYX\""
	"BOARD_ID=GENERIC_F401CCYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCYX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCYX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCYX_MCU}
)
target_link_libraries(GENERIC_F401CCYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CCYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CCYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CCYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CCYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CCYX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CCYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CCYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CCYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CCYX_hid_MAXSIZE 262144)
set(GENERIC_F401CCYX_hid_MAXDATASIZE 65536)
set(GENERIC_F401CCYX_hid_MCU cortex-m4)
set(GENERIC_F401CCYX_hid_FPCONF "-")
add_library(GENERIC_F401CCYX_hid INTERFACE)
target_compile_options(GENERIC_F401CCYX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCYX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CCYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CCYX"
	"BOARD_NAME=\"GENERIC_F401CCYX\""
	"BOARD_ID=GENERIC_F401CCYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CCYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CCYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CCYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CCYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CCYX_hid_MCU}
)
target_link_libraries(GENERIC_F401CCYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CDUX
# -----------------------------------------------------------------------------

set(GENERIC_F401CDUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CDUX_MAXSIZE 393216)
set(GENERIC_F401CDUX_MAXDATASIZE 98304)
set(GENERIC_F401CDUX_MCU cortex-m4)
set(GENERIC_F401CDUX_FPCONF "-")
add_library(GENERIC_F401CDUX INTERFACE)
target_compile_options(GENERIC_F401CDUX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDUX_MCU}
)
target_compile_definitions(GENERIC_F401CDUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CDUX"
	"BOARD_NAME=\"GENERIC_F401CDUX\""
	"BOARD_ID=GENERIC_F401CDUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CDUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CDUX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CDUX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CDUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDUX_MCU}
)
target_link_libraries(GENERIC_F401CDUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CDUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CDUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CDUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CDUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CDUX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CDUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CDUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CDUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CDUX_hid_MAXSIZE 393216)
set(GENERIC_F401CDUX_hid_MAXDATASIZE 98304)
set(GENERIC_F401CDUX_hid_MCU cortex-m4)
set(GENERIC_F401CDUX_hid_FPCONF "-")
add_library(GENERIC_F401CDUX_hid INTERFACE)
target_compile_options(GENERIC_F401CDUX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDUX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CDUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CDUX"
	"BOARD_NAME=\"GENERIC_F401CDUX\""
	"BOARD_ID=GENERIC_F401CDUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CDUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CDUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CDUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CDUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDUX_hid_MCU}
)
target_link_libraries(GENERIC_F401CDUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CDYX
# -----------------------------------------------------------------------------

set(GENERIC_F401CDYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CDYX_MAXSIZE 393216)
set(GENERIC_F401CDYX_MAXDATASIZE 98304)
set(GENERIC_F401CDYX_MCU cortex-m4)
set(GENERIC_F401CDYX_FPCONF "-")
add_library(GENERIC_F401CDYX INTERFACE)
target_compile_options(GENERIC_F401CDYX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDYX_MCU}
)
target_compile_definitions(GENERIC_F401CDYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CDYX"
	"BOARD_NAME=\"GENERIC_F401CDYX\""
	"BOARD_ID=GENERIC_F401CDYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CDYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CDYX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CDYX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CDYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDYX_MCU}
)
target_link_libraries(GENERIC_F401CDYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CDYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CDYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CDYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CDYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CDYX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CDYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CDYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CDYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CDYX_hid_MAXSIZE 393216)
set(GENERIC_F401CDYX_hid_MAXDATASIZE 98304)
set(GENERIC_F401CDYX_hid_MCU cortex-m4)
set(GENERIC_F401CDYX_hid_FPCONF "-")
add_library(GENERIC_F401CDYX_hid INTERFACE)
target_compile_options(GENERIC_F401CDYX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDYX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CDYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CDYX"
	"BOARD_NAME=\"GENERIC_F401CDYX\""
	"BOARD_ID=GENERIC_F401CDYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CDYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CDYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CDYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CDYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CDYX_hid_MCU}
)
target_link_libraries(GENERIC_F401CDYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CEUX
# -----------------------------------------------------------------------------

set(GENERIC_F401CEUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CEUX_MAXSIZE 524288)
set(GENERIC_F401CEUX_MAXDATASIZE 98304)
set(GENERIC_F401CEUX_MCU cortex-m4)
set(GENERIC_F401CEUX_FPCONF "-")
add_library(GENERIC_F401CEUX INTERFACE)
target_compile_options(GENERIC_F401CEUX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEUX_MCU}
)
target_compile_definitions(GENERIC_F401CEUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CEUX"
	"BOARD_NAME=\"GENERIC_F401CEUX\""
	"BOARD_ID=GENERIC_F401CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CEUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CEUX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CEUX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CEUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEUX_MCU}
)
target_link_libraries(GENERIC_F401CEUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CEUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CEUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CEUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CEUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CEUX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CEUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CEUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CEUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CEUX_hid_MAXSIZE 524288)
set(GENERIC_F401CEUX_hid_MAXDATASIZE 98304)
set(GENERIC_F401CEUX_hid_MCU cortex-m4)
set(GENERIC_F401CEUX_hid_FPCONF "-")
add_library(GENERIC_F401CEUX_hid INTERFACE)
target_compile_options(GENERIC_F401CEUX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEUX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CEUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CEUX"
	"BOARD_NAME=\"GENERIC_F401CEUX\""
	"BOARD_ID=GENERIC_F401CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CEUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CEUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CEUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CEUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEUX_hid_MCU}
)
target_link_libraries(GENERIC_F401CEUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401CEYX
# -----------------------------------------------------------------------------

set(GENERIC_F401CEYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CEYX_MAXSIZE 524288)
set(GENERIC_F401CEYX_MAXDATASIZE 98304)
set(GENERIC_F401CEYX_MCU cortex-m4)
set(GENERIC_F401CEYX_FPCONF "-")
add_library(GENERIC_F401CEYX INTERFACE)
target_compile_options(GENERIC_F401CEYX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEYX_MCU}
)
target_compile_definitions(GENERIC_F401CEYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CEYX"
	"BOARD_NAME=\"GENERIC_F401CEYX\""
	"BOARD_ID=GENERIC_F401CEYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CEYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CEYX_VARIANT_PATH}
)

target_link_options(GENERIC_F401CEYX INTERFACE
  "LINKER:--default-script=${GENERIC_F401CEYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEYX_MCU}
)
target_link_libraries(GENERIC_F401CEYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401CEYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401CEYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401CEYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401CEYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401CEYX_serial_none INTERFACE)
target_compile_options(GENERIC_F401CEYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401CEYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401CEYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401CC(F-U-Y)_F401C(B-D-E)(U-Y)")
set(GENERIC_F401CEYX_hid_MAXSIZE 524288)
set(GENERIC_F401CEYX_hid_MAXDATASIZE 98304)
set(GENERIC_F401CEYX_hid_MCU cortex-m4)
set(GENERIC_F401CEYX_hid_FPCONF "-")
add_library(GENERIC_F401CEYX_hid INTERFACE)
target_compile_options(GENERIC_F401CEYX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEYX_hid_MCU}
)
target_compile_definitions(GENERIC_F401CEYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401CEYX"
	"BOARD_NAME=\"GENERIC_F401CEYX\""
	"BOARD_ID=GENERIC_F401CEYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401CEYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401CEYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401CEYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401CEYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401CEYX_hid_MCU}
)
target_link_libraries(GENERIC_F401CEYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401RBTX
# -----------------------------------------------------------------------------

set(GENERIC_F401RBTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RBTX_MAXSIZE 131072)
set(GENERIC_F401RBTX_MAXDATASIZE 65536)
set(GENERIC_F401RBTX_MCU cortex-m4)
set(GENERIC_F401RBTX_FPCONF "-")
add_library(GENERIC_F401RBTX INTERFACE)
target_compile_options(GENERIC_F401RBTX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RBTX_MCU}
)
target_compile_definitions(GENERIC_F401RBTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RBTX"
	"BOARD_NAME=\"GENERIC_F401RBTX\""
	"BOARD_ID=GENERIC_F401RBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RBTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RBTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401RBTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401RBTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RBTX_MCU}
)
target_link_libraries(GENERIC_F401RBTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401RBTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401RBTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401RBTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401RBTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401RBTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401RBTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401RBTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401RBTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RBTX_hid_MAXSIZE 131072)
set(GENERIC_F401RBTX_hid_MAXDATASIZE 65536)
set(GENERIC_F401RBTX_hid_MCU cortex-m4)
set(GENERIC_F401RBTX_hid_FPCONF "-")
add_library(GENERIC_F401RBTX_hid INTERFACE)
target_compile_options(GENERIC_F401RBTX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RBTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401RBTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RBTX"
	"BOARD_NAME=\"GENERIC_F401RBTX\""
	"BOARD_ID=GENERIC_F401RBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RBTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RBTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401RBTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401RBTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RBTX_hid_MCU}
)
target_link_libraries(GENERIC_F401RBTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401RCTX
# -----------------------------------------------------------------------------

set(GENERIC_F401RCTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RCTX_MAXSIZE 262144)
set(GENERIC_F401RCTX_MAXDATASIZE 65536)
set(GENERIC_F401RCTX_MCU cortex-m4)
set(GENERIC_F401RCTX_FPCONF "-")
add_library(GENERIC_F401RCTX INTERFACE)
target_compile_options(GENERIC_F401RCTX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RCTX_MCU}
)
target_compile_definitions(GENERIC_F401RCTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RCTX"
	"BOARD_NAME=\"GENERIC_F401RCTX\""
	"BOARD_ID=GENERIC_F401RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RCTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RCTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401RCTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401RCTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RCTX_MCU}
)
target_link_libraries(GENERIC_F401RCTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401RCTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401RCTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401RCTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401RCTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401RCTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401RCTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401RCTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401RCTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RCTX_hid_MAXSIZE 262144)
set(GENERIC_F401RCTX_hid_MAXDATASIZE 65536)
set(GENERIC_F401RCTX_hid_MCU cortex-m4)
set(GENERIC_F401RCTX_hid_FPCONF "-")
add_library(GENERIC_F401RCTX_hid INTERFACE)
target_compile_options(GENERIC_F401RCTX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RCTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401RCTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RCTX"
	"BOARD_NAME=\"GENERIC_F401RCTX\""
	"BOARD_ID=GENERIC_F401RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RCTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RCTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401RCTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401RCTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RCTX_hid_MCU}
)
target_link_libraries(GENERIC_F401RCTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401RDTX
# -----------------------------------------------------------------------------

set(GENERIC_F401RDTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RDTX_MAXSIZE 393216)
set(GENERIC_F401RDTX_MAXDATASIZE 98304)
set(GENERIC_F401RDTX_MCU cortex-m4)
set(GENERIC_F401RDTX_FPCONF "-")
add_library(GENERIC_F401RDTX INTERFACE)
target_compile_options(GENERIC_F401RDTX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RDTX_MCU}
)
target_compile_definitions(GENERIC_F401RDTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RDTX"
	"BOARD_NAME=\"GENERIC_F401RDTX\""
	"BOARD_ID=GENERIC_F401RDTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RDTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RDTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401RDTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401RDTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RDTX_MCU}
)
target_link_libraries(GENERIC_F401RDTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401RDTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401RDTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401RDTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401RDTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401RDTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401RDTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401RDTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401RDTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RDTX_hid_MAXSIZE 393216)
set(GENERIC_F401RDTX_hid_MAXDATASIZE 98304)
set(GENERIC_F401RDTX_hid_MCU cortex-m4)
set(GENERIC_F401RDTX_hid_FPCONF "-")
add_library(GENERIC_F401RDTX_hid INTERFACE)
target_compile_options(GENERIC_F401RDTX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RDTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401RDTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RDTX"
	"BOARD_NAME=\"GENERIC_F401RDTX\""
	"BOARD_ID=GENERIC_F401RDTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RDTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RDTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401RDTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401RDTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RDTX_hid_MCU}
)
target_link_libraries(GENERIC_F401RDTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401RETX
# -----------------------------------------------------------------------------

set(GENERIC_F401RETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RETX_MAXSIZE 524288)
set(GENERIC_F401RETX_MAXDATASIZE 98304)
set(GENERIC_F401RETX_MCU cortex-m4)
set(GENERIC_F401RETX_FPCONF "-")
add_library(GENERIC_F401RETX INTERFACE)
target_compile_options(GENERIC_F401RETX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RETX_MCU}
)
target_compile_definitions(GENERIC_F401RETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RETX"
	"BOARD_NAME=\"GENERIC_F401RETX\""
	"BOARD_ID=GENERIC_F401RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RETX_VARIANT_PATH}
)

target_link_options(GENERIC_F401RETX INTERFACE
  "LINKER:--default-script=${GENERIC_F401RETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RETX_MCU}
)
target_link_libraries(GENERIC_F401RETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401RETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401RETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401RETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401RETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401RETX_serial_none INTERFACE)
target_compile_options(GENERIC_F401RETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401RETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401RETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(GENERIC_F401RETX_hid_MAXSIZE 524288)
set(GENERIC_F401RETX_hid_MAXDATASIZE 98304)
set(GENERIC_F401RETX_hid_MCU cortex-m4)
set(GENERIC_F401RETX_hid_FPCONF "-")
add_library(GENERIC_F401RETX_hid INTERFACE)
target_compile_options(GENERIC_F401RETX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RETX_hid_MCU}
)
target_compile_definitions(GENERIC_F401RETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401RETX"
	"BOARD_NAME=\"GENERIC_F401RETX\""
	"BOARD_ID=GENERIC_F401RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401RETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401RETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401RETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401RETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401RETX_hid_MCU}
)
target_link_libraries(GENERIC_F401RETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401VBTX
# -----------------------------------------------------------------------------

set(GENERIC_F401VBTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VBTX_MAXSIZE 131072)
set(GENERIC_F401VBTX_MAXDATASIZE 65536)
set(GENERIC_F401VBTX_MCU cortex-m4)
set(GENERIC_F401VBTX_FPCONF "-")
add_library(GENERIC_F401VBTX INTERFACE)
target_compile_options(GENERIC_F401VBTX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VBTX_MCU}
)
target_compile_definitions(GENERIC_F401VBTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VBTX"
	"BOARD_NAME=\"GENERIC_F401VBTX\""
	"BOARD_ID=GENERIC_F401VBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VBTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VBTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401VBTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401VBTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VBTX_MCU}
)
target_link_libraries(GENERIC_F401VBTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401VBTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401VBTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401VBTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401VBTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401VBTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401VBTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401VBTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401VBTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VBTX_hid_MAXSIZE 131072)
set(GENERIC_F401VBTX_hid_MAXDATASIZE 65536)
set(GENERIC_F401VBTX_hid_MCU cortex-m4)
set(GENERIC_F401VBTX_hid_FPCONF "-")
add_library(GENERIC_F401VBTX_hid INTERFACE)
target_compile_options(GENERIC_F401VBTX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VBTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401VBTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VBTX"
	"BOARD_NAME=\"GENERIC_F401VBTX\""
	"BOARD_ID=GENERIC_F401VBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VBTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VBTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401VBTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401VBTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VBTX_hid_MCU}
)
target_link_libraries(GENERIC_F401VBTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401VCTX
# -----------------------------------------------------------------------------

set(GENERIC_F401VCTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VCTX_MAXSIZE 262144)
set(GENERIC_F401VCTX_MAXDATASIZE 65536)
set(GENERIC_F401VCTX_MCU cortex-m4)
set(GENERIC_F401VCTX_FPCONF "-")
add_library(GENERIC_F401VCTX INTERFACE)
target_compile_options(GENERIC_F401VCTX INTERFACE
  "SHELL:-DSTM32F401xC   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VCTX_MCU}
)
target_compile_definitions(GENERIC_F401VCTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VCTX"
	"BOARD_NAME=\"GENERIC_F401VCTX\""
	"BOARD_ID=GENERIC_F401VCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VCTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VCTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401VCTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401VCTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VCTX_MCU}
)
target_link_libraries(GENERIC_F401VCTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401VCTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401VCTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401VCTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401VCTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401VCTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401VCTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401VCTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401VCTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VCTX_hid_MAXSIZE 262144)
set(GENERIC_F401VCTX_hid_MAXDATASIZE 65536)
set(GENERIC_F401VCTX_hid_MCU cortex-m4)
set(GENERIC_F401VCTX_hid_FPCONF "-")
add_library(GENERIC_F401VCTX_hid INTERFACE)
target_compile_options(GENERIC_F401VCTX_hid INTERFACE
  "SHELL:-DSTM32F401xC  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VCTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401VCTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VCTX"
	"BOARD_NAME=\"GENERIC_F401VCTX\""
	"BOARD_ID=GENERIC_F401VCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VCTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VCTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401VCTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401VCTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=65536"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VCTX_hid_MCU}
)
target_link_libraries(GENERIC_F401VCTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401VDTX
# -----------------------------------------------------------------------------

set(GENERIC_F401VDTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VDTX_MAXSIZE 393216)
set(GENERIC_F401VDTX_MAXDATASIZE 98304)
set(GENERIC_F401VDTX_MCU cortex-m4)
set(GENERIC_F401VDTX_FPCONF "-")
add_library(GENERIC_F401VDTX INTERFACE)
target_compile_options(GENERIC_F401VDTX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VDTX_MCU}
)
target_compile_definitions(GENERIC_F401VDTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VDTX"
	"BOARD_NAME=\"GENERIC_F401VDTX\""
	"BOARD_ID=GENERIC_F401VDTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VDTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VDTX_VARIANT_PATH}
)

target_link_options(GENERIC_F401VDTX INTERFACE
  "LINKER:--default-script=${GENERIC_F401VDTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VDTX_MCU}
)
target_link_libraries(GENERIC_F401VDTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401VDTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401VDTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401VDTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401VDTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401VDTX_serial_none INTERFACE)
target_compile_options(GENERIC_F401VDTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401VDTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401VDTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VDTX_hid_MAXSIZE 393216)
set(GENERIC_F401VDTX_hid_MAXDATASIZE 98304)
set(GENERIC_F401VDTX_hid_MCU cortex-m4)
set(GENERIC_F401VDTX_hid_FPCONF "-")
add_library(GENERIC_F401VDTX_hid INTERFACE)
target_compile_options(GENERIC_F401VDTX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VDTX_hid_MCU}
)
target_compile_definitions(GENERIC_F401VDTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VDTX"
	"BOARD_NAME=\"GENERIC_F401VDTX\""
	"BOARD_ID=GENERIC_F401VDTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VDTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VDTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401VDTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401VDTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=393216"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VDTX_hid_MCU}
)
target_link_libraries(GENERIC_F401VDTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F401VETX
# -----------------------------------------------------------------------------

set(GENERIC_F401VETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VETX_MAXSIZE 524288)
set(GENERIC_F401VETX_MAXDATASIZE 98304)
set(GENERIC_F401VETX_MCU cortex-m4)
set(GENERIC_F401VETX_FPCONF "-")
add_library(GENERIC_F401VETX INTERFACE)
target_compile_options(GENERIC_F401VETX INTERFACE
  "SHELL:-DSTM32F401xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VETX_MCU}
)
target_compile_definitions(GENERIC_F401VETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VETX"
	"BOARD_NAME=\"GENERIC_F401VETX\""
	"BOARD_ID=GENERIC_F401VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VETX_VARIANT_PATH}
)

target_link_options(GENERIC_F401VETX INTERFACE
  "LINKER:--default-script=${GENERIC_F401VETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VETX_MCU}
)
target_link_libraries(GENERIC_F401VETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F401VETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F401VETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F401VETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F401VETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F401VETX_serial_none INTERFACE)
target_compile_options(GENERIC_F401VETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F401VETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F401VETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401V(B-C-D-E)T")
set(GENERIC_F401VETX_hid_MAXSIZE 524288)
set(GENERIC_F401VETX_hid_MAXDATASIZE 98304)
set(GENERIC_F401VETX_hid_MCU cortex-m4)
set(GENERIC_F401VETX_hid_FPCONF "-")
add_library(GENERIC_F401VETX_hid INTERFACE)
target_compile_options(GENERIC_F401VETX_hid INTERFACE
  "SHELL:-DSTM32F401xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VETX_hid_MCU}
)
target_compile_definitions(GENERIC_F401VETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F401VETX"
	"BOARD_NAME=\"GENERIC_F401VETX\""
	"BOARD_ID=GENERIC_F401VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F401VETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F401VETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F401VETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F401VETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F401VETX_hid_MCU}
)
target_link_libraries(GENERIC_F401VETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F405RGTX
# -----------------------------------------------------------------------------

set(GENERIC_F405RGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(GENERIC_F405RGTX_MAXSIZE 1048576)
set(GENERIC_F405RGTX_MAXDATASIZE 131072)
set(GENERIC_F405RGTX_MCU cortex-m4)
set(GENERIC_F405RGTX_FPCONF "-")
add_library(GENERIC_F405RGTX INTERFACE)
target_compile_options(GENERIC_F405RGTX INTERFACE
  "SHELL:-DSTM32F405xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F405RGTX_MCU}
)
target_compile_definitions(GENERIC_F405RGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F405RGTX"
	"BOARD_NAME=\"GENERIC_F405RGTX\""
	"BOARD_ID=GENERIC_F405RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F405RGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F405RGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F405RGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F405RGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F405RGTX_MCU}
)
target_link_libraries(GENERIC_F405RGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F405RGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F405RGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F405RGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F405RGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F405RGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F405RGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F405RGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F405RGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(GENERIC_F405RGTX_hid_MAXSIZE 1048576)
set(GENERIC_F405RGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F405RGTX_hid_MCU cortex-m4)
set(GENERIC_F405RGTX_hid_FPCONF "-")
add_library(GENERIC_F405RGTX_hid INTERFACE)
target_compile_options(GENERIC_F405RGTX_hid INTERFACE
  "SHELL:-DSTM32F405xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F405RGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F405RGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F405RGTX"
	"BOARD_NAME=\"GENERIC_F405RGTX\""
	"BOARD_ID=GENERIC_F405RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F405RGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F405RGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F405RGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F405RGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F405RGTX_hid_MCU}
)
target_link_libraries(GENERIC_F405RGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F407VETX
# -----------------------------------------------------------------------------

set(GENERIC_F407VETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F407VETX_MAXSIZE 524288)
set(GENERIC_F407VETX_MAXDATASIZE 131072)
set(GENERIC_F407VETX_MCU cortex-m4)
set(GENERIC_F407VETX_FPCONF "-")
add_library(GENERIC_F407VETX INTERFACE)
target_compile_options(GENERIC_F407VETX INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VETX_MCU}
)
target_compile_definitions(GENERIC_F407VETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407VETX"
	"BOARD_NAME=\"GENERIC_F407VETX\""
	"BOARD_ID=GENERIC_F407VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407VETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407VETX_VARIANT_PATH}
)

target_link_options(GENERIC_F407VETX INTERFACE
  "LINKER:--default-script=${GENERIC_F407VETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VETX_MCU}
)
target_link_libraries(GENERIC_F407VETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F407VETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F407VETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F407VETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F407VETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F407VETX_serial_none INTERFACE)
target_compile_options(GENERIC_F407VETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F407VETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F407VETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F407VETX_hid_MAXSIZE 524288)
set(GENERIC_F407VETX_hid_MAXDATASIZE 131072)
set(GENERIC_F407VETX_hid_MCU cortex-m4)
set(GENERIC_F407VETX_hid_FPCONF "-")
add_library(GENERIC_F407VETX_hid INTERFACE)
target_compile_options(GENERIC_F407VETX_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VETX_hid_MCU}
)
target_compile_definitions(GENERIC_F407VETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407VETX"
	"BOARD_NAME=\"GENERIC_F407VETX\""
	"BOARD_ID=GENERIC_F407VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407VETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407VETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F407VETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F407VETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VETX_hid_MCU}
)
target_link_libraries(GENERIC_F407VETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F407VGTX
# -----------------------------------------------------------------------------

set(GENERIC_F407VGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F407VGTX_MAXSIZE 1048576)
set(GENERIC_F407VGTX_MAXDATASIZE 131072)
set(GENERIC_F407VGTX_MCU cortex-m4)
set(GENERIC_F407VGTX_FPCONF "-")
add_library(GENERIC_F407VGTX INTERFACE)
target_compile_options(GENERIC_F407VGTX INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VGTX_MCU}
)
target_compile_definitions(GENERIC_F407VGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407VGTX"
	"BOARD_NAME=\"GENERIC_F407VGTX\""
	"BOARD_ID=GENERIC_F407VGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407VGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407VGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F407VGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F407VGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VGTX_MCU}
)
target_link_libraries(GENERIC_F407VGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F407VGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F407VGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F407VGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F407VGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F407VGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F407VGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F407VGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F407VGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F407VGTX_hid_MAXSIZE 1048576)
set(GENERIC_F407VGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F407VGTX_hid_MCU cortex-m4)
set(GENERIC_F407VGTX_hid_FPCONF "-")
add_library(GENERIC_F407VGTX_hid INTERFACE)
target_compile_options(GENERIC_F407VGTX_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F407VGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407VGTX"
	"BOARD_NAME=\"GENERIC_F407VGTX\""
	"BOARD_ID=GENERIC_F407VGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407VGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407VGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F407VGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F407VGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407VGTX_hid_MCU}
)
target_link_libraries(GENERIC_F407VGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F407ZETX
# -----------------------------------------------------------------------------

set(GENERIC_F407ZETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F407ZETX_MAXSIZE 524288)
set(GENERIC_F407ZETX_MAXDATASIZE 131072)
set(GENERIC_F407ZETX_MCU cortex-m4)
set(GENERIC_F407ZETX_FPCONF "-")
add_library(GENERIC_F407ZETX INTERFACE)
target_compile_options(GENERIC_F407ZETX INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZETX_MCU}
)
target_compile_definitions(GENERIC_F407ZETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407ZETX"
	"BOARD_NAME=\"GENERIC_F407ZETX\""
	"BOARD_ID=GENERIC_F407ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407ZETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407ZETX_VARIANT_PATH}
)

target_link_options(GENERIC_F407ZETX INTERFACE
  "LINKER:--default-script=${GENERIC_F407ZETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZETX_MCU}
)
target_link_libraries(GENERIC_F407ZETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F407ZETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F407ZETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F407ZETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F407ZETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F407ZETX_serial_none INTERFACE)
target_compile_options(GENERIC_F407ZETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F407ZETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F407ZETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F407ZETX_hid_MAXSIZE 524288)
set(GENERIC_F407ZETX_hid_MAXDATASIZE 131072)
set(GENERIC_F407ZETX_hid_MCU cortex-m4)
set(GENERIC_F407ZETX_hid_FPCONF "-")
add_library(GENERIC_F407ZETX_hid INTERFACE)
target_compile_options(GENERIC_F407ZETX_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZETX_hid_MCU}
)
target_compile_definitions(GENERIC_F407ZETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407ZETX"
	"BOARD_NAME=\"GENERIC_F407ZETX\""
	"BOARD_ID=GENERIC_F407ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407ZETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407ZETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F407ZETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F407ZETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZETX_hid_MCU}
)
target_link_libraries(GENERIC_F407ZETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F407ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F407ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F407ZGTX_MAXSIZE 1048576)
set(GENERIC_F407ZGTX_MAXDATASIZE 131072)
set(GENERIC_F407ZGTX_MCU cortex-m4)
set(GENERIC_F407ZGTX_FPCONF "-")
add_library(GENERIC_F407ZGTX INTERFACE)
target_compile_options(GENERIC_F407ZGTX INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZGTX_MCU}
)
target_compile_definitions(GENERIC_F407ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407ZGTX"
	"BOARD_NAME=\"GENERIC_F407ZGTX\""
	"BOARD_ID=GENERIC_F407ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F407ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F407ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZGTX_MCU}
)
target_link_libraries(GENERIC_F407ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F407ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F407ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F407ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F407ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F407ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F407ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F407ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F407ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F407ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F407ZGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F407ZGTX_hid_MCU cortex-m4)
set(GENERIC_F407ZGTX_hid_FPCONF "-")
add_library(GENERIC_F407ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F407ZGTX_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F407ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F407ZGTX"
	"BOARD_NAME=\"GENERIC_F407ZGTX\""
	"BOARD_ID=GENERIC_F407ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F407ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F407ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F407ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F407ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F407ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F407ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410C8TX
# -----------------------------------------------------------------------------

set(GENERIC_F410C8TX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)T")
set(GENERIC_F410C8TX_MAXSIZE 65536)
set(GENERIC_F410C8TX_MAXDATASIZE 32768)
set(GENERIC_F410C8TX_MCU cortex-m4)
set(GENERIC_F410C8TX_FPCONF "-")
add_library(GENERIC_F410C8TX INTERFACE)
target_compile_options(GENERIC_F410C8TX INTERFACE
  "SHELL:-DSTM32F410Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8TX_MCU}
)
target_compile_definitions(GENERIC_F410C8TX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410C8TX"
	"BOARD_NAME=\"GENERIC_F410C8TX\""
	"BOARD_ID=GENERIC_F410C8TX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410C8TX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410C8TX_VARIANT_PATH}
)

target_link_options(GENERIC_F410C8TX INTERFACE
  "LINKER:--default-script=${GENERIC_F410C8TX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8TX_MCU}
)
target_link_libraries(GENERIC_F410C8TX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410C8TX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410C8TX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410C8TX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410C8TX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410C8TX_serial_none INTERFACE)
target_compile_options(GENERIC_F410C8TX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410C8TX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410C8TX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)T")
set(GENERIC_F410C8TX_hid_MAXSIZE 65536)
set(GENERIC_F410C8TX_hid_MAXDATASIZE 32768)
set(GENERIC_F410C8TX_hid_MCU cortex-m4)
set(GENERIC_F410C8TX_hid_FPCONF "-")
add_library(GENERIC_F410C8TX_hid INTERFACE)
target_compile_options(GENERIC_F410C8TX_hid INTERFACE
  "SHELL:-DSTM32F410Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8TX_hid_MCU}
)
target_compile_definitions(GENERIC_F410C8TX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410C8TX"
	"BOARD_NAME=\"GENERIC_F410C8TX\""
	"BOARD_ID=GENERIC_F410C8TX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410C8TX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410C8TX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410C8TX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410C8TX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8TX_hid_MCU}
)
target_link_libraries(GENERIC_F410C8TX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410C8UX
# -----------------------------------------------------------------------------

set(GENERIC_F410C8UX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)U")
set(GENERIC_F410C8UX_MAXSIZE 65536)
set(GENERIC_F410C8UX_MAXDATASIZE 32768)
set(GENERIC_F410C8UX_MCU cortex-m4)
set(GENERIC_F410C8UX_FPCONF "-")
add_library(GENERIC_F410C8UX INTERFACE)
target_compile_options(GENERIC_F410C8UX INTERFACE
  "SHELL:-DSTM32F410Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8UX_MCU}
)
target_compile_definitions(GENERIC_F410C8UX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410C8UX"
	"BOARD_NAME=\"GENERIC_F410C8UX\""
	"BOARD_ID=GENERIC_F410C8UX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410C8UX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410C8UX_VARIANT_PATH}
)

target_link_options(GENERIC_F410C8UX INTERFACE
  "LINKER:--default-script=${GENERIC_F410C8UX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8UX_MCU}
)
target_link_libraries(GENERIC_F410C8UX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410C8UX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410C8UX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410C8UX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410C8UX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410C8UX_serial_none INTERFACE)
target_compile_options(GENERIC_F410C8UX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410C8UX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410C8UX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)U")
set(GENERIC_F410C8UX_hid_MAXSIZE 65536)
set(GENERIC_F410C8UX_hid_MAXDATASIZE 32768)
set(GENERIC_F410C8UX_hid_MCU cortex-m4)
set(GENERIC_F410C8UX_hid_FPCONF "-")
add_library(GENERIC_F410C8UX_hid INTERFACE)
target_compile_options(GENERIC_F410C8UX_hid INTERFACE
  "SHELL:-DSTM32F410Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8UX_hid_MCU}
)
target_compile_definitions(GENERIC_F410C8UX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410C8UX"
	"BOARD_NAME=\"GENERIC_F410C8UX\""
	"BOARD_ID=GENERIC_F410C8UX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410C8UX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410C8UX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410C8UX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410C8UX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410C8UX_hid_MCU}
)
target_link_libraries(GENERIC_F410C8UX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410CBTX
# -----------------------------------------------------------------------------

set(GENERIC_F410CBTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)T")
set(GENERIC_F410CBTX_MAXSIZE 131072)
set(GENERIC_F410CBTX_MAXDATASIZE 32768)
set(GENERIC_F410CBTX_MCU cortex-m4)
set(GENERIC_F410CBTX_FPCONF "-")
add_library(GENERIC_F410CBTX INTERFACE)
target_compile_options(GENERIC_F410CBTX INTERFACE
  "SHELL:-DSTM32F410Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBTX_MCU}
)
target_compile_definitions(GENERIC_F410CBTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410CBTX"
	"BOARD_NAME=\"GENERIC_F410CBTX\""
	"BOARD_ID=GENERIC_F410CBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410CBTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410CBTX_VARIANT_PATH}
)

target_link_options(GENERIC_F410CBTX INTERFACE
  "LINKER:--default-script=${GENERIC_F410CBTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBTX_MCU}
)
target_link_libraries(GENERIC_F410CBTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410CBTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410CBTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410CBTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410CBTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410CBTX_serial_none INTERFACE)
target_compile_options(GENERIC_F410CBTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410CBTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410CBTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)T")
set(GENERIC_F410CBTX_hid_MAXSIZE 131072)
set(GENERIC_F410CBTX_hid_MAXDATASIZE 32768)
set(GENERIC_F410CBTX_hid_MCU cortex-m4)
set(GENERIC_F410CBTX_hid_FPCONF "-")
add_library(GENERIC_F410CBTX_hid INTERFACE)
target_compile_options(GENERIC_F410CBTX_hid INTERFACE
  "SHELL:-DSTM32F410Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBTX_hid_MCU}
)
target_compile_definitions(GENERIC_F410CBTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410CBTX"
	"BOARD_NAME=\"GENERIC_F410CBTX\""
	"BOARD_ID=GENERIC_F410CBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410CBTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410CBTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410CBTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410CBTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBTX_hid_MCU}
)
target_link_libraries(GENERIC_F410CBTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410CBUX
# -----------------------------------------------------------------------------

set(GENERIC_F410CBUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)U")
set(GENERIC_F410CBUX_MAXSIZE 131072)
set(GENERIC_F410CBUX_MAXDATASIZE 32768)
set(GENERIC_F410CBUX_MCU cortex-m4)
set(GENERIC_F410CBUX_FPCONF "-")
add_library(GENERIC_F410CBUX INTERFACE)
target_compile_options(GENERIC_F410CBUX INTERFACE
  "SHELL:-DSTM32F410Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBUX_MCU}
)
target_compile_definitions(GENERIC_F410CBUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410CBUX"
	"BOARD_NAME=\"GENERIC_F410CBUX\""
	"BOARD_ID=GENERIC_F410CBUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410CBUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410CBUX_VARIANT_PATH}
)

target_link_options(GENERIC_F410CBUX INTERFACE
  "LINKER:--default-script=${GENERIC_F410CBUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBUX_MCU}
)
target_link_libraries(GENERIC_F410CBUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410CBUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410CBUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410CBUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410CBUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410CBUX_serial_none INTERFACE)
target_compile_options(GENERIC_F410CBUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410CBUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410CBUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410C(8-B)U")
set(GENERIC_F410CBUX_hid_MAXSIZE 131072)
set(GENERIC_F410CBUX_hid_MAXDATASIZE 32768)
set(GENERIC_F410CBUX_hid_MCU cortex-m4)
set(GENERIC_F410CBUX_hid_FPCONF "-")
add_library(GENERIC_F410CBUX_hid INTERFACE)
target_compile_options(GENERIC_F410CBUX_hid INTERFACE
  "SHELL:-DSTM32F410Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBUX_hid_MCU}
)
target_compile_definitions(GENERIC_F410CBUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410CBUX"
	"BOARD_NAME=\"GENERIC_F410CBUX\""
	"BOARD_ID=GENERIC_F410CBUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410CBUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410CBUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410CBUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410CBUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410CBUX_hid_MCU}
)
target_link_libraries(GENERIC_F410CBUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410R8IX
# -----------------------------------------------------------------------------

set(GENERIC_F410R8IX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410R8IX_MAXSIZE 65536)
set(GENERIC_F410R8IX_MAXDATASIZE 32768)
set(GENERIC_F410R8IX_MCU cortex-m4)
set(GENERIC_F410R8IX_FPCONF "-")
add_library(GENERIC_F410R8IX INTERFACE)
target_compile_options(GENERIC_F410R8IX INTERFACE
  "SHELL:-DSTM32F410Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8IX_MCU}
)
target_compile_definitions(GENERIC_F410R8IX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410R8IX"
	"BOARD_NAME=\"GENERIC_F410R8IX\""
	"BOARD_ID=GENERIC_F410R8IX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410R8IX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410R8IX_VARIANT_PATH}
)

target_link_options(GENERIC_F410R8IX INTERFACE
  "LINKER:--default-script=${GENERIC_F410R8IX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8IX_MCU}
)
target_link_libraries(GENERIC_F410R8IX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410R8IX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410R8IX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410R8IX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410R8IX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410R8IX_serial_none INTERFACE)
target_compile_options(GENERIC_F410R8IX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410R8IX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410R8IX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410R8IX_hid_MAXSIZE 65536)
set(GENERIC_F410R8IX_hid_MAXDATASIZE 32768)
set(GENERIC_F410R8IX_hid_MCU cortex-m4)
set(GENERIC_F410R8IX_hid_FPCONF "-")
add_library(GENERIC_F410R8IX_hid INTERFACE)
target_compile_options(GENERIC_F410R8IX_hid INTERFACE
  "SHELL:-DSTM32F410Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8IX_hid_MCU}
)
target_compile_definitions(GENERIC_F410R8IX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410R8IX"
	"BOARD_NAME=\"GENERIC_F410R8IX\""
	"BOARD_ID=GENERIC_F410R8IX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410R8IX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410R8IX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410R8IX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410R8IX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8IX_hid_MCU}
)
target_link_libraries(GENERIC_F410R8IX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410R8TX
# -----------------------------------------------------------------------------

set(GENERIC_F410R8TX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410R8TX_MAXSIZE 65536)
set(GENERIC_F410R8TX_MAXDATASIZE 32768)
set(GENERIC_F410R8TX_MCU cortex-m4)
set(GENERIC_F410R8TX_FPCONF "-")
add_library(GENERIC_F410R8TX INTERFACE)
target_compile_options(GENERIC_F410R8TX INTERFACE
  "SHELL:-DSTM32F410Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8TX_MCU}
)
target_compile_definitions(GENERIC_F410R8TX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410R8TX"
	"BOARD_NAME=\"GENERIC_F410R8TX\""
	"BOARD_ID=GENERIC_F410R8TX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410R8TX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410R8TX_VARIANT_PATH}
)

target_link_options(GENERIC_F410R8TX INTERFACE
  "LINKER:--default-script=${GENERIC_F410R8TX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8TX_MCU}
)
target_link_libraries(GENERIC_F410R8TX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410R8TX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410R8TX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410R8TX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410R8TX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410R8TX_serial_none INTERFACE)
target_compile_options(GENERIC_F410R8TX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410R8TX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410R8TX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410R8TX_hid_MAXSIZE 65536)
set(GENERIC_F410R8TX_hid_MAXDATASIZE 32768)
set(GENERIC_F410R8TX_hid_MCU cortex-m4)
set(GENERIC_F410R8TX_hid_FPCONF "-")
add_library(GENERIC_F410R8TX_hid INTERFACE)
target_compile_options(GENERIC_F410R8TX_hid INTERFACE
  "SHELL:-DSTM32F410Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8TX_hid_MCU}
)
target_compile_definitions(GENERIC_F410R8TX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410R8TX"
	"BOARD_NAME=\"GENERIC_F410R8TX\""
	"BOARD_ID=GENERIC_F410R8TX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410R8TX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410R8TX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410R8TX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410R8TX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410R8TX_hid_MCU}
)
target_link_libraries(GENERIC_F410R8TX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410RBIX
# -----------------------------------------------------------------------------

set(GENERIC_F410RBIX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410RBIX_MAXSIZE 131072)
set(GENERIC_F410RBIX_MAXDATASIZE 32768)
set(GENERIC_F410RBIX_MCU cortex-m4)
set(GENERIC_F410RBIX_FPCONF "-")
add_library(GENERIC_F410RBIX INTERFACE)
target_compile_options(GENERIC_F410RBIX INTERFACE
  "SHELL:-DSTM32F410Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBIX_MCU}
)
target_compile_definitions(GENERIC_F410RBIX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410RBIX"
	"BOARD_NAME=\"GENERIC_F410RBIX\""
	"BOARD_ID=GENERIC_F410RBIX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410RBIX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410RBIX_VARIANT_PATH}
)

target_link_options(GENERIC_F410RBIX INTERFACE
  "LINKER:--default-script=${GENERIC_F410RBIX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBIX_MCU}
)
target_link_libraries(GENERIC_F410RBIX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410RBIX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410RBIX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410RBIX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410RBIX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410RBIX_serial_none INTERFACE)
target_compile_options(GENERIC_F410RBIX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410RBIX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410RBIX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410RBIX_hid_MAXSIZE 131072)
set(GENERIC_F410RBIX_hid_MAXDATASIZE 32768)
set(GENERIC_F410RBIX_hid_MCU cortex-m4)
set(GENERIC_F410RBIX_hid_FPCONF "-")
add_library(GENERIC_F410RBIX_hid INTERFACE)
target_compile_options(GENERIC_F410RBIX_hid INTERFACE
  "SHELL:-DSTM32F410Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBIX_hid_MCU}
)
target_compile_definitions(GENERIC_F410RBIX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410RBIX"
	"BOARD_NAME=\"GENERIC_F410RBIX\""
	"BOARD_ID=GENERIC_F410RBIX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410RBIX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410RBIX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410RBIX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410RBIX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBIX_hid_MCU}
)
target_link_libraries(GENERIC_F410RBIX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410RBTX
# -----------------------------------------------------------------------------

set(GENERIC_F410RBTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410RBTX_MAXSIZE 131072)
set(GENERIC_F410RBTX_MAXDATASIZE 32768)
set(GENERIC_F410RBTX_MCU cortex-m4)
set(GENERIC_F410RBTX_FPCONF "-")
add_library(GENERIC_F410RBTX INTERFACE)
target_compile_options(GENERIC_F410RBTX INTERFACE
  "SHELL:-DSTM32F410Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBTX_MCU}
)
target_compile_definitions(GENERIC_F410RBTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410RBTX"
	"BOARD_NAME=\"GENERIC_F410RBTX\""
	"BOARD_ID=GENERIC_F410RBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410RBTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410RBTX_VARIANT_PATH}
)

target_link_options(GENERIC_F410RBTX INTERFACE
  "LINKER:--default-script=${GENERIC_F410RBTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBTX_MCU}
)
target_link_libraries(GENERIC_F410RBTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410RBTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410RBTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410RBTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410RBTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410RBTX_serial_none INTERFACE)
target_compile_options(GENERIC_F410RBTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410RBTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410RBTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410R(8-B)(I-T)")
set(GENERIC_F410RBTX_hid_MAXSIZE 131072)
set(GENERIC_F410RBTX_hid_MAXDATASIZE 32768)
set(GENERIC_F410RBTX_hid_MCU cortex-m4)
set(GENERIC_F410RBTX_hid_FPCONF "-")
add_library(GENERIC_F410RBTX_hid INTERFACE)
target_compile_options(GENERIC_F410RBTX_hid INTERFACE
  "SHELL:-DSTM32F410Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBTX_hid_MCU}
)
target_compile_definitions(GENERIC_F410RBTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410RBTX"
	"BOARD_NAME=\"GENERIC_F410RBTX\""
	"BOARD_ID=GENERIC_F410RBTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410RBTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410RBTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410RBTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410RBTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410RBTX_hid_MCU}
)
target_link_libraries(GENERIC_F410RBTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410T8YX
# -----------------------------------------------------------------------------

set(GENERIC_F410T8YX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410T(8-B)Y")
set(GENERIC_F410T8YX_MAXSIZE 65536)
set(GENERIC_F410T8YX_MAXDATASIZE 32768)
set(GENERIC_F410T8YX_MCU cortex-m4)
set(GENERIC_F410T8YX_FPCONF "-")
add_library(GENERIC_F410T8YX INTERFACE)
target_compile_options(GENERIC_F410T8YX INTERFACE
  "SHELL:-DSTM32F410Tx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410T8YX_MCU}
)
target_compile_definitions(GENERIC_F410T8YX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410T8YX"
	"BOARD_NAME=\"GENERIC_F410T8YX\""
	"BOARD_ID=GENERIC_F410T8YX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410T8YX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410T8YX_VARIANT_PATH}
)

target_link_options(GENERIC_F410T8YX INTERFACE
  "LINKER:--default-script=${GENERIC_F410T8YX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410T8YX_MCU}
)
target_link_libraries(GENERIC_F410T8YX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410T8YX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410T8YX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410T8YX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410T8YX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410T8YX_serial_none INTERFACE)
target_compile_options(GENERIC_F410T8YX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410T8YX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410T8YX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410T(8-B)Y")
set(GENERIC_F410T8YX_hid_MAXSIZE 65536)
set(GENERIC_F410T8YX_hid_MAXDATASIZE 32768)
set(GENERIC_F410T8YX_hid_MCU cortex-m4)
set(GENERIC_F410T8YX_hid_FPCONF "-")
add_library(GENERIC_F410T8YX_hid INTERFACE)
target_compile_options(GENERIC_F410T8YX_hid INTERFACE
  "SHELL:-DSTM32F410Tx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410T8YX_hid_MCU}
)
target_compile_definitions(GENERIC_F410T8YX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410T8YX"
	"BOARD_NAME=\"GENERIC_F410T8YX\""
	"BOARD_ID=GENERIC_F410T8YX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410T8YX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410T8YX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410T8YX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410T8YX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=65536"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410T8YX_hid_MCU}
)
target_link_libraries(GENERIC_F410T8YX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F410TBYX
# -----------------------------------------------------------------------------

set(GENERIC_F410TBYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410T(8-B)Y")
set(GENERIC_F410TBYX_MAXSIZE 131072)
set(GENERIC_F410TBYX_MAXDATASIZE 32768)
set(GENERIC_F410TBYX_MCU cortex-m4)
set(GENERIC_F410TBYX_FPCONF "-")
add_library(GENERIC_F410TBYX INTERFACE)
target_compile_options(GENERIC_F410TBYX INTERFACE
  "SHELL:-DSTM32F410Tx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410TBYX_MCU}
)
target_compile_definitions(GENERIC_F410TBYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410TBYX"
	"BOARD_NAME=\"GENERIC_F410TBYX\""
	"BOARD_ID=GENERIC_F410TBYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410TBYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410TBYX_VARIANT_PATH}
)

target_link_options(GENERIC_F410TBYX INTERFACE
  "LINKER:--default-script=${GENERIC_F410TBYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410TBYX_MCU}
)
target_link_libraries(GENERIC_F410TBYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F410TBYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F410TBYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F410TBYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F410TBYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F410TBYX_serial_none INTERFACE)
target_compile_options(GENERIC_F410TBYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F410TBYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F410TBYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F410T(8-B)Y")
set(GENERIC_F410TBYX_hid_MAXSIZE 131072)
set(GENERIC_F410TBYX_hid_MAXDATASIZE 32768)
set(GENERIC_F410TBYX_hid_MCU cortex-m4)
set(GENERIC_F410TBYX_hid_FPCONF "-")
add_library(GENERIC_F410TBYX_hid INTERFACE)
target_compile_options(GENERIC_F410TBYX_hid INTERFACE
  "SHELL:-DSTM32F410Tx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410TBYX_hid_MCU}
)
target_compile_definitions(GENERIC_F410TBYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F410TBYX"
	"BOARD_NAME=\"GENERIC_F410TBYX\""
	"BOARD_ID=GENERIC_F410TBYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F410TBYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F410TBYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F410TBYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F410TBYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=131072"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=32768"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F410TBYX_hid_MCU}
)
target_link_libraries(GENERIC_F410TBYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411CCUX
# -----------------------------------------------------------------------------

set(GENERIC_F411CCUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CCUX_MAXSIZE 262144)
set(GENERIC_F411CCUX_MAXDATASIZE 131072)
set(GENERIC_F411CCUX_MCU cortex-m4)
set(GENERIC_F411CCUX_FPCONF "-")
add_library(GENERIC_F411CCUX INTERFACE)
target_compile_options(GENERIC_F411CCUX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCUX_MCU}
)
target_compile_definitions(GENERIC_F411CCUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CCUX"
	"BOARD_NAME=\"GENERIC_F411CCUX\""
	"BOARD_ID=GENERIC_F411CCUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CCUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CCUX_VARIANT_PATH}
)

target_link_options(GENERIC_F411CCUX INTERFACE
  "LINKER:--default-script=${GENERIC_F411CCUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCUX_MCU}
)
target_link_libraries(GENERIC_F411CCUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411CCUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411CCUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411CCUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411CCUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411CCUX_serial_none INTERFACE)
target_compile_options(GENERIC_F411CCUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411CCUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411CCUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CCUX_hid_MAXSIZE 262144)
set(GENERIC_F411CCUX_hid_MAXDATASIZE 131072)
set(GENERIC_F411CCUX_hid_MCU cortex-m4)
set(GENERIC_F411CCUX_hid_FPCONF "-")
add_library(GENERIC_F411CCUX_hid INTERFACE)
target_compile_options(GENERIC_F411CCUX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCUX_hid_MCU}
)
target_compile_definitions(GENERIC_F411CCUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CCUX"
	"BOARD_NAME=\"GENERIC_F411CCUX\""
	"BOARD_ID=GENERIC_F411CCUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CCUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CCUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411CCUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411CCUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCUX_hid_MCU}
)
target_link_libraries(GENERIC_F411CCUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411CCYX
# -----------------------------------------------------------------------------

set(GENERIC_F411CCYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CCYX_MAXSIZE 262144)
set(GENERIC_F411CCYX_MAXDATASIZE 131072)
set(GENERIC_F411CCYX_MCU cortex-m4)
set(GENERIC_F411CCYX_FPCONF "-")
add_library(GENERIC_F411CCYX INTERFACE)
target_compile_options(GENERIC_F411CCYX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCYX_MCU}
)
target_compile_definitions(GENERIC_F411CCYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CCYX"
	"BOARD_NAME=\"GENERIC_F411CCYX\""
	"BOARD_ID=GENERIC_F411CCYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CCYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CCYX_VARIANT_PATH}
)

target_link_options(GENERIC_F411CCYX INTERFACE
  "LINKER:--default-script=${GENERIC_F411CCYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCYX_MCU}
)
target_link_libraries(GENERIC_F411CCYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411CCYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411CCYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411CCYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411CCYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411CCYX_serial_none INTERFACE)
target_compile_options(GENERIC_F411CCYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411CCYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411CCYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CCYX_hid_MAXSIZE 262144)
set(GENERIC_F411CCYX_hid_MAXDATASIZE 131072)
set(GENERIC_F411CCYX_hid_MCU cortex-m4)
set(GENERIC_F411CCYX_hid_FPCONF "-")
add_library(GENERIC_F411CCYX_hid INTERFACE)
target_compile_options(GENERIC_F411CCYX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCYX_hid_MCU}
)
target_compile_definitions(GENERIC_F411CCYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CCYX"
	"BOARD_NAME=\"GENERIC_F411CCYX\""
	"BOARD_ID=GENERIC_F411CCYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CCYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CCYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411CCYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411CCYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CCYX_hid_MCU}
)
target_link_libraries(GENERIC_F411CCYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411CEUX
# -----------------------------------------------------------------------------

set(GENERIC_F411CEUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CEUX_MAXSIZE 524288)
set(GENERIC_F411CEUX_MAXDATASIZE 131072)
set(GENERIC_F411CEUX_MCU cortex-m4)
set(GENERIC_F411CEUX_FPCONF "-")
add_library(GENERIC_F411CEUX INTERFACE)
target_compile_options(GENERIC_F411CEUX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEUX_MCU}
)
target_compile_definitions(GENERIC_F411CEUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CEUX"
	"BOARD_NAME=\"GENERIC_F411CEUX\""
	"BOARD_ID=GENERIC_F411CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CEUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CEUX_VARIANT_PATH}
)

target_link_options(GENERIC_F411CEUX INTERFACE
  "LINKER:--default-script=${GENERIC_F411CEUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEUX_MCU}
)
target_link_libraries(GENERIC_F411CEUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411CEUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411CEUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411CEUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411CEUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411CEUX_serial_none INTERFACE)
target_compile_options(GENERIC_F411CEUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411CEUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411CEUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CEUX_hid_MAXSIZE 524288)
set(GENERIC_F411CEUX_hid_MAXDATASIZE 131072)
set(GENERIC_F411CEUX_hid_MCU cortex-m4)
set(GENERIC_F411CEUX_hid_FPCONF "-")
add_library(GENERIC_F411CEUX_hid INTERFACE)
target_compile_options(GENERIC_F411CEUX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEUX_hid_MCU}
)
target_compile_definitions(GENERIC_F411CEUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CEUX"
	"BOARD_NAME=\"GENERIC_F411CEUX\""
	"BOARD_ID=GENERIC_F411CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CEUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CEUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411CEUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411CEUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEUX_hid_MCU}
)
target_link_libraries(GENERIC_F411CEUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411CEYX
# -----------------------------------------------------------------------------

set(GENERIC_F411CEYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CEYX_MAXSIZE 524288)
set(GENERIC_F411CEYX_MAXDATASIZE 131072)
set(GENERIC_F411CEYX_MCU cortex-m4)
set(GENERIC_F411CEYX_FPCONF "-")
add_library(GENERIC_F411CEYX INTERFACE)
target_compile_options(GENERIC_F411CEYX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEYX_MCU}
)
target_compile_definitions(GENERIC_F411CEYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CEYX"
	"BOARD_NAME=\"GENERIC_F411CEYX\""
	"BOARD_ID=GENERIC_F411CEYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CEYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CEYX_VARIANT_PATH}
)

target_link_options(GENERIC_F411CEYX INTERFACE
  "LINKER:--default-script=${GENERIC_F411CEYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEYX_MCU}
)
target_link_libraries(GENERIC_F411CEYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411CEYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411CEYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411CEYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411CEYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411CEYX_serial_none INTERFACE)
target_compile_options(GENERIC_F411CEYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411CEYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411CEYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411C(C-E)(U-Y)")
set(GENERIC_F411CEYX_hid_MAXSIZE 524288)
set(GENERIC_F411CEYX_hid_MAXDATASIZE 131072)
set(GENERIC_F411CEYX_hid_MCU cortex-m4)
set(GENERIC_F411CEYX_hid_FPCONF "-")
add_library(GENERIC_F411CEYX_hid INTERFACE)
target_compile_options(GENERIC_F411CEYX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEYX_hid_MCU}
)
target_compile_definitions(GENERIC_F411CEYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411CEYX"
	"BOARD_NAME=\"GENERIC_F411CEYX\""
	"BOARD_ID=GENERIC_F411CEYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411CEYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411CEYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411CEYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411CEYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411CEYX_hid_MCU}
)
target_link_libraries(GENERIC_F411CEYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411RCTX
# -----------------------------------------------------------------------------

set(GENERIC_F411RCTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411R(C-E)T")
set(GENERIC_F411RCTX_MAXSIZE 262144)
set(GENERIC_F411RCTX_MAXDATASIZE 131072)
set(GENERIC_F411RCTX_MCU cortex-m4)
set(GENERIC_F411RCTX_FPCONF "-")
add_library(GENERIC_F411RCTX INTERFACE)
target_compile_options(GENERIC_F411RCTX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RCTX_MCU}
)
target_compile_definitions(GENERIC_F411RCTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411RCTX"
	"BOARD_NAME=\"GENERIC_F411RCTX\""
	"BOARD_ID=GENERIC_F411RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411RCTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411RCTX_VARIANT_PATH}
)

target_link_options(GENERIC_F411RCTX INTERFACE
  "LINKER:--default-script=${GENERIC_F411RCTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RCTX_MCU}
)
target_link_libraries(GENERIC_F411RCTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411RCTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411RCTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411RCTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411RCTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411RCTX_serial_none INTERFACE)
target_compile_options(GENERIC_F411RCTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411RCTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411RCTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411R(C-E)T")
set(GENERIC_F411RCTX_hid_MAXSIZE 262144)
set(GENERIC_F411RCTX_hid_MAXDATASIZE 131072)
set(GENERIC_F411RCTX_hid_MCU cortex-m4)
set(GENERIC_F411RCTX_hid_FPCONF "-")
add_library(GENERIC_F411RCTX_hid INTERFACE)
target_compile_options(GENERIC_F411RCTX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RCTX_hid_MCU}
)
target_compile_definitions(GENERIC_F411RCTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411RCTX"
	"BOARD_NAME=\"GENERIC_F411RCTX\""
	"BOARD_ID=GENERIC_F411RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411RCTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411RCTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411RCTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411RCTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RCTX_hid_MCU}
)
target_link_libraries(GENERIC_F411RCTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F411RETX
# -----------------------------------------------------------------------------

set(GENERIC_F411RETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411R(C-E)T")
set(GENERIC_F411RETX_MAXSIZE 524288)
set(GENERIC_F411RETX_MAXDATASIZE 131072)
set(GENERIC_F411RETX_MCU cortex-m4)
set(GENERIC_F411RETX_FPCONF "-")
add_library(GENERIC_F411RETX INTERFACE)
target_compile_options(GENERIC_F411RETX INTERFACE
  "SHELL:-DSTM32F411xE   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RETX_MCU}
)
target_compile_definitions(GENERIC_F411RETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411RETX"
	"BOARD_NAME=\"GENERIC_F411RETX\""
	"BOARD_ID=GENERIC_F411RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411RETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411RETX_VARIANT_PATH}
)

target_link_options(GENERIC_F411RETX INTERFACE
  "LINKER:--default-script=${GENERIC_F411RETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RETX_MCU}
)
target_link_libraries(GENERIC_F411RETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F411RETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F411RETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F411RETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F411RETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F411RETX_serial_none INTERFACE)
target_compile_options(GENERIC_F411RETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F411RETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F411RETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411R(C-E)T")
set(GENERIC_F411RETX_hid_MAXSIZE 524288)
set(GENERIC_F411RETX_hid_MAXDATASIZE 131072)
set(GENERIC_F411RETX_hid_MCU cortex-m4)
set(GENERIC_F411RETX_hid_FPCONF "-")
add_library(GENERIC_F411RETX_hid INTERFACE)
target_compile_options(GENERIC_F411RETX_hid INTERFACE
  "SHELL:-DSTM32F411xE  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RETX_hid_MCU}
)
target_compile_definitions(GENERIC_F411RETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F411RETX"
	"BOARD_NAME=\"GENERIC_F411RETX\""
	"BOARD_ID=GENERIC_F411RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F411RETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F411RETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F411RETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F411RETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F411RETX_hid_MCU}
)
target_link_libraries(GENERIC_F411RETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412CEUX
# -----------------------------------------------------------------------------

set(GENERIC_F412CEUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412C(E-G)U")
set(GENERIC_F412CEUX_MAXSIZE 524288)
set(GENERIC_F412CEUX_MAXDATASIZE 262144)
set(GENERIC_F412CEUX_MCU cortex-m4)
set(GENERIC_F412CEUX_FPCONF "-")
add_library(GENERIC_F412CEUX INTERFACE)
target_compile_options(GENERIC_F412CEUX INTERFACE
  "SHELL:-DSTM32F412Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CEUX_MCU}
)
target_compile_definitions(GENERIC_F412CEUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412CEUX"
	"BOARD_NAME=\"GENERIC_F412CEUX\""
	"BOARD_ID=GENERIC_F412CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412CEUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412CEUX_VARIANT_PATH}
)

target_link_options(GENERIC_F412CEUX INTERFACE
  "LINKER:--default-script=${GENERIC_F412CEUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CEUX_MCU}
)
target_link_libraries(GENERIC_F412CEUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412CEUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412CEUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412CEUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412CEUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412CEUX_serial_none INTERFACE)
target_compile_options(GENERIC_F412CEUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412CEUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412CEUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412C(E-G)U")
set(GENERIC_F412CEUX_hid_MAXSIZE 524288)
set(GENERIC_F412CEUX_hid_MAXDATASIZE 262144)
set(GENERIC_F412CEUX_hid_MCU cortex-m4)
set(GENERIC_F412CEUX_hid_FPCONF "-")
add_library(GENERIC_F412CEUX_hid INTERFACE)
target_compile_options(GENERIC_F412CEUX_hid INTERFACE
  "SHELL:-DSTM32F412Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CEUX_hid_MCU}
)
target_compile_definitions(GENERIC_F412CEUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412CEUX"
	"BOARD_NAME=\"GENERIC_F412CEUX\""
	"BOARD_ID=GENERIC_F412CEUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412CEUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412CEUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412CEUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412CEUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CEUX_hid_MCU}
)
target_link_libraries(GENERIC_F412CEUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412CGUX
# -----------------------------------------------------------------------------

set(GENERIC_F412CGUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412C(E-G)U")
set(GENERIC_F412CGUX_MAXSIZE 1048576)
set(GENERIC_F412CGUX_MAXDATASIZE 262144)
set(GENERIC_F412CGUX_MCU cortex-m4)
set(GENERIC_F412CGUX_FPCONF "-")
add_library(GENERIC_F412CGUX INTERFACE)
target_compile_options(GENERIC_F412CGUX INTERFACE
  "SHELL:-DSTM32F412Cx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CGUX_MCU}
)
target_compile_definitions(GENERIC_F412CGUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412CGUX"
	"BOARD_NAME=\"GENERIC_F412CGUX\""
	"BOARD_ID=GENERIC_F412CGUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412CGUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412CGUX_VARIANT_PATH}
)

target_link_options(GENERIC_F412CGUX INTERFACE
  "LINKER:--default-script=${GENERIC_F412CGUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CGUX_MCU}
)
target_link_libraries(GENERIC_F412CGUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412CGUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412CGUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412CGUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412CGUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412CGUX_serial_none INTERFACE)
target_compile_options(GENERIC_F412CGUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412CGUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412CGUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412C(E-G)U")
set(GENERIC_F412CGUX_hid_MAXSIZE 1048576)
set(GENERIC_F412CGUX_hid_MAXDATASIZE 262144)
set(GENERIC_F412CGUX_hid_MCU cortex-m4)
set(GENERIC_F412CGUX_hid_FPCONF "-")
add_library(GENERIC_F412CGUX_hid INTERFACE)
target_compile_options(GENERIC_F412CGUX_hid INTERFACE
  "SHELL:-DSTM32F412Cx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CGUX_hid_MCU}
)
target_compile_definitions(GENERIC_F412CGUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412CGUX"
	"BOARD_NAME=\"GENERIC_F412CGUX\""
	"BOARD_ID=GENERIC_F412CGUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412CGUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412CGUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412CGUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412CGUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412CGUX_hid_MCU}
)
target_link_libraries(GENERIC_F412CGUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412RETX
# -----------------------------------------------------------------------------

set(GENERIC_F412RETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RETX_MAXSIZE 524288)
set(GENERIC_F412RETX_MAXDATASIZE 262144)
set(GENERIC_F412RETX_MCU cortex-m4)
set(GENERIC_F412RETX_FPCONF "-")
add_library(GENERIC_F412RETX INTERFACE)
target_compile_options(GENERIC_F412RETX INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RETX_MCU}
)
target_compile_definitions(GENERIC_F412RETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RETX"
	"BOARD_NAME=\"GENERIC_F412RETX\""
	"BOARD_ID=GENERIC_F412RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RETX_VARIANT_PATH}
)

target_link_options(GENERIC_F412RETX INTERFACE
  "LINKER:--default-script=${GENERIC_F412RETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RETX_MCU}
)
target_link_libraries(GENERIC_F412RETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412RETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412RETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412RETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412RETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412RETX_serial_none INTERFACE)
target_compile_options(GENERIC_F412RETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412RETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412RETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RETX_hid_MAXSIZE 524288)
set(GENERIC_F412RETX_hid_MAXDATASIZE 262144)
set(GENERIC_F412RETX_hid_MCU cortex-m4)
set(GENERIC_F412RETX_hid_FPCONF "-")
add_library(GENERIC_F412RETX_hid INTERFACE)
target_compile_options(GENERIC_F412RETX_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RETX_hid_MCU}
)
target_compile_definitions(GENERIC_F412RETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RETX"
	"BOARD_NAME=\"GENERIC_F412RETX\""
	"BOARD_ID=GENERIC_F412RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412RETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412RETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RETX_hid_MCU}
)
target_link_libraries(GENERIC_F412RETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412REYX
# -----------------------------------------------------------------------------

set(GENERIC_F412REYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412REYX_MAXSIZE 524288)
set(GENERIC_F412REYX_MAXDATASIZE 262144)
set(GENERIC_F412REYX_MCU cortex-m4)
set(GENERIC_F412REYX_FPCONF "-")
add_library(GENERIC_F412REYX INTERFACE)
target_compile_options(GENERIC_F412REYX INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYX_MCU}
)
target_compile_definitions(GENERIC_F412REYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412REYX"
	"BOARD_NAME=\"GENERIC_F412REYX\""
	"BOARD_ID=GENERIC_F412REYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412REYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412REYX_VARIANT_PATH}
)

target_link_options(GENERIC_F412REYX INTERFACE
  "LINKER:--default-script=${GENERIC_F412REYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYX_MCU}
)
target_link_libraries(GENERIC_F412REYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412REYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412REYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412REYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412REYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412REYX_serial_none INTERFACE)
target_compile_options(GENERIC_F412REYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412REYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412REYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412REYX_hid_MAXSIZE 524288)
set(GENERIC_F412REYX_hid_MAXDATASIZE 262144)
set(GENERIC_F412REYX_hid_MCU cortex-m4)
set(GENERIC_F412REYX_hid_FPCONF "-")
add_library(GENERIC_F412REYX_hid INTERFACE)
target_compile_options(GENERIC_F412REYX_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYX_hid_MCU}
)
target_compile_definitions(GENERIC_F412REYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412REYX"
	"BOARD_NAME=\"GENERIC_F412REYX\""
	"BOARD_ID=GENERIC_F412REYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412REYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412REYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412REYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412REYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYX_hid_MCU}
)
target_link_libraries(GENERIC_F412REYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412REYXP
# -----------------------------------------------------------------------------

set(GENERIC_F412REYXP_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412REYXP_MAXSIZE 524288)
set(GENERIC_F412REYXP_MAXDATASIZE 262144)
set(GENERIC_F412REYXP_MCU cortex-m4)
set(GENERIC_F412REYXP_FPCONF "-")
add_library(GENERIC_F412REYXP INTERFACE)
target_compile_options(GENERIC_F412REYXP INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYXP_MCU}
)
target_compile_definitions(GENERIC_F412REYXP INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412REYXP"
	"BOARD_NAME=\"GENERIC_F412REYXP\""
	"BOARD_ID=GENERIC_F412REYXP"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412REYXP INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412REYXP_VARIANT_PATH}
)

target_link_options(GENERIC_F412REYXP INTERFACE
  "LINKER:--default-script=${GENERIC_F412REYXP_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYXP_MCU}
)
target_link_libraries(GENERIC_F412REYXP INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412REYXP_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412REYXP_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412REYXP_serial_generic INTERFACE)
target_compile_options(GENERIC_F412REYXP_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412REYXP_serial_none INTERFACE)
target_compile_options(GENERIC_F412REYXP_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412REYXP_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412REYXP_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412REYXP_hid_MAXSIZE 524288)
set(GENERIC_F412REYXP_hid_MAXDATASIZE 262144)
set(GENERIC_F412REYXP_hid_MCU cortex-m4)
set(GENERIC_F412REYXP_hid_FPCONF "-")
add_library(GENERIC_F412REYXP_hid INTERFACE)
target_compile_options(GENERIC_F412REYXP_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYXP_hid_MCU}
)
target_compile_definitions(GENERIC_F412REYXP_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412REYXP"
	"BOARD_NAME=\"GENERIC_F412REYXP\""
	"BOARD_ID=GENERIC_F412REYXP"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412REYXP_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412REYXP_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412REYXP_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412REYXP_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412REYXP_hid_MCU}
)
target_link_libraries(GENERIC_F412REYXP_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412RGTX
# -----------------------------------------------------------------------------

set(GENERIC_F412RGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGTX_MAXSIZE 1048576)
set(GENERIC_F412RGTX_MAXDATASIZE 262144)
set(GENERIC_F412RGTX_MCU cortex-m4)
set(GENERIC_F412RGTX_FPCONF "-")
add_library(GENERIC_F412RGTX INTERFACE)
target_compile_options(GENERIC_F412RGTX INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGTX_MCU}
)
target_compile_definitions(GENERIC_F412RGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGTX"
	"BOARD_NAME=\"GENERIC_F412RGTX\""
	"BOARD_ID=GENERIC_F412RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGTX_MCU}
)
target_link_libraries(GENERIC_F412RGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412RGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412RGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412RGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412RGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412RGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F412RGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412RGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412RGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGTX_hid_MAXSIZE 1048576)
set(GENERIC_F412RGTX_hid_MAXDATASIZE 262144)
set(GENERIC_F412RGTX_hid_MCU cortex-m4)
set(GENERIC_F412RGTX_hid_FPCONF "-")
add_library(GENERIC_F412RGTX_hid INTERFACE)
target_compile_options(GENERIC_F412RGTX_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F412RGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGTX"
	"BOARD_NAME=\"GENERIC_F412RGTX\""
	"BOARD_ID=GENERIC_F412RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGTX_hid_MCU}
)
target_link_libraries(GENERIC_F412RGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412RGYX
# -----------------------------------------------------------------------------

set(GENERIC_F412RGYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGYX_MAXSIZE 1048576)
set(GENERIC_F412RGYX_MAXDATASIZE 262144)
set(GENERIC_F412RGYX_MCU cortex-m4)
set(GENERIC_F412RGYX_FPCONF "-")
add_library(GENERIC_F412RGYX INTERFACE)
target_compile_options(GENERIC_F412RGYX INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYX_MCU}
)
target_compile_definitions(GENERIC_F412RGYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGYX"
	"BOARD_NAME=\"GENERIC_F412RGYX\""
	"BOARD_ID=GENERIC_F412RGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGYX_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGYX INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYX_MCU}
)
target_link_libraries(GENERIC_F412RGYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412RGYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412RGYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412RGYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F412RGYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412RGYX_serial_none INTERFACE)
target_compile_options(GENERIC_F412RGYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412RGYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412RGYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGYX_hid_MAXSIZE 1048576)
set(GENERIC_F412RGYX_hid_MAXDATASIZE 262144)
set(GENERIC_F412RGYX_hid_MCU cortex-m4)
set(GENERIC_F412RGYX_hid_FPCONF "-")
add_library(GENERIC_F412RGYX_hid INTERFACE)
target_compile_options(GENERIC_F412RGYX_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYX_hid_MCU}
)
target_compile_definitions(GENERIC_F412RGYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGYX"
	"BOARD_NAME=\"GENERIC_F412RGYX\""
	"BOARD_ID=GENERIC_F412RGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYX_hid_MCU}
)
target_link_libraries(GENERIC_F412RGYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F412RGYXP
# -----------------------------------------------------------------------------

set(GENERIC_F412RGYXP_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGYXP_MAXSIZE 1048576)
set(GENERIC_F412RGYXP_MAXDATASIZE 262144)
set(GENERIC_F412RGYXP_MCU cortex-m4)
set(GENERIC_F412RGYXP_FPCONF "-")
add_library(GENERIC_F412RGYXP INTERFACE)
target_compile_options(GENERIC_F412RGYXP INTERFACE
  "SHELL:-DSTM32F412Rx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYXP_MCU}
)
target_compile_definitions(GENERIC_F412RGYXP INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGYXP"
	"BOARD_NAME=\"GENERIC_F412RGYXP\""
	"BOARD_ID=GENERIC_F412RGYXP"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGYXP INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGYXP_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGYXP INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGYXP_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYXP_MCU}
)
target_link_libraries(GENERIC_F412RGYXP INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F412RGYXP_serial_disabled INTERFACE)
target_compile_options(GENERIC_F412RGYXP_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F412RGYXP_serial_generic INTERFACE)
target_compile_options(GENERIC_F412RGYXP_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F412RGYXP_serial_none INTERFACE)
target_compile_options(GENERIC_F412RGYXP_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F412RGYXP_hid
# -----------------------------------------------------------------------------

set(GENERIC_F412RGYXP_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F412R(E-G)(T-Y)x(P)")
set(GENERIC_F412RGYXP_hid_MAXSIZE 1048576)
set(GENERIC_F412RGYXP_hid_MAXDATASIZE 262144)
set(GENERIC_F412RGYXP_hid_MCU cortex-m4)
set(GENERIC_F412RGYXP_hid_FPCONF "-")
add_library(GENERIC_F412RGYXP_hid INTERFACE)
target_compile_options(GENERIC_F412RGYXP_hid INTERFACE
  "SHELL:-DSTM32F412Rx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYXP_hid_MCU}
)
target_compile_definitions(GENERIC_F412RGYXP_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F412RGYXP"
	"BOARD_NAME=\"GENERIC_F412RGYXP\""
	"BOARD_ID=GENERIC_F412RGYXP"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F412RGYXP_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F412RGYXP_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F412RGYXP_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F412RGYXP_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=262144"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F412RGYXP_hid_MCU}
)
target_link_libraries(GENERIC_F412RGYXP_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413CGUX
# -----------------------------------------------------------------------------

set(GENERIC_F413CGUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F413CGUX_MAXSIZE 1048576)
set(GENERIC_F413CGUX_MAXDATASIZE 327680)
set(GENERIC_F413CGUX_MCU cortex-m4)
set(GENERIC_F413CGUX_FPCONF "-")
add_library(GENERIC_F413CGUX INTERFACE)
target_compile_options(GENERIC_F413CGUX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CGUX_MCU}
)
target_compile_definitions(GENERIC_F413CGUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413CGUX"
	"BOARD_NAME=\"GENERIC_F413CGUX\""
	"BOARD_ID=GENERIC_F413CGUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413CGUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413CGUX_VARIANT_PATH}
)

target_link_options(GENERIC_F413CGUX INTERFACE
  "LINKER:--default-script=${GENERIC_F413CGUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CGUX_MCU}
)
target_link_libraries(GENERIC_F413CGUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413CGUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413CGUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413CGUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413CGUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413CGUX_serial_none INTERFACE)
target_compile_options(GENERIC_F413CGUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413CGUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413CGUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F413CGUX_hid_MAXSIZE 1048576)
set(GENERIC_F413CGUX_hid_MAXDATASIZE 327680)
set(GENERIC_F413CGUX_hid_MCU cortex-m4)
set(GENERIC_F413CGUX_hid_FPCONF "-")
add_library(GENERIC_F413CGUX_hid INTERFACE)
target_compile_options(GENERIC_F413CGUX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CGUX_hid_MCU}
)
target_compile_definitions(GENERIC_F413CGUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413CGUX"
	"BOARD_NAME=\"GENERIC_F413CGUX\""
	"BOARD_ID=GENERIC_F413CGUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413CGUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413CGUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413CGUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413CGUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CGUX_hid_MCU}
)
target_link_libraries(GENERIC_F413CGUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413CHUX
# -----------------------------------------------------------------------------

set(GENERIC_F413CHUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F413CHUX_MAXSIZE 1572864)
set(GENERIC_F413CHUX_MAXDATASIZE 327680)
set(GENERIC_F413CHUX_MCU cortex-m4)
set(GENERIC_F413CHUX_FPCONF "-")
add_library(GENERIC_F413CHUX INTERFACE)
target_compile_options(GENERIC_F413CHUX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CHUX_MCU}
)
target_compile_definitions(GENERIC_F413CHUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413CHUX"
	"BOARD_NAME=\"GENERIC_F413CHUX\""
	"BOARD_ID=GENERIC_F413CHUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413CHUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413CHUX_VARIANT_PATH}
)

target_link_options(GENERIC_F413CHUX INTERFACE
  "LINKER:--default-script=${GENERIC_F413CHUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CHUX_MCU}
)
target_link_libraries(GENERIC_F413CHUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413CHUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413CHUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413CHUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413CHUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413CHUX_serial_none INTERFACE)
target_compile_options(GENERIC_F413CHUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413CHUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413CHUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F413CHUX_hid_MAXSIZE 1572864)
set(GENERIC_F413CHUX_hid_MAXDATASIZE 327680)
set(GENERIC_F413CHUX_hid_MCU cortex-m4)
set(GENERIC_F413CHUX_hid_FPCONF "-")
add_library(GENERIC_F413CHUX_hid INTERFACE)
target_compile_options(GENERIC_F413CHUX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CHUX_hid_MCU}
)
target_compile_definitions(GENERIC_F413CHUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413CHUX"
	"BOARD_NAME=\"GENERIC_F413CHUX\""
	"BOARD_ID=GENERIC_F413CHUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413CHUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413CHUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413CHUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413CHUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413CHUX_hid_MCU}
)
target_link_libraries(GENERIC_F413CHUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413RGTX
# -----------------------------------------------------------------------------

set(GENERIC_F413RGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F413RGTX_MAXSIZE 1048576)
set(GENERIC_F413RGTX_MAXDATASIZE 327680)
set(GENERIC_F413RGTX_MCU cortex-m4)
set(GENERIC_F413RGTX_FPCONF "-")
add_library(GENERIC_F413RGTX INTERFACE)
target_compile_options(GENERIC_F413RGTX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RGTX_MCU}
)
target_compile_definitions(GENERIC_F413RGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413RGTX"
	"BOARD_NAME=\"GENERIC_F413RGTX\""
	"BOARD_ID=GENERIC_F413RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413RGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413RGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F413RGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F413RGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RGTX_MCU}
)
target_link_libraries(GENERIC_F413RGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413RGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413RGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413RGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413RGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413RGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F413RGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413RGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413RGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F413RGTX_hid_MAXSIZE 1048576)
set(GENERIC_F413RGTX_hid_MAXDATASIZE 327680)
set(GENERIC_F413RGTX_hid_MCU cortex-m4)
set(GENERIC_F413RGTX_hid_FPCONF "-")
add_library(GENERIC_F413RGTX_hid INTERFACE)
target_compile_options(GENERIC_F413RGTX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F413RGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413RGTX"
	"BOARD_NAME=\"GENERIC_F413RGTX\""
	"BOARD_ID=GENERIC_F413RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413RGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413RGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413RGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413RGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RGTX_hid_MCU}
)
target_link_libraries(GENERIC_F413RGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413RHTX
# -----------------------------------------------------------------------------

set(GENERIC_F413RHTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F413RHTX_MAXSIZE 1572864)
set(GENERIC_F413RHTX_MAXDATASIZE 327680)
set(GENERIC_F413RHTX_MCU cortex-m4)
set(GENERIC_F413RHTX_FPCONF "-")
add_library(GENERIC_F413RHTX INTERFACE)
target_compile_options(GENERIC_F413RHTX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RHTX_MCU}
)
target_compile_definitions(GENERIC_F413RHTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413RHTX"
	"BOARD_NAME=\"GENERIC_F413RHTX\""
	"BOARD_ID=GENERIC_F413RHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413RHTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413RHTX_VARIANT_PATH}
)

target_link_options(GENERIC_F413RHTX INTERFACE
  "LINKER:--default-script=${GENERIC_F413RHTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RHTX_MCU}
)
target_link_libraries(GENERIC_F413RHTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413RHTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413RHTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413RHTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413RHTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413RHTX_serial_none INTERFACE)
target_compile_options(GENERIC_F413RHTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413RHTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413RHTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F413RHTX_hid_MAXSIZE 1572864)
set(GENERIC_F413RHTX_hid_MAXDATASIZE 327680)
set(GENERIC_F413RHTX_hid_MCU cortex-m4)
set(GENERIC_F413RHTX_hid_FPCONF "-")
add_library(GENERIC_F413RHTX_hid INTERFACE)
target_compile_options(GENERIC_F413RHTX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RHTX_hid_MCU}
)
target_compile_definitions(GENERIC_F413RHTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413RHTX"
	"BOARD_NAME=\"GENERIC_F413RHTX\""
	"BOARD_ID=GENERIC_F413RHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413RHTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413RHTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413RHTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413RHTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413RHTX_hid_MCU}
)
target_link_libraries(GENERIC_F413RHTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413ZGJX
# -----------------------------------------------------------------------------

set(GENERIC_F413ZGJX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZGJX_MAXSIZE 1048576)
set(GENERIC_F413ZGJX_MAXDATASIZE 327680)
set(GENERIC_F413ZGJX_MCU cortex-m4)
set(GENERIC_F413ZGJX_FPCONF "-")
add_library(GENERIC_F413ZGJX INTERFACE)
target_compile_options(GENERIC_F413ZGJX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGJX_MCU}
)
target_compile_definitions(GENERIC_F413ZGJX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZGJX"
	"BOARD_NAME=\"GENERIC_F413ZGJX\""
	"BOARD_ID=GENERIC_F413ZGJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZGJX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZGJX_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZGJX INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZGJX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGJX_MCU}
)
target_link_libraries(GENERIC_F413ZGJX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413ZGJX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413ZGJX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413ZGJX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413ZGJX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413ZGJX_serial_none INTERFACE)
target_compile_options(GENERIC_F413ZGJX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413ZGJX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413ZGJX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZGJX_hid_MAXSIZE 1048576)
set(GENERIC_F413ZGJX_hid_MAXDATASIZE 327680)
set(GENERIC_F413ZGJX_hid_MCU cortex-m4)
set(GENERIC_F413ZGJX_hid_FPCONF "-")
add_library(GENERIC_F413ZGJX_hid INTERFACE)
target_compile_options(GENERIC_F413ZGJX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGJX_hid_MCU}
)
target_compile_definitions(GENERIC_F413ZGJX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZGJX"
	"BOARD_NAME=\"GENERIC_F413ZGJX\""
	"BOARD_ID=GENERIC_F413ZGJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZGJX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZGJX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZGJX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZGJX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGJX_hid_MCU}
)
target_link_libraries(GENERIC_F413ZGJX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F413ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZGTX_MAXSIZE 1048576)
set(GENERIC_F413ZGTX_MAXDATASIZE 327680)
set(GENERIC_F413ZGTX_MCU cortex-m4)
set(GENERIC_F413ZGTX_FPCONF "-")
add_library(GENERIC_F413ZGTX INTERFACE)
target_compile_options(GENERIC_F413ZGTX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGTX_MCU}
)
target_compile_definitions(GENERIC_F413ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZGTX"
	"BOARD_NAME=\"GENERIC_F413ZGTX\""
	"BOARD_ID=GENERIC_F413ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGTX_MCU}
)
target_link_libraries(GENERIC_F413ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F413ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F413ZGTX_hid_MAXDATASIZE 327680)
set(GENERIC_F413ZGTX_hid_MCU cortex-m4)
set(GENERIC_F413ZGTX_hid_FPCONF "-")
add_library(GENERIC_F413ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F413ZGTX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F413ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZGTX"
	"BOARD_NAME=\"GENERIC_F413ZGTX\""
	"BOARD_ID=GENERIC_F413ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F413ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413ZHJX
# -----------------------------------------------------------------------------

set(GENERIC_F413ZHJX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZHJX_MAXSIZE 1572864)
set(GENERIC_F413ZHJX_MAXDATASIZE 327680)
set(GENERIC_F413ZHJX_MCU cortex-m4)
set(GENERIC_F413ZHJX_FPCONF "-")
add_library(GENERIC_F413ZHJX INTERFACE)
target_compile_options(GENERIC_F413ZHJX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHJX_MCU}
)
target_compile_definitions(GENERIC_F413ZHJX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZHJX"
	"BOARD_NAME=\"GENERIC_F413ZHJX\""
	"BOARD_ID=GENERIC_F413ZHJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZHJX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZHJX_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZHJX INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZHJX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHJX_MCU}
)
target_link_libraries(GENERIC_F413ZHJX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413ZHJX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413ZHJX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413ZHJX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413ZHJX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413ZHJX_serial_none INTERFACE)
target_compile_options(GENERIC_F413ZHJX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413ZHJX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413ZHJX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZHJX_hid_MAXSIZE 1572864)
set(GENERIC_F413ZHJX_hid_MAXDATASIZE 327680)
set(GENERIC_F413ZHJX_hid_MCU cortex-m4)
set(GENERIC_F413ZHJX_hid_FPCONF "-")
add_library(GENERIC_F413ZHJX_hid INTERFACE)
target_compile_options(GENERIC_F413ZHJX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHJX_hid_MCU}
)
target_compile_definitions(GENERIC_F413ZHJX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZHJX"
	"BOARD_NAME=\"GENERIC_F413ZHJX\""
	"BOARD_ID=GENERIC_F413ZHJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZHJX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZHJX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZHJX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZHJX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHJX_hid_MCU}
)
target_link_libraries(GENERIC_F413ZHJX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F413ZHTX
# -----------------------------------------------------------------------------

set(GENERIC_F413ZHTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZHTX_MAXSIZE 1572864)
set(GENERIC_F413ZHTX_MAXDATASIZE 327680)
set(GENERIC_F413ZHTX_MCU cortex-m4)
set(GENERIC_F413ZHTX_FPCONF "-")
add_library(GENERIC_F413ZHTX INTERFACE)
target_compile_options(GENERIC_F413ZHTX INTERFACE
  "SHELL:-DSTM32F413xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHTX_MCU}
)
target_compile_definitions(GENERIC_F413ZHTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZHTX"
	"BOARD_NAME=\"GENERIC_F413ZHTX\""
	"BOARD_ID=GENERIC_F413ZHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZHTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZHTX_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZHTX INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZHTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHTX_MCU}
)
target_link_libraries(GENERIC_F413ZHTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F413ZHTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F413ZHTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F413ZHTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F413ZHTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F413ZHTX_serial_none INTERFACE)
target_compile_options(GENERIC_F413ZHTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F413ZHTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F413ZHTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F413ZHTX_hid_MAXSIZE 1572864)
set(GENERIC_F413ZHTX_hid_MAXDATASIZE 327680)
set(GENERIC_F413ZHTX_hid_MCU cortex-m4)
set(GENERIC_F413ZHTX_hid_FPCONF "-")
add_library(GENERIC_F413ZHTX_hid INTERFACE)
target_compile_options(GENERIC_F413ZHTX_hid INTERFACE
  "SHELL:-DSTM32F413xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHTX_hid_MCU}
)
target_compile_definitions(GENERIC_F413ZHTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F413ZHTX"
	"BOARD_NAME=\"GENERIC_F413ZHTX\""
	"BOARD_ID=GENERIC_F413ZHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F413ZHTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F413ZHTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F413ZHTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F413ZHTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F413ZHTX_hid_MCU}
)
target_link_libraries(GENERIC_F413ZHTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F415RGTX
# -----------------------------------------------------------------------------

set(GENERIC_F415RGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(GENERIC_F415RGTX_MAXSIZE 1048576)
set(GENERIC_F415RGTX_MAXDATASIZE 131072)
set(GENERIC_F415RGTX_MCU cortex-m4)
set(GENERIC_F415RGTX_FPCONF "-")
add_library(GENERIC_F415RGTX INTERFACE)
target_compile_options(GENERIC_F415RGTX INTERFACE
  "SHELL:-DSTM32F415xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F415RGTX_MCU}
)
target_compile_definitions(GENERIC_F415RGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F415RGTX"
	"BOARD_NAME=\"GENERIC_F415RGTX\""
	"BOARD_ID=GENERIC_F415RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F415RGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F415RGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F415RGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F415RGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F415RGTX_MCU}
)
target_link_libraries(GENERIC_F415RGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F415RGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F415RGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F415RGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F415RGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F415RGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F415RGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F415RGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F415RGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F405RGT_F415RGT")
set(GENERIC_F415RGTX_hid_MAXSIZE 1048576)
set(GENERIC_F415RGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F415RGTX_hid_MCU cortex-m4)
set(GENERIC_F415RGTX_hid_FPCONF "-")
add_library(GENERIC_F415RGTX_hid INTERFACE)
target_compile_options(GENERIC_F415RGTX_hid INTERFACE
  "SHELL:-DSTM32F415xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F415RGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F415RGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F415RGTX"
	"BOARD_NAME=\"GENERIC_F415RGTX\""
	"BOARD_ID=GENERIC_F415RGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F415RGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F415RGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F415RGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F415RGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F415RGTX_hid_MCU}
)
target_link_libraries(GENERIC_F415RGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F417VETX
# -----------------------------------------------------------------------------

set(GENERIC_F417VETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F417VETX_MAXSIZE 524288)
set(GENERIC_F417VETX_MAXDATASIZE 131072)
set(GENERIC_F417VETX_MCU cortex-m4)
set(GENERIC_F417VETX_FPCONF "-")
add_library(GENERIC_F417VETX INTERFACE)
target_compile_options(GENERIC_F417VETX INTERFACE
  "SHELL:-DSTM32F417xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VETX_MCU}
)
target_compile_definitions(GENERIC_F417VETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417VETX"
	"BOARD_NAME=\"GENERIC_F417VETX\""
	"BOARD_ID=GENERIC_F417VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417VETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417VETX_VARIANT_PATH}
)

target_link_options(GENERIC_F417VETX INTERFACE
  "LINKER:--default-script=${GENERIC_F417VETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VETX_MCU}
)
target_link_libraries(GENERIC_F417VETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F417VETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F417VETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F417VETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F417VETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F417VETX_serial_none INTERFACE)
target_compile_options(GENERIC_F417VETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F417VETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F417VETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F417VETX_hid_MAXSIZE 524288)
set(GENERIC_F417VETX_hid_MAXDATASIZE 131072)
set(GENERIC_F417VETX_hid_MCU cortex-m4)
set(GENERIC_F417VETX_hid_FPCONF "-")
add_library(GENERIC_F417VETX_hid INTERFACE)
target_compile_options(GENERIC_F417VETX_hid INTERFACE
  "SHELL:-DSTM32F417xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VETX_hid_MCU}
)
target_compile_definitions(GENERIC_F417VETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417VETX"
	"BOARD_NAME=\"GENERIC_F417VETX\""
	"BOARD_ID=GENERIC_F417VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417VETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417VETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F417VETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F417VETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VETX_hid_MCU}
)
target_link_libraries(GENERIC_F417VETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F417VGTX
# -----------------------------------------------------------------------------

set(GENERIC_F417VGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F417VGTX_MAXSIZE 1048576)
set(GENERIC_F417VGTX_MAXDATASIZE 131072)
set(GENERIC_F417VGTX_MCU cortex-m4)
set(GENERIC_F417VGTX_FPCONF "-")
add_library(GENERIC_F417VGTX INTERFACE)
target_compile_options(GENERIC_F417VGTX INTERFACE
  "SHELL:-DSTM32F417xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VGTX_MCU}
)
target_compile_definitions(GENERIC_F417VGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417VGTX"
	"BOARD_NAME=\"GENERIC_F417VGTX\""
	"BOARD_ID=GENERIC_F417VGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417VGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417VGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F417VGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F417VGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VGTX_MCU}
)
target_link_libraries(GENERIC_F417VGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F417VGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F417VGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F417VGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F417VGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F417VGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F417VGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F417VGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F417VGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407V(E-G)T_F417V(E-G)T")
set(GENERIC_F417VGTX_hid_MAXSIZE 1048576)
set(GENERIC_F417VGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F417VGTX_hid_MCU cortex-m4)
set(GENERIC_F417VGTX_hid_FPCONF "-")
add_library(GENERIC_F417VGTX_hid INTERFACE)
target_compile_options(GENERIC_F417VGTX_hid INTERFACE
  "SHELL:-DSTM32F417xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F417VGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417VGTX"
	"BOARD_NAME=\"GENERIC_F417VGTX\""
	"BOARD_ID=GENERIC_F417VGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417VGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417VGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F417VGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F417VGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417VGTX_hid_MCU}
)
target_link_libraries(GENERIC_F417VGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F417ZETX
# -----------------------------------------------------------------------------

set(GENERIC_F417ZETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F417ZETX_MAXSIZE 524288)
set(GENERIC_F417ZETX_MAXDATASIZE 131072)
set(GENERIC_F417ZETX_MCU cortex-m4)
set(GENERIC_F417ZETX_FPCONF "-")
add_library(GENERIC_F417ZETX INTERFACE)
target_compile_options(GENERIC_F417ZETX INTERFACE
  "SHELL:-DSTM32F417xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZETX_MCU}
)
target_compile_definitions(GENERIC_F417ZETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417ZETX"
	"BOARD_NAME=\"GENERIC_F417ZETX\""
	"BOARD_ID=GENERIC_F417ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417ZETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417ZETX_VARIANT_PATH}
)

target_link_options(GENERIC_F417ZETX INTERFACE
  "LINKER:--default-script=${GENERIC_F417ZETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZETX_MCU}
)
target_link_libraries(GENERIC_F417ZETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F417ZETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F417ZETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F417ZETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F417ZETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F417ZETX_serial_none INTERFACE)
target_compile_options(GENERIC_F417ZETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F417ZETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F417ZETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F417ZETX_hid_MAXSIZE 524288)
set(GENERIC_F417ZETX_hid_MAXDATASIZE 131072)
set(GENERIC_F417ZETX_hid_MCU cortex-m4)
set(GENERIC_F417ZETX_hid_FPCONF "-")
add_library(GENERIC_F417ZETX_hid INTERFACE)
target_compile_options(GENERIC_F417ZETX_hid INTERFACE
  "SHELL:-DSTM32F417xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZETX_hid_MCU}
)
target_compile_definitions(GENERIC_F417ZETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417ZETX"
	"BOARD_NAME=\"GENERIC_F417ZETX\""
	"BOARD_ID=GENERIC_F417ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417ZETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417ZETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F417ZETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F417ZETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZETX_hid_MCU}
)
target_link_libraries(GENERIC_F417ZETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F417ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F417ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F417ZGTX_MAXSIZE 1048576)
set(GENERIC_F417ZGTX_MAXDATASIZE 131072)
set(GENERIC_F417ZGTX_MCU cortex-m4)
set(GENERIC_F417ZGTX_FPCONF "-")
add_library(GENERIC_F417ZGTX INTERFACE)
target_compile_options(GENERIC_F417ZGTX INTERFACE
  "SHELL:-DSTM32F417xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZGTX_MCU}
)
target_compile_definitions(GENERIC_F417ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417ZGTX"
	"BOARD_NAME=\"GENERIC_F417ZGTX\""
	"BOARD_ID=GENERIC_F417ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F417ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F417ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZGTX_MCU}
)
target_link_libraries(GENERIC_F417ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F417ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F417ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F417ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F417ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F417ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F417ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F417ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F417ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(GENERIC_F417ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F417ZGTX_hid_MAXDATASIZE 131072)
set(GENERIC_F417ZGTX_hid_MCU cortex-m4)
set(GENERIC_F417ZGTX_hid_FPCONF "-")
add_library(GENERIC_F417ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F417ZGTX_hid INTERFACE
  "SHELL:-DSTM32F417xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F417ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F417ZGTX"
	"BOARD_NAME=\"GENERIC_F417ZGTX\""
	"BOARD_ID=GENERIC_F417ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F417ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F417ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F417ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F417ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F417ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F417ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F423CHUX
# -----------------------------------------------------------------------------

set(GENERIC_F423CHUX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F423CHUX_MAXSIZE 1572864)
set(GENERIC_F423CHUX_MAXDATASIZE 327680)
set(GENERIC_F423CHUX_MCU cortex-m4)
set(GENERIC_F423CHUX_FPCONF "-")
add_library(GENERIC_F423CHUX INTERFACE)
target_compile_options(GENERIC_F423CHUX INTERFACE
  "SHELL:-DSTM32F423xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423CHUX_MCU}
)
target_compile_definitions(GENERIC_F423CHUX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423CHUX"
	"BOARD_NAME=\"GENERIC_F423CHUX\""
	"BOARD_ID=GENERIC_F423CHUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423CHUX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423CHUX_VARIANT_PATH}
)

target_link_options(GENERIC_F423CHUX INTERFACE
  "LINKER:--default-script=${GENERIC_F423CHUX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423CHUX_MCU}
)
target_link_libraries(GENERIC_F423CHUX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F423CHUX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F423CHUX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F423CHUX_serial_generic INTERFACE)
target_compile_options(GENERIC_F423CHUX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F423CHUX_serial_none INTERFACE)
target_compile_options(GENERIC_F423CHUX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F423CHUX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F423CHUX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413C(G-H)U_F423CHU")
set(GENERIC_F423CHUX_hid_MAXSIZE 1572864)
set(GENERIC_F423CHUX_hid_MAXDATASIZE 327680)
set(GENERIC_F423CHUX_hid_MCU cortex-m4)
set(GENERIC_F423CHUX_hid_FPCONF "-")
add_library(GENERIC_F423CHUX_hid INTERFACE)
target_compile_options(GENERIC_F423CHUX_hid INTERFACE
  "SHELL:-DSTM32F423xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423CHUX_hid_MCU}
)
target_compile_definitions(GENERIC_F423CHUX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423CHUX"
	"BOARD_NAME=\"GENERIC_F423CHUX\""
	"BOARD_ID=GENERIC_F423CHUX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423CHUX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423CHUX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F423CHUX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F423CHUX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423CHUX_hid_MCU}
)
target_link_libraries(GENERIC_F423CHUX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F423RHTX
# -----------------------------------------------------------------------------

set(GENERIC_F423RHTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F423RHTX_MAXSIZE 1572864)
set(GENERIC_F423RHTX_MAXDATASIZE 327680)
set(GENERIC_F423RHTX_MCU cortex-m4)
set(GENERIC_F423RHTX_FPCONF "-")
add_library(GENERIC_F423RHTX INTERFACE)
target_compile_options(GENERIC_F423RHTX INTERFACE
  "SHELL:-DSTM32F423xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423RHTX_MCU}
)
target_compile_definitions(GENERIC_F423RHTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423RHTX"
	"BOARD_NAME=\"GENERIC_F423RHTX\""
	"BOARD_ID=GENERIC_F423RHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423RHTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423RHTX_VARIANT_PATH}
)

target_link_options(GENERIC_F423RHTX INTERFACE
  "LINKER:--default-script=${GENERIC_F423RHTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423RHTX_MCU}
)
target_link_libraries(GENERIC_F423RHTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F423RHTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F423RHTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F423RHTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F423RHTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F423RHTX_serial_none INTERFACE)
target_compile_options(GENERIC_F423RHTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F423RHTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F423RHTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413R(G-H)T_F423RHT")
set(GENERIC_F423RHTX_hid_MAXSIZE 1572864)
set(GENERIC_F423RHTX_hid_MAXDATASIZE 327680)
set(GENERIC_F423RHTX_hid_MCU cortex-m4)
set(GENERIC_F423RHTX_hid_FPCONF "-")
add_library(GENERIC_F423RHTX_hid INTERFACE)
target_compile_options(GENERIC_F423RHTX_hid INTERFACE
  "SHELL:-DSTM32F423xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423RHTX_hid_MCU}
)
target_compile_definitions(GENERIC_F423RHTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423RHTX"
	"BOARD_NAME=\"GENERIC_F423RHTX\""
	"BOARD_ID=GENERIC_F423RHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423RHTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423RHTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F423RHTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F423RHTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423RHTX_hid_MCU}
)
target_link_libraries(GENERIC_F423RHTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F423ZHJX
# -----------------------------------------------------------------------------

set(GENERIC_F423ZHJX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F423ZHJX_MAXSIZE 1572864)
set(GENERIC_F423ZHJX_MAXDATASIZE 327680)
set(GENERIC_F423ZHJX_MCU cortex-m4)
set(GENERIC_F423ZHJX_FPCONF "-")
add_library(GENERIC_F423ZHJX INTERFACE)
target_compile_options(GENERIC_F423ZHJX INTERFACE
  "SHELL:-DSTM32F423xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHJX_MCU}
)
target_compile_definitions(GENERIC_F423ZHJX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423ZHJX"
	"BOARD_NAME=\"GENERIC_F423ZHJX\""
	"BOARD_ID=GENERIC_F423ZHJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423ZHJX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423ZHJX_VARIANT_PATH}
)

target_link_options(GENERIC_F423ZHJX INTERFACE
  "LINKER:--default-script=${GENERIC_F423ZHJX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHJX_MCU}
)
target_link_libraries(GENERIC_F423ZHJX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F423ZHJX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F423ZHJX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F423ZHJX_serial_generic INTERFACE)
target_compile_options(GENERIC_F423ZHJX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F423ZHJX_serial_none INTERFACE)
target_compile_options(GENERIC_F423ZHJX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F423ZHJX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F423ZHJX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F423ZHJX_hid_MAXSIZE 1572864)
set(GENERIC_F423ZHJX_hid_MAXDATASIZE 327680)
set(GENERIC_F423ZHJX_hid_MCU cortex-m4)
set(GENERIC_F423ZHJX_hid_FPCONF "-")
add_library(GENERIC_F423ZHJX_hid INTERFACE)
target_compile_options(GENERIC_F423ZHJX_hid INTERFACE
  "SHELL:-DSTM32F423xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHJX_hid_MCU}
)
target_compile_definitions(GENERIC_F423ZHJX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423ZHJX"
	"BOARD_NAME=\"GENERIC_F423ZHJX\""
	"BOARD_ID=GENERIC_F423ZHJX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423ZHJX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423ZHJX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F423ZHJX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F423ZHJX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHJX_hid_MCU}
)
target_link_libraries(GENERIC_F423ZHJX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F423ZHTX
# -----------------------------------------------------------------------------

set(GENERIC_F423ZHTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F423ZHTX_MAXSIZE 1572864)
set(GENERIC_F423ZHTX_MAXDATASIZE 327680)
set(GENERIC_F423ZHTX_MCU cortex-m4)
set(GENERIC_F423ZHTX_FPCONF "-")
add_library(GENERIC_F423ZHTX INTERFACE)
target_compile_options(GENERIC_F423ZHTX INTERFACE
  "SHELL:-DSTM32F423xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHTX_MCU}
)
target_compile_definitions(GENERIC_F423ZHTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423ZHTX"
	"BOARD_NAME=\"GENERIC_F423ZHTX\""
	"BOARD_ID=GENERIC_F423ZHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423ZHTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423ZHTX_VARIANT_PATH}
)

target_link_options(GENERIC_F423ZHTX INTERFACE
  "LINKER:--default-script=${GENERIC_F423ZHTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHTX_MCU}
)
target_link_libraries(GENERIC_F423ZHTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F423ZHTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F423ZHTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F423ZHTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F423ZHTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F423ZHTX_serial_none INTERFACE)
target_compile_options(GENERIC_F423ZHTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F423ZHTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F423ZHTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F413Z(G-H)(J-T)_F423ZH(J-T)")
set(GENERIC_F423ZHTX_hid_MAXSIZE 1572864)
set(GENERIC_F423ZHTX_hid_MAXDATASIZE 327680)
set(GENERIC_F423ZHTX_hid_MCU cortex-m4)
set(GENERIC_F423ZHTX_hid_FPCONF "-")
add_library(GENERIC_F423ZHTX_hid INTERFACE)
target_compile_options(GENERIC_F423ZHTX_hid INTERFACE
  "SHELL:-DSTM32F423xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHTX_hid_MCU}
)
target_compile_definitions(GENERIC_F423ZHTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F423ZHTX"
	"BOARD_NAME=\"GENERIC_F423ZHTX\""
	"BOARD_ID=GENERIC_F423ZHTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F423ZHTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F423ZHTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F423ZHTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F423ZHTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1572864"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=327680"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F423ZHTX_hid_MCU}
)
target_link_libraries(GENERIC_F423ZHTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F427ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F427ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F427ZGTX_MAXSIZE 1048576)
set(GENERIC_F427ZGTX_MAXDATASIZE 196608)
set(GENERIC_F427ZGTX_MCU cortex-m4)
set(GENERIC_F427ZGTX_FPCONF "-")
add_library(GENERIC_F427ZGTX INTERFACE)
target_compile_options(GENERIC_F427ZGTX INTERFACE
  "SHELL:-DSTM32F427xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZGTX_MCU}
)
target_compile_definitions(GENERIC_F427ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F427ZGTX"
	"BOARD_NAME=\"GENERIC_F427ZGTX\""
	"BOARD_ID=GENERIC_F427ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F427ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F427ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F427ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F427ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZGTX_MCU}
)
target_link_libraries(GENERIC_F427ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F427ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F427ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F427ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F427ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F427ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F427ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F427ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F427ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F427ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F427ZGTX_hid_MAXDATASIZE 196608)
set(GENERIC_F427ZGTX_hid_MCU cortex-m4)
set(GENERIC_F427ZGTX_hid_FPCONF "-")
add_library(GENERIC_F427ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F427ZGTX_hid INTERFACE
  "SHELL:-DSTM32F427xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F427ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F427ZGTX"
	"BOARD_NAME=\"GENERIC_F427ZGTX\""
	"BOARD_ID=GENERIC_F427ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F427ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F427ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F427ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F427ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F427ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F427ZITX
# -----------------------------------------------------------------------------

set(GENERIC_F427ZITX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F427ZITX_MAXSIZE 2097152)
set(GENERIC_F427ZITX_MAXDATASIZE 196608)
set(GENERIC_F427ZITX_MCU cortex-m4)
set(GENERIC_F427ZITX_FPCONF "-")
add_library(GENERIC_F427ZITX INTERFACE)
target_compile_options(GENERIC_F427ZITX INTERFACE
  "SHELL:-DSTM32F427xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZITX_MCU}
)
target_compile_definitions(GENERIC_F427ZITX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F427ZITX"
	"BOARD_NAME=\"GENERIC_F427ZITX\""
	"BOARD_ID=GENERIC_F427ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F427ZITX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F427ZITX_VARIANT_PATH}
)

target_link_options(GENERIC_F427ZITX INTERFACE
  "LINKER:--default-script=${GENERIC_F427ZITX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZITX_MCU}
)
target_link_libraries(GENERIC_F427ZITX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F427ZITX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F427ZITX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F427ZITX_serial_generic INTERFACE)
target_compile_options(GENERIC_F427ZITX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F427ZITX_serial_none INTERFACE)
target_compile_options(GENERIC_F427ZITX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F427ZITX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F427ZITX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F427ZITX_hid_MAXSIZE 2097152)
set(GENERIC_F427ZITX_hid_MAXDATASIZE 196608)
set(GENERIC_F427ZITX_hid_MCU cortex-m4)
set(GENERIC_F427ZITX_hid_FPCONF "-")
add_library(GENERIC_F427ZITX_hid INTERFACE)
target_compile_options(GENERIC_F427ZITX_hid INTERFACE
  "SHELL:-DSTM32F427xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZITX_hid_MCU}
)
target_compile_definitions(GENERIC_F427ZITX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F427ZITX"
	"BOARD_NAME=\"GENERIC_F427ZITX\""
	"BOARD_ID=GENERIC_F427ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F427ZITX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F427ZITX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F427ZITX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F427ZITX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F427ZITX_hid_MCU}
)
target_link_libraries(GENERIC_F427ZITX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F429ZETX
# -----------------------------------------------------------------------------

set(GENERIC_F429ZETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZETX_MAXSIZE 524288)
set(GENERIC_F429ZETX_MAXDATASIZE 196608)
set(GENERIC_F429ZETX_MCU cortex-m4)
set(GENERIC_F429ZETX_FPCONF "-")
add_library(GENERIC_F429ZETX INTERFACE)
target_compile_options(GENERIC_F429ZETX INTERFACE
  "SHELL:-DSTM32F429xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZETX_MCU}
)
target_compile_definitions(GENERIC_F429ZETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZETX"
	"BOARD_NAME=\"GENERIC_F429ZETX\""
	"BOARD_ID=GENERIC_F429ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZETX_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZETX INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZETX_MCU}
)
target_link_libraries(GENERIC_F429ZETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F429ZETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F429ZETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F429ZETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F429ZETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F429ZETX_serial_none INTERFACE)
target_compile_options(GENERIC_F429ZETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F429ZETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F429ZETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZETX_hid_MAXSIZE 524288)
set(GENERIC_F429ZETX_hid_MAXDATASIZE 196608)
set(GENERIC_F429ZETX_hid_MCU cortex-m4)
set(GENERIC_F429ZETX_hid_FPCONF "-")
add_library(GENERIC_F429ZETX_hid INTERFACE)
target_compile_options(GENERIC_F429ZETX_hid INTERFACE
  "SHELL:-DSTM32F429xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZETX_hid_MCU}
)
target_compile_definitions(GENERIC_F429ZETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZETX"
	"BOARD_NAME=\"GENERIC_F429ZETX\""
	"BOARD_ID=GENERIC_F429ZETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZETX_hid_MCU}
)
target_link_libraries(GENERIC_F429ZETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F429ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F429ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZGTX_MAXSIZE 1048576)
set(GENERIC_F429ZGTX_MAXDATASIZE 196608)
set(GENERIC_F429ZGTX_MCU cortex-m4)
set(GENERIC_F429ZGTX_FPCONF "-")
add_library(GENERIC_F429ZGTX INTERFACE)
target_compile_options(GENERIC_F429ZGTX INTERFACE
  "SHELL:-DSTM32F429xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGTX_MCU}
)
target_compile_definitions(GENERIC_F429ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZGTX"
	"BOARD_NAME=\"GENERIC_F429ZGTX\""
	"BOARD_ID=GENERIC_F429ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGTX_MCU}
)
target_link_libraries(GENERIC_F429ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F429ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F429ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F429ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F429ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F429ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F429ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F429ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F429ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F429ZGTX_hid_MAXDATASIZE 196608)
set(GENERIC_F429ZGTX_hid_MCU cortex-m4)
set(GENERIC_F429ZGTX_hid_FPCONF "-")
add_library(GENERIC_F429ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F429ZGTX_hid INTERFACE
  "SHELL:-DSTM32F429xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F429ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZGTX"
	"BOARD_NAME=\"GENERIC_F429ZGTX\""
	"BOARD_ID=GENERIC_F429ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F429ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F429ZGYX
# -----------------------------------------------------------------------------

set(GENERIC_F429ZGYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZGYX_MAXSIZE 1048576)
set(GENERIC_F429ZGYX_MAXDATASIZE 196608)
set(GENERIC_F429ZGYX_MCU cortex-m4)
set(GENERIC_F429ZGYX_FPCONF "-")
add_library(GENERIC_F429ZGYX INTERFACE)
target_compile_options(GENERIC_F429ZGYX INTERFACE
  "SHELL:-DSTM32F429xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGYX_MCU}
)
target_compile_definitions(GENERIC_F429ZGYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZGYX"
	"BOARD_NAME=\"GENERIC_F429ZGYX\""
	"BOARD_ID=GENERIC_F429ZGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZGYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZGYX_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZGYX INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZGYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGYX_MCU}
)
target_link_libraries(GENERIC_F429ZGYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F429ZGYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F429ZGYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F429ZGYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F429ZGYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F429ZGYX_serial_none INTERFACE)
target_compile_options(GENERIC_F429ZGYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F429ZGYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F429ZGYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZGYX_hid_MAXSIZE 1048576)
set(GENERIC_F429ZGYX_hid_MAXDATASIZE 196608)
set(GENERIC_F429ZGYX_hid_MCU cortex-m4)
set(GENERIC_F429ZGYX_hid_FPCONF "-")
add_library(GENERIC_F429ZGYX_hid INTERFACE)
target_compile_options(GENERIC_F429ZGYX_hid INTERFACE
  "SHELL:-DSTM32F429xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGYX_hid_MCU}
)
target_compile_definitions(GENERIC_F429ZGYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZGYX"
	"BOARD_NAME=\"GENERIC_F429ZGYX\""
	"BOARD_ID=GENERIC_F429ZGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZGYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZGYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZGYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZGYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZGYX_hid_MCU}
)
target_link_libraries(GENERIC_F429ZGYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F429ZITX
# -----------------------------------------------------------------------------

set(GENERIC_F429ZITX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZITX_MAXSIZE 2097152)
set(GENERIC_F429ZITX_MAXDATASIZE 196608)
set(GENERIC_F429ZITX_MCU cortex-m4)
set(GENERIC_F429ZITX_FPCONF "-")
add_library(GENERIC_F429ZITX INTERFACE)
target_compile_options(GENERIC_F429ZITX INTERFACE
  "SHELL:-DSTM32F429xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZITX_MCU}
)
target_compile_definitions(GENERIC_F429ZITX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZITX"
	"BOARD_NAME=\"GENERIC_F429ZITX\""
	"BOARD_ID=GENERIC_F429ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZITX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZITX_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZITX INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZITX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZITX_MCU}
)
target_link_libraries(GENERIC_F429ZITX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F429ZITX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F429ZITX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F429ZITX_serial_generic INTERFACE)
target_compile_options(GENERIC_F429ZITX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F429ZITX_serial_none INTERFACE)
target_compile_options(GENERIC_F429ZITX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F429ZITX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F429ZITX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZITX_hid_MAXSIZE 2097152)
set(GENERIC_F429ZITX_hid_MAXDATASIZE 196608)
set(GENERIC_F429ZITX_hid_MCU cortex-m4)
set(GENERIC_F429ZITX_hid_FPCONF "-")
add_library(GENERIC_F429ZITX_hid INTERFACE)
target_compile_options(GENERIC_F429ZITX_hid INTERFACE
  "SHELL:-DSTM32F429xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZITX_hid_MCU}
)
target_compile_definitions(GENERIC_F429ZITX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZITX"
	"BOARD_NAME=\"GENERIC_F429ZITX\""
	"BOARD_ID=GENERIC_F429ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZITX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZITX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZITX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZITX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZITX_hid_MCU}
)
target_link_libraries(GENERIC_F429ZITX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F429ZIYX
# -----------------------------------------------------------------------------

set(GENERIC_F429ZIYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZIYX_MAXSIZE 2097152)
set(GENERIC_F429ZIYX_MAXDATASIZE 196608)
set(GENERIC_F429ZIYX_MCU cortex-m4)
set(GENERIC_F429ZIYX_FPCONF "-")
add_library(GENERIC_F429ZIYX INTERFACE)
target_compile_options(GENERIC_F429ZIYX INTERFACE
  "SHELL:-DSTM32F429xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZIYX_MCU}
)
target_compile_definitions(GENERIC_F429ZIYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZIYX"
	"BOARD_NAME=\"GENERIC_F429ZIYX\""
	"BOARD_ID=GENERIC_F429ZIYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZIYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZIYX_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZIYX INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZIYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZIYX_MCU}
)
target_link_libraries(GENERIC_F429ZIYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F429ZIYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F429ZIYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F429ZIYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F429ZIYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F429ZIYX_serial_none INTERFACE)
target_compile_options(GENERIC_F429ZIYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F429ZIYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F429ZIYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F429ZIYX_hid_MAXSIZE 2097152)
set(GENERIC_F429ZIYX_hid_MAXDATASIZE 196608)
set(GENERIC_F429ZIYX_hid_MCU cortex-m4)
set(GENERIC_F429ZIYX_hid_FPCONF "-")
add_library(GENERIC_F429ZIYX_hid INTERFACE)
target_compile_options(GENERIC_F429ZIYX_hid INTERFACE
  "SHELL:-DSTM32F429xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZIYX_hid_MCU}
)
target_compile_definitions(GENERIC_F429ZIYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F429ZIYX"
	"BOARD_NAME=\"GENERIC_F429ZIYX\""
	"BOARD_ID=GENERIC_F429ZIYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F429ZIYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F429ZIYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F429ZIYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F429ZIYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F429ZIYX_hid_MCU}
)
target_link_libraries(GENERIC_F429ZIYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F437ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F437ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F437ZGTX_MAXSIZE 1048576)
set(GENERIC_F437ZGTX_MAXDATASIZE 196608)
set(GENERIC_F437ZGTX_MCU cortex-m4)
set(GENERIC_F437ZGTX_FPCONF "-")
add_library(GENERIC_F437ZGTX INTERFACE)
target_compile_options(GENERIC_F437ZGTX INTERFACE
  "SHELL:-DSTM32F437xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZGTX_MCU}
)
target_compile_definitions(GENERIC_F437ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F437ZGTX"
	"BOARD_NAME=\"GENERIC_F437ZGTX\""
	"BOARD_ID=GENERIC_F437ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F437ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F437ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F437ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F437ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZGTX_MCU}
)
target_link_libraries(GENERIC_F437ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F437ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F437ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F437ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F437ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F437ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F437ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F437ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F437ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F437ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F437ZGTX_hid_MAXDATASIZE 196608)
set(GENERIC_F437ZGTX_hid_MCU cortex-m4)
set(GENERIC_F437ZGTX_hid_FPCONF "-")
add_library(GENERIC_F437ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F437ZGTX_hid INTERFACE
  "SHELL:-DSTM32F437xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F437ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F437ZGTX"
	"BOARD_NAME=\"GENERIC_F437ZGTX\""
	"BOARD_ID=GENERIC_F437ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F437ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F437ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F437ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F437ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F437ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F437ZITX
# -----------------------------------------------------------------------------

set(GENERIC_F437ZITX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F437ZITX_MAXSIZE 2097152)
set(GENERIC_F437ZITX_MAXDATASIZE 196608)
set(GENERIC_F437ZITX_MCU cortex-m4)
set(GENERIC_F437ZITX_FPCONF "-")
add_library(GENERIC_F437ZITX INTERFACE)
target_compile_options(GENERIC_F437ZITX INTERFACE
  "SHELL:-DSTM32F437xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZITX_MCU}
)
target_compile_definitions(GENERIC_F437ZITX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F437ZITX"
	"BOARD_NAME=\"GENERIC_F437ZITX\""
	"BOARD_ID=GENERIC_F437ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F437ZITX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F437ZITX_VARIANT_PATH}
)

target_link_options(GENERIC_F437ZITX INTERFACE
  "LINKER:--default-script=${GENERIC_F437ZITX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZITX_MCU}
)
target_link_libraries(GENERIC_F437ZITX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F437ZITX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F437ZITX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F437ZITX_serial_generic INTERFACE)
target_compile_options(GENERIC_F437ZITX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F437ZITX_serial_none INTERFACE)
target_compile_options(GENERIC_F437ZITX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F437ZITX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F437ZITX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F437ZITX_hid_MAXSIZE 2097152)
set(GENERIC_F437ZITX_hid_MAXDATASIZE 196608)
set(GENERIC_F437ZITX_hid_MCU cortex-m4)
set(GENERIC_F437ZITX_hid_FPCONF "-")
add_library(GENERIC_F437ZITX_hid INTERFACE)
target_compile_options(GENERIC_F437ZITX_hid INTERFACE
  "SHELL:-DSTM32F437xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZITX_hid_MCU}
)
target_compile_definitions(GENERIC_F437ZITX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F437ZITX"
	"BOARD_NAME=\"GENERIC_F437ZITX\""
	"BOARD_ID=GENERIC_F437ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F437ZITX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F437ZITX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F437ZITX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F437ZITX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F437ZITX_hid_MCU}
)
target_link_libraries(GENERIC_F437ZITX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F439ZGTX
# -----------------------------------------------------------------------------

set(GENERIC_F439ZGTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZGTX_MAXSIZE 1048576)
set(GENERIC_F439ZGTX_MAXDATASIZE 196608)
set(GENERIC_F439ZGTX_MCU cortex-m4)
set(GENERIC_F439ZGTX_FPCONF "-")
add_library(GENERIC_F439ZGTX INTERFACE)
target_compile_options(GENERIC_F439ZGTX INTERFACE
  "SHELL:-DSTM32F439xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGTX_MCU}
)
target_compile_definitions(GENERIC_F439ZGTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZGTX"
	"BOARD_NAME=\"GENERIC_F439ZGTX\""
	"BOARD_ID=GENERIC_F439ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZGTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZGTX_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZGTX INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZGTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGTX_MCU}
)
target_link_libraries(GENERIC_F439ZGTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F439ZGTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F439ZGTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F439ZGTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F439ZGTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F439ZGTX_serial_none INTERFACE)
target_compile_options(GENERIC_F439ZGTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F439ZGTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F439ZGTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZGTX_hid_MAXSIZE 1048576)
set(GENERIC_F439ZGTX_hid_MAXDATASIZE 196608)
set(GENERIC_F439ZGTX_hid_MCU cortex-m4)
set(GENERIC_F439ZGTX_hid_FPCONF "-")
add_library(GENERIC_F439ZGTX_hid INTERFACE)
target_compile_options(GENERIC_F439ZGTX_hid INTERFACE
  "SHELL:-DSTM32F439xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGTX_hid_MCU}
)
target_compile_definitions(GENERIC_F439ZGTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZGTX"
	"BOARD_NAME=\"GENERIC_F439ZGTX\""
	"BOARD_ID=GENERIC_F439ZGTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZGTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZGTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZGTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZGTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGTX_hid_MCU}
)
target_link_libraries(GENERIC_F439ZGTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F439ZGYX
# -----------------------------------------------------------------------------

set(GENERIC_F439ZGYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZGYX_MAXSIZE 1048576)
set(GENERIC_F439ZGYX_MAXDATASIZE 196608)
set(GENERIC_F439ZGYX_MCU cortex-m4)
set(GENERIC_F439ZGYX_FPCONF "-")
add_library(GENERIC_F439ZGYX INTERFACE)
target_compile_options(GENERIC_F439ZGYX INTERFACE
  "SHELL:-DSTM32F439xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGYX_MCU}
)
target_compile_definitions(GENERIC_F439ZGYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZGYX"
	"BOARD_NAME=\"GENERIC_F439ZGYX\""
	"BOARD_ID=GENERIC_F439ZGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZGYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZGYX_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZGYX INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZGYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGYX_MCU}
)
target_link_libraries(GENERIC_F439ZGYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F439ZGYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F439ZGYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F439ZGYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F439ZGYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F439ZGYX_serial_none INTERFACE)
target_compile_options(GENERIC_F439ZGYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F439ZGYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F439ZGYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZGYX_hid_MAXSIZE 1048576)
set(GENERIC_F439ZGYX_hid_MAXDATASIZE 196608)
set(GENERIC_F439ZGYX_hid_MCU cortex-m4)
set(GENERIC_F439ZGYX_hid_FPCONF "-")
add_library(GENERIC_F439ZGYX_hid INTERFACE)
target_compile_options(GENERIC_F439ZGYX_hid INTERFACE
  "SHELL:-DSTM32F439xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGYX_hid_MCU}
)
target_compile_definitions(GENERIC_F439ZGYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZGYX"
	"BOARD_NAME=\"GENERIC_F439ZGYX\""
	"BOARD_ID=GENERIC_F439ZGYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZGYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZGYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZGYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZGYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZGYX_hid_MCU}
)
target_link_libraries(GENERIC_F439ZGYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F439ZITX
# -----------------------------------------------------------------------------

set(GENERIC_F439ZITX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZITX_MAXSIZE 2097152)
set(GENERIC_F439ZITX_MAXDATASIZE 196608)
set(GENERIC_F439ZITX_MCU cortex-m4)
set(GENERIC_F439ZITX_FPCONF "-")
add_library(GENERIC_F439ZITX INTERFACE)
target_compile_options(GENERIC_F439ZITX INTERFACE
  "SHELL:-DSTM32F439xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZITX_MCU}
)
target_compile_definitions(GENERIC_F439ZITX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZITX"
	"BOARD_NAME=\"GENERIC_F439ZITX\""
	"BOARD_ID=GENERIC_F439ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZITX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZITX_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZITX INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZITX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZITX_MCU}
)
target_link_libraries(GENERIC_F439ZITX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F439ZITX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F439ZITX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F439ZITX_serial_generic INTERFACE)
target_compile_options(GENERIC_F439ZITX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F439ZITX_serial_none INTERFACE)
target_compile_options(GENERIC_F439ZITX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F439ZITX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F439ZITX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZITX_hid_MAXSIZE 2097152)
set(GENERIC_F439ZITX_hid_MAXDATASIZE 196608)
set(GENERIC_F439ZITX_hid_MCU cortex-m4)
set(GENERIC_F439ZITX_hid_FPCONF "-")
add_library(GENERIC_F439ZITX_hid INTERFACE)
target_compile_options(GENERIC_F439ZITX_hid INTERFACE
  "SHELL:-DSTM32F439xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZITX_hid_MCU}
)
target_compile_definitions(GENERIC_F439ZITX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZITX"
	"BOARD_NAME=\"GENERIC_F439ZITX\""
	"BOARD_ID=GENERIC_F439ZITX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZITX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZITX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZITX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZITX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZITX_hid_MCU}
)
target_link_libraries(GENERIC_F439ZITX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F439ZIYX
# -----------------------------------------------------------------------------

set(GENERIC_F439ZIYX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZIYX_MAXSIZE 2097152)
set(GENERIC_F439ZIYX_MAXDATASIZE 196608)
set(GENERIC_F439ZIYX_MCU cortex-m4)
set(GENERIC_F439ZIYX_FPCONF "-")
add_library(GENERIC_F439ZIYX INTERFACE)
target_compile_options(GENERIC_F439ZIYX INTERFACE
  "SHELL:-DSTM32F439xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZIYX_MCU}
)
target_compile_definitions(GENERIC_F439ZIYX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZIYX"
	"BOARD_NAME=\"GENERIC_F439ZIYX\""
	"BOARD_ID=GENERIC_F439ZIYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZIYX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZIYX_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZIYX INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZIYX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZIYX_MCU}
)
target_link_libraries(GENERIC_F439ZIYX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F439ZIYX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F439ZIYX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F439ZIYX_serial_generic INTERFACE)
target_compile_options(GENERIC_F439ZIYX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F439ZIYX_serial_none INTERFACE)
target_compile_options(GENERIC_F439ZIYX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F439ZIYX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F439ZIYX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F427Z(G-I)T_F429ZET_F429Z(G-I)(T-Y)_F437Z(G-I)T_F439Z(G-I)(T-Y)")
set(GENERIC_F439ZIYX_hid_MAXSIZE 2097152)
set(GENERIC_F439ZIYX_hid_MAXDATASIZE 196608)
set(GENERIC_F439ZIYX_hid_MCU cortex-m4)
set(GENERIC_F439ZIYX_hid_FPCONF "-")
add_library(GENERIC_F439ZIYX_hid INTERFACE)
target_compile_options(GENERIC_F439ZIYX_hid INTERFACE
  "SHELL:-DSTM32F439xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZIYX_hid_MCU}
)
target_compile_definitions(GENERIC_F439ZIYX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F439ZIYX"
	"BOARD_NAME=\"GENERIC_F439ZIYX\""
	"BOARD_ID=GENERIC_F439ZIYX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F439ZIYX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F439ZIYX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F439ZIYX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F439ZIYX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=196608"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F439ZIYX_hid_MCU}
)
target_link_libraries(GENERIC_F439ZIYX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F446RCTX
# -----------------------------------------------------------------------------

set(GENERIC_F446RCTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446R(C-E)T")
set(GENERIC_F446RCTX_MAXSIZE 262144)
set(GENERIC_F446RCTX_MAXDATASIZE 131072)
set(GENERIC_F446RCTX_MCU cortex-m4)
set(GENERIC_F446RCTX_FPCONF "-")
add_library(GENERIC_F446RCTX INTERFACE)
target_compile_options(GENERIC_F446RCTX INTERFACE
  "SHELL:-DSTM32F446xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RCTX_MCU}
)
target_compile_definitions(GENERIC_F446RCTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446RCTX"
	"BOARD_NAME=\"GENERIC_F446RCTX\""
	"BOARD_ID=GENERIC_F446RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446RCTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446RCTX_VARIANT_PATH}
)

target_link_options(GENERIC_F446RCTX INTERFACE
  "LINKER:--default-script=${GENERIC_F446RCTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RCTX_MCU}
)
target_link_libraries(GENERIC_F446RCTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F446RCTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F446RCTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F446RCTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F446RCTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F446RCTX_serial_none INTERFACE)
target_compile_options(GENERIC_F446RCTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F446RCTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F446RCTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446R(C-E)T")
set(GENERIC_F446RCTX_hid_MAXSIZE 262144)
set(GENERIC_F446RCTX_hid_MAXDATASIZE 131072)
set(GENERIC_F446RCTX_hid_MCU cortex-m4)
set(GENERIC_F446RCTX_hid_FPCONF "-")
add_library(GENERIC_F446RCTX_hid INTERFACE)
target_compile_options(GENERIC_F446RCTX_hid INTERFACE
  "SHELL:-DSTM32F446xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RCTX_hid_MCU}
)
target_compile_definitions(GENERIC_F446RCTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446RCTX"
	"BOARD_NAME=\"GENERIC_F446RCTX\""
	"BOARD_ID=GENERIC_F446RCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446RCTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446RCTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F446RCTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F446RCTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RCTX_hid_MCU}
)
target_link_libraries(GENERIC_F446RCTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F446RETX
# -----------------------------------------------------------------------------

set(GENERIC_F446RETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446R(C-E)T")
set(GENERIC_F446RETX_MAXSIZE 524288)
set(GENERIC_F446RETX_MAXDATASIZE 131072)
set(GENERIC_F446RETX_MCU cortex-m4)
set(GENERIC_F446RETX_FPCONF "-")
add_library(GENERIC_F446RETX INTERFACE)
target_compile_options(GENERIC_F446RETX INTERFACE
  "SHELL:-DSTM32F446xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RETX_MCU}
)
target_compile_definitions(GENERIC_F446RETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446RETX"
	"BOARD_NAME=\"GENERIC_F446RETX\""
	"BOARD_ID=GENERIC_F446RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446RETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446RETX_VARIANT_PATH}
)

target_link_options(GENERIC_F446RETX INTERFACE
  "LINKER:--default-script=${GENERIC_F446RETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RETX_MCU}
)
target_link_libraries(GENERIC_F446RETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F446RETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F446RETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F446RETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F446RETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F446RETX_serial_none INTERFACE)
target_compile_options(GENERIC_F446RETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F446RETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F446RETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446R(C-E)T")
set(GENERIC_F446RETX_hid_MAXSIZE 524288)
set(GENERIC_F446RETX_hid_MAXDATASIZE 131072)
set(GENERIC_F446RETX_hid_MCU cortex-m4)
set(GENERIC_F446RETX_hid_FPCONF "-")
add_library(GENERIC_F446RETX_hid INTERFACE)
target_compile_options(GENERIC_F446RETX_hid INTERFACE
  "SHELL:-DSTM32F446xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RETX_hid_MCU}
)
target_compile_definitions(GENERIC_F446RETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446RETX"
	"BOARD_NAME=\"GENERIC_F446RETX\""
	"BOARD_ID=GENERIC_F446RETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446RETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446RETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F446RETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F446RETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446RETX_hid_MCU}
)
target_link_libraries(GENERIC_F446RETX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F446VCTX
# -----------------------------------------------------------------------------

set(GENERIC_F446VCTX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446V(C-E)T")
set(GENERIC_F446VCTX_MAXSIZE 262144)
set(GENERIC_F446VCTX_MAXDATASIZE 131072)
set(GENERIC_F446VCTX_MCU cortex-m4)
set(GENERIC_F446VCTX_FPCONF "-")
add_library(GENERIC_F446VCTX INTERFACE)
target_compile_options(GENERIC_F446VCTX INTERFACE
  "SHELL:-DSTM32F446xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VCTX_MCU}
)
target_compile_definitions(GENERIC_F446VCTX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446VCTX"
	"BOARD_NAME=\"GENERIC_F446VCTX\""
	"BOARD_ID=GENERIC_F446VCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446VCTX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446VCTX_VARIANT_PATH}
)

target_link_options(GENERIC_F446VCTX INTERFACE
  "LINKER:--default-script=${GENERIC_F446VCTX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VCTX_MCU}
)
target_link_libraries(GENERIC_F446VCTX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F446VCTX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F446VCTX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F446VCTX_serial_generic INTERFACE)
target_compile_options(GENERIC_F446VCTX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F446VCTX_serial_none INTERFACE)
target_compile_options(GENERIC_F446VCTX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F446VCTX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F446VCTX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446V(C-E)T")
set(GENERIC_F446VCTX_hid_MAXSIZE 262144)
set(GENERIC_F446VCTX_hid_MAXDATASIZE 131072)
set(GENERIC_F446VCTX_hid_MCU cortex-m4)
set(GENERIC_F446VCTX_hid_FPCONF "-")
add_library(GENERIC_F446VCTX_hid INTERFACE)
target_compile_options(GENERIC_F446VCTX_hid INTERFACE
  "SHELL:-DSTM32F446xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VCTX_hid_MCU}
)
target_compile_definitions(GENERIC_F446VCTX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446VCTX"
	"BOARD_NAME=\"GENERIC_F446VCTX\""
	"BOARD_ID=GENERIC_F446VCTX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446VCTX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446VCTX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F446VCTX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F446VCTX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=262144"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VCTX_hid_MCU}
)
target_link_libraries(GENERIC_F446VCTX_hid INTERFACE
  arm_cortexM4lf_math
)


# GENERIC_F446VETX
# -----------------------------------------------------------------------------

set(GENERIC_F446VETX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446V(C-E)T")
set(GENERIC_F446VETX_MAXSIZE 524288)
set(GENERIC_F446VETX_MAXDATASIZE 131072)
set(GENERIC_F446VETX_MCU cortex-m4)
set(GENERIC_F446VETX_FPCONF "-")
add_library(GENERIC_F446VETX INTERFACE)
target_compile_options(GENERIC_F446VETX INTERFACE
  "SHELL:-DSTM32F446xx   "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VETX_MCU}
)
target_compile_definitions(GENERIC_F446VETX INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446VETX"
	"BOARD_NAME=\"GENERIC_F446VETX\""
	"BOARD_ID=GENERIC_F446VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446VETX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446VETX_VARIANT_PATH}
)

target_link_options(GENERIC_F446VETX INTERFACE
  "LINKER:--default-script=${GENERIC_F446VETX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VETX_MCU}
)
target_link_libraries(GENERIC_F446VETX INTERFACE
  arm_cortexM4lf_math
)

add_library(GENERIC_F446VETX_serial_disabled INTERFACE)
target_compile_options(GENERIC_F446VETX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(GENERIC_F446VETX_serial_generic INTERFACE)
target_compile_options(GENERIC_F446VETX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(GENERIC_F446VETX_serial_none INTERFACE)
target_compile_options(GENERIC_F446VETX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# GENERIC_F446VETX_hid
# -----------------------------------------------------------------------------

set(GENERIC_F446VETX_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446V(C-E)T")
set(GENERIC_F446VETX_hid_MAXSIZE 524288)
set(GENERIC_F446VETX_hid_MAXDATASIZE 131072)
set(GENERIC_F446VETX_hid_MCU cortex-m4)
set(GENERIC_F446VETX_hid_FPCONF "-")
add_library(GENERIC_F446VETX_hid INTERFACE)
target_compile_options(GENERIC_F446VETX_hid INTERFACE
  "SHELL:-DSTM32F446xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VETX_hid_MCU}
)
target_compile_definitions(GENERIC_F446VETX_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_GENERIC_F446VETX"
	"BOARD_NAME=\"GENERIC_F446VETX\""
	"BOARD_ID=GENERIC_F446VETX"
	"VARIANT_H=\"variant_generic.h\""
)
target_include_directories(GENERIC_F446VETX_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${GENERIC_F446VETX_hid_VARIANT_PATH}
)

target_link_options(GENERIC_F446VETX_hid INTERFACE
  "LINKER:--default-script=${GENERIC_F446VETX_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${GENERIC_F446VETX_hid_MCU}
)
target_link_libraries(GENERIC_F446VETX_hid INTERFACE
  arm_cortexM4lf_math
)


# NUCLEO_F401RE
# -----------------------------------------------------------------------------

set(NUCLEO_F401RE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F401R(B-C-D-E)T")
set(NUCLEO_F401RE_MAXSIZE 524288)
set(NUCLEO_F401RE_MAXDATASIZE 98304)
set(NUCLEO_F401RE_MCU cortex-m4)
set(NUCLEO_F401RE_FPCONF "fpv4-sp-d16-hard")
add_library(NUCLEO_F401RE INTERFACE)
target_compile_options(NUCLEO_F401RE INTERFACE
  "SHELL:-DSTM32F401xE  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F401RE_MCU}
)
target_compile_definitions(NUCLEO_F401RE INTERFACE
  "STM32F4xx"
	"ARDUINO_NUCLEO_F401RE"
	"BOARD_NAME=\"NUCLEO_F401RE\""
	"BOARD_ID=NUCLEO_F401RE"
	"VARIANT_H=\"variant_NUCLEO_F401RE.h\""
)
target_include_directories(NUCLEO_F401RE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${NUCLEO_F401RE_VARIANT_PATH}
)

target_link_options(NUCLEO_F401RE INTERFACE
  "LINKER:--default-script=${NUCLEO_F401RE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=98304"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F401RE_MCU}
)
target_link_libraries(NUCLEO_F401RE INTERFACE
  arm_cortexM4lf_math
)

add_library(NUCLEO_F401RE_serial_disabled INTERFACE)
target_compile_options(NUCLEO_F401RE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F401RE_serial_generic INTERFACE)
target_compile_options(NUCLEO_F401RE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(NUCLEO_F401RE_serial_none INTERFACE)
target_compile_options(NUCLEO_F401RE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(NUCLEO_F401RE_usb_CDC INTERFACE)
target_compile_options(NUCLEO_F401RE_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(NUCLEO_F401RE_usb_CDCgen INTERFACE)
target_compile_options(NUCLEO_F401RE_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(NUCLEO_F401RE_usb_HID INTERFACE)
target_compile_options(NUCLEO_F401RE_usb_HID INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE"
)
add_library(NUCLEO_F401RE_usb_none INTERFACE)
target_compile_options(NUCLEO_F401RE_usb_none INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F401RE_xusb_FS INTERFACE)
target_compile_options(NUCLEO_F401RE_xusb_FS INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F401RE_xusb_HS INTERFACE)
target_compile_options(NUCLEO_F401RE_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(NUCLEO_F401RE_xusb_HSFS INTERFACE)
target_compile_options(NUCLEO_F401RE_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
)

# NUCLEO_F411RE
# -----------------------------------------------------------------------------

set(NUCLEO_F411RE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F411R(C-E)T")
set(NUCLEO_F411RE_MAXSIZE 524288)
set(NUCLEO_F411RE_MAXDATASIZE 131072)
set(NUCLEO_F411RE_MCU cortex-m4)
set(NUCLEO_F411RE_FPCONF "fpv4-sp-d16-hard")
add_library(NUCLEO_F411RE INTERFACE)
target_compile_options(NUCLEO_F411RE INTERFACE
  "SHELL:-DSTM32F411xE  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F411RE_MCU}
)
target_compile_definitions(NUCLEO_F411RE INTERFACE
  "STM32F4xx"
	"ARDUINO_NUCLEO_F411RE"
	"BOARD_NAME=\"NUCLEO_F411RE\""
	"BOARD_ID=NUCLEO_F411RE"
	"VARIANT_H=\"variant_NUCLEO_F411RE.h\""
)
target_include_directories(NUCLEO_F411RE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${NUCLEO_F411RE_VARIANT_PATH}
)

target_link_options(NUCLEO_F411RE INTERFACE
  "LINKER:--default-script=${NUCLEO_F411RE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F411RE_MCU}
)
target_link_libraries(NUCLEO_F411RE INTERFACE
  arm_cortexM4lf_math
)

add_library(NUCLEO_F411RE_serial_disabled INTERFACE)
target_compile_options(NUCLEO_F411RE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F411RE_serial_generic INTERFACE)
target_compile_options(NUCLEO_F411RE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(NUCLEO_F411RE_serial_none INTERFACE)
target_compile_options(NUCLEO_F411RE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(NUCLEO_F411RE_usb_CDC INTERFACE)
target_compile_options(NUCLEO_F411RE_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(NUCLEO_F411RE_usb_CDCgen INTERFACE)
target_compile_options(NUCLEO_F411RE_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(NUCLEO_F411RE_usb_HID INTERFACE)
target_compile_options(NUCLEO_F411RE_usb_HID INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE"
)
add_library(NUCLEO_F411RE_usb_none INTERFACE)
target_compile_options(NUCLEO_F411RE_usb_none INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F411RE_xusb_FS INTERFACE)
target_compile_options(NUCLEO_F411RE_xusb_FS INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F411RE_xusb_HS INTERFACE)
target_compile_options(NUCLEO_F411RE_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(NUCLEO_F411RE_xusb_HSFS INTERFACE)
target_compile_options(NUCLEO_F411RE_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
)

# NUCLEO_F446RE
# -----------------------------------------------------------------------------

set(NUCLEO_F446RE_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F446R(C-E)T")
set(NUCLEO_F446RE_MAXSIZE 524288)
set(NUCLEO_F446RE_MAXDATASIZE 131072)
set(NUCLEO_F446RE_MCU cortex-m4)
set(NUCLEO_F446RE_FPCONF "fpv4-sp-d16-hard")
add_library(NUCLEO_F446RE INTERFACE)
target_compile_options(NUCLEO_F446RE INTERFACE
  "SHELL:-DSTM32F446xx  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F446RE_MCU}
)
target_compile_definitions(NUCLEO_F446RE INTERFACE
  "STM32F4xx"
	"ARDUINO_NUCLEO_F446RE"
	"BOARD_NAME=\"NUCLEO_F446RE\""
	"BOARD_ID=NUCLEO_F446RE"
	"VARIANT_H=\"variant_NUCLEO_F446RE.h\""
)
target_include_directories(NUCLEO_F446RE INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${NUCLEO_F446RE_VARIANT_PATH}
)

target_link_options(NUCLEO_F446RE INTERFACE
  "LINKER:--default-script=${NUCLEO_F446RE_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=524288"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${NUCLEO_F446RE_MCU}
)
target_link_libraries(NUCLEO_F446RE INTERFACE
  arm_cortexM4lf_math
)

add_library(NUCLEO_F446RE_serial_disabled INTERFACE)
target_compile_options(NUCLEO_F446RE_serial_disabled INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F446RE_serial_generic INTERFACE)
target_compile_options(NUCLEO_F446RE_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(NUCLEO_F446RE_serial_none INTERFACE)
target_compile_options(NUCLEO_F446RE_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(NUCLEO_F446RE_usb_CDC INTERFACE)
target_compile_options(NUCLEO_F446RE_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(NUCLEO_F446RE_usb_CDCgen INTERFACE)
target_compile_options(NUCLEO_F446RE_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(NUCLEO_F446RE_usb_HID INTERFACE)
target_compile_options(NUCLEO_F446RE_usb_HID INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE"
)
add_library(NUCLEO_F446RE_usb_none INTERFACE)
target_compile_options(NUCLEO_F446RE_usb_none INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F446RE_xusb_FS INTERFACE)
target_compile_options(NUCLEO_F446RE_xusb_FS INTERFACE
  "SHELL:"
)
add_library(NUCLEO_F446RE_xusb_HS INTERFACE)
target_compile_options(NUCLEO_F446RE_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(NUCLEO_F446RE_xusb_HSFS INTERFACE)
target_compile_options(NUCLEO_F446RE_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
)

# VCCGND_F407ZG_MINI
# -----------------------------------------------------------------------------

set(VCCGND_F407ZG_MINI_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(VCCGND_F407ZG_MINI_MAXSIZE 1048576)
set(VCCGND_F407ZG_MINI_MAXDATASIZE 131072)
set(VCCGND_F407ZG_MINI_MCU cortex-m4)
set(VCCGND_F407ZG_MINI_FPCONF "-")
add_library(VCCGND_F407ZG_MINI INTERFACE)
target_compile_options(VCCGND_F407ZG_MINI INTERFACE
  "SHELL:-DSTM32F407xx   "
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${VCCGND_F407ZG_MINI_MCU}
)
target_compile_definitions(VCCGND_F407ZG_MINI INTERFACE
  "STM32F4xx"
	"ARDUINO_VCCGND_F407ZG_MINI"
	"BOARD_NAME=\"VCCGND_F407ZG_MINI\""
	"BOARD_ID=VCCGND_F407ZG_MINI"
	"VARIANT_H=\"variant_VCCGND_F407ZG_MINI.h\""
)
target_include_directories(VCCGND_F407ZG_MINI INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${VCCGND_F407ZG_MINI_VARIANT_PATH}
)

target_link_options(VCCGND_F407ZG_MINI INTERFACE
  "LINKER:--default-script=${VCCGND_F407ZG_MINI_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${VCCGND_F407ZG_MINI_MCU}
)
target_link_libraries(VCCGND_F407ZG_MINI INTERFACE
  arm_cortexM4lf_math
)

add_library(VCCGND_F407ZG_MINI_serial_disabled INTERFACE)
target_compile_options(VCCGND_F407ZG_MINI_serial_disabled INTERFACE
  "SHELL:"
)
add_library(VCCGND_F407ZG_MINI_serial_generic INTERFACE)
target_compile_options(VCCGND_F407ZG_MINI_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(VCCGND_F407ZG_MINI_serial_none INTERFACE)
target_compile_options(VCCGND_F407ZG_MINI_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)

# VCCGND_F407ZG_MINI_hid
# -----------------------------------------------------------------------------

set(VCCGND_F407ZG_MINI_hid_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/F407Z(E-G)T_F417Z(E-G)T")
set(VCCGND_F407ZG_MINI_hid_MAXSIZE 1048576)
set(VCCGND_F407ZG_MINI_hid_MAXDATASIZE 131072)
set(VCCGND_F407ZG_MINI_hid_MCU cortex-m4)
set(VCCGND_F407ZG_MINI_hid_FPCONF "-")
add_library(VCCGND_F407ZG_MINI_hid INTERFACE)
target_compile_options(VCCGND_F407ZG_MINI_hid INTERFACE
  "SHELL:-DSTM32F407xx  -DHAL_UART_MODULE_ENABLED -DBL_HID -DVECT_TAB_OFFSET=0x4000"
  "SHELL:-DCUSTOM_PERIPHERAL_PINS"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${VCCGND_F407ZG_MINI_hid_MCU}
)
target_compile_definitions(VCCGND_F407ZG_MINI_hid INTERFACE
  "STM32F4xx"
	"ARDUINO_VCCGND_F407ZG_MINI"
	"BOARD_NAME=\"VCCGND_F407ZG_MINI\""
	"BOARD_ID=VCCGND_F407ZG_MINI"
	"VARIANT_H=\"variant_VCCGND_F407ZG_MINI.h\""
)
target_include_directories(VCCGND_F407ZG_MINI_hid INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${VCCGND_F407ZG_MINI_hid_VARIANT_PATH}
)

target_link_options(VCCGND_F407ZG_MINI_hid INTERFACE
  "LINKER:--default-script=${VCCGND_F407ZG_MINI_hid_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0x4000"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${VCCGND_F407ZG_MINI_hid_MCU}
)
target_link_libraries(VCCGND_F407ZG_MINI_hid INTERFACE
  arm_cortexM4lf_math
)


