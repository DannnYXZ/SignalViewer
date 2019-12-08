#ifndef DATAVIEWER_FILE_LOADER_H
#define DATAVIEWER_FILE_LOADER_H

#include <fstream>
#include <iostream>
#include <cstring>
#include "signal_file.h"

using namespace std;

char *load_file(char *filepath) {
    ifstream is(filepath, ifstream::binary);
    char *buffer = nullptr;
    if (is) {
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        buffer = new char[length];
        is.read(buffer, length);
        is.close();
        //cout.write(buffer, length);
    }
    return buffer;
}

signal_file_t *load_signal_file(char *filepath) {
    return reinterpret_cast<signal_file_t *>(load_file(filepath));
}

#endif //DATAVIEWER_FILE_LOADER_H
