#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <stb_image.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "shader.h"
#include "camera/fps_camera.h"
#include "controller/fps_camera_controller.h"
#include "controller/timeline_ortho_controller.h"
#include "utils.h"
#include "portable-file-dialogs.h"
#include "nfd.h"
#include "gui.h"
#include "signal_file.h"
#include "signal_drawer.h"
#include "serial_port.h"

using namespace glm;

static void error_callback(int error, const char *description);

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

static void process_input(GLFWwindow *window);

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

struct {
    vec2 resolution{800, 800};
    GLenum polygon_mode = GL_FILL;
    float line_width = 3;
    bool drawPoints = true;
    int cursor_type = GLFW_CURSOR_NORMAL; // GLFW_CURSOR_DISABLED
    bool start_maximized = true;
    int transparent_background = GLFW_FALSE; // GLFW_TRUE
} global_settings;

float prev_time;
float loop_deltatime;
SignalDrawer *signal_drawer;

int main() {
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, global_settings.transparent_background);
    GLFWwindow *window = glfwCreateWindow((int) global_settings.resolution.x,
                                          (int) global_settings.resolution.y,
                                          "OpenGL", NULL, NULL);
    if (global_settings.start_maximized) glfwMaximizeWindow(window);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, global_settings.cursor_type);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // setup ImGui context
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    // setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
//    ==============================================================================

    signal_drawer = new SignalDrawer(global_settings.resolution);
    glLineWidth(global_settings.line_width);
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_PROGRAM_POINT_SIZE);

    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
//        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        if (global_settings.transparent_background) glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, global_settings.polygon_mode);

        // time stuff
        double time = glfwGetTime();
        loop_deltatime = time - prev_time;
        prev_time = time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(&show_demo_window);
        draw_signal_manager(signal_drawer);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
// ========================================================================================================
        // SPF-meter
        static double last_fps_time = 0;
        static int frames_cnt = 0;
        frames_cnt++;
        if (time - last_fps_time >= 1.0) {
            printf("%f ms/frame\n", 1000.0 / double(frames_cnt));
            frames_cnt = 0;
            last_fps_time += 1.0;
        }
        process_input(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    global_settings.resolution = vec2(width, height);
    glViewport(0, 0, width, height);
    // mouse coordinates now are not the same
}

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (global_settings.polygon_mode == GL_LINE)
            global_settings.polygon_mode = GL_FILL;
        else
            global_settings.polygon_mode = GL_LINE;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        global_settings.drawPoints = !global_settings.drawPoints;
}

static void process_input(GLFWwindow *window) {
    int trackable_keys[] = {
            GLFW_KEY_W,
            GLFW_KEY_A,
            GLFW_KEY_S,
            GLFW_KEY_D,
            GLFW_KEY_Q,
            GLFW_KEY_E,
            GLFW_KEY_Z,
            GLFW_KEY_C,
            GLFW_KEY_SPACE,
            GLFW_KEY_LEFT_SHIFT
    };

    for (int trackable_key : trackable_keys) {
        if (glfwGetKey(window, trackable_key) == GLFW_PRESS) {
            signal_drawer->process_key(trackable_key, loop_deltatime);
        }
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    static bool first_mouse = true;
    static vec2 last_mouse_p{};
    if (first_mouse) {
        first_mouse = false;
        last_mouse_p = vec2(xpos, ypos);
    }
    vec2 d = vec2(xpos - last_mouse_p.x, ypos - last_mouse_p.y);
    last_mouse_p = vec2(xpos, ypos);
    signal_drawer->process_mouse(d.x, d.y);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    signal_drawer->process_scroll(xoffset, yoffset);
}