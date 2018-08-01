//
// Created by marti on 19-04-2018.
//


#include "Node.h"
#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif


Node::Node(std::string ip, uint16_t port, std::string name) {
    if (ip == "localhost") {
        ip = "127.0.0.1";
    }
    //this->serverName = "";
    this->ip = std::move(ip);
    this->port = port;
    this->name = std::move(name);
    this->table = *new Table();
}

int Node::run() {
    return 0;
}

int Node::sendMessage(const std::string ip_src, const std::string port_src,
                      const std::string ip_dest, const std::string port_dest,
                      const int type, const std::string message, const int sd, int sequenceNumber, int serverBit) {
    //std::cout << ip_src << ":" << port_src << std::endl;
    //std::cout << ip_dest << ":" << port_dest << " " << type << " " << message;
    return 0;
}


Table* Node::getTable() {
    return &(this->table);
}

uint16_t Node::getSrcPort(const unsigned char* packet) {
    return packet[5] | uint16_t(packet[4]) << 8;
}

uint16_t Node::getDestPort(const unsigned char* packet) {
    return packet[11] | uint16_t(packet[10]) << 8;
}

uint16_t Node::getTotalLength(const unsigned char* packet) {
    return packet[14] | uint16_t(packet[13]) << 8;
}

uint16_t Node::getOffset(const unsigned char* packet) {
    return packet[17] | uint16_t(packet[16]) << 8;
}

void Node::setOffset(unsigned char* packet, uint16_t offset) {
    packet[16] = (unsigned char) (offset >> 8);
    packet[17] = (unsigned char) (offset & 0xFF);
}

int Node::getLastBit(const unsigned char* packet) {
    return (int) packet[15];
}

void Node::setLastBit(unsigned char* packet, int lastBit) {
    packet[15] = (unsigned char) lastBit;
}


int Node::getType(const unsigned char* packet) {
    return (int) packet[12];
}

int Node::getFragmentBit(const unsigned char* packet) {
    return (int) packet[18];
}

void Node::setFragmentBit(unsigned char* packet, int fragmentBit) {
    packet[18] = (unsigned char) fragmentBit;
}

int Node::getServerBit(const unsigned char* packet) {
    return (int) packet[19];
}

void Node::setServerBit(unsigned char* packet, int serverBit) {
    packet[19] = (unsigned char) serverBit;
}

int Node::getSeqNum(const unsigned char* packet) {
    return (int) packet[20];
}

void Node::setSeqNum(unsigned char* packet, int seqNum) {
    packet[20] = (unsigned char) seqNum;
}

std::string Node::getSrcIp(const unsigned char* packet) {
    std::string ip;
    std::string ip_delimiter = ".";
    ip += std::to_string(packet[0]);
    ip += ip_delimiter;
    ip += std::to_string(packet[1]);
    ip += ip_delimiter;
    ip += std::to_string(packet[2]);
    ip += ip_delimiter;
    ip += std::to_string(packet[3]);
    return ip;
}

std::string Node::getDestIp(const unsigned char* packet) {
    std::string ip;
    std::string ip_delimiter = ".";
    ip += std::to_string(packet[6]);
    ip += ip_delimiter;
    ip += std::to_string(packet[7]);
    ip += ip_delimiter;
    ip += std::to_string(packet[8]);
    ip += ip_delimiter;
    ip += std::to_string(packet[9]);
    return ip;
}

std::string Node::getMessage(const unsigned char* packet) {
    uint16_t total_length = this->getTotalLength(packet);
    int data_length = total_length - HEADER_SIZE;
    char message[data_length];
    substring(HEADER_SIZE, (size_t) total_length,(const char*)packet, message, total_length);
    return std::string(message);
}

void Node::printPacket(const unsigned char* packet) {
    std::cout << "Src IP: "<< this->getSrcIp(packet) << std::endl;
    std::cout << "Src Port: "<< this->getSrcPort(packet) << std::endl;
    std::cout << "Dest IP: "<< this->getDestIp(packet) << std::endl;
    std::cout << "Dest Port: "<< this->getDestPort(packet) << std::endl;
    std::cout << "Type: "<< this->getType(packet) << std::endl;
    std::cout << "Total Length: "<< this->getTotalLength(packet) << std::endl;
    std::cout << "Fragmented: "<< this->getFragmentBit(packet) << std::endl;
    std::cout << "Offset: "<< this->getOffset(packet) << std::endl;
    std::cout << "Last: "<< this->getLastBit(packet) << std::endl;
    std::cout << "Message: "<< this->getMessage(packet) << std::endl;
}

void swap(unsigned char** a, unsigned char** b) {
    unsigned char* t = *a;
    *a = *b;
    *b = t;
}

