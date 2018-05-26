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
#include "ThreadUtils.h"

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

void receiveTh(Node *n, int sd){
    int buf_size = 1024;
    char buffer[buf_size] = {0};
    int valread = 1;
    while (valread > 0) {
        valread = read(sd, buffer, buf_size);

        auto * to = (char *)malloc(valread * sizeof(char));
        copyBuffer(buffer, &to, valread);
        std::cout << "Received packet" << std::endl;

        (n->mtx).lock();
        (n->message_queue).push_back((unsigned  char*)to);
        (n->mtx).unlock();
    }
}

void sendTh(T *n) {
    unsigned char* packet;
    std::string name;
    std::string ip_src, port_src, ip_dest, port_dest;
    while (1) {
        if (n->iAmAServer){
            n->serverCond.notify_one();
            std::cout << "See ya soon!" << std::endl;
            return;
        } else {
            (n->mtx).lock();
            if (!n->message_queue.empty()) {
                packet = (n->message_queue).front();
                (n->message_queue).pop_front();

                if (n->getType(packet) == TABLE_MESSAGE) {
                    n->processTablePacket(packet);
                    (n->mtx).unlock();
                } else {
                    ip_src = n->getSrcIp(packet);
                    port_src = std::to_string(n->getSrcPort(packet));
                    ip_dest = n->getDestIp(packet);
                    port_dest = std::to_string(n->getDestPort(packet));
                    name = ip_dest;
                    name += ":";
                    name += port_dest;

                    sendOneFragmentedMessage(n, packet, name);

                    (n->mtx).unlock();
                }
            } else {
                (n->mtx).unlock();
            }
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
        if (c->iAmAServer){
            c->serverCond.notify_one();
            std::cout << "Goodbye Everubody!" << std::endl;
            return;
        } else {

            (c->mtx).lock();
            if (!c->message_queue.empty()) {
                packet = (c->message_queue).front();
                (c->message_queue).pop_front();
                (c->mtx).unlock();
                ip = c->getSrcIp(packet);
                port = std::to_string(c->getSrcPort(packet));
                name = ip;
                name += ":";
                name += port;

                cClient(c, packet, name, ip, port);

            } else {
                (c->mtx).unlock();
            }

        }
        sleep(5);
    }
}

void cServerTh(C *c){
    std::cout << "I am a Server" << std::endl;
    unsigned char* packet;
    std::string ipSrc;
    std::string portSrc;
    std::string nameSrc;
    std::string ipDest;
    std::string portDest;
    std::string nameDest;
    while (1){
        if (!c->iAmAServer){
            c->serverCond.notify_one();
            std::cout << "Felt good while being a server" << std::endl;
            return;
        } else {

            (c->mtx).lock();
            if (!c->message_queue.empty()) {
                packet = (c->message_queue).front();
                (c->message_queue).pop_front();
                (c->mtx).unlock();

                ipSrc = c->getSrcIp(packet);
                portSrc = std::to_string(c->getSrcPort(packet));
                nameSrc = ipSrc;
                nameSrc += ":";
                nameSrc += portSrc;

                ipDest = c->getDestIp(packet);
                portDest = std::to_string(c->getDestPort(packet));
                nameDest = ipDest;
                nameDest += ":";
                nameDest += portDest;

                if (nameDest == c->ip + ":" + std::to_string(c->port)) {

                    cClient(c, packet, nameSrc, ipSrc, portSrc);

                } else {

                    cServer(c, packet, nameSrc, nameDest, ipSrc, portSrc, ipDest, portDest);

                }

            } else {
                (c->mtx).unlock();
            }

        }
        sleep(5);
    }
}

void tServerTh(T* n){
    std::cout << "Yes! I'm a server at least!" << std::endl;
    unsigned char* packet;
    int sd;
    std::string usefulRouter;
    std::string ipSrc;
    std::string portSrc;
    std::string nameSrc;
    std::string ipDest;
    std::string portDest;
    std::string nameDest;
    while (1) {
        if (!n->iAmAServer){
            n->serverCond.notify_one();
            std::cout << "Felt good while being a server" << std::endl;
            return;
        } else {
            (n->mtx).lock();
            if (!n->message_queue.empty()) {
                packet = (n->message_queue).front();
                (n->message_queue).pop_front();
                (n->mtx).unlock();

                ipSrc = n->getSrcIp(packet);
                portSrc = std::to_string(n->getSrcPort(packet));
                nameSrc = ipSrc;
                nameSrc += ":";
                nameSrc += portSrc;

                ipDest = n->getDestIp(packet);
                portDest = std::to_string(n->getDestPort(packet));
                nameDest = ipDest;
                nameDest += ":";
                nameDest += portDest;

                //std::cout << "got message of type: " << n->getType(packet) << std::endl;
                if (n->getType(packet) == TABLE_MESSAGE) {
                    n->processTablePacket(packet);
                } else if (n->getType(packet) == CHAT_MESSAGE) {

                    if (n->getFragmentBit(packet)) {
                        int found = 0;
                        for (int i = 0; i < n->serverFragmentedPackets.size(); i++) {
                            if (nameSrc == n->serverFragmentedPackets[i].first.first &&
                                nameDest == n->serverFragmentedPackets[i].first.second) {

                                if (n->getSeqNum(packet) == n->serverFragmentedPackets[i].second.first) {

                                    n->serverFragmentedPackets[i].second.second.push_back(packet);

                                    std::pair<int, std::string> result = n->checkFragmentArrival(
                                            n->serverFragmentedPackets[i].second.second);

                                    if (result.first) {
                                        std::cout << "Paso el mensaje de " << nameSrc << " para " << nameDest
                                                  << std::endl;

                                        usefulRouter = n->searchConnectedRouter(nameSrc);
                                        sd = n->getSocketDescriptor(usefulRouter);

                                        //send ack
                                        sleep(n->getDelay(usefulRouter));
                                        n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc, SACK_MESSAGE,
                                                       std::string(""), sd, n->getSeqNum(packet));

                                        std::cout << "Envie SACK a " << nameSrc << std::endl;

                                        packet = n->makePacket(std::move(ipSrc), std::move(portSrc), std::move(ipDest),
                                                               std::move(portDest), CHAT_MESSAGE, result.second,
                                                               n->getSeqNum(packet));

                                        sendFragmentedMessages(n, nameDest, packet);

                                        std::cout << "Envie mensaje a " << nameDest << std::endl;

                                        n->serverFragmentedPackets.erase(n->serverFragmentedPackets.begin() + i);

                                        n->serverWaitingForAcks.push_back({nameSrc, nameDest});
                                    }

                                }

                                found = 1;
                                break;
                            }
                        }
                        if (found == 0) {
                            std::vector<unsigned char *> v;
                            v.push_back(packet);
                            std::pair<std::pair<std::string, std::string>, std::pair<int, std::vector<unsigned char *>>> newFragmentedPacket = {{nameSrc, nameDest}, {n->getSeqNum(packet) ,v}};
                            n->serverFragmentedPackets.push_back(newFragmentedPacket);
                        }
                    } else {
                        std::cout << "Paso mensaje de " << nameSrc << " para " << nameDest << std::endl;

                        usefulRouter = n->searchConnectedRouter(nameSrc);
                        sd = n->getSocketDescriptor(usefulRouter);

                        //send ack
                        sleep(n->getDelay(usefulRouter));
                        n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc, SACK_MESSAGE,
                                       std::string(""), sd, n->getSeqNum(packet));

                        std::cout << "Envie SACK a " << nameSrc << std::endl;

                        sendFragmentedMessages(n, nameDest, packet);

                        std::cout << "Envie mensaje a " << nameDest << std::endl;

                        n->serverWaitingForAcks.push_back({nameSrc, nameDest});
                    }

                } else {
                    int found = 0;
                    for (int i = 0; i < n->serverWaitingForAcks.size(); i++) {
                        if (nameDest == n->serverWaitingForAcks[i].first &&
                            nameSrc == n->serverWaitingForAcks[i].second) {
                            std::cout << "Paso ACK de " << nameSrc << " para " << nameDest << std::endl;
                            n->serverWaitingForAcks.erase(n->serverWaitingForAcks.begin() + i);

                            usefulRouter = n->searchConnectedRouter(nameSrc);
                            sd = n->getSocketDescriptor(usefulRouter);

                            //send ack
                            sleep(n->getDelay(usefulRouter));
                            n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc, SACK_MESSAGE,
                                           std::string(""), sd, n->getSeqNum(packet));

                            std::cout << "Envie SACK a " << portSrc << std::endl;

                            //send message
                            usefulRouter = n->searchConnectedRouter(nameDest);
                            sd = n->getSocketDescriptor(usefulRouter);

                            sleep(n->getDelay(usefulRouter));
                            send(sd, packet, (size_t) n->getTotalLength(packet), 0);

                            std::cout << "Envie ACK a " << nameDest << std::endl;

                            found = 1;

                            break;
                        }
                    }

                    if (!found) {
                        std::cout << "????" << std::endl;
                    }
                }
            } else {
                (n->mtx).unlock();
            }
        }
        sleep(1);
    }
}




