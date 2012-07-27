
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

add_custom_command(OUTPUT libllsyms.o
                   COMMAND ${LLVM_AS_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/libllsyms.o ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libllsyms.ll
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libllsyms.ll )
add_custom_target( libllsyms-o ALL DEPENDS libllsyms.o)


add_custom_command(OUTPUT libllsyms.lst
                   COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libllsyms.ll | grep "\"^declare\"" | sed "\"s/declare .* @\\([^(]*\\).*$$/\\1/\"" > ${CMAKE_CURRENT_BINARY_DIR}/libllsyms.lst
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patmos/libllsyms.ll )
add_custom_target( libllsyms-lst ALL DEPENDS libllsyms.lst)

install( TARGETS ${LIBNAME} DESTINATION ${TRIPLE}/lib )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libllsyms.o ${CMAKE_CURRENT_BINARY_DIR}/libllsyms.lst DESTINATION ${TRIPLE}/lib )
