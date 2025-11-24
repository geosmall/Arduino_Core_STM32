# 3. Library Test

This example demonstrates how to link Arduino libraries to your sketch using the CMake build system.

## Library Dependencies

The key difference from the previous examples is the `DEPENDS` clause in `build_sketch()`:

```cmake
include(build_sketch)
build_sketch(TARGET "3_Lib_Test"
  SOURCES
  3_Lib_Test.ino

  # Link the SPI library from libraries/ folder
  DEPENDS
  SPI
)
```

The `DEPENDS` keyword accepts a list of library names. These can be:
- Libraries from the `libraries/` folder of Arduino_Core_STM32 (e.g., SPI, Wire, SoftwareSerial)
- User libraries declared with `external_library()`

When you specify a library dependency, CMake will:
1. Find the library in the appropriate location
2. Compile the library sources
3. Link the library to your sketch
4. Make the library's include paths available to your sketch

## Building

```sh
cmake -G Ninja -S cmake/examples/3_Lib_Test -B cmake/examples/3_Lib_Test/build
cmake --build cmake/examples/3_Lib_Test/build
```

The default board is NUCLEO_F411RE. After building, you will find "3_Lib_Test", "3_Lib_Test.bin", "3_Lib_Test.hex" in the build directory.

## Adding More Libraries

To add additional libraries, simply extend the DEPENDS list:

```cmake
build_sketch(TARGET "3_Lib_Test"
  SOURCES
  3_Lib_Test.ino

  DEPENDS
  SPI
  Wire
  SoftwareSerial
)
```

## External Libraries

For libraries outside the core's `libraries/` folder, use `external_library()` first:

```cmake
include(external_library)
external_library(
  NAME MyLibrary
  PATH ${USER_LIBS}/MyLibrary
)

build_sketch(TARGET "MySketch"
  SOURCES
  MySketch.ino

  DEPENDS
  MyLibrary
  SPI
)
```
