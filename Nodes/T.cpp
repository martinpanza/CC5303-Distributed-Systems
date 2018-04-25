//
// Created by marti on 19-04-2018.
//

#include "T.h"
#include <random>
#include <time.h>
#include "socket.h"
#include "threadFun.h"
#include "../utils.h"
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
        if (words[0] == connect_ and words.size() == 3) {
            int client_sd = clientSocket(stoi(words[2]));

            this->addConnection(words[1], words[2]);
            this->socketDescriptors.push_back(std::pair<int, std::string>(client_sd, words[1] + ":" + words[2]));

            std::thread receiver (receiveTh, client_sd);
            receiver.detach();
        }

    }
}

void T::addConnection(std::string ip, std::string port) {
    int delay = random_int(2, 10);
    int MTU = random_mtu();
    std::pair<int, int> p = std::pair<int, int>(delay,MTU);
    std::pair<std::string, std::pair<int, int>> P = std::pair<std::string, std::pair<int, int>>(ip + ":" + port, p);
    this->connections.push_back(P);
}

int T::sendMessage(std::string ip_dest, std::string port_dest, int type, std::string message) {
    return 0;
}
