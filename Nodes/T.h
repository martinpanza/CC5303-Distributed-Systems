//
// Created by marti y alonSort on 19-04-2018.
//
#include "Node.h"

#ifndef CC5303_DISTRIBUTED_SYSTEMS_T_H
#define CC5303_DISTRIBUTED_SYSTEMS_T_H


class T : public Node {
    using Node::Node;

    public:
        int run() override;
        void addConnection(std::string ip, std::string port);
        int sendMessage(std::string ip_dest, std::string port_dest, int type, std::string message);

};


#endif //CC5303_DISTRIBUTED_SYSTEMS_T_H
