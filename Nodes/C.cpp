//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "ThreadFun.h"
#include "../utils.h"
#include <arpa/inet.h>
#include <thread>
#include <vector>

void C::addConnection(std::string ip, std::string port) {
    this->connections.push_back(std::pair<std::string, std::pair<int, int>>(ip + ":" + port , std::pair<int, int>(1,512)));
    this->getTable()->direct_routers.push_back(ip + ":" + port);
}

int C::sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type,
                   std::string message, int sd, int sequenceNumber, int serverBit) {
    //std::cout << "sending message..." << std::endl;
    unsigned char* packet = this->makePacket(std::move(ip_src), std::move(port_src), std::move(ip_dest),
                                             std::move(port_dest), type, message, sequenceNumber, serverBit);
    //std::cout << this->getMessage(packet) << std::endl;
    while(this->getTotalLength(packet) > this->connections.front().second.second){
        std::pair<unsigned char*, unsigned char*> f_packets = this->fragment(packet, this->connections.front().second.second);
        sleep(this->connections.front().second.first);
        send(sd, f_packets.first, (size_t) this->getTotalLength(f_packets.first), 0);
        packet = f_packets.second;
    }
    sleep(this->connections.front().second.first);
    send(sd, packet, (size_t) this->getTotalLength(packet), 0);
    return 0;
}

int C::run() {
    int server_fd = serverSocket(this->port);

    std::thread accepter (acceptTh, this, server_fd);
    accepter.detach();

    std::thread cProcessor (cProcessTh, this);
    cProcessor.detach();

    std::string s;
    std::string connect_ = "connect";
    std::string message_ = "message";
    std::string startServer_ = "start_server";
    std::string stopServer_ = "stop_server";
    std::string backToNormal_ = "back_to_normal";
    std::string migrate_ = "migrate";
    std::vector<std::string> words;
    int client_sd = -1;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[1] == "localhost"){
            words[1] = "127.0.0.1";
        }
        // cliente necesita tener tipo? solo se conecta a otros T
        if (words[0] == connect_ and words.size() >= 3) {
            client_sd = clientSocket(stoi(words[2]));

            if (client_sd == -1) {
                std::cout << "Check if ip and port are correct and try again." << std::endl;
                continue;
            }

            this->addConnection(words[1], words[2]);
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));

            std::thread receiver (receiveTh, this, client_sd);
            receiver.detach();


        } else if (words[0] == message_ and words.size() >= 4) {

            if (client_sd == -1) {
                std::cout << "Try connecting to a router again." << std::endl;
                continue;
            }

            std::string m = "";
            for (int i = 3; i < words.size() - 1; i++) {
                m += words[i];
                m += ' ';
            }
            m += words[words.size() - 1];

            this->sendMessage(this->ip, std::to_string(this->port), words[1], words[2],
                              CHAT_MESSAGE, m, client_sd, this->currentSequenceNumber, 0);
            printf("Message sent\n");

            this->sentMessage = m;
            this->ipSent = words[1];
            this->portSent = words[2];
            this->waitingForSack = 1;
            this->waitingForAck = 1;

            std::unique_lock<std::mutex> lk(this->listen_mutex);
            this->cond.wait(lk);
            lk.unlock();
        } else if (words[0] == startServer_) {

            this->iAmAServer = 1;
            this->off = 0;

            std::unique_lock<std::mutex> lk(this->serverMutex);
            this->serverCond.wait(lk);
            lk.unlock();
            std::cout << "announcing server" << std::endl;
            this->announceServer(this->ip + ":" + std::to_string(this->port), "");

            std::thread server (cServerTh, this);
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

            std::thread cProcessor (cProcessTh, this);
            cProcessor.detach();
        } else if (words[0] == migrate_){
            this->iAmAServer = 0;
            this->off = 0;
            this->migrating = 1;

            if (words[1] == "localhost"){
                words[1] = "127.0.0.1";
            }

            std::unique_lock<std::mutex> lk(this->serverMutex);
            this->serverCond.wait(lk);
            lk.unlock();

            std::thread migrateServer (cMigrateServerTh, this, words[1], words[2], words[3]);
            migrateServer.detach();

            std::unique_lock<std::mutex> lck(this->serverMutex);
            this->serverCond.wait(lck);
            lck.unlock();

            std::thread cProcessor (cProcessTh, this);
            cProcessor.detach();
        }
    }
}

void C::increaseSequenceNumber() {
    this->currentSequenceNumber = (this->currentSequenceNumber + 1) % MAX_SEQ_NUMBER;
}

int C::sendPacket(unsigned char *packet) {
    while(this->getTotalLength(packet) > this->connections.front().second.second){
        std::pair<unsigned char*, unsigned char*> f_packets = this->fragment(packet, this->connections.front().second.second);
        sleep(this->connections.front().second.first);
        send(this->getSocketDescriptor(this->getTable()->direct_routers.front()), f_packets.first, (size_t) this->getTotalLength(f_packets.first), 0);
        packet = f_packets.second;
    }
    sleep(this->connections.front().second.first);
    send(this->getSocketDescriptor(this->getTable()->direct_routers.front()), packet, (size_t) this->getTotalLength(packet), 0);
    return 0;
}
