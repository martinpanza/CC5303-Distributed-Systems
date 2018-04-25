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

void receiveTh(int sd){
    std::cout << "ready to receive messages" << std::endl;
    char *hello="Hello from server";
    char buffer[1024] = {0};
    int valread = 1;

    while (valread > 0){
        valread = read(sd, buffer, 1024);
        printf("%s\n", buffer);
        send(sd, hello, strlen(hello), 0);
        printf("Hello message sent\n");
    }
}

void sendTh(C c, int sd) {
    std::string s;
    std::string message_ = "message";
    std::vector<std::string> words;
    while(std::getline(std::cin, s)) {
        splitString(s, words, ' ');
        if (words[0] == message_ and words.size() == 4) {
            c.sendMessage(words[1], words[2], CHAT_MESSAGE, words[3]);
        }
    }
}
