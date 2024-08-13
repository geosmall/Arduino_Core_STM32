:: Batch archive: lib_h7.bat
@echo off

:: set a variable AR to C:\Users\geoma\AppData\Local\Arduino15\packages\STMicroelectronics\tools\xpack-arm-none-eabi-gcc\10.3.1-2.3/bin/arm-none-eabi-gcc-ar
set AR=C:\Users\geoma\AppData\Local\Arduino15\packages\STMicroelectronics\tools\xpack-arm-none-eabi-gcc\10.3.1-2.3/bin/arm-none-eabi-gcc-ar

set TEMP_PATH=C:\Users\geoma\AppData\Local\Temp

:: Use TEMP_BUILD_FOLDER if is provided in command line, else use default
if "%1" == "" (
    set TEMP_BUILD_FOLDER=arduino_build_95603
) else (
    set TEMP_BUILD_FOLDER=%1
)

:: Combine the temporary path and the temporary build folder
set TEMP_BUILD_PATH=%TEMP_PATH%\%TEMP_BUILD_FOLDER%

:: if the archive file lib_h7.a exists, delete it
if exist lib_h7.a (
    del lib_h7.a
)

@echo on

:: Use variable AR to create the archive file lib_h7.a
%AR% rcs lib_h7.a %TEMP_BUILD_PATH%\core\stm32\HAL\*.o
%AR% rcs lib_h7.a %TEMP_BUILD_PATH%\core\stm32\LL\*.o