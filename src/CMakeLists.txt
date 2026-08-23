
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})   

set( KLV_DEP       "")

set( KLV_INCLUDE   klv_config.h
                   klv_parser.h
                   misb0102.h
                   misb0601.h
                   misb0604.h
                   misb0903.h
   )

add_library( ${CMAKE_PROJECT_NAME} INTERFACE ${KLV_INClUDE} )

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
