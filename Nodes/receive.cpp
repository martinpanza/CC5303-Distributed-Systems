//
// Created by marti on 24-04-2018.
//

#include "receive.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void receive(int sd){
    char buffer[1024] = {0};

    int valread = read(sd, buffer, 1024);

    std::cout << buffer << std::endl;
}