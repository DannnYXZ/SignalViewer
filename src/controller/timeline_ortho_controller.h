#ifndef CG_TIMELINE_ORTHO_CONTROLLER_H
#define CG_TIMELINE_ORTHO_CONTROLLER_H

#include "../utils.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>

using namespace std;

class TimelineOrthoController {
    float EPS = 1e-6;
    float MOUSE_SENS = 3.f;
    float MOVE_SPEED = 1.f;
    float SCALE_SPEED = 0.01f;
    float FIDELITY = 1e-9f;
private:
    quat saved_rot{};
    vec2 last_mv{0, 0};
    bool drag_on = false;
    vec3 v_trans{0, 0, 0};
    vec2 scale{};
public:

    TimelineOrthoController() = default;

    explicit TimelineOrthoController(vec2 scale) : scale(scale) {
    }

    void mouseMove(double xpos, double ypos) {
    }

    void mouseButton(int button, int action, int mods, double x, double y) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            drag_on = true;
            last_mv = vec2(x, y);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            drag_on = false;
        }
    }

    void process_scroll(double xoffset, double yoffset) {
        xoffset != 0 ? scale.x *= 1 + sign(xoffset) * SCALE_SPEED : scale.x = scale.x;
        yoffset != 0 ? scale.y *= 1 + sign(yoffset) * SCALE_SPEED : scale.y = scale.y;
        scale = max(vec2(FIDELITY), scale);
        print(vec3(scale, 1));
    }

    void process_key(int key, float deltatime) {
        if (key == GLFW_KEY_A)
            v_trans += vec3(-1, 0, 0) * MOVE_SPEED * scale.x * deltatime;
        if (key == GLFW_KEY_D)
            v_trans += vec3(1, 0, 0) * MOVE_SPEED * scale.x * deltatime;
    }

    mat4 view() {
        return translate(mat4(1), -v_trans);
    }

    mat4 proj() {
        return ortho(-scale.x,
                     scale.x,
                     -scale.y,
                     scale.y, 1.f, -1.f);
    }

    void set_frustum_h(float h) {
        scale.y = (float) h;
    }

    vec2 &get_wnd() {
        return scale;
    }

    vec3 &get_pos() {
        return v_trans;
    }
};

#endif //CG_TIMELINE_ORTHO_CONTROLLER_H
