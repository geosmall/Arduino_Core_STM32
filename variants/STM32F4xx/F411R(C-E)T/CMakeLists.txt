# v3.21 implemented semantic changes regarding $<TARGET_OBJECTS:...>
# See https://cmake.org/cmake/help/v3.21/command/target_link_libraries.html#linking-object-libraries-via-target-objects
cmake_minimum_required(VERSION 3.21)

# Declare another INTERFACE library named 'variant_usage'.
add_library(variant_usage INTERFACE)

# Specify that when other targets link to 'variant_usage', they should add
# the current directory (.) to their list of include directories.
target_include_directories(variant_usage INTERFACE
  .
)

# Specify that when other targets link to 'variant_usage', they should also
# link to 'base_config'.
target_link_libraries(variant_usage INTERFACE
  base_config
)

# Declare an OBJECT library named 'variant_bin' that is made from the source
# file 'variant_NUCLEO_F411RE.cpp'. An OBJECT library is like a STATIC library,
# but it doesn't archive the object files into a single library file. Instead,
# the object files are compiled and linked into any targets that depend on
# the object library. The 'EXCLUDE_FROM_ALL' argument means that this library
# won't be built unless another target depends on it.
add_library(variant_bin OBJECT EXCLUDE_FROM_ALL
  variant_NUCLEO_F411RE.cpp
)

# Specify that when other targets link to 'variant_bin', they should also
# link to 'variant_usage'. The 'PUBLIC' keyword means that the 'variant_usage'
# library will be linked to 'variant_bin' and any targets that link to
# 'variant_bin'.  ('PRIVATE' would mean that 'variant_usage' would only be
# linked to 'variant_bin')
target_link_libraries(variant_bin PUBLIC variant_usage)
