//
// Created by marti on 19-04-2018.
//

#include "T.h"
#include <random>
#include "socket.h"
#include "ThreadFun.h"
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

    std::thread accepter (acceptTh, this, server_fd);
    accepter.detach();

    std::thread sender (sendTh, this);
    sender.detach();

    std::string s;
    std::string connect_ = "connect";
    std::string startServer_ = "start_server";
    std::string stopServer_ = "stop_server";
    std::string backToNormal_ = "back_to_normal";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[1] == "localhost"){
            words[1] = "127.0.0.1";
        }
        // connect ip port type
        if (words[0] == connect_ and words.size() == 4) {
            int client_sd = clientSocket(stoi(words[2]));

            if (client_sd == -1) {
                std::cout << "Check if ip and port are correct and try again." << std::endl;
                continue;
            }

            // THIS MUST GO FIRST
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));
            // THIS MUST GO SECOND
            this->addConnection(words[1], words[2], words[3]);

            std::thread receiver (receiveTh, this ,client_sd);
            receiver.detach();
        } else if (words[0] == startServer_) {

            this->iAmAServer = 1;
            this->off = 0;

            std::unique_lock<std::mutex> lk(this->serverMutex);
            this->serverCond.wait(lk);
            lk.unlock();

            std::thread server (tServerTh, this);
            server.detach();

        } else if (words[0] == stopServer_) {
            this->iAmAServer = 0;
            this->off = 1;

            std::unique_lock<std::mutex> lk(this->serverMutex);
            this->serverCond.wait(lk);
            lk.unlock();

            std::thread offServer (offServerTh, this);
            offServer.detach();
        } else if (words[0] == backToNormal_) {
            this->iAmAServer = 0;
            this->off = 0;

            std::unique_lock<std::mutex> lk(this->serverMutex);
            this->serverCond.wait(lk);
            lk.unlock();

            std::thread sender (sendTh, this);
            sender.detach();
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
    if (!(*direct_clients).empty()) {
        for (int i = 0; i < (*direct_clients).size() - 1; i++) {
            message += (*direct_clients)[i];
            message += delimiter;
        }
        message += (*direct_clients)[(*direct_clients).size() - 1];
        if (!(*reachable_clients).empty()) {
            message += delimiter;
        }
    }
    if (!(*reachable_clients).empty()) {
        for (int i = 0; i < (*reachable_clients).size() - 1; i++) {
            message += ((*reachable_clients)[i]).first;
            message += delimiter;
        }
        message += (*reachable_clients)[(*reachable_clients).size() - 1].first;
    }
   // std::cout << "Table message created: " << message << std::endl;
    return message;
}

void T::broadcastTable() {
    // Should share the table with all the direct routers
    std::vector<std::string>* directRouters =(this->getTable())->getDirectRouters();
    std::vector<std::string> ipport;
    for (std::string router : (*directRouters)) {
        splitString(router, ipport, ':');
        this->sendMessage(this->ip, std::to_string(this->port), ipport[0], ipport[1], TABLE_MESSAGE,
                          this->makeTableMessage(), this->getSocketDescriptor(router), 255);
    }
}

void T::shareTable(std::string ip, std::string port, int sd) {
    if (this->makeTableMessage() != "") {
        //std::cout << "sharetable" << std::endl;
        //std::cout << this->makeTableMessage() << std::endl;
        this->sendMessage(this->ip, std::to_string(this->port), std::move(ip), std::move(port),
                          TABLE_MESSAGE, this->makeTableMessage(), sd, 255);
    }
}

void T::processTablePacket(const unsigned char* packet) {
    std::string srcIpPort = this->getSrcIp(packet) + ":" + std::to_string(this->getSrcPort(packet));
    std::string tableMessage = this->getMessage(packet);
    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    std::vector<std::string>* direct_clients = (this->getTable())->getDirectClients();
    std::vector<std::string> new_clients;
    splitString(tableMessage, new_clients, ';');

    /*std::cout << "clients that arrived" << std::endl;
    for (int i = 0; i < new_clients.size(); i++) {
        std::cout << new_clients[i] << std::endl;
    }*/

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
    if (type == "C" || type == "c") {
        delay = 1;
        MTU = 512;
        (this->getTable())->addDirectClient(ipport);
        (this->broadcastTable());
    } else if (type == "T" || type == "t") {
        (this->getTable())->addDirectRouter(ipport);
        // Should share table instantly?
        // std::cout << "friend router sck descp " << this->getSocketDescriptor(ipport) << std::endl;
        this->shareTable(ip, port, this->getSocketDescriptor(ipport));
    }
    // add to connections
    std::pair<int, int> p = std::pair<int, int>(delay,MTU);
    std::pair<std::string, std::pair<int, int>> P = std::pair<std::string, std::pair<int, int>>(ipport, p);
    this->connections.push_back(P);
}

int T::sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type,
                   std::string message, int sd, int sequenceNumber) {
    // std::cout << "sending message..." << std::endl;
    unsigned char* packet = this->makePacket(std::move(ip_src), std::move(port_src), std::move(ip_dest),
                                             std::move(port_dest), type, message, sequenceNumber);
    auto totalLength = (size_t) this->getTotalLength(packet);
    send(sd, packet, totalLength, 0);
    return 0;
}


