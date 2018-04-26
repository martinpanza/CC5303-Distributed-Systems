//
// Created by marti on 19-04-2018.
//

#include "T.h"
#include <random>
#include "socket.h"
#include "threadFun.h"
#include <thread>

int random_int(int min, int max) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> int_distribution(min, max);

    return int_distribution(rng);
}

int random_mtu() {
    srand ( time(NULL) ); //initialize the random seed

    const int arrayNum[4] = {32, 64, 128, 256};
    int RandIndex = rand() % 4; //generates a random number between 0 and 3
    return arrayNum[RandIndex];
}

int T::run() {
    int server_fd = serverSocket(this->port);

    std::thread accepter (acceptTh, *this, server_fd);
    accepter.detach();

    std::string s;
    std::string connect_ = "connect";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        // connect ip port type
        if (words[0] == connect_ and words.size() == 4) {
            int client_sd = clientSocket(stoi(words[2]));

            this->addConnection(words[1], words[2], words[3]);
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));

            std::thread receiver (receiveTh, client_sd);
            receiver.detach();
        }
    }
}

std::string T::makeTableMessage() {
    std::string message;
    std::string delimiter = ";";
    std::vector<std::string>* direct_clients = (this->getTable())->getDirectClients();
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();

    // Add the ones directly connected to this router
    if ((*direct_clients).empty()) {
        for (int i = 0; i < (*direct_clients).size() - 1; i++) {
            message += (*direct_clients)[i];
            message += delimiter;
        }
        message += (*direct_clients)[(*direct_clients).size() - 1];
        if ((*reachable_clients).empty()) {
            message += delimiter;
        }
    }

    // Add the reachable ones
    if ((*reachable_clients).empty()) {
        for (int i = 0; i < (*reachable_clients).size() - 1; i++) {
            message += ((*reachable_clients)[i]).first;
            message += delimiter;
        }
        message += (*reachable_clients)[(*reachable_clients).size() - 1].first;
    }
    return message;
}

void T::broadcastTable() {
    // Should share the table with all the direct routers
    std::vector<std::string>* directRouters =(this->getTable())->getDirectRouters();
    std::vector<std::string> ipport;
    for (auto const &router : (*directRouters)) {
        splitString(router, ipport, ':');
        this->sendMessage(ipport[0], ipport[1], TABLE_MESSAGE, this->makeTableMessage());
    }
}

void T::shareTable(std::string ip, std::string port) {
    this->sendMessage(std::move(ip), std::move(port), TABLE_MESSAGE, this->makeTableMessage());
}

void T::processTablePacket(const unsigned char* packet) {
    std::string srcIpPort = this->getSrcIp(packet) + ":" + std::to_string(this->getSrcPort(packet));
    std::string tableMessage = this->getMessage(packet);
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    std::vector<std::string>* direct_clients = (this->getTable())->getDirectClients();
    std::vector<std::string> new_clients;
    splitString(tableMessage, new_clients, ';');
    bool oldClient, oldRouter, isDirectClient, sendUpdate = false;
    for (int i = 0; i < new_clients.size(); i++) {
        oldClient = false;
        // Check if im not directly connected
        isDirectClient = false;
        for (int m = 0; m < (*direct_clients).size(); m++) {
            if (new_clients[i] == (*direct_clients)[m]) {
                isDirectClient = true;
                break;
            }
        }
        if (isDirectClient) {
            continue;
        }

        // Check if it's already in my table
        for (int j = 0; j < (*reachable_clients).size(); j++) {
            if (((*reachable_clients)[j]).first == new_clients[i]) {
                oldClient = true;
                oldRouter = false;
                for (int k = 0; k < (((*reachable_clients)[j]).second).size(); k++) {
                    // check if the router is already in the list
                    if (((*reachable_clients)[j]).second[k] == srcIpPort) {
                        oldRouter = true;
                        break;
                    }
                }
                if (not oldRouter) {
                    ((*reachable_clients)[j]).second.push_back(srcIpPort);
                }
            }
            if (oldClient) {
                break;
            }
        }
        if (not oldClient) {
            std::vector<std::string> v = {srcIpPort};
            (this->table).addReachableClient(new_clients[i], v);
            sendUpdate = true;
        }
    }
    if (sendUpdate) {
        this->broadcastTable();
    }
}

void T::addConnection(std::string ip, std::string port, std::string type) {
    std::string ipport = ip + ":" + port;
    int delay = random_int(2, 10);
    int MTU = random_mtu();
    // Put in direct routers or clients
    if (type == "C") {
        delay = 1;
        MTU = 512;
        (this->getTable())->addDirectClient(ipport);
    } else if (type == "T") {
        (this->getTable())->addDirectRouter(ipport);
        // Should share table instantly?
        this->shareTable(ip, port);
    }
    // add to connections
    std::pair<int, int> p = std::pair<int, int>(delay,MTU);
    std::pair<std::string, std::pair<int, int>> P = std::pair<std::string, std::pair<int, int>>(ipport, p);
    this->connections.push_back(P);
}

int T::sendMessage(std::string ip_dest, std::string port_dest, int type, std::string message) {
    return 0;
}
