#ifndef DATAVIEWER_FILE_LOADER_H
#define DATAVIEWER_FILE_LOADER_H

#include <fstream>
#include <iostream>
#include <cstring>
#include "signal_struct.h"

using namespace std;

char *load_file(char *filepath) {
    ifstream is(filepath, ifstream::binary);
    char *buffer = nullptr;
    if (is) {
        is.seekg(0, is.end);
        int length = is.tellg();
        is.seekg(0, is.beg);
        // allocate memory:
        buffer = new char[length];
        // read data as a block:
        is.read(buffer, length);
        is.close();
        // print content:
        //cout.write(buffer, length);
    }
    return buffer;
}

signal_file_t *load_signal_file(char *filepath) {
    return reinterpret_cast<signal_file_t*>(load_file(filepath));
}

#endif //DATAVIEWER_FILE_LOADER_H
