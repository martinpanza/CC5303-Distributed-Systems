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
#include "Node.h"

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
        std::cout << "Received packet: " << valread << std::endl;

        if (valread == 0){
            std::string name = n->getNameBySocketDescriptor(sd);
            n->getTable()->direct_clients.erase(std::remove(n->getTable()->direct_clients.begin(),
                                                            n->getTable()->direct_clients.end(), name),
                                                n->getTable()->direct_clients.end());
            n->downC.push_back(name);
            return;
        }

        (n->mtx).lock();
        (n->message_queue).push_back((unsigned  char*)to);
        (n->mtx).unlock();
    }
}

void sendTh(T *n) {
    unsigned char* packet;
    std::string nameSrc, nameDest;
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
                (n->mtx).unlock();

                ip_src = n->getSrcIp(packet);
                port_src = std::to_string(n->getSrcPort(packet));
                ip_dest = n->getDestIp(packet);
                port_dest = std::to_string(n->getDestPort(packet));
                nameSrc = ip_src;
                nameSrc += ":";
                nameSrc += port_src;
                nameDest = ip_dest;
                nameDest += ":";
                nameDest += port_dest;

                if (n->checkC(nameDest)){
                    std::cout << "C is down" << std::endl;
                    std::string usefulRouter = n->searchConnectedRouter(nameSrc);
                    int sd = n->getSocketDescriptor(usefulRouter);

                    //send nack
                    sleep(n->getDelay(usefulRouter));
                    n->sendMessage(n->ip, std::to_string(n->port), ip_src, port_src, NACK_MESSAGE,
                                   std::string(""), sd, 0, 1);
                    continue;
                }

                if (n->getType(packet) == TABLE_MESSAGE) {
                    n->processTablePacket(packet);
                } else if (n->getType(packet) == NEW_SRV_MESSAGE) {
                    std::cout << "NEW SERVER MESSAGE" << std::endl;
                    n->processServerMessage(packet);
                } else if (n->getType(packet) == MIGRATE_MESSAGE) {
                    std::cout << "NEW MIGRATE MESSAGE Dest: " << nameDest << " myName: " << n->ip + ":" + std::to_string(n->port) << std::endl;
                    if (n->getMessage(packet) == "") {
                        std::cout << "it is empty" << std::endl;
                        if (nameDest != n->ip + ":" + std::to_string(n->port)){
                            std::cout << "for me" << std::endl;
                            unsigned char * packet = n->makePacket(n->ip, std::to_string(n->port), ip_dest, port_dest, MIGRATE_MESSAGE, "", 0, 0);
                            for (auto element: n->getTable()->direct_routers){
                                if (element == nameSrc){
                                    continue;
                                }
                                sleep((unsigned int) n->getDelay(element));
                                int sd = n->getSocketDescriptor(element);
                                send(sd, packet, (size_t) n->getTotalLength(packet), 0);
                            }
                        } else {
                            n->getTable()->prepareNewServer();
                            n->announceServer(n->ip + ":" + std::to_string(n->port), "", 0);
                        }
                    } else {
                        std::cout << "it is not empty" << std::endl;
                        if (nameDest != n->ip + ":" + std::to_string(n->port)) {
                            std::cout << "not for me" << std::endl;
                            int isAClient = 0;
                            std::vector<std::string> ipport;
                            auto* reachableClients = n->getTable()->getReachableClients();
                            auto* directClients = n->getTable()->getDirectClients();
                            for (int i = 0; i < directClients->size(); i++) {
                                if ((*directClients)[i] == nameDest) {
                                    isAClient = 1;
                                    std::cout << "it is for a direct client!" << std::endl;
                                    splitString((*directClients)[i], ipport, ':');
                                    break;
                                }
                            }
                            for (int i = 0; i < reachableClients->size(); i++) {
                                if ((*reachableClients)[i].first == nameDest) {
                                    isAClient = 1;
                                    std::cout << "it is for a client!" << std::endl;
                                    splitString((*reachableClients)[i].first, ipport, ':');
                                    break;
                                }
                            }
                            // Message is for a client, i need to use the tables
                            if (isAClient) {
                                n->setServerBit(packet, 1);
                                sendOneFragmentedMessage(n, packet, ipport[0] + ":" + ipport[1]);
                            } else {
                                std::cout << "for router" << std::endl;
                                // Message is for a router, i have the path in my table
                                n->setServerBit(packet, 0);
                                sendOneFragmentedMessage(n, packet, nameDest);
                            }
                        } else {
                            // its for me!
                            if (n->getFragmentBit(packet)) {
                                int found = 0;
                                for (int i = 0; i < n->fragmentedPackets.size(); i++) {
                                    if (nameSrc == n->fragmentedPackets[i].first) {
                                        n->fragmentedPackets[i].second.push_back(packet);
                                        std::pair<int, std::string> result = n->checkFragmentArrival(
                                                n->fragmentedPackets[i].second);
                                        if (result.first) {
                                            n->iAmAServer = 1;
                                            n->announceServer(n->ip + ":" + std::to_string(n->port), "", 0);
                                            std::thread server(tServerTh, n);
                                            server.detach();
                                            n->fragmentedPackets.erase(n->fragmentedPackets.begin() + i);
                                        }

                                        found = 1;
                                        break;
                                    }
                                }
                                if (found == 0) {
                                    std::vector<unsigned char *> v;
                                    v.push_back(packet);
                                    std::pair<std::string, std::vector<unsigned char *>> newFragmentedPacket = {nameSrc, v};
                                    n->fragmentedPackets.push_back(newFragmentedPacket);
                                }
                            } else {
                                processMigrateMessage(n, n->getMessage(packet));

                                n->announceServer(n->ip + ":" + std::to_string(n->port), "", 0);
                                n->iAmAServer = 1;
                                std::thread server(tServerTh, n);
                                server.detach();
                            }
                        }
                        /*
                        // ESTO ESTABA ANTES
                        if (nameDest != n->ip + ":" + std::to_string(n->port)){
                            unsigned char * packet = n->makePacket(ip_src, port_src, ip_dest, port_dest, MIGRATE_MESSAGE, "", 0, 0);
                            for (auto element: n->getTable()->direct_routers){
                                if (element == nameSrc){
                                    continue;
                                }
                                int sd = n->getSocketDescriptor(element);
                                send(sd, packet, (size_t) n->getTotalLength(packet), 0);
                            }
                        } else {
                            if (n->getMessage(packet) == ""){
                                n->announceServer(n->ip + ":" + std::to_string(n->port), "");
                            } else {
                                if (n->getFragmentBit(packet)) {
                                    int found = 0;
                                    for (int i = 0; i < n->fragmentedPackets.size(); i++) {
                                        if (nameSrc == n->fragmentedPackets[i].first) {
                                            n->fragmentedPackets[i].second.push_back(packet);
                                            std::pair<int, std::string> result = n->checkFragmentArrival(
                                                    n->fragmentedPackets[i].second);
                                            if (result.first) {
                                                std::cout << "Llego mensaje de migracion: " << result.second << std::endl;
                                                std::string m = result.second;
                                                processMigrateMessage(n, m);

                                                std::string usefulRouter = n->searchConnectedRouter(nameSrc);
                                                int sd = n->getSocketDescriptor(usefulRouter);

                                        //send mack
                                        sleep(n->getDelay(usefulRouter));
                                        n->sendMessage(n->ip, std::to_string(n->port), ip_src, port_src, MACK_MESSAGE,
                                                       std::string(""), sd, 0, n->getServerBit(packet));

                                                std::cout << "ACK de migracion enviado para " << nameDest << std::endl;


                                                n->fragmentedPackets.erase(n->fragmentedPackets.begin() + i);
                                            }

                                            found = 1;
                                            break;
                                        }
                                    }
                                    if (found == 0) {
                                        std::vector<unsigned char *> v;
                                        v.push_back(packet);
                                        std::pair<std::string, std::vector<unsigned char *>> newFragmentedPacket = {nameSrc,
                                                                                                                    v};
                                        n->fragmentedPackets.push_back(newFragmentedPacket);
                                    }
                                } else {
                                    std::cout << "Llego mensaje de migracion: " << n->getMessage(packet) << std::endl;
                                    processMigrateMessage(n, n->getMessage(packet));

                                    //TODO:enviar MACK al sevidor


                                    std::string usefulRouter = n->searchConnectedRouter(nameSrc);
                                    int sd = n->getSocketDescriptor(usefulRouter);

                                    //send mack
                                    sleep(n->getDelay(usefulRouter));
                                    n->sendMessage(n->ip, std::to_string(n->port), ip_src, port_src, MACK_MESSAGE,
                                                   std::string(""), sd, 0);

                                    std::cout << "ACK de migracion enviado para " << nameDest << std::endl;

                                }
                            }
                        }
                    */
                    }
                } else {
                    sendOneFragmentedMessage(n, packet, nameDest);
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
    std::string ipsrc;
    std::string portsrc;
    std::string namesrc;
    std::string ipdest;
    std::string portdest;
    std::string namedest;
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
                ipsrc = c->getSrcIp(packet);
                portsrc = std::to_string(c->getSrcPort(packet));
                namesrc = ipsrc;
                namesrc += ":";
                namesrc += portsrc;
                ipdest = c->getDestIp(packet);
                portdest = std::to_string(c->getDestPort(packet));
                namedest = ipdest;
                namedest += ":";
                namedest += portdest;

                if (namedest == c->ip + ":" + std::to_string(c->port)) {
                    cClient(c, packet, namesrc, ipsrc, portsrc);
                }
            } else {
                (c->mtx).unlock();
            }

        }
        sleep(5);
    }
}

