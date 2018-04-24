//
// Created by marti on 19-04-2018.
//
#include "Table.h"
#include <string>
#include <iostream>
#include <deque>

#define ACK_MESSAGE 1
#define CHAT_MESSAGE 2
#define TABLE_MESSAGE 3

#ifndef CC5303_DISTRIBUTED_SYSTEMS_NODE_H
#define CC5303_DISTRIBUTED_SYSTEMS_NODE_H


class Node {
    protected:
        Table table;
        std::string ip;
        int port;
        std::string name;
        std::vector<std::pair<std::string, std::pair<int, int>>> connections;
        std::deque<char*> message_queue;
        int connectionIndex = 0;


    public:
        explicit Node(std::string ip, int port, std::string name);
        int establishConnection(int ip, int port);
        int receivePacket(char* p);
        void receiveTablePacket();
        void listening();
        virtual void run();
        std::pair<char *, char *> fragment(size_t packet, int MTU);
        void sendNextPacket();
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_NODE_H
