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
#include <algorithm>
#include <thread>
#include "ThreadFun.h"

void copyBuffer(const char* buffer, char** to, int size) {
    for (int i = 0; i < size; i++) {
        (*to)[i] = (unsigned char) buffer[i];
    }
}

void sendOneFragmentedMessage(T *n, unsigned char *packet, std::string name) {
    int sd;
    int type = n->getType(packet);
    std::string usefulRouter;
    if ((type == ACK_MESSAGE || type == CHAT_MESSAGE || type == MIGRATE_MESSAGE) && !n->getServerBit(packet)) {
        std::cout << "buscar path" << std::endl;
        usefulRouter = n->searchPathToServer();
        sd = n->getSocketDescriptor(usefulRouter);

    } else {
        std::cout << "no buscar path" << std::endl;
        usefulRouter = n->searchConnectedRouter(name);
        sd = n->getSocketDescriptor(usefulRouter);
    }

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
    int serverBit = 0;
    if (c->iAmAServer) {
        serverBit = 1;
    }
    c->setServerBit(packet, serverBit);

    if (c->getType(packet) == CHAT_MESSAGE) {
       if (std::find(c->serverWaitingForAcks.begin(), c->serverWaitingForAcks.end(), std::pair<std::string,
               std::string> {nameSrc, nameDest}) != c->serverWaitingForAcks.end()){
           c->sendPacket(packet);
       } else {
           if (c->getFragmentBit(packet)) {
               int found = 0;
               for (int i = 0; i < c->serverFragmentedPackets.size(); i++) {
                   if (nameSrc == c->serverFragmentedPackets[i].first.first &&
                       nameDest == c->serverFragmentedPackets[i].first.second) {

                       if (c->getSeqNum(packet) == c->serverFragmentedPackets[i].second.first) {

                           c->serverFragmentedPackets[i].second.second.push_back(packet);

                           std::pair<int, std::string> result = c->checkFragmentArrival(
                                   c->serverFragmentedPackets[i].second.second);

                           if (result.first) {
                               std::cout << "Paso el mensaje de " << nameSrc << " para " << nameDest << std::endl;
                               //send ack
                               sleep(c->connections.front().second.first);
                               c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                                              std::string(""),
                                              c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                                              c->getSeqNum(packet), serverBit);
                               //send message
                               sleep(c->connections.front().second.first);
                               c->sendMessage(ipSrc, portSrc, ipDest, portDest, CHAT_MESSAGE, result.second,
                                              c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                                              c->getSeqNum(packet), c->getServerBit(packet));
                               c->serverFragmentedPackets.erase(c->serverFragmentedPackets.begin() + i);

                               c->serverWaitingForAcks.push_back({nameSrc, nameDest});
                           }
                       }

                       found = 1;
                       break;
                   }
               }
               if (found == 0) {
                   std::vector<unsigned char *> v;
                   v.push_back(packet);
                   std::pair<std::pair<std::string, std::string>, std::pair<int, std::vector<unsigned char *>>> newFragmentedPacket = {{nameSrc,        nameDest},
                                                                                                                                       {c->getSeqNum(
                                                                                                                                               packet), v}};
                   c->serverFragmentedPackets.push_back(newFragmentedPacket);
               }
           } else {
               std::cout << "Paso mensaje de " << nameSrc << " para " << nameDest << std::endl;
               c->serverWaitingForAcks.push_back({nameSrc, nameDest});

               //send ack
               sleep(c->connections.front().second.first);
               c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                              std::string(""),
                              c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet), c->getServerBit(packet));

               //Send Packet
               /*
               while(c->getTotalLength(packet) > c->connections.front().second.second){
                   std::pair<unsigned char*, unsigned char*> f_packets = c->fragment(packet, c->connections.front().second.second);
                   sleep(c->connections.front().second.first);
                   send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), f_packets.first, (size_t) c->getTotalLength(f_packets.first), 0);
                   packet = f_packets.second;
               }
               sleep(c->connections.front().second.first);
               send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);*/

               c->sendPacket(packet);
           }
       }

    } else if (c->getType(packet) == ACK_MESSAGE){
        int found = 0;
        for (int i = 0; i < c->serverWaitingForAcks.size(); i++) {
            if (nameDest == c->serverWaitingForAcks[i].first &&
                nameSrc == c->serverWaitingForAcks[i].second) {
                std::cout << "Paso ACK de " << nameSrc << " para " << nameDest << std::endl;
                c->serverWaitingForAcks.erase(c->serverWaitingForAcks.begin() + i);

                //send ack
                sleep(c->connections.front().second.first);
                c->sendMessage(c->ip, std::to_string(c->port), ipSrc, portSrc, SACK_MESSAGE,
                               nameDest,
                               c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet), serverBit);

                //Send packet
                sleep(c->connections.front().second.first);
                send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);
                found = 1;

                break;
            }
        }

        if (!found){
            //send it anyway
            std::cout << "unknown ACK, letting it pass anyway" << std::endl;
            sleep(c->connections.front().second.first);
            send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);
        }
    } else {
        sleep(c->connections.front().second.first);
        send(c->getSocketDescriptor(c->getTable()->direct_routers.front()), packet, (size_t) c->getTotalLength(packet), 0);
    }
}