void cServerTh(C *c){
    std::cout << "I am a Server" << std::endl;

    (c->mtx).lock();
    c->message_queue.clear();
    (c->mtx).unlock();
    if (c->waitingForSack) {
        c->increaseSequenceNumber();
        c->sendMessage(c->ip, std::to_string(c->port), c->ipSent, c->portSent,
                       CHAT_MESSAGE, c->sentMessage,
                       c->getSocketDescriptor(c->getTable()->direct_routers.front()), c->currentSequenceNumber,
                       1);
    }
    c->resendAcks(1);
    cSendResendMessages(getResendList(c), c);
    increaseExpectedSeqNumber(c);

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

    (n->mtx).lock();
    n->message_queue.clear();
    (n->mtx).unlock();
    tSendResendMessages(getResendList(n), n);
    increaseExpectedSeqNumber(n);

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

                int alreadyProcessed = n->getServerBit(packet);

                // should work
                n->setServerBit(packet, 1);

                if (n->checkC(nameDest)){
                    std::cout << "C is down" << std::endl;
                    std::string usefulRouter = n->searchConnectedRouter(nameSrc);
                    int sd = n->getSocketDescriptor(usefulRouter);

                    //send nack
                    sleep(n->getDelay(usefulRouter));
                    n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc, NACK_MESSAGE,
                                   std::string(""), sd, 0, 1);
                    continue;
                }

                //std::cout << "got message of type: " << n->getType(packet) << std::endl;
                if (n->getType(packet) == TABLE_MESSAGE) {
                    n->processTablePacket(packet);
                } else if (n->getType(packet) == CHAT_MESSAGE) {
                    if (!alreadyProcessed) {
                        if (std::find(n->serverWaitingForAcks.begin(), n->serverWaitingForAcks.end(),
                                      std::pair<std::string,
                                              std::string>{nameSrc, nameDest}) != n->serverWaitingForAcks.end()) {
                            sendFragmentedMessages(n, nameDest, packet);
                        } else {
                            if (n->getFragmentBit(packet)) {
                                int found = 0;
                                for (int i = 0; i < n->serverFragmentedPackets.size(); i++) {
                                    if (nameSrc == n->serverFragmentedPackets[i].first.first &&
                                        nameDest == n->serverFragmentedPackets[i].first.second) {
                                        std::cout << "Frag: " << n->getMessage(packet) << std::endl;
                                        std::cout << "Expecting " << n->serverFragmentedPackets[i].second.first
                                                  << std::endl;

                                        if (n->getSeqNum(packet) == n->serverFragmentedPackets[i].second.first) {
                                            std::cout << "Good seqNUm " << n->getSeqNum(packet) << std::endl;

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
                                                n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc,
                                                               SACK_MESSAGE,
                                                               std::string(""), sd, n->getSeqNum(packet),
                                                               n->getServerBit(packet));

                                                std::cout << "Envie SACK a " << nameSrc << std::endl;

                                                packet = n->makePacket(std::move(ipSrc), std::move(portSrc),
                                                                       std::move(ipDest),
                                                                       std::move(portDest), CHAT_MESSAGE, result.second,
                                                                       n->getSeqNum(packet), n->getServerBit(packet));

                                                sendFragmentedMessages(n, nameDest, packet);

                                                std::cout << "Envie mensaje a " << nameDest << std::endl;

                                                n->serverFragmentedPackets.erase(
                                                        n->serverFragmentedPackets.begin() + i);

                                                n->serverWaitingForAcks.push_back({nameSrc, nameDest});
                                            }

                                        } else {
                                            std::cout << "Wrong seqNUm " << n->getSeqNum(packet) << std::endl;
                                        }

                                        found = 1;
                                        break;
                                    }
                                }
                                if (found == 0) {
                                    std::vector<unsigned char *> v;
                                    v.push_back(packet);
                                    std::pair<std::pair<std::string, std::string>, std::pair<int, std::vector<unsigned char *>>> newFragmentedPacket = {{nameSrc,        nameDest},
                                                                                                                                                        {n->getSeqNum(
                                                                                                                                                                packet), v}};
                                    n->serverFragmentedPackets.push_back(newFragmentedPacket);
                                }
                            } else {
                                std::cout << "Paso mensaje de " << nameSrc << " para " << nameDest << std::endl;

                                usefulRouter = n->searchConnectedRouter(nameSrc);
                                sd = n->getSocketDescriptor(usefulRouter);

                                //send ack
                                sleep(n->getDelay(usefulRouter));
                                n->sendMessage(n->ip, std::to_string(n->port), ipSrc, portSrc, SACK_MESSAGE,
                                               std::string(""), sd, n->getSeqNum(packet), n->getServerBit(packet));

                                std::cout << "Envie SACK a " << nameSrc << std::endl;

                                sendFragmentedMessages(n, nameDest, packet);

                                std::cout << "Envie mensaje a " << nameDest << std::endl;

                                n->serverWaitingForAcks.push_back({nameSrc, nameDest});
                            }
                        }
                    } else {
                        sendOneFragmentedMessage(n, packet, nameDest);
                    }
                } else if (n->getType(packet) == SACK_MESSAGE
                        || n->getType(packet) == RESEND_MESSAGE) {
                    usefulRouter = n->searchConnectedRouter(nameDest);
                    sd = n->getSocketDescriptor(usefulRouter);

                    sleep(n->getDelay(usefulRouter));
                    send(sd, packet, (size_t) n->getTotalLength(packet), 0);

                } else if (n->getType(packet) == MIGRATE_MESSAGE) {
                    std::cout << "ignoring MIGRATE message" << std::endl;
                } else if (n->getType(packet) == NEW_SRV_MESSAGE) {
                    std::cout << "NEW SERVER MESSAGE" << std::endl;
                    //n->processServerMessage(packet);
                }


                else {
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
                                               nameDest, sd, n->getSeqNum(packet), n->getServerBit(packet));

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
                        //send it anyway
                        usefulRouter = n->searchConnectedRouter(nameDest);
                        sd = n->getSocketDescriptor(usefulRouter);

                        sleep(n->getDelay(usefulRouter));
                        send(sd, packet, (size_t) n->getTotalLength(packet), 0);
                    } else {
                        sendOneFragmentedMessage(n, packet, nameDest);
                    }
                }
            } else {
                (n->mtx).unlock();
            }
        }
        sleep(1);
    }
}

