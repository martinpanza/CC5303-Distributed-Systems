//
// Created by marti on 19-04-2018.
//

#include "Node.h"
#ifndef CC5303_DISTRIBUTED_SYSTEMS_C_H
#define CC5303_DISTRIBUTED_SYSTEMS_C_H


class C : public Node{
    using Node::Node;

public:
    int waitingForAck = 0;
    int waitingForSack = 0;
    int currentSequenceNumber = 0;
    std::string sentMessage = "";
    std::string ipSent = "";
    std::string portSent = "";
    std::vector<std::pair<std::string, int>> sentAcks;
    int run() override;
    int sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type,
                    std::string message, int sd, int sequenceNumber, int serverBit) override;
    int sendPacket(unsigned char* packet);
    void addConnection(std::string ip, std::string port);
    void increaseSequenceNumber();

    void resendAcks(int serverBit);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_C_H
