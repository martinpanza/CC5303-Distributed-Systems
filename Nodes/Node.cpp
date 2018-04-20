//
// Created by marti on 19-04-2018.
//

#include<stdio.h>
#include <iostream>
#include "Node.h"
#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

Node::Node(std::string ip, int port, std::string name) {
    this->ip = ip;
    this->port = port;
    this->name = name;
    this->table = *new Table();
}

int establishConnection(int ip, int port) {

}

int receivePacket(Packet p);
int sendPacket(Packet p);
void listening();

void Node::run() {
    std::cout << "ready!";
}
