#ifndef DATAVIEWER_FILE_REPOSITORY_H
#define DATAVIEWER_FILE_REPOSITORY_H

#include <zconf.h>
#include <vector>
#include <glad.h>
#include "../signal_struct.h"

using namespace std;

struct signal_view {
    signal_file *sfile;
    uint VAO;
};

vector<signal_view *> signal_views;

void add_signal_file(signal_file *file) {
    uint signalVAO, signalBAO;
    glGenBuffers(1, &signalBAO);
    glGenVertexArrays(1, &signalVAO);
    glBindVertexArray(signalVAO);
    glBindBuffer(GL_VERTEX_ARRAY, signalBAO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(4 * file->data_size), file->data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 4, file->data);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    signal_views.push_back(new signal_view{file, signalVAO});
}

void draw_signal(signal_view *signal_view) {
    glBindVertexArray(signal_view->VAO);
    glDrawArrays(GL_LINE_STRIP, 0, signal_view->sfile->data_size);
}

#endif //DATAVIEWER_FILE_REPOSITORY_H
