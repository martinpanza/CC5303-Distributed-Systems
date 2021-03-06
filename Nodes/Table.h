//
// Created by MarTeam and AlonSort on 19-04-2018.
//
#include <vector>
#include <set>
#include <string>
#ifndef CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
#define CC5303_DISTRIBUTED_SYSTEMS_TABLE_H


class Table {
public:

    std::set<std::string> noticedNodes;
    std::set<std::string> noticedClients;
    std::set<std::string> pathToServer;

    std::vector<std::string> direct_clients;
    std::vector<std::string> direct_routers;
    // Cliente, <Caminos posibles>
    std::vector<std::pair<std::string, std::vector<std::string>>> reachable_clients;

    void addDirectClient(std::string name);
    void addDirectRouter(std::string name);
    void addReachableClient(std::string name, std::vector<std::string> ways);
    void addNoticedNodes(std::string name);
    void addNoticedClients(std::string name);
    void addPathToServer(std::string name);
    void prepareNewServer();
    void printTable();

    std::vector<std::pair<std::string, std::vector<std::string>>>* getReachableClients();
    std::vector<std::string>* getDirectClients();
    std::vector<std::string>* getDirectRouters();
    std::set<std::string>* getPathToServer();
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