void cClient(C* c, unsigned char* packet, std::string nameSrc, std::string ipSrc, std::string portSrc) {
    int serverBit = 0;
    if (c->iAmAServer) {
        serverBit = 1;
    }
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
                                       c->getSeqNum(packet), serverBit);
                        c->fragmentedPackets.erase(c->fragmentedPackets.begin() + i);
                        c->sentAcks.push_back(std::pair<std::string, int>{nameSrc, c->getSeqNum(packet)});
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
                           c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->getSeqNum(packet),
                           serverBit);
            c->sentAcks.push_back(std::pair<std::string, int>{nameSrc, c->getSeqNum(packet)});
        }
    } else if (c->getType(packet) == SACK_MESSAGE) {
        if (c->getMessage(packet) == "") {
            c->waitingForSack = 0;
            std::cout << "Su mensaje ha llegado al servidor" << std::endl;
        } else {
            for (int i = 0; i < c->sentAcks.size(); i++) {
                if (c->getMessage(packet) == c->sentAcks[i].first) {
                    std::cout << "Su ACK para " << c->getMessage(packet) << "ha llegado al servidor" << std::endl;
                    c->sentAcks.erase(c->sentAcks.begin() + i);
                }
            }
        }
    } else if (c->getType(packet) == RESEND_MESSAGE) {
        if (c->waitingForSack) {
            c->increaseSequenceNumber();
            c->sendMessage(c->ip, std::to_string(c->port), c->ipSent, c->portSent,
                           CHAT_MESSAGE, c->sentMessage,
                           c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->currentSequenceNumber,
                           serverBit);
        }
        c->resendAcks(serverBit);
    } else if (c->getType(packet) == NACK_MESSAGE) {
        c->waitingForAck = 0;
        c->waitingForSack = 0;
        std::cout << c->ipSent << ":" << c->portSent << " is down" << std::endl;
        c->sentMessage = "";
        c->ipSent = "";
        c->portSent = "";
        c->increaseSequenceNumber();
        c->cond.notify_one();

    } else if (c->getType(packet) == NEW_SRV_MESSAGE) {
        std::cout << "NEW_SRV_MESSAGE received" << std::endl;
    } else if (c->getType(packet) == MIGRATE_MESSAGE) {
        if (c->getFragmentBit(packet)) {
            int found = 0;
            for (int i = 0; i < c->fragmentedPackets.size(); i++) {
                if (nameSrc == c->fragmentedPackets[i].first) {
                    c->fragmentedPackets[i].second.push_back(packet);

                    std::pair<int, std::string> result = c->checkFragmentArrival(
                            c->fragmentedPackets[i].second);
                    if (result.first) {
                        std::cout << "Llego mensaje de migracion: " << result.second << std::endl;
                        std::string m = result.second;
                        processMigrateMessage(c, m);

                        c->fragmentedPackets.erase(c->fragmentedPackets.begin() + i);

                        c->getTable()->prepareNewServer();
                        c->announceServer(c->ip + ":" + std::to_string(c->port), "");
                        c->iAmAServer = 1;
                        std::thread server (cServerTh, c);
                        server.detach();
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
            std::cout << "Llego mensaje de migracion: " << c->getMessage(packet) << std::endl;
            processMigrateMessage(c, c->getMessage(packet));
            c->getTable()->prepareNewServer();
            c->announceServer(c->ip + ":" + std::to_string(c->port), "");
            c->iAmAServer = 1;
            std::thread server (cServerTh, c);
            server.detach();
        }
    } else {
        if (c->getSeqNum(packet) == c->currentSequenceNumber){
            c->waitingForSack = 0;
            c->waitingForAck = 0;
            c->sentMessage = "";
            c->ipSent = "";
            c->portSent = "";
            std::cout << "Su mensaje ha sido recibido" << std::endl;
            c->increaseSequenceNumber();
            c->cond.notify_one();
        } else {
            //ignore
        }
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

void sendThroughRouter(T* n, std::string name, unsigned char* packet){
    int sd = n->getSocketDescriptor(name);

    while(n->getTotalLength(packet) > n->getMTU(name)){
        std::pair<unsigned char*, unsigned char*> f_packets = n->fragment(packet, n->getMTU(name));
        sleep(n->getDelay(name));
        send(sd, f_packets.first, (size_t) n->getTotalLength(f_packets.first), 0);
        packet = f_packets.second;
    }
    sleep(n->getDelay(name));
    send(sd, packet, (size_t) n->getTotalLength(packet), 0);
}

std::vector<std::string> getResendList(Node* n){
    std::string nameSrc;
    std::vector<std::string> resend;
    for (auto element: n->serverFragmentedPackets){
        resend.push_back(element.first.first);
    }
    for (auto element: n->serverWaitingForAcks){
        resend.push_back(element.second);
    }
    resend.erase( unique( resend.begin(), resend.end() ), resend.end() );
    return resend;
}

void tSendResendMessages(std::vector<std::string> resend, T* n){
    std::string usefulRouter;
    std::vector<std::string> split;
    int serverBit = 0;
    if (n->iAmAServer) {
        serverBit = 1;
    }
    int sd;
    for (auto nameSrc: resend){
        usefulRouter = n->searchConnectedRouter(nameSrc);
        sd = n->getSocketDescriptor(usefulRouter);

        splitString(nameSrc, split, ':');

        sleep(n->getDelay(usefulRouter));
        n->sendMessage(n->ip, std::to_string(n->port), split[0], split[1], RESEND_MESSAGE,
                       std::string(""), sd, 0, serverBit);
    }
}

void cSendResendMessages(std::vector<std::string> resend, C* c){
    std::string usefulRouter;
    std::vector<std::string> split;
    int serverBit = 0;
    if (c->iAmAServer) {
        serverBit = 1;
    }
    for (auto nameSrc: resend){
        splitString(nameSrc, split, ':');

        sleep(c->connections.front().second.first);
        c->sendMessage(c->ip, std::to_string(c->port), split[0], split[1], RESEND_MESSAGE,
                       std::string(""),
                       c->getSocketDescriptor(c->getTable()->direct_routers.front()),
                       0, serverBit);
    }
}

void increaseExpectedSeqNumber(Node *n) {
    for (int i = 0; i < n->serverFragmentedPackets.size(); i++){
        std::cout << "Before expecting seqNUm " << n->serverFragmentedPackets[i].second.first << std::endl;
        n->serverFragmentedPackets[i].second.first = (n->serverFragmentedPackets[i].second.first + 1) %  MAX_SEQ_NUMBER;
        std::cout << "Now expecting seqNUm " << n->serverFragmentedPackets[i].second.first << std::endl;
        n->serverFragmentedPackets[i].second.second.clear();
        std::cout << "Size: " << n->serverFragmentedPackets[i].second.second.size() << std::endl;

    }
}

void processMigrateMessage(Node *n, std::string m) {
    std::vector<std::string> server_vectors;
    std::vector<std::string> serverFragmentedPackets;
    std::vector<std::string> serverWaitingForACKS;
    if (m == "$"){
        return;
    }
    splitString(m, server_vectors, '$');
    std::cout << "first split" << std::endl;
    splitString(server_vectors[0], serverFragmentedPackets, ';');
    std::cout << "first for " << server_vectors[0] << std::endl;
    for (auto fragmented: serverFragmentedPackets){
        if (fragmented == ""){
            continue;
        }
        std::vector<std::string> row;
        std::vector<unsigned char*> v;
        splitString(fragmented, row, ',');
        std::cout << row[0] << " " << row[1] << " " << row[2] << std::endl;
        n->serverFragmentedPackets.push_back({{row[0], row[1]}, {stoi(row[2]), v}});
    }
    std::cout << "second split" << std::endl;
    splitString(server_vectors[1], serverWaitingForACKS, ';');
    std::cout << "second for" << std::endl;
    for (auto acks: serverWaitingForACKS){
        if (acks == ""){
            continue;
        }
        std::vector<std::string> row;
        splitString(acks, row, ',');
        std::cout << row[0] << " " << row[1] << std::endl;
        n->serverWaitingForAcks.push_back({row[0], row[1]});
    }
}
