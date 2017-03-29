set(SRCS main.cpp)

add_executable(client ${SRCS})
target_link_libraries(client PRIVATE ${CMAKE_THREAD_LIBS_INIT})
