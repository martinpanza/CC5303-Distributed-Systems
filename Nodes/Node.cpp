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

int receivePacket(std::string p);
int sendPacket(std::string p);
void listening();



int Node::run() {
    std::cout << "ready!";
}

std::pair<char *, char *> Node::fragment(size_t packet, int MTU) {
    //TODO: reduce size of packet to MTU size, and return both new packets
    return std::pair<char*, char*>();
}

void Node::sendNextPacket() {
    char* packet = this->message_queue.back();
    this->message_queue.pop_back();

    int connection_mtu = this->connections[this->connectionIndex].second.second;

    if (sizeof(packet) > connection_mtu){
        std::pair<char*, char*> fragmentedPackets = fragment(sizeof(packet), connection_mtu);
        packet = fragmentedPackets.first;
        this->message_queue.push_back(fragmentedPackets.second);
    }

    //TODO: serialize packet
    //TODO: send through this->connections[this->connectionIndex];

    this->connectionIndex = (this->connectionIndex + 1) % this->connections.size();

}
