//
// Created by MarTeam and AlonSort on 19-04-2018.
//
#include <vector>
#include <string>
#ifndef CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
#define CC5303_DISTRIBUTED_SYSTEMS_TABLE_H


class Table {
public:
    std::vector<std::string> noticedRouters;

    std::vector<std::string> direct_clients;
    std::vector<std::string> direct_routers;
    // Cliente, <Caminos posibles>
    std::vector<std::pair<std::string, std::vector<std::string>>> reachable_clients;

    void addDirectClient(std::string name);
    void addDirectRouter(std::string name);
    void addReachableClient(std::string name, std::vector<std::string> ways);
    void printTable();

    std::vector<std::pair<std::string, std::vector<std::string>>>* getReachableClients();
    std::vector<std::string>* getDirectClients();
    std::vector<std::string>* getDirectRouters();
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
