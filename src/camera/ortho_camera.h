#ifndef CG_FPS_CAMERA_H
#define CG_FPS_CAMERA_H

#include <glm.hpp>
#include <gtc/constants.hpp>

using namespace glm;

class OrthoCamera {
private:
    vec3 m_pos;
    float m_pitch = 0, m_yaw = 0;

public:
    vec3 xaxis{}, yaxis{}, zaxis{};

    explicit OrthoCamera(vec3 pos) : m_pos(pos) {

    }

    mat4 view() {
        return ortho();
    }

    void translate(vec3 v) {
        m_pos += v;
    }
};

#endif //CG_FPS_CAMERA_H
