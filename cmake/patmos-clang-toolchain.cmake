INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME patmos)
SET(CMAKE_SYSTEM_PROCESSOR patmos)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find clang for cross compiling
find_program(CLANG_EXECUTABLE NAMES patmos-clang clang DOC "Path to the clang front-end.")

if(NOT CLANG_EXECUTABLE)
  message(FATAL_ERROR "clang required for a Patmos build.")
endif()

# read the env var for patmos gold
if(NOT PATMOS_GOLD)
    set(PATMOS_GOLD $ENV{PATMOS_GOLD})
endif()
find_program(PATMOS_GOLD NAMES patmos-gold patmos-ld DOC "Path to the Patmos ELF linker.")

if( PATMOS_GOLD )
  set( PATMOS_GOLD_ENV "/usr/bin/env PATMOS_GOLD=${PATMOS_GOLD} " )
  #set( ENV{PATMOS_GOLD} ${PATMOS_GOLD_BIN} )
endif( PATMOS_GOLD )


CMAKE_FORCE_C_COMPILER(  ${CLANG_EXECUTABLE} GNU)
CMAKE_FORCE_CXX_COMPILER(${CLANG_EXECUTABLE} GNU)


# the clang triple, also used for installation
set(TRIPLE patmos-unknown-elf)

# set some compiler-related variables;
set(CMAKE_C_COMPILE_OBJECT   "<CMAKE_C_COMPILER>   -ccc-host-triple ${TRIPLE} -fno-builtin -emit-llvm <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> -ccc-host-triple ${TRIPLE} -fno-builtin -emit-llvm <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_C_LINK_EXECUTABLE  "${PATMOS_GOLD_ENV}<CMAKE_C_COMPILER> -ccc-host-triple ${TRIPLE} -fno-builtin <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_FORCE_C_OUTPUT_EXTENSION ".bc" FORCE)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-config
find_program(LLVM_CONFIG_EXECUTABLE NAMES patmos-llvm-config llvm-config DOC "Path to the llvm-config tool.")

if(NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "LLVM required for a Patmos build.")
endif()

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --targets-built
                OUTPUT_VARIABLE LLVM_TARGETS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT (${LLVM_TARGETS} MATCHES "Patmos"))
  message(FATAL_ERROR "llvm-config '${LLVM_CONFIG_EXECUTABLE}' does not report 'Patmos' as supported target.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find ar (use gold ar with LTO plugin (patmos-ar) if available; llvm-ar does not work)
find_program(LLVM_AR_EXECUTABLE NAMES patmos-ar ar DOC "Path to the ar tool.")

if(NOT LLVM_AR_EXECUTABLE)
  message(FATAL_ERROR "ar required for a Patmos build.")
endif()


set(CMAKE_AR ${LLVM_AR_EXECUTABLE} CACHE FILEPATH "Archiver")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-as
find_program(LLVM_AS_EXECUTABLE NAMES patmos-llvm-as llvm-as DOC "Path to the llvm-as tool.")

if(NOT LLVM_AS_EXECUTABLE)
  message(FATAL_ERROR "llvm-as required for a Patmos build.")
endif()

set(CMAKE_AS ${LLVM_AS_EXECUTABLE} CACHE FILEPATH "LLVM assembler")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-ranlib
find_program(LLVM_RANLIB_EXECUTABLE NAMES patmos-ranlib patmos-llvm-ranlib llvm-ranlib DOC "Path to the llvm-ranlib tool.")

if(NOT LLVM_RANLIB_EXECUTABLE)
  message(FATAL_ERROR "llvm-ranlib required for a Patmos build.")
endif()

set(CMAKE_RANLIB ${LLVM_RANLIB_EXECUTABLE} CACHE FILEPATH "Ranlib tool")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-ld
find_program(LLVM_LD_EXECUTABLE NAMES patmos-llvm-ld llvm-ld DOC "Path to the llvm-ld tool.")

if(NOT LLVM_LD_EXECUTABLE)
  message(FATAL_ERROR "llvm-ld required for a Patmos build.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-dis
find_program(LLVM_DIS_EXECUTABLE NAMES patmos-llvm-dis llvm-dis DOC "Path to the llvm-dis tool.")

if(NOT LLVM_DIS_EXECUTABLE)
  message(FATAL_ERROR "llvm-dis required for a Patmos build.")
endif()

set(CMAKE_DIS ${LLVM_DIS_EXECUTABLE} CACHE FILEPATH "LLVM disassembler")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find llvm-nm
find_program(LLVM_NM_EXECUTABLE NAMES patmos-llvm-nm llvm-nm DOC "Path to the llvm-nm tool.")

if(NOT LLVM_NM_EXECUTABLE)
  message(FATAL_ERROR "llvm-nm required for a Patmos build.")
endif()

set(CMAKE_NM ${LLVM_NM_EXECUTABLE} CACHE FILEPATH "Archive inspector")

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# find simulator

find_program(PASIM_EXECUTABLE NAMES pasim DOC "Path to the Patmos simulator pasim.")

set(PASIM_OPTIONS "" CACHE STRING "Additional command-line options passed to the Patmos simulator.")

if(PASIM_EXECUTABLE)
  set(ENABLE_TESTING true)
  macro (run_io name prog in out ref)
    add_test(NAME ${name} COMMAND ${PASIM_EXECUTABLE} ${prog} -o ${name}.stats -I ${in} -O ${out} ${PASIM_OPTIONS})
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${out} ${name}.stats)

    add_test(NAME ${name}-cmp COMMAND ${CMAKE_COMMAND} -E compare_files ${out} ${ref})
    set_tests_properties(${name}-cmp PROPERTIES DEPENDS ${name})
  endmacro (run_io)
else()
  message(WARNING "pasim not found, testing is disabled.")
  #message(FATAL_ERROR "pasim required for a Patmos build.")
endif()

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
