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
        void add_connection(std::string name);
        int send_message(std::string message);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_C_H
