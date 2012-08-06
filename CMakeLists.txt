project(patmos-compiler-rt)

cmake_minimum_required(VERSION 2.8)

# we are only using C at the moment
enable_language(C)


# global configuration options
if("${CMAKE_SYSTEM_NAME}" MATCHES "patmos")
  set(IS_BIGENDIAN true)
endif()

set(LIB_SUPPORT   llspt)
set(LIB_SOFTFLOAT llsflt)

add_subdirectory(lib)

