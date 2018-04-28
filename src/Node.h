//
// Created by marti on 19-04-2018.
//
#include "Table.h"
#include <string>
#include <iostream>
#include <deque>
#include "../utils.h"
#include <mutex>
#include <stdio.h>
#include <algorithm>
#include <condition_variable>
#include <condition_variable>
#include <vector>
#include <sys/socket.h>
#include <stdlib.h>

#define ACK_MESSAGE 1
#define CHAT_MESSAGE 2
#define TABLE_MESSAGE 3

#define HEADER_SIZE 19

#ifndef CC5303_DISTRIBUTED_SYSTEMS_NODE_H
#define CC5303_DISTRIBUTED_SYSTEMS_NODE_H


class Node {

public:
    std::mutex mtx;
    std::mutex listen_mutex;
    std::condition_variable cond;
    Table table;
    std::string ip;
    uint16_t port;
    std::string name;
    std::vector<std::pair<std::string, std::pair<int, int>>> connections;
    std::deque<unsigned char*> message_queue;
    int connectionIndex = 0;
    std::vector<std::pair<int, std::string>> socketDescriptors;

    std::vector<std::string> searchConnectedRouter(std::string name);
    explicit Node(std::string ip, uint16_t port);
    virtual int receivePacket(char* p);
    virtual void receiveTablePacket();
    virtual int run();
    Table* getTable();
    std::string getSrcIp(const unsigned char* packet);
    std::string getDestIp(const unsigned char* packet);
    std::string getMessage(const unsigned char* packet);
    int getLastBit(const unsigned char* packet);
    int getFragmentBit(const unsigned char* packet);
    uint16_t getTotalLength(const unsigned char* packet);
    int getType(const unsigned char* packet);
    static uint16_t getOffset(const unsigned char* packet);
    static uint16_t getSrcPort(const unsigned char* packet);
    static uint16_t getDestPort(const unsigned char* packet);

    void setFragmentBit(unsigned char* packet, int fragmentBit);
    void setOffset(unsigned char* packet, uint16_t offset);
    void setLastBit(unsigned char* packet, int lastBit);

    void printPacket(const unsigned char* packet);
    unsigned char* makePacket(std::string ip_src, std::string port_src, std::string ip_dest,
                              std::string port_dest, int type, std::string message);
    virtual int sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type, std::string message, int sd);
    int getSocketDescriptor(std::string basic_string);
    int getDelay(std::string basic_string);

    int getMTU(std::string name);
    std::pair<unsigned char *, unsigned char*> fragment(unsigned char* packet, int MTU);

    std::vector<std::pair<std::string, std::vector<unsigned char *>>> fragmentedPackets;
    std::pair<int, std::string> checkFragmentArrival(std::vector<unsigned char *> fragments);

    int partition (std::vector<unsigned char*>fragments, int low, int high);
    void quickSort(std::vector<unsigned char*>fragments, int low, int high);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_NODE_H
