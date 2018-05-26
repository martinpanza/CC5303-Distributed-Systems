//
// Created by martin on 24-05-18.
//

#include "ThreadUtils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

void copyBuffer(const char* buffer, char** to, int size) {
    for (int i = 0; i < size; i++) {
        (*to)[i] = (unsigned char) buffer[i];
    }
}

void sendOneFragmentedMessage(T *n, unsigned char *packet, std::string name) {
    std::string usefulRouter = n->searchConnectedRouter(name);
    int sd = n->getSocketDescriptor(usefulRouter);

    if (n->getTotalLength(packet) > n->getMTU(usefulRouter)) {
        std::pair<unsigned char *, unsigned char *> f_packets = n->fragment(packet,
                                                                            n->getMTU(usefulRouter));
        packet = f_packets.first;
        n->message_queue.push_front(f_packets.second);
    }
    sleep((unsigned int) n->getDelay(usefulRouter));
    send(sd, packet, n->getTotalLength(packet), 0);
}

void cServer(C* c, unsigned char* packet, std::string nameSrc, std::string nameDest, std::string ipSrc,
             std::string portSrc, std::string ipDest, std::string portDest) {


    if (c->getType(packet) == CHAT_MESSAGE) {

        if (c->getFragmentBit(packet)) {
            int found = 0;
            for (int i = 0; i < c->serverFragmentedPackets.size(); i++) {
                if (nameSrc == c->serverFragmentedPackets[i].first.first &&
                    nameDest == c->serverFragmentedPackets[i].first.second
                    // && TODO: ver que el mensaje no este en la lista de ACK que se esperan
                        ) {
                    c->serverFragmentedPackets[i].second.push_back(packet);

                    std::pair<int, std::string> result = c->checkFragmentArrival(
                            c->fragmentedPackets[i].second);

                    if (result.first) {
                        std::cout << "Paso el mensaje de " << nameSrc << " para " << nameDest << std::endl;
                        //send ack
                        sleep(c->connections.front().second.first);
                        c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                                       std::string(""),
                                       c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                                       c->getSeqNum(packet));
                        //send message
                        sleep(c->connections.front().second.first);
                        c->sendMessage(ipSrc, portSrc, ipDest, portDest, CHAT_MESSAGE, result.second,
                                       c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                                       c->getSeqNum(packet));
                        c->serverFragmentedPackets.erase(c->serverFragmentedPackets.begin() + i);

                        c->serverWaitingForAcks.push_back({nameSrc, nameDest});
                    }

                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                std::vector<unsigned char *> v;
                v.push_back(packet);
                std::pair<std::pair<std::string, std::string>, std::vector<unsigned char *>> newFragmentedPacket = {{nameSrc, nameDest}, v};
                c->serverFragmentedPackets.push_back(newFragmentedPacket);
            }
        } else {
            std::cout << "Paso mensaje de " << nameSrc << " para " << nameDest << std::endl;
            c->serverWaitingForAcks.push_back({nameSrc, nameDest});

            //send ack
            sleep(c->connections.front().second.first);
            c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                           std::string(""),
                           c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet));

            //Send Packet
            while(c->getTotalLength(packet) > c->connections.front().second.second){
                std::pair<unsigned char*, unsigned char*> f_packets = c->fragment(packet, c->connections.front().second.second);
                sleep(c->connections.front().second.first);
                send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), f_packets.first, (size_t) c->getTotalLength(f_packets.first), 0);
                packet = f_packets.second;
            }
            sleep(c->connections.front().second.first);
            send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);
        }

    } else {
        int found = 0;
        for (int i = 0; i < c->serverWaitingForAcks.size(); i++) {
            if (nameSrc == c->serverWaitingForAcks[i].first &&
                nameDest == c->serverWaitingForAcks[i].second) {
                std::cout << "Paso ACK de " << nameSrc << " para " << nameDest << std::endl;
                c->serverWaitingForAcks.erase(c->serverWaitingForAcks.begin() + i);

                //send ack
                sleep(c->connections.front().second.first);
                c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                               std::string(""),
                               c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet));

                //Send packet
                sleep(c->connections.front().second.first);
                send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);
                found = 1;

                break;
            }
        }

        if (!found){
            std::cout << "????" << std::endl;
        }
    }
}

void cClient(C* c, unsigned char* packet, std::string nameSrc, std::string ipSrc, std::string portSrc) {
    if (c->getType(packet) == CHAT_MESSAGE) {

        if (c->getFragmentBit(packet)) {
            int found = 0;
            for (int i = 0; i < c->fragmentedPackets.size(); i++) {
                if (nameSrc == c->fragmentedPackets[i].first) {
                    c->fragmentedPackets[i].second.push_back(packet);

                    std::pair<int, std::string> result = c->checkFragmentArrival(
                            c->fragmentedPackets[i].second);
                    if (result.first) {
                        std::cout << "Llego mensaje de " << nameSrc << "->" << result.second << std::endl;
                        sleep((unsigned int) c->connections.front().second.first);
                        c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, ACK_MESSAGE,
                                       std::string(""),
                                       c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                                       c->getSeqNum(packet));
                        c->fragmentedPackets.erase(c->fragmentedPackets.begin() + i);
                        c->sentAcks.insert(std::pair<std::string, int>(std::to_string('test'),1));
                    }

                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                std::vector<unsigned char *> v;
                v.push_back(packet);
                std::pair<std::string, std::vector<unsigned char *>> newFragmentedPacket = {nameSrc, v};
                c->fragmentedPackets.push_back(newFragmentedPacket);
            }
        } else {
            std::cout << "Llego mensaje de " << nameSrc << "->" << c->getMessage(packet) << std::endl;
            sleep((unsigned int) c->connections.front().second.first);
            c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, ACK_MESSAGE, std::string(""),
                           c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet));
            c->sentAcks.insert(std::pair<std::string, int>(std::to_string('test'),1));
        }
    } else if (c->getType(packet) == SACK_MESSAGE) {
        c->waitingForSack = 0;
        c->sentAcks.erase(std::to_string('test'));
        std::cout << "Su mensaje ha llegado al servidor" << std::endl;
    } else {
        c->waitingForSack = 0;
        c->waitingForAck = 0;
        c->sentMessage = "";
        std::cout << "Su mensaje ha sido recibido" << std::endl;
        c->cond.notify_one();
    }
}

void sendFragmentedMessages(T* n, std::string nameDest, unsigned char* packet){
    std::string usefulRouter = n->searchConnectedRouter(nameDest);
    int sd = n->getSocketDescriptor(usefulRouter);

    while(n->getTotalLength(packet) > n->getMTU(usefulRouter)){
        std::pair<unsigned char*, unsigned char*> f_packets = n->fragment(packet, n->getMTU(usefulRouter));
        sleep(n->getDelay(usefulRouter));
        send(sd, f_packets.first, (size_t) n->getTotalLength(f_packets.first), 0);
        packet = f_packets.second;
        usefulRouter = n->searchConnectedRouter(nameDest);
        sd = n->getSocketDescriptor(usefulRouter);
    }
    sleep(n->getDelay(usefulRouter));
    send(sd, packet, (size_t) n->getTotalLength(packet), 0);
}