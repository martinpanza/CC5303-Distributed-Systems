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

Node::Node(std::string ip, uint16_t port, std::string name) {
    if (ip == "localhost") {
        ip = "127.0.0.1";
    }
    this->ip = std::move(ip);
    this->port = port;
    this->name = std::move(name);
    this->table = *new Table();
}

int Node::run() {
    return 0;
}

int Node::receivePacket(char* p) {
    return 0;
}

int Node::sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type, std::string message, int sd) {
    std::cout << ip_dest << ":" << port_dest << " " << type << " " << message;
    return 0;
}

void Node::receiveTablePacket() {
    return;
}

Table* Node::getTable() {
    return &(this->table);
}

uint16_t Node::getSrcPort(const unsigned char* packet) {
    return packet[5] | uint16_t(packet[4]) << 8;
}

uint16_t Node::getDestPort(const unsigned char* packet) {
    return packet[11] | uint16_t(packet[10]) << 8;
}

uint16_t Node::getTotalLength(const unsigned char* packet) {
    return packet[14] | uint16_t(packet[13]) << 8;
}

uint16_t Node::getOffset(const unsigned char* packet) {
    return packet[17] | uint16_t(packet[16]) << 8;
}

int Node::getLastBit(const unsigned char* packet) {
    return (int) packet[15];
}

int Node::getType(const unsigned char* packet) {
    return (int) packet[12];
}

int Node::getFragmentBit(const unsigned char* packet) {
    return (int) packet[18];
}

std::string Node::getSrcIp(const unsigned char* packet) {
    std::string ip;
    std::string ip_delimiter = ".";
    ip += std::to_string(packet[0]);
    ip += ip_delimiter;
    ip += std::to_string(packet[1]);
    ip += ip_delimiter;
    ip += std::to_string(packet[2]);
    ip += ip_delimiter;
    ip += std::to_string(packet[3]);
    return ip;
}

std::string Node::getDestIp(const unsigned char* packet) {
    std::string ip;
    std::string ip_delimiter = ".";
    ip += std::to_string(packet[6]);
    ip += ip_delimiter;
    ip += std::to_string(packet[7]);
    ip += ip_delimiter;
    ip += std::to_string(packet[8]);
    ip += ip_delimiter;
    ip += std::to_string(packet[9]);
    return ip;
}

std::string Node::getMessage(const unsigned char* packet) {
    uint16_t total_length = this->getTotalLength(packet);
    int data_length = total_length - HEADER_SIZE;
    char message[data_length];
    substring(HEADER_SIZE, (size_t) data_length,(const char*)packet, message, total_length);
    return std::string(message);
}

void Node::printPacket(const unsigned char* packet) {
    std::cout << "Src IP: "<< this->getSrcIp(packet) << std::endl;
    std::cout << "Src Port: "<< this->getSrcPort(packet) << std::endl;
    std::cout << "Dest IP: "<< this->getDestIp(packet) << std::endl;
    std::cout << "Dest Port: "<< this->getDestPort(packet) << std::endl;
    std::cout << "Type: "<< this->getType(packet) << std::endl;
    std::cout << "Total Length: "<< this->getTotalLength(packet) << std::endl;
    std::cout << "Fragmented: "<< this->getFragmentBit(packet) << std::endl;
    std::cout << "Offset: "<< this->getOffset(packet) << std::endl;
    std::cout << "Last: "<< this->getLastBit(packet) << std::endl;
    std::cout << "Message: "<< this->getMessage(packet) << std::endl;
}

unsigned char* Node::makePacket(std::string ip_src, std::string port_src, std::string ip_dest,
                                std::string port_dest, int type, std::string message) {
    auto packet = (unsigned char*) malloc((HEADER_SIZE + message.length()) * sizeof(unsigned char));

    std::vector<std::string> ip, ip_d;
    //splitString(this->ip, ip, '.');
    splitString(ip_src, ip, '.');
    splitString(ip_dest, ip_d, '.');
    // Source IP
    for (int i = 0; i < ip.size(); i++) {
        packet[i] = (unsigned char)(stoi(ip[i]));
    }

    // Source port
    auto sport = (uint16_t) stoi(port_src);
    //packet[4] = (unsigned char) (this->port >> 8); // hi
    //packet[5] = (unsigned char) (this->port & 0xFF); // lo
    packet[4] = (unsigned char) (sport >> 8); // hi
    packet[5] = (unsigned char) (sport & 0xFF); // lo


    // Destination IP
    for (int i = 0; i < ip_d.size(); i++) {
        packet[i + 6] = (unsigned char)(stoi(ip_d[i]));
    }

    // Destination port
    auto dport = (uint16_t) stoi(port_dest);
    packet[10] = (unsigned char) (dport >> 8); // hi
    packet[11] = (unsigned char) (dport & 0xFF); // lo

    // Type
    packet[12] = (unsigned char) type;

    // Total package length
    packet[13] = (unsigned char) ((uint16_t) (HEADER_SIZE + message.length()) >> 8);
    packet[14] = (unsigned char) ((uint16_t) (HEADER_SIZE + message.length()) & 0xFF);

    // Fragmented
    packet[15] = (unsigned char) 0;

    // Offset
    uint16_t starting_offset = 0;
    packet[16] = (unsigned char) (starting_offset >> 8);
    packet[17] = (unsigned char) (starting_offset & 0xFF);

    // Last bit
    packet[18] = (unsigned char) 0;
    // Message
    for (int i = 0; i < message.length(); i++) {
        packet[i + 19] = (unsigned char) message[i];
    }
    return packet;
}

std::vector<std::string> Node::searchConnectedRouter(std::string name) {
    std::vector<std::string> usefulRouters;
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    std::cout << reachable_clients->empty() << std::endl;
    for (int i = 0; i < reachable_clients->size(); i++) {
        if ((*reachable_clients)[i].first == name) {
            usefulRouters = (*reachable_clients)[i].second;
            break;
        }
    }

    std::vector<std::string>* direct_clients =
            (this->getTable())->getDirectClients();
    std::cout << direct_clients->empty() << std::endl;
    for (int i = 0; i < direct_clients->size(); i++) {
        if ((*direct_clients)[i] == name) {
            usefulRouters = std::vector<std::string>();
            usefulRouters.push_back(name);
            break;
        }
    }


    return usefulRouters;
}

std::pair<char *, char *> Node::fragment(size_t packet, int MTU) {
    //TODO: reduce size of packet to MTU size, and return both new packets
    return {};
}

void Node::sendNextPacket() {
    //TODO: Modificar para que se traiga un vector con todas las rutas UTILES de acuerdo al destino del paquete
    unsigned char* packet = this->message_queue.back();
    this->message_queue.pop_back();

    int connection_mtu = this->connections[this->connectionIndex].second.second;
    /*
    if (sizeof(packet) > connection_mtu){
        std::pair<unsigned char*, unsigned char*> fragmentedPackets = fragment(sizeof(packet), connection_mtu);
        packet = fragmentedPackets.first;
        this->message_queue.push_back(fragmentedPackets.second);
    }

    //TODO: send through this->connections[this->connectionIndex];

    this->connectionIndex = (int) ((this->connectionIndex + 1) % this->connections.size());
    */
}

int Node::getSocketDescriptor(std::string name) {
    for (int i = 0; i < this->socketDescriptors.size(); i++){
        if (name == this->socketDescriptors[i].second){
            return this->socketDescriptors[i].first;
        }
    }
    return -1;
}


