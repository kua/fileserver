set(SRCS IoService.cpp Session.cpp main.cpp)

add_executable(server ${SRCS})
target_link_libraries(server PRIVATE ${CMAKE_THREAD_LIBS_INIT})
