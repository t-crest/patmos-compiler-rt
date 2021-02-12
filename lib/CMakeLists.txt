
set( SRC_RT
 ashldi3.c ashrdi3.c lshrdi3.c
 patmos/clzsi2.c
 muldi3.c
 patmos/udivsi3.c patmos/udivmodsi4.c # following depend on udivsi3.c
 umodsi3.c divsi3.c modsi3.c divmodsi4.c 
 udivmoddi4.c # following depend on udivmoddi4.c
 divmoddi4.c udivdi3.c umoddi3.c divdi3.c moddi3.c
 )

set( SRC_RTSFLT
 patmos/initfpu.c
 addsf3.c subsf3.c mulsf3.c divsf3.c comparesf2.c
 adddf3.c subdf3.c muldf3.c divdf3.c comparedf2.c
 truncdfsf2.c extendsfdf2.c
 floatsisf.c floatunsisf.c
 floatsidf.c floatunsidf.c
 floatdisf.c floatundisf.c
 floatdidf.c floatundidf.c
 fixsfsi.c fixunssfsi.c
 fixdfsi.c fixunsdfsi.c
 fixsfdi.c fixunssfdi.c
 fixdfdi.c fixunsdfdi.c
 )


# TODO export __clzXi2,__ctzXi2, __ffsXi2, __ffsti2, __parityXi2, __popcountXi2 (with X = s|d|t) ?



add_library( ${LIB_RT} STATIC ${SRC_RT} )
add_library( ${LIB_RTSFLT} STATIC ${SRC_RTSFLT} )
set_target_properties( ${LIB_RT} ${LIB_RTSFLT}
                       PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} -U__i386__ -D__PATMOS__ -O2 ")

install( TARGETS ${LIB_RT} ${LIB_RTSFLT}
         DESTINATION ${TRIPLE}/lib )

# the .ll / .o / .lst file containing the symbols

# the libc symbols (only one)
foreach( SYMFILE c m)
  set( SYMFBASE lib${SYMFILE}syms )

  # set target triple
  configure_file( patmos/${SYMFBASE}.ll.in ${SYMFBASE}.ll.tmp @ONLY )

  add_custom_command(OUTPUT ${SYMFBASE}.ll
                     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/patmos/markused.sh ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll.tmp ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll
		     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll.tmp )

  add_custom_command(OUTPUT ${SYMFBASE}.o
                     COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll )

  add_custom_command(OUTPUT ${SYMFBASE}.lst
                     COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll | grep "\"^declare\"" | sed "\"s/declare .* @\\([^(]*\\).*$$/\\1/\"" > ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll )

  add_custom_target( ${SYMFBASE} ALL DEPENDS ${SYMFBASE}.lst ${SYMFBASE}.o)

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o
                 ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
           DESTINATION ${TRIPLE}/lib )

endforeach( SYMFILE )


# configure for the right LLVM_*_EXECUTABLEs
configure_file( patmos/getdecls.sh.in getdecls.sh @ONLY )

# the support / softfloat symbols
foreach( SYMFILE ${LIB_RT} ${LIB_RTSFLT} )
  set( SYMFBASE lib${SYMFILE}syms )

  add_custom_command(OUTPUT ${SYMFBASE}.ll.tmp ${SYMFBASE}.lst
                     COMMAND ${CMAKE_CURRENT_BINARY_DIR}/getdecls.sh
                             ${CMAKE_CURRENT_BINARY_DIR}/lib${SYMFILE}.a
                             ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll.tmp
                     DEPENDS ${SYMFILE} ${CMAKE_CURRENT_BINARY_DIR}/getdecls.sh )

  add_custom_command(OUTPUT ${SYMFBASE}.ll
                     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/patmos/markused.sh ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll.tmp ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll
		     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll.tmp )

  add_custom_command(OUTPUT ${SYMFBASE}.o
                     COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll )

  add_custom_target( ${SYMFBASE} ALL DEPENDS ${SYMFBASE}.o ${SYMFBASE}.lst)

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o
                 ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
           DESTINATION ${TRIPLE}/lib )

endforeach( SYMFILE )


