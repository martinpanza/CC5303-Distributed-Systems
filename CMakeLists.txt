cmake_minimum_required(VERSION 3.9)
project(CC5303-Distributed-Systems)

set(CMAKE_CXX_STANDARD 11)

find_package (Threads)

set(SOURCE_FILES main.cpp src/Node.h src/C.cpp src/C.h src/T.cpp src/T.h src/Table.cpp src/Table.h src/Node.cpp src/socket.cpp src/socket.h src/ThreadFun.h src/ThreadFun.cpp utils.h utils.cpp)
add_executable(CC5303-Distributed-Systems ${SOURCE_FILES})

target_link_libraries (CC5303-Distributed-Systems ${CMAKE_THREAD_LIBS_INIT})