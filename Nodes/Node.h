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
#include <map>

#define ACK_MESSAGE 1
#define CHAT_MESSAGE 2
#define TABLE_MESSAGE 3
#define SACK_MESSAGE 4
#define NEW_SRV_MESSAGE 5
#define RESEND_MESSAGE 6
#define MIGRATE_MESSAGE 7
#define MACK_MESSAGE 7

#define HEADER_SIZE 21

#define MAX_SEQ_NUMBER 128

#ifndef CC5303_DISTRIBUTED_SYSTEMS_NODE_H
#define CC5303_DISTRIBUTED_SYSTEMS_NODE_H


class Node {

public:
    std::mutex mtx;
    std::mutex listen_mutex;
    std::condition_variable cond;

    std::mutex turnedOffMutex;
    std::condition_variable turnedOffCond;

    std::mutex serverMutex;
    std::condition_variable serverCond;

    std::string serverName;

    int iAmAServer = 0;
    int off = 0;
    int migrating = 0;

    Table table;
    std::string ip;
    uint16_t port;
    std::string name;
    std::vector<std::pair<std::string, std::pair<int, int>>> connections;
    std::deque<unsigned char*> message_queue;
    int connectionIndex = 0;
    std::vector<std::pair<int, std::string>> socketDescriptors;

    std::string searchConnectedRouter(std::string name);
    explicit Node(std::string ip, uint16_t port, std::string name);
    virtual int run();
    Table* getTable();
    std::string getSrcIp(const unsigned char* packet);
    std::string getDestIp(const unsigned char* packet);
    std::string getMessage(const unsigned char* packet);
    unsigned char* makeServerPacket();
    int getServerBit(const unsigned char* packet);
    void setServerBit(unsigned char* packet, int serverBit);
    int getSeqNum(const unsigned char* packet);
    void setSeqNum(unsigned char* packet, int seqNum);
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
                              std::string port_dest, int type, std::string message, int sequenceNumber);
    virtual int sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest,
                            int type, std::string message, int sd, int sequenceNumber);
    int getSocketDescriptor(std::string basic_string);
    int getDelay(std::string basic_string);

    int getMTU(std::string name);
    std::pair<unsigned char *, unsigned char*> fragment(unsigned char* packet, int MTU);

    std::vector<std::pair<std::string, std::vector<unsigned char *>>> fragmentedPackets;
    std::vector<std::pair<std::pair<std::string, std::string>, std::pair<int, std::vector<unsigned char *>>>> serverFragmentedPackets;
    std::vector<std::pair<std::string, std::string>> serverWaitingForAcks;
    std::pair<int, std::string> checkFragmentArrival(std::vector<unsigned char *> fragments);

    int allMessageArrived(std::vector<unsigned char *> fragments);

    void announceServer(std::string message, std::string initialSender);
    void processServerMessage(const unsigned char* packet);
    int isDirectConnection(std::string name);

    int partition (std::vector<unsigned char*>* fragments, int low, int high);
    void quickSort(std::vector<unsigned char*>* fragments, int low, int high);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_NODE_H
