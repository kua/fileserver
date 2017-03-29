cmake_minimum_required(VERSION 3.1.4)
set(CMAKE_CXX_STANDARD 11)

project(file-server)

find_package(Threads)

add_subdirectory(server)
add_subdirectory(client)
