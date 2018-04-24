//
// Created by marti on 19-04-2018.
//

#include "C.h"
#include "socket.cpp"

size_t splitString(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

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
