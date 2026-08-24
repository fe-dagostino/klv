cmake_minimum_required (VERSION 3.28)
project(klv
        VERSION 0.1.0
        DESCRIPTION "KLV Parser"
        LANGUAGES CXX C
       )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}-config.h)

# If not set, the latest supported standard for your compiler is used
# You can later set fine-grained standards for each target using `target_compile_features`
# Note: linking together projects compiled with different C++ standards may work, but
# it is not recommended because of possible issues with ABI
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD   23)

# Strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++23 and -std=gnu++23 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS           ON )
set(CMAKE_CXX_STANDARD_REQUIRED    ON )
set(CMAKE_C_EXTENSIONS             OFF)
set(CMAKE_C_STANDARD_REQUIRED      ON )

set(CMAKE_EXPORT_COMPILE_COMMANDS  ON )

# Set compiler options using this form since one of the dependency is
# using an old version of CMake and is not compatibile with expression generator.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
endif()

set(POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
message(STATUS "${PROJECT_SOURCE_DIR}")

option(KLV_BUILD_TOOLS          "Enable/Disable build of tools."                          ON)
option(KLV_BUILD_TESTS          "Enable/Disable build of tests."                         OFF)
option(KLV_BUILD_DOCS           "Enable/Disable doxygen generator"                        ON)
option(KLV_ENABLE_CPPCHECK      "Enable/Disable cppcheck"                                 ON)
option(KLV_ENABLE_IWYU          "Enable/Disable include what you use."                    ON)
option(KLV_ENABLE_PACKAGING     "Enable/Disable packaging."                               ON)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include/            )

# Add subdirectory with main application
add_subdirectory(src)

# Add subdirectory with tools
if(KVL_BUILD_TOOLS)
  add_subdirectory(tools)
endif()

# Add subdirectory with tests
if(KLV_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Add subdirectory with docs 
if(KLV_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Enable cppcheck
if(KLV_ENABLE_CPPCHECK)
  include(cppcheck)
endif()

# Enable packaging
if(KLV_ENABLE_PACKAGING)
  include(packages)
endif()
