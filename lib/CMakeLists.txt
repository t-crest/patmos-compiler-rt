
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

add_custom_command(OUTPUT lib${LIBNAME}syms.o
                   COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}syms.o ${CMAKE_CURRENT_SOURCE_DIR}/patmos/lib${LIBNAME}syms.ll
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/lib${LIBNAME}syms.ll )
add_custom_target( lib${LIBNAME}syms-o ALL DEPENDS lib${LIBNAME}syms.o)

add_custom_command(OUTPUT libcsyms.o
                   COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/libcsyms.o ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libcsyms.ll
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libcsyms.ll )
add_custom_target( libcsyms-o ALL DEPENDS libcsyms.o)

add_custom_command(OUTPUT lib${LIBNAME}syms.lst
                   COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/patmos/lib${LIBNAME}syms.ll | grep "\"^declare\"" | sed "\"s/declare .* @\\([^(]*\\).*$$/\\1/\"" > ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}syms.lst
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/lib${LIBNAME}syms.ll )
add_custom_target( lib${LIBNAME}syms-lst ALL DEPENDS lib${LIBNAME}syms.lst)

install( TARGETS ${LIBNAME} DESTINATION ${TRIPLE}/lib )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}syms.o
               ${CMAKE_CURRENT_BINARY_DIR}/libcsyms.o
               ${CMAKE_CURRENT_BINARY_DIR}/lib${LIBNAME}syms.lst
         DESTINATION ${TRIPLE}/lib )
