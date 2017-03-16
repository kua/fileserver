cmake_minimum_required(VERSION 3.1.4)
set(CMAKE_CXX_STANDARD 11)

project(file-server)

set(SRCS IoService.cpp Session.cpp main.cpp)

add_executable(server ${SRCS})
target_link_libraries(server pthread)
