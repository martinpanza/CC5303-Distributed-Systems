cmake_minimum_required(VERSION 3.9)
project(CC5303-Distributed-Systems)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Packets/ChatPacket.cpp Packets/ChatPacket.h Packets/ConnectionPacket.cpp Packets/ConnectionPacket.h Packets/TablePackets.cpp Packets/TablePackets.h Packets/Packet.h Nodes/Node.h Nodes/C.cpp Nodes/C.h Nodes/T.cpp Nodes/T.h Nodes/Table.cpp Nodes/Table.h Nodes/Node.cpp)
add_executable(CC5303-Distributed-Systems ${SOURCE_FILES})