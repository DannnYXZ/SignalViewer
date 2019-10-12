#ifndef DATAVIEWER_SIGNAL_DRAWER_H
#define DATAVIEWER_SIGNAL_DRAWER_H

#include "file_repository.h"
#include "shader.h"

class SignalDrawer {
private:
    Shader *signal_shader;
public:
    SignalDrawer() {
        signal_shader = new Shader("shaders/signal.vs", "shaders/signal.fs");
        uint dxUniform = glGetUniformLocation(signal_shader->ID, "dx");
        uint signal_model_u = glGetUniformLocation(signal_shader->ID, "model");
        uint signal_view_u = glGetUniformLocation(signal_shader->ID, "view");
        uint signal_proj_u = glGetUniformLocation(signal_shader->ID, "proj");
    }

    void draw_signals(const vector<signal_view_t *> *views) {
        signal_shader->use();
        for (int i = 0; i < views->size(); i++) {
            signal_view_t *signal_view = (*views)[i];
            glBindVertexArray(signal_view->VAO);

            mat4 model(1.0f);
//            for (int i = 0; i < signal_views.size(); i++) {
//                glUniformMatrix4fv(signal_view_u, 1, GL_FALSE, value_ptr(global_view));
//                glUniformMatrix4fv(signal_proj_u, 1, GL_FALSE, value_ptr(global_proj));
//                glUniformMatrix4fv(signal_model_u, 1, GL_FALSE, value_ptr(model));
//                glUniform1f(dxUniform, 0.1f);
//                draw_signal(signal_views[i]);
//            }
        }
        //glDrawArrays(GL_LINE_STRIP, 0, signal_view->sfile->data_size);
    }
};

#endif //DATAVIEWER_SIGNAL_DRAWER_H
