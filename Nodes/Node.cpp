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
    this->ip = std::move(ip);
    this->port = port;
    this->name = std::move(name);
    this->table = *new Table();
}

int Node::run() {
    return 0;
}

int Node::receivePacket(char* p) {
    return 0;
}

int Node::sendMessage(const std::string ip_src, const std::string port_src,
                      const std::string ip_dest, const std::string port_dest,
                      const int type, const std::string message, const int sd) {
    std::cout << ip_src << ":" << port_src << std::endl;
    std::cout << ip_dest << ":" << port_dest << " " << type << " " << message;
    return 0;
}

void Node::receiveTablePacket() {}

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
int Node::partition(std::vector<unsigned char*>fragments, int low, int high) {
    int pivot = this->getOffset(fragments[high]);    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++) {
        // If current element is smaller than or
        // equal to pivot
        if (this->getOffset(fragments[j]) <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&fragments[i], &fragments[j]);
        }
    }
    swap(&fragments[i + 1], &fragments[high]);
    return (i + 1);
}

/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void Node::quickSort(std::vector<unsigned char*>fragments, int low, int high) {
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
    std::string ip_src = this->getSrcIp(packet), ip_dest= this->getDestIp(packet);
    std::string port_src = std::to_string(this->getSrcPort(packet)),
            port_dest = std::to_string(this->getDestPort(packet));
    int type = this->getType(packet);
    uint16_t original_offset = this->getOffset(packet);
    std::string message = this->getMessage(packet);

    int top_message_size = MTU - HEADER_SIZE;
    int bot_message_size = (int) message.length() - MTU - HEADER_SIZE;

    std::string top_message = message.substr(0, (unsigned long) top_message_size),
            bot_message = message.substr(0, (unsigned long) bot_message_size);


    unsigned char* top_packet = this->makePacket(ip_src, port_src, ip_dest, port_dest, type, top_message);
    unsigned char* bot_packet = this->makePacket(ip_dest, port_dest, ip_dest, port_dest, type, bot_message);

    this->setFragmentBit(top_packet, 1);
    this->setFragmentBit(bot_packet, 1);
    this->setLastBit(top_packet, 0);
    this->setLastBit(bot_packet, 1);

    uint16_t bot_packet_offset = original_offset + (uint16_t) top_message_size;
    this->setOffset(top_packet, original_offset);
    this->setOffset(bot_packet, bot_packet_offset);

    return {top_packet, bot_packet};
}


unsigned char* Node::makePacket(std::string ip_src, std::string port_src, std::string ip_dest,
                                std::string port_dest, int type, std::string message) {
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

    // Fragmented
    packet[15] = (unsigned char) 0;

    // Offset
    uint16_t starting_offset = 0;
    packet[16] = (unsigned char) (starting_offset >> 8);
    packet[17] = (unsigned char) (starting_offset & 0xFF);

    // Last bit
    packet[18] = (unsigned char) 0;
    // Message
    for (int i = 0; i < message.length(); i++) {
        packet[i + 19] = (unsigned char) message[i];
    }
    return packet;
}

std::vector<std::string> Node::searchConnectedRouter(std::string name) {
    std::vector<std::string> usefulRouters;

    std::vector<std::string>* direct_clients =
            (this->getTable())->getDirectClients();
    for (int i = 0; i < direct_clients->size(); i++) {
        if ((*direct_clients)[i] == name) {
            usefulRouters = std::vector<std::string>();
            usefulRouters.push_back(name);
            return usefulRouters;
        }
    }


    std::vector<std::pair<std::string, std::vector<std::string>>>* reachable_clients =
            (this->getTable())->getReachableClients();
    for (int i = 0; i < reachable_clients->size(); i++) {
        if ((*reachable_clients)[i].first == name) {
            usefulRouters = (*reachable_clients)[i].second;
            // Round robin
            std::pair<std::string, std::vector<std::string>> channel = (*reachable_clients)[i];
            reachable_clients->erase(reachable_clients->begin() + i);
            reachable_clients->push_back(channel);
            break;
        }
    }

    return usefulRouters;
}

void Node::sendNextPacket() {
    //TODO: Modificar para que se traiga un vector con todas las rutas UTILES de acuerdo al destino del paquete
    unsigned char* packet = this->message_queue.back();
    this->message_queue.pop_back();

    int connection_mtu = this->connections[this->connectionIndex].second.second;
    /*
    if (sizeof(packet) > connection_mtu){
        std::pair<unsigned char*, unsigned char*> fragmentedPackets = fragment(sizeof(packet), connection_mtu);
        packet = fragmentedPackets.first;
        this->message_queue.push_back(fragmentedPackets.second);
    }

    //TODO: send through this->connections[this->connectionIndex];

    this->connectionIndex = (int) ((this->connectionIndex + 1) % this->connections.size());
    */
}

int Node::getSocketDescriptor(std::string name) {
    for (int i = 0; i < this->socketDescriptors.size(); i++){
        if (name == this->socketDescriptors[i].second){
            return this->socketDescriptors[i].first;
        }
    }
    return -1;
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
    this->quickSort(fragments, 0, (int) fragments.size() - 1);
    int lastPacketArrived = 0;
    uint16_t totalSum = 0;
    std::string message;

    for (int i = 0; i < fragments.size(); i++) {
        totalSum += this->getOffset(fragments[i]);
        message += this->getMessage(fragments[i]);
        if (this->getLastBit(fragments[i])) {
            lastPacketArrived = 1;
        }
    }

    if (lastPacketArrived) {
        result.first = 1;
        result.second = message;
    }
    return result;
};

