
set( SRC_RT
 ashldi3.c ashrdi3.c lshrdi3.c
 clzsi2.c
 muldi3.c
 divsi3.c modsi3.c udivsi3.c umodsi3.c
 divdi3.c moddi3.c udivdi3.c umoddi3.c
 divmodsi4.c udivmodsi4.c
 divmoddi4.c udivmoddi4.c
 )

set( SRC_RTSFLT
 addsf3.c subsf3.c mulsf3.c divsf3.c comparesf2.c
 adddf3.c subdf3.c muldf3.c divdf3.c comparedf2.c
 truncdfsf2.c extendsfdf2.c
 floatsisf.c floatunsisf.c
 floatsidf.c floatunsidf.c
 fixsfsi.c fixunssfsi.c
 fixdfsi.c fixunsdfsi.c
 )


# TODO export __clzXi2,__ctzXi2, __ffsXi2, __ffsti2, __parityXi2, __popcountXi2 (with X = s|d|t) ?



add_library( ${LIB_RT} STATIC ${SRC_RT} )
add_library( ${LIB_RTSFLT} STATIC ${SRC_RTSFLT} )
set_target_properties( ${LIB_RT} ${LIB_RTSFLT}
                       PROPERTIES COMPILE_FLAGS "-U__i386__ -D__PATMOS__ -O2 ")

install( TARGETS ${LIB_RT} ${LIB_RTSFLT}
         DESTINATION ${TRIPLE}/lib )

# the .ll / .o / .lst file containing the symbols

# the libc symbols (only one)
foreach( SYMFILE c)
  set( SYMFBASE lib${SYMFILE}syms )


  add_custom_command(OUTPUT ${SYMFBASE}.o
                     COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll )

  add_custom_command(OUTPUT ${SYMFBASE}.lst
                     COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll | grep "\"^declare\"" | sed "\"s/declare .* @\\([^(]*\\).*$$/\\1/\"" > ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll )

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

  add_custom_command(OUTPUT ${SYMFBASE}.ll ${SYMFBASE}.lst
                     COMMAND ${CMAKE_CURRENT_BINARY_DIR}/getdecls.sh
                             ${CMAKE_CURRENT_BINARY_DIR}/lib${SYMFILE}.a
                     DEPENDS ${SYMFILE} ${CMAKE_CURRENT_BINARY_DIR}/getdecls.sh )

  add_custom_command(OUTPUT ${SYMFBASE}.o
                     COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.ll )

  add_custom_target( ${SYMFBASE} ALL DEPENDS ${SYMFBASE}.o ${SYMFBASE}.lst)

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o
                 ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
           DESTINATION ${TRIPLE}/lib )

endforeach( SYMFILE )



