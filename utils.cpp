//
// Created by areye on 24-Apr-18.
//
#include "utils.h"

size_t splitString(const std::string &txt, std::vector<std::string> &strs, char ch) {
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}


char *substring(size_t start, size_t stop, const char *src, char *dst, size_t size) {
    int count = stop - start;
    if ( count >= --size )
    {
        count = size;
    }
    sprintf(dst, "%.*s", count, src + start);
    return dst;
}

std::vector<std::string> decomposeIpPort(std::string ipPort) {
    std::vector<std::string> p;
    splitString(ipPort, p, ':');
    return p;
}