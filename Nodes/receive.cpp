//
// Created by marti on 24-04-2018.
//

#include "receive.h"

void receive(int sd){
    char buffer[1024] = {0};

    valread = read(sd, buffer, 1024);
}