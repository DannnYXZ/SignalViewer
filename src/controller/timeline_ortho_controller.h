#ifndef CG_TIMELINE_ORTHO_CONTROLLER_H
#define CG_TIMELINE_ORTHO_CONTROLLER_H

#include "../utils.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>

using namespace std;

class TimelineOrthoController {
    float EPS = 1e-6;
    float MOUSE_SENS = 3.f;
    float MOVE_SPEED = 10.0f;
    float SCALE_SPEED = 0.1f;
private:
    // ArcballCamera *camera;
    quat saved_rot;
    //int &scr_width, &scr_height;
    int scr_width, scr_height;
    vec2 last_mv{0, 0};
    bool drag_on = false;
    vec3 v_trans{0, 0, 0};
    vec2 scale{1, 1};
public:

    TimelineOrthoController() {}

    TimelineOrthoController(int scr_width, int scr_height) : scr_height(scr_height), scr_width(scr_width) {
    }

    void mouseMove(double xpos, double ypos) {

    }

    void mouseButton(int button, int action, int mods, double x, double y) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            drag_on = true;
            // saved_rot = quat(camera->orient);
            last_mv = vec2(x, y);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            drag_on = false;
        }
    }

    void mouseScroll(double xoffset, double yoffset) {
        scale += vec2(2.0 * xoffset * SCALE_SPEED, yoffset * SCALE_SPEED);
        print(vec3(scale, 1));
    }

    void processKey(int key, float deltatime) {
        if (key == GLFW_KEY_W)
            v_trans += vec3(0, 0, 1) * MOVE_SPEED * deltatime;
        if (key == GLFW_KEY_A)
            v_trans += vec3(-1, 0, 0) * MOVE_SPEED * (1 / scale.x) * deltatime;
        if (key == GLFW_KEY_S)
            v_trans += vec3(0, 0, -1) * MOVE_SPEED * deltatime;
        if (key == GLFW_KEY_D)
            v_trans += vec3(1, 0, 0) * MOVE_SPEED * (1 / scale.x) * deltatime;
    }

    mat4 view() {
        return translate(mat4(1), -v_trans);
    }

    mat4 proj() {
        return ortho(-scr_width / 2.0f / scale.x,
                     scr_width / 2.0f / scale.x,
                     -scr_height / 2.0f / scale.y,
                     scr_height / 2.0f / scale.y, 0.1f, -100.f);
    }

    void set_frustum_h(float h) {
        scr_height = (float) h;
    }
};

#endif //CG_TIMELINE_ORTHO_CONTROLLER_H
