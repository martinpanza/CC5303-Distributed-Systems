//
// Created by MarTeam and AlonSort on 19-04-2018.
//
#include <vector>
#include <string>
#ifndef CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
#define CC5303_DISTRIBUTED_SYSTEMS_TABLE_H


class Table {
    private:
        std::vector<std::string> direct_clients;
        std::vector<std::string> direct_routers;
        std::vector<std::vector<std::pair<std::string, std::vector<std::string>>>> reachable_clientes;
    public:
        void add_direct_client(std::string name);
        void add_direct_router(std::string name);
        void add_reachable_client(std::string name, std::vector<std::string> ways);
};


#endif //CC5303_DISTRIBUTED_SYSTEMS_TABLE_H
