//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "threadFun.h"
#include "../utils.h"
#include <thread>

int C::run() {
    std::string s;
    std::string connect_ = "connect";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[0] == connect_ and words.size() == 3) {
            this->addConnection(words[1], words[2]);
            int sd = create_socket(this->port);
            std::thread sender (sendTh, *this, sd);
            std::thread receiver (receiveTh, sd);
            std::cout << "Sender and Receiver are now executing concurrently" << std::endl;
            sender.join();
            receiver.join();
            std::cout << "Sender and Receiver Completed" << std::endl;
            return 0;


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