void offServerTh(Node* n) {
    std::cout << "Off" << std::endl;
    while (1) {
        if (!n->off) {
            n->serverCond.notify_one();
            std::cout << "Not doing anything has a bright side" << std::endl;
            return;
        }
        sleep(3);
    }
}



void tMigrateServerTh(T *n, std::string sIP, std::string sPort, std::string type) {
    int t = (type == "T" || type == "t");
    unsigned char * packet;
    std::cout << "Migrating..." << std::endl;
    std::string m = "";
    std::cout << "first for" << std::endl;
    for (auto element: n->serverFragmentedPackets){
        m += element.first.first + "," + element.first.second + ',';
        m += std::to_string(element.second.first) + ";";
    }
    m += "$";
    std::cout << "second for" << std::endl;
    for (auto element: n->serverWaitingForAcks){
        m += element.first + "," + element.second + ';';
    }
    std::cout << "make Packet" << std::endl;

    if (t) {
        packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, "", 0, 0);
        for (auto element: n->getTable()->direct_routers){
            int sd = n->getSocketDescriptor(element);
            send(sd, packet, (size_t) n->getTotalLength(packet), 0);
        }
    } else {
        packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, m, 0, 0);

        std::cout << "sending messages..." << std::endl;
        sendFragmentedMessages(n, sIP + ":" + sPort, packet);
        std::cout << "sent messages..." << std::endl;


        n->serverFragmentedPackets.clear();
        n->serverWaitingForAcks.clear();
    }

    while (1) {
        if (!n->migrating) {
            n->serverCond.notify_one();
            std::cout << "Not doing anything has a bright side" << std::endl;
            return;
        }
        (n->mtx).lock();
        if (!n->message_queue.empty()) {
            packet = (n->message_queue).front();
            (n->message_queue).pop_front();
            (n->mtx).unlock();
            std::cout << "checking..." << std::endl;

            if (n->getType(packet) == NEW_SRV_MESSAGE) {
                std::cout << "NEW SERVER MESSAGE" << std::endl;
                n->processServerMessage(packet);

                if (t) {
                    packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, m, 0, 1);

                    std::cout << "sending messages..." << std::endl;
                    sendThroughRouter(n, n->getTable()->getDirectRouters()->front(), packet);
                    std::cout << "sent messages..." << std::endl;


                    n->serverFragmentedPackets.clear();
                    n->serverWaitingForAcks.clear();
                }

                n->migrating = 0;
            } else if (n->getType(packet) == MIGRATE_MESSAGE) {
                sendThroughRouter(n, n->getTable()->getDirectRouters()->back(), packet);
            } else {
                // don't care
            }
        } else {
            (n->mtx).unlock();
        }
        sleep(1);
    }
}

