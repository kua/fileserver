set(SRCS main.cpp)

add_executable(client ${SRCS})
target_link_libraries(client dto ${PROTOBUF_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

