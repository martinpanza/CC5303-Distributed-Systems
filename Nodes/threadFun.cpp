//
// Created by marti on 24-04-2018.
//

#include "threadFun.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "../utils.h"
#include <thread>


void acceptTh(Node *n, int sd) {
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

        std::thread receiver (receiveTh, n, new_socket);
        receiver.detach();
    }

}

void receiveTh(Node *n, int sd){
    char buffer[1024] = {0};
    int valread = 1;
    while (1) {
        valread = read(sd, buffer, 1024);
        auto to = (char*) malloc(valread * sizeof(char));
        strncpy(to, buffer, valread);
        (n->mtx).lock();
        (n->message_queue).push_back((unsigned  char*)to);
        (n->mtx).unlock();
        // printf("%s\n", buffer);
    }
}

void sendTh(Node *n) {
    unsigned char* packet;
    std::string name;
    std::string ip;
    std::string port;
    std::vector<std::string> usefulRouters;
    while (1) {
        (n->mtx).lock();
        if (!n->message_queue.empty()) {
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();
            (n->mtx).unlock();
            ip = n->getDestIp(packet);
            port = std::to_string(n->getDestPort(packet));
            name = ip + ":" + port;
            usefulRouters = n->searchConnectedRouter(name);
            int sd = n->getSocketDescriptor(usefulRouters.front());
            n->sendMessage(ip, port, n->getType(packet), n->getMessage(packet), sd);
        } else {
            (n->mtx).unlock();
        }
        sleep(1);
    }
}

void cProcessTh(Node *n, int sd) {
    unsigned char* packet;
    while (1){
        (n->mtx).lock();
        if (!n->message_queue.empty()){
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();
            (n->mtx).unlock();
            if (n->getType(packet) == CHAT_MESSAGE){
                std::cout << "llego mensaje de " << n->getDestPort(packet) << " : " << n->getMessage(packet);
                //TODO:send ACK
            } else{
                //TODO: es ACK y desbloquea nodo
            }
        } else{
            (n->mtx).unlock();
        }
        sleep(5);
    }
}

