//
// Created by marti on 19-04-2018.
//
#include "Table.h"
#include "../Packets/Packet.h"
#include <string>
#include <deque>

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
        char* makeHeader(std::string ip_dest);
        int establishConnection(int ip, int port);
        int receivePacket(char* p);
        void receiveTablePacket();
        void listening();
        void run();
        std::pair<Packet, Packet> fragment(Packet packet, int MTU);
        void send_next_packet();
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_NODE_H
