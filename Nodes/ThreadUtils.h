//
// Created by martin on 24-05-18.
//

#ifndef CC5303_DISTRIBUTED_SYSTEMS_THREADUTILS_H
#define CC5303_DISTRIBUTED_SYSTEMS_THREADUTILS_H

#endif //CC5303_DISTRIBUTED_SYSTEMS_THREADUTILS_H

#include "C.h"
#include "T.h"

void copyBuffer(const char* buffer, char** to, int size);
void sendOneFragmentedMessage(T *n, unsigned char *packet, std::string name);
void cServer(C* c, unsigned char* packet, std::string nameSrc, std::string nameDest, std::string ipSrc,
             std::string portSrc, std::string ipDest, std::string portDest);
void cClient(C* c, unsigned char* packet, std::string nameSrc, std::string ipSrc, std::string portSrc);
void sendFragmentedMessages(T* n, std::string nameDest, unsigned char* packet);