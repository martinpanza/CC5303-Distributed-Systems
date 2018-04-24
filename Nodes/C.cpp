//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.h"
#include "receive.h"
#include "../utils.h"
#include <thread>

void C::run() {
    std::string s;
    std::string connect_ = "connect";
    std::string send_ = "send";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[0] == connect_ and words.size() == 3) {
            this->addConnection(words[1], words[2]);
            int sd = create_socket(this->port);
            std::thread receiver (receive, sd);
        }
        else if (words[0] == send_ and words.size() == 4) {
            this->sendMessage(words[1], words[2], CHAT_MESSAGE, words[3]);
        }
    }
}

/*
Hacer header: ip_src, port_ src, ip_dest, port_dest, type, num_seq,
        header_length, total_package_length, fragmented, offset(+refrag_offset), last_bit
offset: de donde parto
*/
std::string C::makeHeader(std::string ip_dest, std::string port_dest, int type) {
    std::string header = "";
    return header;
}

int C::sendMessage(std::string ip_dest, std::string port_dest, int type, std::string message) {
    std::string header = this->makeHeader(ip_dest, port_dest, type);
    return 0;
}

void C::addConnection(std::string ip, std::string port) {
    this->connections.push_back(std::pair<std::string, std::pair<int, int>>(ip + ":" + port , std::pair<int, int>(1,512)));
}
