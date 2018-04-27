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
#include <sgtty.h>


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

        std::cout << "connnection established" << std::endl;

        std::thread receiver (receiveTh, n, new_socket);
        receiver.detach();
    }

}

void copyBuffer(char* buffer, char** to, int size) {
    for (int i = 0; i < size; i++) {
        (*to)[i] = (unsigned char) buffer[i];
    }
}

void receiveTh(Node *n, int sd){
    char buffer[1024] = {0};
    int valread = 1;
    while (valread > 0) {
        valread = read(sd, buffer, 1024);

        char * to = (char *)malloc(valread * sizeof(char));
        copyBuffer(buffer, &to, valread);
        n->printPacket((unsigned char*)to);

        (n->mtx).lock();
        (n->message_queue).push_back((unsigned  char*)to);
        (n->mtx).unlock();
    }
}

void sendTh(Node *n) {
    unsigned char* packet;
    std::string name;
    std::string ip_src, port_src, ip_dest, port_dest;
    std::vector<std::string> usefulRouters;
    while (1) {
        (n->mtx).lock();
        if (!n->message_queue.empty()) {
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();
            (n->mtx).unlock();

            ip_src = n->getSrcIp(packet);
            port_src = std::to_string(n->getSrcPort(packet));
            ip_dest = n->getDestIp(packet);
            port_dest = std::to_string(n->getDestPort(packet));
            name = ip_dest + ":" + port_dest;

            std::cout << "Searching for Routers..." << std::endl;
            usefulRouters = n->searchConnectedRouter(name);

            int sd = n->getSocketDescriptor(usefulRouters.front());
            send(sd, packet, n->getTotalLength(packet), 0);
        } else {
            (n->mtx).unlock();
        }
        sleep(1);
    }
}

void cProcessTh(Node *n, int sd) {
    unsigned char* packet;
    std::string ip;
    std::string port;
    std::string name;
    while (1){
        (n->mtx).lock();
        if (!n->message_queue.empty()){
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();
            (n->mtx).unlock();
            if (n->getType(packet) == CHAT_MESSAGE){
                ip = n->getSrcIp(packet);
                port = std::to_string(n->getSrcPort(packet));
                name = ip + ":" + port;
                std::cout << "llego mensaje de " << name << "->" << n->getMessage(packet) << std::endl;
                std::cout << n->getTable()->direct_routers.front() << std::endl;
                n->sendMessage(n->ip, std::to_string(n->port), ip, port, ACK_MESSAGE, std::string(""), n->getSocketDescriptor(n->getTable()->direct_routers.front()));
            } else {
                std::cout << "llego Ack" << std::endl;
                n->cond.notify_one();
            }
        } else{
            (n->mtx).unlock();
        }
        sleep(5);
    }
}

