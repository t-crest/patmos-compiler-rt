
set( LLSOURCES
 divsi3.c modsi3.c udivsi3.c umodsi3.c
 clzsi2.c
 muldi3.c divdi3.c moddi3.c udivdi3.c umoddi3.c udivmoddi4.c
 ashldi3.c ashrdi3.c lshrdi3.c
 addsf3.c subsf3.c mulsf3.c divsf3.c comparesf2.c
 adddf3.c subdf3.c muldf3.c divdf3.c comparedf2.c
 truncdfsf2.c extendsfdf2.c
 floatsisf.c floatsidf.c floatunsidf.c
 fixsfsi.c fixdfsi.c fixunsdfsi.c
 )

add_library( ${LIBNAME} STATIC ${LLSOURCES} )
set_target_properties( ${LIBNAME} PROPERTIES COMPILE_FLAGS "-U__i386__ -D__PATMOS__ -O2 ")


foreach( SYMFILE ${LIBNAME} c)
  set( SYMFBASE lib${SYMFILE}syms )


  add_custom_command(OUTPUT ${SYMFBASE}.o
                     COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll )
  add_custom_target( ${SYMFBASE}-o ALL DEPENDS ${SYMFBASE}.o)

  add_custom_command(OUTPUT ${SYMFBASE}.lst
                     COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll | grep "\"^declare\"" | sed "\"s/declare .* @\\([^(]*\\).*$$/\\1/\"" > ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/${SYMFBASE}.ll )
  add_custom_target( ${SYMFBASE}-lst ALL DEPENDS ${SYMFBASE}.lst)

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.o
                 ${CMAKE_CURRENT_BINARY_DIR}/${SYMFBASE}.lst
           DESTINATION ${TRIPLE}/lib )

endforeach( SYMFILE )





install( TARGETS ${LIBNAME} DESTINATION ${TRIPLE}/lib )
