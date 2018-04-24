//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "../utils.h"

void C::run() {
    std::string s;
    std::string connect_ = "connect";
    std::string send_ = "send";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[0] == connect_ and words.size() == 3) {
            this->addConnection(words[1], words[2]);
            int sd = create_socket();
        }
        else if (words[0] == send_ and words.size() == 4) {
            this->sendMessage(words[1], words[2], words[3]);
        }
    }
}

int C::sendMessage(std::string ip, std::string port, std::string message) {
    return 0;
}

void C::addConnection(std::string ip, std::string port) {
    this->connections.push_back(std::pair<std::string, std::pair<int, int>>(ip + ":" + port , std::pair<int, int>(1,512)));
}
