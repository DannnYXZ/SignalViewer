#ifndef DATAVIEWER_SIGNAL_DRAWER_H
#define DATAVIEWER_SIGNAL_DRAWER_H

#include "file_repository.h"
#include "shader.h"
#include "serial_com.h"

class SignalDrawer {
private:
    Shader *shader_signal, *shader_adaptive_grid;
    TimelineOrthoController *ortho_controller;
    FPSCameraController *fps_controller;
    uint u_dx, u_model, u_view, u_proj, u_offset, u_base_index;
    uint u_res, u_wnd_p, u_wnd_a;
    uint quadVAO, quadVBO;
    vec2 &resolution;
public:
    SignalDrawer(vec2 &resolution) : resolution(resolution) {
        shader_signal = new Shader("shaders/signal.vs", "shaders/signal.fs");
        shader_adaptive_grid = new Shader("shaders/adaptive_grid.vs", "shaders/adaptive_grid.fs");
        u_dx = glGetUniformLocation(shader_signal->ID, "dx");
        u_offset = glGetUniformLocation(shader_signal->ID, "offset");
        u_model = glGetUniformLocation(shader_signal->ID, "model");
        u_view = glGetUniformLocation(shader_signal->ID, "view");
        u_proj = glGetUniformLocation(shader_signal->ID, "proj");

        u_res = glGetUniformLocation(shader_adaptive_grid->ID, "resolution");
        u_wnd_p = glGetUniformLocation(shader_adaptive_grid->ID, "wnd_p");
        u_wnd_a = glGetUniformLocation(shader_adaptive_grid->ID, "wnd_a");

        glGenVertexArrays(1, &quadVAO);

        ortho_controller = new TimelineOrthoController(vec2{1, 1});
        fps_controller = new FPSCameraController(new FPSCamera(vec3(0, 0, 3)), resolution);
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
        shader_signal->use();
        mat4 model(1.0f);
        glUniformMatrix4fv(u_view, 1, GL_FALSE, value_ptr(ortho_controller->view()));
        glUniformMatrix4fv(u_proj, 1, GL_FALSE, value_ptr(ortho_controller->proj()));
//        glUniformMatrix4fv(u_view, 1, GL_FALSE, value_ptr(fps_controller->view()));
//        glUniformMatrix4fv(u_proj, 1, GL_FALSE, value_ptr(fps_controller->proj()));
        glUniformMatrix4fv(u_model, 1, GL_FALSE, value_ptr(model));
        for (auto group : *groups) {
            draw_signal_group(group);
        }
    }

    void draw_grid() {
        shader_adaptive_grid->use();
        glUniform2fv(u_res, 1, value_ptr(resolution));
        glUniform2fv(u_wnd_p, 1, value_ptr(ortho_controller->get_pos()));
        glUniform2fv(u_wnd_a, 1, value_ptr(ortho_controller->get_wnd()));
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void draw_rolling_signal(float *rolling_buffer, int index, int buffer_size, uint rollingVAO, uint rollingBAO) {
        shader_signal->use();
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

    void process_key(int key, float deltatime) {
        ortho_controller->process_key(key, deltatime);
        fps_controller->process_key(key, deltatime);
    }

    void process_mouse(float dx, float dy) {
        fps_controller->process_mouse(dx, dy);
    }

    void process_scroll(double xoffset, double yoffset) {
        ortho_controller->process_scroll(xoffset, yoffset);
        fps_controller->process_scroll(xoffset, yoffset);
    }

    void zoom(float height) {
        ortho_controller->set_frustum_h(height);
    }
};

#endif //DATAVIEWER_SIGNAL_DRAWER_H
