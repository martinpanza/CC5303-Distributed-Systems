//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "threadFun.h"
#include "../utils.h"
#include <arpa/inet.h>
#include <thread>

int C::run() {
    int server_fd = serverSocket(this->port);

    std::thread accepter (acceptTh, *this, server_fd);
    accepter.detach();

    std::string s;
    std::string connect_ = "connect";
    std::string message_ = "message";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[0] == connect_ and words.size() == 3) {
            int client_sd = clientSocket(stoi(words[2]));

            this->addConnection(words[1], words[2]);
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));

            std::thread receiver (receiveTh, client_sd);
            receiver.detach();


        } else if (words[0] == message_ and words.size() == 4) {
            //this->sendMessage(words[1], words[2], CHAT_MESSAGE, words[3]);

            char *hello="Hello from server";
            send(this->socketDescriptors.front().first, hello, strlen(hello), 0);
            printf("Hello message sent\n");
        }

    }
}

int C::sendMessage(std::string ip_dest, std::string port_dest, int type, std::string message) {
    //std::string header = this->makeHeader(ip_dest, port_dest, type);
    std::cout << "send message" << std::endl;
    return 0;
}

void C::addConnection(std::string ip, std::string port) {
    this->connections.push_back(std::pair<std::string, std::pair<int, int>>(ip + ":" + port , std::pair<int, int>(1,512)));
}
