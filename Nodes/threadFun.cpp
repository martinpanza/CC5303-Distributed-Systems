//
// Created by marti on 24-04-2018.
//

#include "threadFun.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "../utils.h"
#include <thread>


void acceptTh(Node n, int sd) {
    while(1){
        int new_socket;
        struct sockaddr_in address;
        int addrlen = sizeof(address);

        if ((new_socket = accept(sd, (struct sockaddr *)&address,
                                 (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::string ip = inet_ntoa(address.sin_addr);
        std::string port = std::to_string(ntohs(address.sin_port));

        std::cout << "connnection established with " << ip << ":" << port << std::endl;

        std::thread receiver (receiveTh, new_socket);
        receiver.detach();
    }

}

void receiveTh(Node *n, int sd){
    char buffer[1024] = {0};
    int valread = 1;
    while (1) {
        valread = read(sd, buffer, 1024);
        n->mtx.lock();
        n->message_queue.push_back(buffer);
        n->mtx.unlock();
        // printf("%s\n", buffer);
    }
}

void sendTh(Node *n) {
    unsigned char* packet;
    std::string name;
    std::vector usefulRouters;
    while (1) {
        n->mtx.lock();
        if (!n->message_queue.empty()) {
            packet = n->message_queue.front();
            n->message_queue.pop_front();
            n->mtx.unlock();
            name = n->getDestIp(packet) + ":" + n->getDestPort(packet);
            usefulRouters = n->searchConnectedRouter(name);
        } else {
            n->mtx.unlock();
        }
    }
}