int Node::partition(std::vector<unsigned char*>* fragments, int low, int high) {
    int pivot = this->getOffset((*fragments)[high]);    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++) {
        // If current element is smaller than or
        // equal to pivot
        if (this->getOffset((*fragments)[j]) <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&((*fragments)[i]), &((*fragments)[j]));
        }
    }
    swap(&((*fragments)[i + 1]), &((*fragments)[high]));
    return (i + 1);
}

/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void Node::quickSort(std::vector<unsigned char*>* fragments, int low, int high) {
    if (low < high) {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(fragments, low, high);

        // Separately sort elements before
        // partition and after partition
        this->quickSort(fragments, low, pi - 1);
        this->quickSort(fragments, pi + 1, high);
    }
}

std::pair<unsigned char *, unsigned char*> Node::fragment(unsigned char* packet, int MTU) {
    std::string ip_src = this->getSrcIp(packet), ip_dest = this->getDestIp(packet);
    std::string port_src = std::to_string(this->getSrcPort(packet)),
            port_dest = std::to_string(this->getDestPort(packet));
    int type = this->getType(packet);
    uint16_t original_offset = this->getOffset(packet);
    std::string message = this->getMessage(packet);

    int top_message_size = MTU - HEADER_SIZE;
    //int bot_message_size = (int) message.length() - MTU - HEADER_SIZE;

    std::string top_message = message.substr(0, (unsigned long) top_message_size),
            bot_message = message.substr(top_message_size);


    unsigned char* top_packet = this->makePacket(ip_src, port_src, ip_dest, port_dest,
                                                 type, top_message, this->getSeqNum(packet), this->getServerBit(packet));
    unsigned char* bot_packet = this->makePacket(ip_src, port_src, ip_dest, port_dest,
                                                 type, bot_message, this->getSeqNum(packet), this->getServerBit(packet));

    if (!this->getFragmentBit(packet)){
        this->setLastBit(bot_packet, 1);
    } else{
        if (this->getLastBit(packet)){
            this->setLastBit(bot_packet, 1);
        }
    }

    this->setFragmentBit(top_packet, 1);
    this->setFragmentBit(bot_packet, 1);
    this->setLastBit(top_packet, 0);
    //this->setLastBit(bot_packet, 1);

    uint16_t bot_packet_offset = original_offset + (uint16_t) top_message_size;
    this->setOffset(top_packet, original_offset);
    this->setOffset(bot_packet, bot_packet_offset);

    return {top_packet, bot_packet};
}


unsigned char* Node::makePacket(std::string ip_src, std::string port_src, std::string ip_dest,
                                std::string port_dest, int type, std::string message, int sequenceNumber, int serverBit) {
    auto packet = (unsigned char*) malloc((HEADER_SIZE + message.length()) * sizeof(unsigned char));

    std::vector<std::string> ip, ip_d;
    //splitString(this->ip, ip, '.');
    splitString(ip_src, ip, '.');
    splitString(ip_dest, ip_d, '.');

    // Source IP
    for (int i = 0; i < ip.size(); i++) {
        packet[i] = (unsigned char)(stoi(ip[i]));
    }

    // Source port
    auto sport = (uint16_t) stoi(port_src);
    //packet[4] = (unsigned char) (this->port >> 8); // hi
    //packet[5] = (unsigned char) (this->port & 0xFF); // lo
    packet[4] = (unsigned char) (sport >> 8); // hi
    packet[5] = (unsigned char) (sport & 0xFF); // lo

    // Destination IP
    for (int i = 0; i < ip_d.size(); i++) {
        packet[i + 6] = (unsigned char)(stoi(ip_d[i]));
    }

    // Destination port
    auto dport = (uint16_t) stoi(port_dest);
    packet[10] = (unsigned char) (dport >> 8); // hi
    packet[11] = (unsigned char) (dport & 0xFF); // lo

    // Type
    packet[12] = (unsigned char) type;

    // Total package length
    packet[13] = (unsigned char) ((uint16_t) (HEADER_SIZE + message.length()) >> 8);
    packet[14] = (unsigned char) ((uint16_t) (HEADER_SIZE + message.length()) & 0xFF);

    // Last bit
    packet[15] = (unsigned char) 0;

    // Offset
    uint16_t starting_offset = 0;
    packet[16] = (unsigned char) (starting_offset >> 8);
    packet[17] = (unsigned char) (starting_offset & 0xFF);

    // Fragmented
    packet[18] = (unsigned char) 0;

    // Server bit
    packet[19] = (unsigned char) serverBit;

    // Sequence number
    packet[20] = (unsigned char) sequenceNumber;

    // Message
    for (int i = 0; i < message.length(); i++) {
        packet[i + 21] = (unsigned char) message[i];
    }
    return packet;
}

