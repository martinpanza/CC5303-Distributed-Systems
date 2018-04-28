//
// Created by marti on 19-04-2018.
//

#include "Node.h"
#ifndef CC5303_DISTRIBUTED_SYSTEMS_C_H
#define CC5303_DISTRIBUTED_SYSTEMS_C_H


class C : public Node{
    using Node::Node;

private:
    int waitingForAck = 0;

public:

    int run() override;
    int sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type, std::string message, int sd) override;
    void addConnection(std::string ip, std::string port);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_C_H
