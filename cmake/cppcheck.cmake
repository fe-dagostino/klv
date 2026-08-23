# Find cppcheck
find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
  
set(CMAKE_CXX_CPPCHECK   "${CMAKE_CXX_CPPCHECK}"
                         --enable=all
                         --inconclusive
                         --std=c++23
                         --inline-suppr
                         --suppress=uninitvar
                         --quiet
   )
