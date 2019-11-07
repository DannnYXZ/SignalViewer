#ifndef DATAVIEWER_SIGNAL_DRAWER_H
#define DATAVIEWER_SIGNAL_DRAWER_H

#include "file_repository.h"
#include "shader.h"

class SignalDrawer {
private:
    Shader *signal_shader;
    TimelineOrthoController *ortho_controller;
    uint u_dx, u_model, u_view, u_proj, u_offset, u_base_index;
public:
    SignalDrawer() {
        signal_shader = new Shader("shaders/signal.vs", "shaders/signal.fs");
        u_dx = glGetUniformLocation(signal_shader->ID, "dx");
        u_offset = glGetUniformLocation(signal_shader->ID, "offset");
        u_model = glGetUniformLocation(signal_shader->ID, "model");
        u_view = glGetUniformLocation(signal_shader->ID, "view");
        u_proj = glGetUniformLocation(signal_shader->ID, "proj");
        u_base_index = glGetUniformLocation(signal_shader->ID, "base_index");
        ortho_controller = new TimelineOrthoController(10, 10); // TODO: deside
    }

    void draw_signal_group(signal_group_t *group) {
        float offset = 0;
        for (auto signal_view : group->signal_views) {
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

    void draw_rolling_signal(float *rolling_buffer, int index, int buffer_size, uint rollingVAO, uint rollingBAO) {
        signal_shader->use();
        mat4 model(1.0f);
        glUniformMatrix4fv(u_view, 1, GL_FALSE, value_ptr(ortho_controller->view()));
        glUniformMatrix4fv(u_proj, 1, GL_FALSE, value_ptr(ortho_controller->proj()));
        glUniformMatrix4fv(u_model, 1, GL_FALSE, value_ptr(model));

        glBindVertexArray(rollingVAO);
        glBindBuffer(GL_ARRAY_BUFFER, rollingBAO);
        glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), rolling_buffer, GL_STATIC_DRAW);

        float dx = .01;
        glUniform1f(u_dx, dx);
        glUniform1f(u_offset, 0);
        glUniform1i(u_base_index, index);
        glDrawArrays(GL_LINE_STRIP, index, buffer_size - index);
        glUniform1i(u_base_index, 0);
        glUniform1f(u_offset, dx * (buffer_size - index));
        glDrawArrays(GL_LINE_STRIP, 0, index);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void processKey(int key, float deltatime) {
        ortho_controller->processKey(key, deltatime);
    }

    void mouseScroll(double xoffset, double yoffset) {
        ortho_controller->mouseScroll(xoffset, yoffset);
    }

    void zoom(float height) {
        ortho_controller->set_frustum_h(height);
    }
};

#endif //DATAVIEWER_SIGNAL_DRAWER_H
