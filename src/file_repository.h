#ifndef DATAVIEWER_FILE_REPOSITORY_H
#define DATAVIEWER_FILE_REPOSITORY_H

#include <zconf.h>
#include <vector>
#include <glad.h>
#include "signal_file.h"

using namespace std;

struct signal_view_t {
    const char *file_name;
    signal_file_t *signal_file;
    uint vao;
};

struct signal_group_t {
    bool keep = true;
    int current_item = 0;
    vector<signal_view_t> signal_views;
    vector<ImVec4> channel_colors;
    signal_file_t *merged_info;
};

static vector<signal_group_t *> signal_groups;

const vector<signal_group_t *> *get_signal_views() {
    return &signal_groups;
}

void recalc_merged_signal_info(signal_group_t *group) {
    signal_file_t *merged_info = group->merged_info;
    merged_info->total_time_s = 0;
    memset(merged_info, 0, sizeof(signal_file_t));
    for (auto &signal_view : group->signal_views) {
        signal_file_t *curr_signal_file = signal_view.signal_file;
        merged_info->total_time_s += curr_signal_file->total_time_s;
        merged_info->n_blocks += curr_signal_file->n_blocks;
        merged_info->n_blocks_captured += curr_signal_file->n_blocks_captured;
        merged_info->data_size += curr_signal_file->data_size;
        merged_info->max_value = std::max(merged_info->max_value, curr_signal_file->max_value);
        merged_info->min_value = std::min(merged_info->min_value, curr_signal_file->min_value);

        merged_info->samples_per_block = curr_signal_file->samples_per_block;
        merged_info->time_per_block_s = curr_signal_file->time_per_block_s;
        merged_info->n_channels = curr_signal_file->n_channels;
        merged_info->n_spectra_lines = curr_signal_file->n_spectra_lines;
        merged_info->cutoff_freq = curr_signal_file->cutoff_freq;
        merged_info->time_per_block_s = curr_signal_file->time_per_block_s;
        merged_info->spectra_step_hz = curr_signal_file->spectra_step_hz;
    }
}

void move_current_up(signal_group_t *group) {
    int index = group->current_item;
    if (index - 1 >= 0) {
        swap(group->signal_views[index], group->signal_views[index - 1]);
        group->current_item--;
    }
}

void move_current_down(signal_group_t *group) {
    int index = group->current_item;
    if (index + 1 < group->signal_views.size()) {
        swap(group->signal_views[index], group->signal_views[index + 1]);
        group->current_item++;
    }
}

void remove_current(signal_group_t *group) {
    group->signal_views.erase(group->signal_views.begin() + group->current_item);
    group->current_item = std::max(0, group->current_item - 1);
    recalc_merged_signal_info(group);
}

void remove_group(int group_id) {
    signal_groups.erase(signal_groups.begin() + group_id);
}

bool compatible(signal_file_t *sf1, signal_file_t *sf2) {
    return (sf1->n_channels == sf2->n_channels)
           && (sf1->time_per_block_s == sf2->time_per_block_s)
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
        auto *signal_group = new signal_group_t();
        signal_group->signal_views.push_back(signal_view_t{base_name, file, signalVAO});
        signal_group->merged_info = new signal_file_t();
        recalc_merged_signal_info(signal_group);
        signal_groups.push_back(signal_group);
    } else {
        if (signal_groups[group]->signal_views.empty()
            || compatible(signal_groups[group]->signal_views[0].signal_file, file)) {
            signal_groups[group]->signal_views.push_back(signal_view_t{base_name, file, signalVAO});
            recalc_merged_signal_info(signal_groups[group]);
        } else {
            // TODO: what to do?
        }
    }
}

#endif //DATAVIEWER_FILE_REPOSITORY_H
