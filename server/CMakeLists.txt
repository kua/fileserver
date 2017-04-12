set(SRCS IoService.cpp Session.cpp main.cpp)

add_executable(server ${SRCS})
target_link_libraries(server dto ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
