//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "threadFun.h"
#include "../utils.h"
#include <arpa/inet.h>
#include <thread>
#include <vector>

void C::addConnection(std::string ip, std::string port) {
    this->connections.push_back(std::pair<std::string, std::pair<int, int>>(ip + ":" + port , std::pair<int, int>(1,512)));
    this->getTable()->direct_routers.push_back(ip + ":" + port);
}

int C::sendMessage(std::string ip_src, std::string port_src, std::string ip_dest, std::string port_dest, int type, std::string message, int sd) {
    std::cout << "sending message..." << std::endl;
    unsigned char* packet = this->makePacket(std::move(ip_src), std::move(port_src), std::move(ip_dest), std::move(port_dest), type, message);
    auto totalLength = (size_t) this->getTotalLength(packet);
    std::cout << this->getMessage(packet) << std::endl;
    std::cout << totalLength << std::endl;

    send(sd, packet, totalLength, 0);
    return 0;
}

int C::run() {
    int server_fd = serverSocket(this->port);

    std::thread accepter (acceptTh, this, server_fd);
    accepter.detach();

    std::thread cProcessor (cProcessTh, this, server_fd);
    cProcessor.detach();

    std::string s;
    std::string connect_ = "connect";
    std::string message_ = "message";
    std::vector<std::string> words;
    int client_sd;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[1] == "localhost"){
            words[1] = "127.0.0.1";
        }
        // cliente necesita tener tipo? solo se conecta a otros T
        if (words[0] == connect_ and words.size() >= 3) {
            client_sd = clientSocket(stoi(words[2]));

            this->addConnection(words[1], words[2]);
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));

            std::thread receiver (receiveTh, this, client_sd);
            receiver.detach();


        } else if (words[0] == message_ and words.size() >= 4) {
            std::string m = "";
            for (int i = 3; i < words.size() - 1; i++) {
                m += words[i];
                m += ' ';
            }
            m += words[words.size() - 1];
            this->sendMessage(this->ip, std::to_string(this->port), words[1], words[2], CHAT_MESSAGE, m, client_sd);
            printf("Message sent\n");
        }
    }
}

