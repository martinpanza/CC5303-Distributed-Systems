//
// Created by marti on 19-04-2018.
//

#include "Table.h"
#include <iostream>
#include <vector>
#include <stdlib.h>

std::vector<std::pair<std::string, std::vector<std::string>>>* Table::getReachableClients() {
    return &(this->reachable_clients);
}

std::vector<std::string>* Table::getDirectClients() {
    return &(this->direct_clients);
}
std::vector<std::string>* Table::getDirectRouters() {
    return &(this->direct_routers);
}

std::set<std::string>* Table::getPathToServer() {
    return &(this->pathToServer);
}

void Table::addDirectClient(std::string name) {
    this->direct_clients.push_back(name);
}

void Table::addDirectRouter(std::string name) {
    this->direct_routers.push_back(name);
}

void Table::addReachableClient(std::string name, std::vector<std::string> ways) {
    std::pair<std::string, std::vector<std::string>> p = std::pair<std::string, std::vector<std::string>>(name, ways);
    this->reachable_clients.push_back(p);
}

void Table::addNoticedNodes(std::string name) {
    this->noticedNodes.insert(name);
}

void Table::addNoticedClients(std::string name) {
    this->noticedClients.insert(name);
}

void Table::addPathToServer(std::string name) {
    this->pathToServer.insert(name);
}
void Table::prepareNewServer() {
    this->pathToServer.clear();
    this->noticedNodes.clear();
    // Should I clear noticed clients?
}

void Table::printTable() {
    std::cout << "Direct Clients: " << std::endl;
    for (std::string client : direct_clients) {
        std::cout << client << "; ";
    }
    std::cout << std::endl;
    std::cout << "Direct Routers: " << std::endl;
    for (std::string router : direct_routers) {
        std::cout << router << "; ";
    }
    std::cout << std::endl;
    std::cout << "Reachable Clients: " << std::endl;
    for (std::pair<std::string, std::vector<std::string>> conn : reachable_clients) {
        std::cout << conn.first << ": ";
        for (std::string rt : conn.second) {
            std::cout << rt << "; ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

