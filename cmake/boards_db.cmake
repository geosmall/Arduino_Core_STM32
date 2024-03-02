# BLACKPILL_F401CC
# -----------------------------------------------------------------------------

set(BLACKPILL_F401CC_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32F4xx/Dev_Blackpill_F401CC")
set(BLACKPILL_F401CC_MAXSIZE 262144)
set(BLACKPILL_F401CC_MAXDATASIZE 65536)
set(BLACKPILL_F401CC_MCU cortex-m4)
set(BLACKPILL_F401CC_FPCONF "fpv4-sp-d16-hard")
add_library(BLACKPILL_F401CC INTERFACE)
target_compile_options(BLACKPILL_F401CC INTERFACE
  "SHELL:-DSTM32F401xC  "
  "SHELL:"
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
add_library(BLACKPILL_F401CC_usb_CDC INTERFACE)
target_compile_options(BLACKPILL_F401CC_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(BLACKPILL_F401CC_usb_CDCgen INTERFACE)
target_compile_options(BLACKPILL_F401CC_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(BLACKPILL_F401CC_usb_none INTERFACE)
target_compile_options(BLACKPILL_F401CC_usb_none INTERFACE
  "SHELL:"
)
add_library(BLACKPILL_F401CC_xusb_FS INTERFACE)
target_compile_options(BLACKPILL_F401CC_xusb_FS INTERFACE
  "SHELL:"
)
add_library(BLACKPILL_F401CC_xusb_HS INTERFACE)
target_compile_options(BLACKPILL_F401CC_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(BLACKPILL_F401CC_xusb_HSFS INTERFACE)
target_compile_options(BLACKPILL_F401CC_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
)

# DevEBoxH743VITX
# -----------------------------------------------------------------------------

set(DevEBoxH743VITX_VARIANT_PATH "${CMAKE_CURRENT_LIST_DIR}/../variants/STM32H7xx/Dev_DevEBoxH7xx")
set(DevEBoxH743VITX_MAXSIZE 2097152)
set(DevEBoxH743VITX_MAXDATASIZE 524288)
set(DevEBoxH743VITX_MCU cortex-m7)
set(DevEBoxH743VITX_FPCONF "fpv4-sp-d16-hard")
add_library(DevEBoxH743VITX INTERFACE)
target_compile_options(DevEBoxH743VITX INTERFACE
  "SHELL:-DCORE_CM7 -DSTM32H743xx  "
  "SHELL:"
  "SHELL:"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DevEBoxH743VITX_MCU}
)
target_compile_definitions(DevEBoxH743VITX INTERFACE
  "STM32H7xx"
	"ARDUINO_DevEBoxH743VITX"
	"BOARD_NAME=\"DevEBoxH743VITX\""
	"BOARD_ID=DevEBoxH743VITX"
	"VARIANT_H=\"variant_DevEBoxH7xx.h\""
)
target_include_directories(DevEBoxH743VITX INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/../system/STM32H7xx
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32H7xx_HAL_Driver/Inc
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/STM32H7xx_HAL_Driver/Src
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32H7xx/Include/
  ${CMAKE_CURRENT_LIST_DIR}/../system/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/
  ${DevEBoxH743VITX_VARIANT_PATH}
)

target_link_options(DevEBoxH743VITX INTERFACE
  "LINKER:--default-script=${DevEBoxH743VITX_VARIANT_PATH}/ldscript.ld"
  "LINKER:--defsym=LD_FLASH_OFFSET=0"
	"LINKER:--defsym=LD_MAX_SIZE=2097152"
	"LINKER:--defsym=LD_MAX_DATA_SIZE=524288"
  "SHELL:-mfpu=fpv4-sp-d16 -mfloat-abi=hard"
  -mcpu=${DevEBoxH743VITX_MCU}
)
target_link_libraries(DevEBoxH743VITX INTERFACE
  arm_cortexM7lfsp_math
)

add_library(DevEBoxH743VITX_serial_disabled INTERFACE)
target_compile_options(DevEBoxH743VITX_serial_disabled INTERFACE
  "SHELL:"
)
add_library(DevEBoxH743VITX_serial_generic INTERFACE)
target_compile_options(DevEBoxH743VITX_serial_generic INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED"
)
add_library(DevEBoxH743VITX_serial_none INTERFACE)
target_compile_options(DevEBoxH743VITX_serial_none INTERFACE
  "SHELL:-DHAL_UART_MODULE_ENABLED -DHWSERIAL_NONE"
)
add_library(DevEBoxH743VITX_usb_CDC INTERFACE)
target_compile_options(DevEBoxH743VITX_usb_CDC INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC -DDISABLE_GENERIC_SERIALUSB"
)
add_library(DevEBoxH743VITX_usb_CDCgen INTERFACE)
target_compile_options(DevEBoxH743VITX_usb_CDCgen INTERFACE
  "SHELL:-DUSBCON  -DUSBD_VID=0 -DUSBD_PID=0 -DHAL_PCD_MODULE_ENABLED -DUSBD_USE_CDC"
)
add_library(DevEBoxH743VITX_usb_none INTERFACE)
target_compile_options(DevEBoxH743VITX_usb_none INTERFACE
  "SHELL:"
)
add_library(DevEBoxH743VITX_xusb_FS INTERFACE)
target_compile_options(DevEBoxH743VITX_xusb_FS INTERFACE
  "SHELL:"
)
add_library(DevEBoxH743VITX_xusb_HS INTERFACE)
target_compile_options(DevEBoxH743VITX_xusb_HS INTERFACE
  "SHELL:-DUSE_USB_HS"
)
add_library(DevEBoxH743VITX_xusb_HSFS INTERFACE)
target_compile_options(DevEBoxH743VITX_xusb_HSFS INTERFACE
  "SHELL:-DUSE_USB_HS -DUSE_USB_HS_IN_FS"
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

