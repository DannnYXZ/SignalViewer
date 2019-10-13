#ifndef DATAVIEWER_SIGNAL_DRAWER_H
#define DATAVIEWER_SIGNAL_DRAWER_H

#include "file_repository.h"
#include "shader.h"

class SignalDrawer {
private:
    Shader *signal_shader;
    TimelineOrthoController *ortho_controller;
    uint u_dx, u_model, u_view, u_proj, u_offset;
public:
    SignalDrawer() {
        signal_shader = new Shader("shaders/signal.vs", "shaders/signal.fs");
        u_dx = glGetUniformLocation(signal_shader->ID, "dx");
        u_offset = glGetUniformLocation(signal_shader->ID, "offset");
        u_model = glGetUniformLocation(signal_shader->ID, "model");
        u_view = glGetUniformLocation(signal_shader->ID, "view");
        u_proj = glGetUniformLocation(signal_shader->ID, "proj");
        ortho_controller = new TimelineOrthoController();
    }

    void draw_signal_group(signal_group_t *group) {
        float offset = 0;
        for (int i = 0; i < group->signal_views.size(); i++) {
            signal_view_t *signal_view;
            glBindVertexArray(signal_view->vao);
            glUniform1f(u_offset, offset);
            glUniform1f(u_dx, signal_view->signal_file->time_per_block_s / signal_view->signal_file->samples_per_block);
            glDrawArrays(GL_LINE_STRIP, 0, signal_view->signal_file->data_size);
            offset += signal_view->signal_file->time_per_block_s * signal_view->signal_file->n_blocks_captured;
        }
    }

    void draw_signal_groups(const vector<signal_group_t *> *groups) {
        signal_shader->use();
        mat4 model(1.0f);
        glUniformMatrix4fv(u_view, 1, GL_FALSE, value_ptr(ortho_controller->view()));
        glUniformMatrix4fv(u_proj, 1, GL_FALSE, value_ptr(ortho_controller->proj()));
        glUniformMatrix4fv(u_model, 1, GL_FALSE, value_ptr(model));
        for (auto group : *groups) {
            draw_signal_group(group);
        }
    }
};

#endif //DATAVIEWER_SIGNAL_DRAWER_H