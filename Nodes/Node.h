//
// Created by marti on 19-04-2018.
//
#include "Table.h"
#include <string>
#include <iostream>
#include <deque>
#include "../utils.h"

#define ACK_MESSAGE 1
#define CHAT_MESSAGE 2
#define TABLE_MESSAGE 3

#define HEADER_SIZE 19

#ifndef CC5303_DISTRIBUTED_SYSTEMS_NODE_H
#define CC5303_DISTRIBUTED_SYSTEMS_NODE_H


class Node {
    protected:
        Table table;
        std::string ip;
        uint16_t port;
        std::string name;
        std::vector<std::pair<std::string, std::pair<int, int>>> connections;
        std::deque<char*> message_queue;
        int connectionIndex = 0;


    public:
        explicit Node(std::string ip, uint16_t port, std::string name);
        int establishConnection(int ip, int port);
        int receivePacket(char* p);
        void receiveTablePacket();
        void listening();
        virtual void run();
        std::string getSrcIp(const unsigned char* packet);
        std::string getDestIp(const unsigned char* packet);
        std::string getMessage(const unsigned char* packet);
        int getLastBit(const unsigned char* packet);
        int getFragmentBit(const unsigned char* packet);
        uint16_t getTotalLength(const unsigned char* packet);
        int getType(const unsigned char* packet);
        uint16_t getOffset(const unsigned char* packet);
        uint16_t getSrcPort(const unsigned char* packet);
        uint16_t getDestPort(const unsigned char* packet);
        void printPacket(const unsigned char* packet);
        unsigned char* makePacket(std::string ip_dest, std::string port_dest, int type, std::string message);
        std::pair<char *, char *> fragment(size_t packet, int MTU);
        void sendNextPacket();
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_NODE_H
