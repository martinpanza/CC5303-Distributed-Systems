//
// Created by marti on 19-04-2018.
//

#include "C.h"

int C::send_message(std::string message) {

    return 0;
}

void C::add_connection(std::string name) {
    this->connections.push_back(new std::vector(new std::pair(1, 512)));
}
