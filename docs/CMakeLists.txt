# Find Doxygen
find_package(Doxygen REQUIRED COMPONENTS dot)

# Add documentation target
if(Doxygen_FOUND)
  set(DOXYFILE_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile   )

  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")   


  # Generate documentation for the project
  add_custom_target(docs 
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating documentation with Doxygen"
                    VERBATIM
                   )
endif()  


