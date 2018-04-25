//
// Created by areye on 24-Apr-18.
//
#include <vector>
#include <string>

#ifndef CC5303_DISTRIBUTED_SYSTEMS_UTILS_H
#define CC5303_DISTRIBUTED_SYSTEMS_UTILS_H

#endif //CC5303_DISTRIBUTED_SYSTEMS_UTILS_H


size_t splitString(const std::string &txt, std::vector<std::string> &strs, char ch);
char *substring(size_t start, size_t stop, const char *src, char *dst, size_t size);
std::vector<std::string> decomposeIpPort(std::string ipPort);