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

# FC_REVO_F405RG
# -----------------------------------------------------------------------------

set(FC_REVO_F405RG_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/FC_Revo_F405RG")
set(FC_REVO_F405RG_MAXSIZE 1048576)
set(FC_REVO_F405RG_MAXDATASIZE 131072)
set(FC_REVO_F405RG_MCU cortex-m4)
set(FC_REVO_F405RG_FPCONF "fpv4-sp-d16-hard")
add_library(FC_REVO_F405RG INTERFACE)
target_compile_options(FC_REVO_F405RG INTERFACE
  "SHELL:-DSTM32F405xx  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FC_REVO_F405RG_MCU}
)
target_compile_definitions(FC_REVO_F405RG INTERFACE
  "STM32F4xx"
	"ARDUINO_FC_REVO_F405RG"
	"BOARD_NAME=\"FC_REVO_F405RG\""
	"BOARD_ID=FC_REVO_F405RG"
	"VARIANT_H=\"variant_FC_REVO_F405RG.h\""
)
target_include_directories(FC_REVO_F405RG INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32F4xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32F4xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/
  ${FC_REVO_F405RG_VARIANT_PATH}
)

target_link_options(FC_REVO_F405RG INTERFACE
  "LINKER:--default-script=${FC_REVO_F405RG_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=1048576"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=131072"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${FC_REVO_F405RG_MCU}
)
target_link_libraries(FC_REVO_F405RG INTERFACE
  arm_cortexM4lf_math
)

add_library(FC_REVO_F405RG_serial_disabled INTERFACE)
target_compile_options(FC_REVO_F405RG_serial_disabled INTERFACE
  "SHELL:"
)
add_library(FC_REVO_F405RG_serial_generic INTERFACE)
target_compile_options(FC_REVO_F405RG_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(FC_REVO_F405RG_serial_none INTERFACE)
target_compile_options(FC_REVO_F405RG_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(FC_REVO_F405RG_usb_CDC INTERFACE)
target_compile_options(FC_REVO_F405RG_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(FC_REVO_F405RG_usb_CDCgen INTERFACE)
target_compile_options(FC_REVO_F405RG_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(FC_REVO_F405RG_usb_HID INTERFACE)
target_compile_options(FC_REVO_F405RG_usb_HID INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_HID_COMPOSITE"
)
add_library(FC_REVO_F405RG_usb_none INTERFACE)
target_compile_options(FC_REVO_F405RG_usb_none INTERFACE
  "SHELL:"
)
add_library(FC_REVO_F405RG_xusb_FS INTERFACE)
target_compile_options(FC_REVO_F405RG_xusb_FS INTERFACE
  "SHELL:"
)
add_library(FC_REVO_F405RG_xusb_HS INTERFACE)
target_compile_options(FC_REVO_F405RG_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(FC_REVO_F405RG_xusb_HSFS INTERFACE)
target_compile_options(FC_REVO_F405RG_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
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

