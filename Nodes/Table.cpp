//
// Created by marti on 19-04-2018.
//

#include "Table.h"
#include <iostream>
#include <vector>
#include <stdlib.h>

void Table::add_direct_client(std::string name) {
    this->direct_clients.push_back(name);
}

void Table::add_direct_router(std::string name) {
    this->direct_routers.push_back(name);
}

void Table::add_reachable_client(std::string name, std::vector<std::string> ways) {
    this->reachable_clientes.push_back(new std::vector(new std::pair(name, ways)));
}
