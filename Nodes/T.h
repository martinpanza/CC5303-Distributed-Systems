//
// Created by marti on 19-04-2018.
//
#include "Node.h"

#ifndef CC5303_DISTRIBUTED_SYSTEMS_T_H
#define CC5303_DISTRIBUTED_SYSTEMS_T_H


class T : public Node {
    using Node::Node;

    public:
        void add_connection(std::string name);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_T_H
