#ifndef CG_FPS_CAMERA_CONTROLLER_H
#define CG_FPS_CAMERA_CONTROLLER_H

#include <GLFW/glfw3.h>
#include "../camera/fps_camera.h"

class FPSCameraController {
private:
    FPSCamera *camera;
    vec2 &resolution;
public:
    float MOVE_SPEED = 10.0f;
    float ROT_SPEED = 1.5f;
    float MOUSE_SENSITIVITY = 0.005f;
    float SCALE_SPEED = 0.01f;
    float FIDELITY = 1e-5f;
    float FOV = 45.f;

    explicit FPSCameraController(FPSCamera *camera, vec2 &resolution) : resolution(resolution) {
        this->camera = camera;
    }

    void process_key(int key, float deltatime) {
        if (key == GLFW_KEY_W)
            camera->translate(-camera->zaxis * MOVE_SPEED * deltatime);
        if (key == GLFW_KEY_A)
            camera->translate(-camera->xaxis * MOVE_SPEED * deltatime);
        if (key == GLFW_KEY_S)
            camera->translate(camera->zaxis * MOVE_SPEED * deltatime);
        if (key == GLFW_KEY_D)
            camera->translate(camera->xaxis * MOVE_SPEED * deltatime);
        if (key == GLFW_KEY_SPACE)
            camera->translate(camera->yaxis * MOVE_SPEED * deltatime);
        if (key == GLFW_KEY_LEFT_SHIFT)
            camera->translate(-camera->yaxis * MOVE_SPEED * deltatime);
    }

    void process_mouse(float dx, float dy) {
        camera->yaw(-dx * MOUSE_SENSITIVITY * ROT_SPEED);
        camera->pitch(dy * MOUSE_SENSITIVITY * ROT_SPEED);
    }

    void process_scroll(double xoffset, double yoffset) {
        xoffset != 0 ? FOV *= 1 + sign(xoffset) * SCALE_SPEED : FOV = FOV;
        yoffset != 0 ? FOV *= 1 + sign(yoffset) * SCALE_SPEED : FOV = FOV;
        FOV = max(FIDELITY, FOV);
    }

    mat4 view() {
        return camera->view();
    }

    mat4 proj() {
        return perspective(radians(FOV),
                           resolution.x * 1.f / resolution.y,
                           0.1f, 1000.f);
    }

};

#endif //CG_FPS_CAMERA_CONTROLLER_H
