
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD   23)

add_executable(klv-parser klv-parser.cpp)
target_link_libraries(klv-parser klv)

