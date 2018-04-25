//
// Created by marti on 19-04-2018.
//

#include "T.h"
#include <random>
#include <time.h>

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
    return 0;
}

std::string T::makeTableMessage() {
    std::string message;
    std::string delimiter = ";";
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    for (int i = 0; i < (*reachable_clients).size() - 1; i++) {
        message += ((*reachable_clients)[i]).first;
        message += delimiter;
    }
    message += (*reachable_clients)[(*reachable_clients).size() - 1].first;
    return message;
}



void T::processTablePacket(const unsigned char* packet) {
    std::string srcIpPort = this->getSrcIp(packet) + std::to_string(this->getSrcPort(packet));
    std::string tableMessage = this->getMessage(packet);
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    std::vector<std::string>* direct_clients = (this->getTable())->getDirectClients();
    std::vector<std::string> new_clients;
    splitString(tableMessage, new_clients, ';');
    bool oldClient, oldRouter, is_direct_client, sendUpdate = false;
    for (int i = 0; i < new_clients.size(); i++) {
        oldClient = false;
        // Check if im not directly connected
        is_direct_client = false;
        for (int m = 0; m < (*direct_clients).size(); m++) {
            if (new_clients[i] == (*direct_clients)[m]) {
                is_direct_client = true;
                break;
            }
        }
        if (is_direct_client) {
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
        //this->sendMessage()
    }
}

void T::addConnection(std::string ip, std::string port) {
    int delay = random_int(2, 10);
    int MTU = random_mtu();
    std::pair<int, int> p = std::pair<int, int>(delay,MTU);
    std::pair<std::string, std::pair<int, int>> P = std::pair<std::string, std::pair<int, int>>(ip + ":" + port, p);
    this->connections.push_back(P);
}