std::string Node::searchPathToServer() {
    std::string usefulRouter;
    //std::cout << "getting path to server" << std::endl;
    std::set<std::string>* pathToServer = this->getTable()->getPathToServer();

    auto pathIterator = pathToServer->begin();
    usefulRouter = *pathIterator;
    return usefulRouter;
}

std::string Node::searchConnectedRouter(std::string name) {
    std::string usefulRouter;
    //std::cout << "searching conn to " << name << std::endl;
    std::vector<std::string>* direct_clients =
            (this->getTable())->getDirectClients();
    for (int i = 0; i < direct_clients->size(); i++) {
        if ((*direct_clients)[i] == name) {
            return name;
        }
    }

    //std::cout << "searched clients" << std::endl;


    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    for (int i = 0; i < reachable_clients->size(); i++) {
        //std::cout << "reachable: " << (*reachable_clients)[i].first << std::endl;
        if ((*reachable_clients)[i].first == name) {
            usefulRouter = (*reachable_clients)[i].second.front();
            // Round robin
            (*reachable_clients)[i].second.erase((*reachable_clients)[i].second.begin());
            (*reachable_clients)[i].second.push_back(usefulRouter);
            break;
        }
    }
    //std::cout << "wtf" << std::endl;
    return usefulRouter;
}

int Node::getSocketDescriptor(std::string name) {
    for (int i = 0; i < this->socketDescriptors.size(); i++){
        if (name == this->socketDescriptors[i].second){
            return this->socketDescriptors[i].first;
        }
    }
    return -1;
}

std::string Node::getNameBySocketDescriptor(int sd){
    for (int i = 0; i < this->socketDescriptors.size(); i++){
        if (sd == this->socketDescriptors[i].first){
            return this->socketDescriptors[i].second;
        }
    }
    return "";
}

int Node::getDelay(std::string name) {
    std::vector<std::pair<std::string, std::pair<int,int>>> c = this->connections;
    for (int i = 0; i <  c.size(); i++){
        if (c[i].first == name){
            return c[i].second.first;
        }
    }
    return 0;
}

int Node::getMTU(std::string name) {
    std::vector<std::pair<std::string, std::pair<int,int>>> c = this->connections;
    for (int i = 0; i <  c.size(); i++){
        if (c[i].first == name){
            return c[i].second.second;
        }
    }
    return 0;
}

std::pair<int, std::string> Node::checkFragmentArrival(std::vector<unsigned char *> fragments) {
    std::pair<int, std::string> result = {0, ""};
    this->quickSort(&fragments, 0, (int) fragments.size() - 1);
    int lastPacketArrived = 0;
    uint16_t totalLength = 0;
    uint16_t totalSum = 0;
    std::string message;

    for (int i = 0; i < fragments.size(); i++) {
        totalSum += this->getMessage(fragments[i]).size();
        message += this->getMessage(fragments[i]);
        if (this->getLastBit(fragments[i])) {
            lastPacketArrived = 1;
            totalLength = (uint16_t) (this->getOffset(fragments[i]) + this->getMessage(fragments[i]).size());
        }
    }
    //std::cout << message << std::endl;]

    if (totalLength != 0) {
        if (lastPacketArrived and totalLength == totalSum) {
            result.first = 1;
            result.second = message;
        }
    }
    return result;
};
// message is the serverName
// format of message will be serverName-router_1;router_2;...;router_n-client_1;client_2
void Node::announceServer(std::string message, std::string initialSender) {
    std::vector<std::string>* directRouters =(this->getTable())->getDirectRouters();
    std::vector<std::string>* directClients =(this->getTable())->getDirectClients();
    std::vector<std::string> ipport;
    std::set<std::string> routerSet, clientSet, clientDiff, diff;
    message += "-";
    //message += ";";
    //std::cout << "getting routers" << std::endl;
    if (directRouters->size() > 0) {
        for (int i = 0; i < directRouters->size() - 1; i++) {
            message += (*directRouters)[i];
            message += ";";
            routerSet.insert((*directRouters)[i]);
        }
        message += (*directRouters)[directRouters->size() - 1];
        routerSet.insert((*directRouters)[directRouters->size() - 1]);
    }

    message += "-";

    //std::cout << "getting clients" << std::endl;
    if (directClients->size() > 0) {
        for (int i = 0; i < directClients->size() - 1; i++) {
            message += (*directClients)[i];
            message += ";";
            clientSet.insert((*directClients)[i]);
        }
        message += (*directClients)[directClients->size() - 1];
        clientSet.insert((*directClients)[directClients->size() - 1]);
    }
    // dont want to send the message twice to a node
    std::set_difference(routerSet.begin(), routerSet.end(), this->getTable()->noticedNodes.begin(), this->getTable()->noticedNodes.end(), std::inserter(diff, diff.end()));
    //std::cout << "senting to unnoticed ones" << std::endl;
    for (std::string router : diff) {
        //std::cout << "unnoticed router: " << router << std::endl;
        if (router != initialSender) {
            splitString(router, ipport, ':');
            this->sendMessage(this->ip, std::to_string(this->port), ipport[0], ipport[1], NEW_SRV_MESSAGE, message,
                              this->getSocketDescriptor(router), 0, 1);

            this->getTable()->addNoticedNodes(router);
        }
    }

    std::set_difference(clientSet.begin(), clientSet.end(), this->getTable()->noticedClients.begin(), this->getTable()->noticedClients.end(), std::inserter(clientDiff, clientDiff.end()));

    for (std::string client : clientDiff) {
        //std::cout << "unnoticed client: " << client << std::endl;
        splitString(client, ipport, ':');
        this->sendMessage(this->ip, std::to_string(this->port), ipport[0], ipport[1], NEW_SRV_MESSAGE, message, this->getSocketDescriptor(client), 0, 1);
        this->getTable()->addNoticedClients(client);
    }

}

