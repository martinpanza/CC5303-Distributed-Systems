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
    std::cout << "ready to receive messages" << std::endl;
    char *hello="Hello from server";
    char buffer[1024] = {0};
    int valread;

    valread = read(sd, buffer, 1024);
    printf("%s\n", buffer);
    send(sd, hello, strlen(hello), 0);
    printf("Hello message sent\n");
}