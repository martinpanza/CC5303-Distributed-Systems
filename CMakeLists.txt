cmake_minimum_required(VERSION 3.9)
project(CC5303-Distributed-Systems)

set(CMAKE_CXX_STANDARD 11)

find_package (Threads)

set(SOURCE_FILES main.cpp Nodes/Node.h Nodes/C.cpp Nodes/C.h Nodes/T.cpp Nodes/T.h Nodes/Table.cpp Nodes/Table.h Nodes/Node.cpp Nodes/socket.cpp Nodes/socket.h Nodes/ThreadFun.h Nodes/ThreadFun.cpp utils.h utils.cpp Nodes/ThreadUtils.h Nodes/ThreadUtils.cpp)
add_executable(CC5303-Distributed-Systems ${SOURCE_FILES})

target_link_libraries (CC5303-Distributed-Systems ${CMAKE_THREAD_LIBS_INIT})