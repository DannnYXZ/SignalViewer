#ifndef DATAVIEWER_FILE_REPOSITORY_H
#define DATAVIEWER_FILE_REPOSITORY_H

#include <zconf.h>
#include <vector>
#include <glad.h>
#include "../signal_struct.h"

using namespace std;

struct signal_view {
    bool keep = true;
    vector<signal_file *> signal_files;
    vector<const char *> file_names;
    uint VAO;
};

vector<signal_view *> signal_views;

// group = -1 for new group
void add_signal_file(signal_file *file, char *filepath, int group) {
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

    char *base_name = strdup(basename(filepath));
    if (group == -1) {
        auto *signal_view = new struct signal_view();
        signal_view->signal_files.push_back(file);
        signal_view->file_names.push_back(base_name);
        signal_views.push_back(signal_view);
    } else {
        // TODO: check file compatibility
        signal_views[group]->file_names.push_back(base_name);
        signal_views[group]->signal_files.push_back(file);
    }
}

void draw_signal(signal_view *signal_view) {
    glBindVertexArray(signal_view->VAO);
    //glDrawArrays(GL_LINE_STRIP, 0, signal_view->sfile->data_size);
}

#endif //DATAVIEWER_FILE_REPOSITORY_H
