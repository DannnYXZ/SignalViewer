#ifndef DATAVIEWER_FILE_REPOSITORY_H
#define DATAVIEWER_FILE_REPOSITORY_H

#include <zconf.h>
#include <vector>
#include <glad.h>
#include "signal_struct.h"

using namespace std;

struct signal_view_t {
    bool keep = true;
    int current_item = 0;
    vector<signal_file_t *> signal_files;
    vector<const char *> file_names;
    signal_file_t *merged_info;
    uint VAO;
};
static vector<signal_view_t *> signal_views;

const vector<signal_view_t *> *get_signal_views() {
    return &signal_views;
}

void recalc_merged_signal_info(signal_view_t *view) {
    signal_file_t *merged_info = view->merged_info;
    merged_info->total_time_s = 0;
    // FIXME: zeroing merged info
    for (int i = 0; i < view->signal_files.size(); i++) {
        auto curr_signal_file = view->signal_files[i];
        merged_info->total_time_s += curr_signal_file->total_time_s;
        merged_info->n_blocks += curr_signal_file->n_blocks;
        merged_info->n_blocks_captured += curr_signal_file->n_blocks_captured;
        merged_info->data_size += curr_signal_file->data_size;
        merged_info->max_value = std::max(merged_info->max_value, curr_signal_file->max_value);
        merged_info->min_value = std::min(merged_info->min_value, curr_signal_file->min_value);

        merged_info->samples_per_block = curr_signal_file->samples_per_block;
        merged_info->block_time_s = curr_signal_file->block_time_s;
        merged_info->n_channels = curr_signal_file->n_channels;
        merged_info->n_spectra_lines = curr_signal_file->n_spectra_lines;
        merged_info->cutoff_freq = curr_signal_file->cutoff_freq;
        merged_info->block_time_s = curr_signal_file->block_time_s;
        merged_info->spectra_step_hz = curr_signal_file->spectra_step_hz;
    }
}

void move_current_up(signal_view_t *view) {
    int index = view->current_item;
    if (index - 1 >= 0) {
        swap(view->file_names[index], view->file_names[index - 1]);
        swap(view->signal_files[index], view->signal_files[index - 1]);
        view->current_item--;
    }
}

void move_current_down(signal_view_t *view) {
    int index = view->current_item;
    if (index + 1 < view->file_names.size()) {
        swap(view->file_names[index], view->file_names[index + 1]);
        swap(view->signal_files[index], view->signal_files[index + 1]);
        view->current_item++;
    }
}

void remove_current(signal_view_t *view) {
    view->file_names.erase(view->file_names.begin() + view->current_item);
    view->signal_files.erase(view->signal_files.begin() + view->current_item);
    view->current_item = std::max(0, view->current_item - 1);
}

bool compatible(signal_file_t *sf1, signal_file_t *sf2) {
    return (sf1->n_channels == sf2->n_channels)
           && (sf1->block_time_s == sf2->block_time_s)
           && (sf1->samples_per_block == sf2->samples_per_block);
}

// group == -1 for new group
void add_signal_file(char *filepath, int group) {
    signal_file_t *file = load_signal_file(filepath);
    if (!file)
        return;
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
        auto *signal_view = new signal_view_t();
        signal_view->signal_files.push_back(file);
        signal_view->file_names.push_back(base_name);
        signal_view->merged_info = new signal_file_t();
        recalc_merged_signal_info(signal_view);
        signal_views.push_back(signal_view);
    } else {
        if (signal_views[group]->signal_files.empty()
            || compatible(signal_views[group]->signal_files[0], file)) {
            signal_views[group]->file_names.push_back(base_name);
            signal_views[group]->signal_files.push_back(file);
            recalc_merged_signal_info(signal_views[group]);
        } else {
            // TODO: what to do?
        }
    }
}

#endif //DATAVIEWER_FILE_REPOSITORY_H
