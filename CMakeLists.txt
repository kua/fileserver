cmake_minimum_required(VERSION 3.1.4)
set(CMAKE_CXX_STANDARD 11)

project(file-server)

find_package(Threads)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

add_subdirectory(dto)
add_subdirectory(server)
add_subdirectory(client)
