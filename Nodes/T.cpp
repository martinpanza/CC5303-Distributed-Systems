//
// Created by marti on 19-04-2018.
//

#include "T.h"
#include <random>
#include <cmath>
#include <climits>
#include <stdlib.h>
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

void T::add_connection(std::string name) {
    int delay = random_int(2, 10);
    int MTU = random_mtu();
    this->connections.push_back(new std::vector(new std::pair(delay, MTU)));
}
