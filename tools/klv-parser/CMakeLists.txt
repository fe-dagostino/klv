
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD   23)

add_executable(klv-parser klv-parser.cpp)

target_compile_options(klv-parser PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

target_link_libraries(klv-parser klv)

