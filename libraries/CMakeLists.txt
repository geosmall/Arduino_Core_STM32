# just a trampoline to avoid overwhelming the main CMakeLists.txt

# Core libraries
add_subdirectory(CMSIS_DSP)
add_subdirectory(SEGGER_RTT)
add_subdirectory(SoftwareSerial)
add_subdirectory(SPI)
add_subdirectory(Wire)

# STM32-specific libraries
add_subdirectory(STM32RTC)

# Robotics libraries
add_subdirectory(AUnit-1.7.1)
add_subdirectory(DShot)
add_subdirectory(EmbeddedCLI)
add_subdirectory(imu)
add_subdirectory(libPrintf)
add_subdirectory(LittleFS)
add_subdirectory(minIniStorage)
add_subdirectory(ms4525do)
add_subdirectory(PrecompLib)
add_subdirectory(Scheduler)
add_subdirectory(SDFS)
add_subdirectory(SerialRx)
add_subdirectory(Storage)
add_subdirectory(TimerPWM)
add_subdirectory(TinyGPSPlus)
add_subdirectory(xensiv-dps3xx)
