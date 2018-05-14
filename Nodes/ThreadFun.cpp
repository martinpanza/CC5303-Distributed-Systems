//
// Created by marti on 24-04-2018.
//

#include "ThreadFun.h"
#include <iostream>

#include <stdio.h>
#include <unistd.h>

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

        std::cout << "connection established" << std::endl;

        std::thread receiver (receiveTh, n, new_socket);
        receiver.detach();
    }

}

void copyBuffer(const char* buffer, char** to, int size) {
    for (int i = 0; i < size; i++) {
        (*to)[i] = (unsigned char) buffer[i];
    }
}

void receiveTh(Node *n, int sd){
    int buf_size = 1024;
    char buffer[buf_size] = {0};
    int valread = 1;
    while (valread > 0) {
        valread = read(sd, buffer, buf_size);

        auto * to = (char *)malloc(valread * sizeof(char));
        copyBuffer(buffer, &to, valread);
        std::cout << "Received packet" << std::endl;
        //n->printPacket((unsigned char*)to);
        n->getMessage((unsigned char*) to);

        (n->mtx).lock();
        (n->message_queue).push_back((unsigned  char*)to);
        (n->mtx).unlock();
    }
}

void sendTh(T *n) {
    unsigned char* packet;
    std::string name;
    std::string ip_src, port_src, ip_dest, port_dest;
    std::string usefulRouter;
    while (1) {
        (n->mtx).lock();
        if (!n->message_queue.empty()) {
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();

            //std::cout << "got message of type: " << n->getType(packet) << std::endl;
            if (n->getType(packet) == TABLE_MESSAGE) {
                n->processTablePacket(packet);
                //std::cout << "Printing table" << std::endl;
                //n->getTable()->printTable();
                (n->mtx).unlock();
            } else {
                ip_src = n->getSrcIp(packet);
                port_src = std::to_string(n->getSrcPort(packet));
                ip_dest = n->getDestIp(packet);
                port_dest = std::to_string(n->getDestPort(packet));
                name = ip_dest;
                name += ":";
                name += port_dest;

                //std::cout << "Searching for Routers..." << std::endl;
                usefulRouter = n->searchConnectedRouter(name);
                //std::cout << "useful router: " << usefulRouter << std::endl;
                int sd = n->getSocketDescriptor(usefulRouter);

                if (n->getTotalLength(packet) > n->getMTU(usefulRouter)) {
                    //std::cout << "fragmenting. plen: " << n->getTotalLength(packet) << ". MTU: " << n->getMTU(usefulRouter) << std::endl;
                    std::pair<unsigned char *, unsigned char *> f_packets = n->fragment(packet, n->getMTU(usefulRouter));
                    packet = f_packets.first;
                    n->message_queue.push_front(f_packets.second);
                }
                (n->mtx).unlock();
                sleep((unsigned int) n->getDelay(usefulRouter));
                send(sd, packet, n->getTotalLength(packet), 0);
            }
        } else {
            (n->mtx).unlock();
        }
        sleep(1);
    }
}

void cProcessTh(C *c) {
    unsigned char* packet;
    std::string ip;
    std::string port;
    std::string name;
    while (1){
        (c->mtx).lock();
        if (!c->message_queue.empty()){
            packet = (c->message_queue).front();
            (c->message_queue).pop_front();
            (c->mtx).unlock();
            if (c->getType(packet) == CHAT_MESSAGE){
                ip = c->getSrcIp(packet);
                port = std::to_string(c->getSrcPort(packet));
                name = ip;
                name += ":";
                name += port;

                if (c->getFragmentBit(packet)){
                    int found = 0;
                    for (int i = 0; i < c->fragmentedPackets.size(); i++){
                        if (name == c->fragmentedPackets[i].first){
                            c->fragmentedPackets[i].second.push_back(packet);

                            std::pair<int, std::string> result = c->checkFragmentArrival(c->fragmentedPackets[i].second);
                            if (result.first){
                                std::cout << "Llego mensaje de " << name << "->" << result.second << std::endl;
                                sleep((unsigned int) c->connections.front().second.first);
                                c->sendMessage(c->ip, std::to_string(c->port), ip, port, ACK_MESSAGE, std::string(""),
                                               c->getSocketDescriptor(c->getTable()->direct_routers.front()));
                                c->fragmentedPackets.erase (c->fragmentedPackets.begin()+i);
                            }

                            found = 1;
                            break;
                        }
                    }
                    if(found == 0){
                        std::vector<unsigned char*> v;
                        v.push_back(packet);
                        std::pair<std::string, std::vector<unsigned char *>> newFragmentedPacket = {name, v};
                        c->fragmentedPackets.push_back(newFragmentedPacket);
                    }
                } else {
                    std::cout << "Llego mensaje de " << name << "->" << c->getMessage(packet) << std::endl;
                    sleep((unsigned int) c->connections.front().second.first);
                    c->sendMessage(c->ip, std::to_string(c->port), ip, port, ACK_MESSAGE, std::string(""),
                                   c->getSocketDescriptor(c->getTable()->direct_routers.front()));
                }
            } else {
                std::cout << "Su mensaje ha sido recibido" << std::endl;
                c->cond.notify_one();
            }
        } else{
            (c->mtx).unlock();
        }
        sleep(5);
    }
}