int Node::isDirectConnection(std::string name) {
    int directConn = 0;
    std::vector<std::string>* directRouters =(this->getTable())->getDirectRouters();
    std::vector<std::string>* directClients =(this->getTable())->getDirectClients();
    for (std::string d : (*directClients)) {
        if (d == name) {
            directConn = 1;
            break;
        }
    }
    if (!directConn) {
        for (std::string d : (*directRouters)) {
            if (d == name) {
                directConn = 1;
                break;
            }
        }
    }
    return directConn;
}


void Node::processServerMessage(const unsigned char* packet) {
    std::vector<std::string> routersVec, clientsVec, serverRoutersClients;
    // routers from packet and my routers
    std::set<std::string> routers, myRouters, diff;
    std::string packetServer, srcName, myName, packetMessage;
    std::vector<std::string>* myDirectRouters =(this->getTable())->getDirectRouters();
    int directToServer = 0;
    packetMessage = this->getMessage(packet);
    //std::cout << "separating all" << std::endl;
    splitString(packetMessage, serverRoutersClients, '-');
    //std::cout << "separating routers" << std::endl;
    splitString(serverRoutersClients[1], routersVec, ';');
    //std::cout << "separating clients" << std::endl;
    splitString(serverRoutersClients[2], clientsVec, ';');
    //std::cout << "getting server" << std::endl;
    packetServer = serverRoutersClients[0];

    srcName = this->getSrcIp(packet) + ":" + std::to_string(this->getSrcPort(packet));
    myName = this->ip + ":" + std::to_string((this->port));
    //std::cout << "process server message from: " << srcName << std::endl;
    //std::cout << "server: " << packetServer << std::endl;

    int j = 0;
    int found = 0;
    for (j; j < this->serverName.size(); j++){
        if (this->serverName[j] == packetServer){
            found = 1;
            break;
        }
    }

    //std::cout << found << std::endl;

    if (!found){
        this->serverName.push_back(packetServer);
        this->getTable()->prepareNewServer();
    }

    // if the sender is the server itself
    if (srcName == packetServer) {
        if (!found) {
            this->getTable()->pathToServer.push_back({packetServer});
        } else {
            this->getTable()->pathToServer[j].insert(packetServer);
        }
    } else {
        // check if i should add to the path to the server by checking the difference in direct routers
        // if it has at least 1 direct router
        if (!this->isDirectConnection(packetServer)) {
            // the sender router and its direct routers
            routers.insert(srcName);
            for (int i = 0; i < routersVec.size(); i++) {
                    routers.insert(routersVec[i]);
            }

            // me and my direct routers
            myRouters.insert(myName);
            for (std::string router : (*myDirectRouters)) {
                myRouters.insert(router);
            }

            // if mine contains all of the other then there is a loop so i dont need to add it to my pathToServer
            std::set_difference(routers.begin(), routers.end(), myRouters.begin(),
                                myRouters.end(), std::inserter(diff, diff.end()));

            for (int i = 0; i < routersVec.size(); i++) {
                if (routersVec[i] == packetServer) {
                    directToServer = 1;
                    break;
                }
            }

            if (directToServer != 1) {
                for (int i = 0; i < clientsVec.size(); i++) {
                    if (clientsVec[i] == packetServer) {
                        directToServer = 1;
                        break;
                    }
                }
            }

            // if there is at least one that the other has that i dont
            if (!diff.empty() || directToServer == 1) {
                if (!found) {
                    if (this->getTable()->pathToServer.size() == 0) {
                        this->getTable()->pathToServer.push_back({srcName});
                    } else {
                        this->getTable()->pathToServer.back().insert(srcName);
                    }
                } else {
                    this->getTable()->pathToServer[j].insert(srcName);
                }
            }
        }
    }
    this->announceServer(packetServer, srcName);
}