void cMigrateServerTh(C *n, std::string sIP, std::string sPort, std::string type) {
    int t = (type == "T" || type == "t");
    unsigned char * packet;
    std::cout << "Migrating..." << std::endl;
    std::string m = "";
    std::cout << "first for" << std::endl;
    for (auto element: n->serverFragmentedPackets){
        m += element.first.first + "," + element.first.second + ',';
        m += std::to_string(element.second.first) + ";";
    }
    m += "$";
    std::cout << "second for" << std::endl;
    for (auto element: n->serverWaitingForAcks){
        m += element.first + "," + element.second + ';';
    }
    std::cout << "make Packet" << std::endl;

    if (t) {
        packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, "", 0, 0);
        n->sendPacket(packet);
    } else {
        packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, m, 0, 0);

        std::cout << "sending messages..." << std::endl;
        n->sendPacket(packet);
        std::cout << "sent messages..." << std::endl;


        n->serverFragmentedPackets.clear();
        n->serverWaitingForAcks.clear();
    }


    if (t) {

        while (1) {
            if (!n->migrating) {
                n->serverCond.notify_one();
                std::cout << "Finish migrating" << std::endl;
                return;
            }
            (n->mtx).lock();
            if (!n->message_queue.empty()) {
                packet = (n->message_queue).front();
                (n->message_queue).pop_front();
                (n->mtx).unlock();
                std::cout << "checking..." << std::endl;

                if (n->getType(packet) == NEW_SRV_MESSAGE) {
                    std::cout << "NEW SERVER MESSAGE" << std::endl;
                    //n->processServerMessage(packet);

                    packet = n->makePacket(n->ip, std::to_string(n->port), sIP, sPort, MIGRATE_MESSAGE, m, 0, 1);

                    std::cout << "sending messages..." << std::endl;
                    n->sendPacket(packet);
                    std::cout << "sent messages..." << std::endl;

                    n->serverFragmentedPackets.clear();
                    n->serverWaitingForAcks.clear();

                    n->migrating = 0;
                } else if (n->getType(packet) == MIGRATE_MESSAGE) {
                    n->sendPacket(packet);
                } else {
                    // don't care
                }
            } else {
                (n->mtx).unlock();
            }
            sleep(1);
        }

    } else {
        n->migrating = 0;
        n->serverCond.notify_one();
        std::cout << "Not doing anything has a bright side" << std::endl;
        return;
    }
}