INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME patmos)
SET(CMAKE_SYSTEM_PROCESSOR patmos)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find clang for cross compiling
find_program(CLANG_EXECUTABLE NAMES clang DOC "Path to the clang front-end.")

if(NOT CLANG_EXECUTABLE)
  message(FATAL_ERROR "clang required for a Patmos build.")
endif()

CMAKE_FORCE_C_COMPILER(${CLANG_EXECUTABLE} GNU)
CMAKE_FORCE_CXX_COMPILER(${CLANG_EXECUTABLE} GNU)

# the clang triple, also used for installation
set(TRIPLE patmos-unknown-elf)

# set some compiler-related variables;
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> -ccc-host-triple ${TRIPLE} -fno-builtin -emit-llvm <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> -ccc-host-triple ${TRIPLE} -fno-builtin <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> -ccc-host-triple ${TRIPLE} -emit-llvm -fno-builtin <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_FORCE_C_OUTPUT_EXTENSION ".bc" FORCE)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-config
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config DOC "Path to the llvm-config tool.")

if(NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "LLVM required for a Patmos build.")
endif()

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --targets-built
                OUTPUT_VARIABLE LLVM_TARGETS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT (${LLVM_TARGETS} MATCHES "Patmos"))
  message(FATAL_ERROR "LLVM target 'patmos' required for a Patmos build.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-ar
find_program(LLVM_AR_EXECUTABLE NAMES llvm-ar DOC "Path to the llvm-ar tool.")

if(NOT LLVM_AR_EXECUTABLE)
  message(FATAL_ERROR "llvm-ar required for a Patmos build.")
endif()

set(CMAKE_FORCE_AR ${LLVM_AR_EXECUTABLE})

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-as
find_program(LLVM_AS_EXECUTABLE NAMES llvm-as DOC "Path to the llvm-as tool.")

if(NOT LLVM_AS_EXECUTABLE)
  message(FATAL_ERROR "llvm-as required for a Patmos build.")
endif()

set(CMAKE_FORCE_AS ${LLVM_AS_EXECUTABLE})

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-ranlib
find_program(LLVM_RANLIB_EXECUTABLE NAMES llvm-ranlib DOC "Path to the llvm-ranlib tool.")

if(NOT LLVM_RANLIB_EXECUTABLE)
  message(FATAL_ERROR "llvm-ranlib required for a Patmos build.")
endif()

set(CMAKE_FORCE_RANLIB ${LLVM_RANLIB_EXECUTABLE})

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-ld
find_program(LLVM_LD_EXECUTABLE NAMES llvm-ld DOC "Path to the llvm-ld tool.")

if(NOT LLVM_LD_EXECUTABLE)
  message(FATAL_ERROR "llvm-ld required for a Patmos build.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find simulator

find_program(PASIM_EXECUTABLE NAMES pasim DOC "Path to the Patmos simulator pasim.")

if(PASIM_EXECUTABLE)
  set(ENABLE_TESTING true)
  macro (run_io name prog in out ref)
    add_test(NAME ${name} COMMAND ${PASIM_EXECUTABLE} ${prog} -o ${name}.stats -I ${in} -O ${out})
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${out} ${name}.stats)

    add_test(NAME ${name}-cmp COMMAND ${CMAKE_COMMAND} -E compare_files ${out} ${ref})
    set_tests_properties(${name}-cmp PROPERTIES DEPENDS ${name})
  endmacro (run_io)
else()
  message(FATAL_ERROR "pasim required for a Patmos build.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
