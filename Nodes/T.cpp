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

void T::add_connection(std::string ip, std::string port) {
    int delay = random_int(2, 10);
    int MTU = random_mtu();
    std::pair<int, int> p = std::pair<int, int>(delay,MTU);
    std::pair<std::string, std::pair<int, int>> P = std::pair<std::string, std::pair<int, int>>(ip + ":" + port, p);
    this->connections.push_back(P);
}
