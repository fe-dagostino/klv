
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})   

set( KLV_DEP       "")

set( KLV_INCLUDE   ${PROJECT_SOURCE_DIR}/include/klv-common.h
                   ${PROJECT_SOURCE_DIR}/include/klv-config.h
                   ${PROJECT_SOURCE_DIR}/include/klv-parser.h
                   ${PROJECT_SOURCE_DIR}/include/misb0102.h
                   ${PROJECT_SOURCE_DIR}/include/misb0601.h
                   ${PROJECT_SOURCE_DIR}/include/misb0604.h
                   ${PROJECT_SOURCE_DIR}/include/misb0903.h
   )

message( INFO " add_library( ${CMAKE_PROJECT_NAME} INTERFACE ${KLV_INCLUDE} )" )
add_library( ${CMAKE_PROJECT_NAME} INTERFACE ${KLV_INCLUDE} )

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
