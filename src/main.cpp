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
#include "camera/look_at_camera.h"
#include "camera/fps_camera.h"
#include "controller/fps_camera_controller.h"
#include "camera/arcball_camera.h"
#include "controller/arcball_camera_controller.h"
#include "controller/timeline_ortho_controller.h"
#include "utils.h"
#include "portable-file-dialogs.h"
#include "nfd.h"
#include "gui.h"
#include "signal_file.h"
#include "signal_drawer.h"

using namespace glm;

int SCR_WIDTH = 800;
int SCR_HEIGHT = 800;

static void error_callback(int error, const char *description);

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

static void process_input(GLFWwindow *window);

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

LookAtCamera look_at_camera(vec3(0, 0, 3), vec3(0, 0, 0), vec3(0, 1, 0));
FPSCamera fps_camera(vec3(0, 0, 3));
FPSCameraController fps_controller(&fps_camera);
ArcballCamera arcball_camera(identity<quat>(), vec3(0, 0, 0), vec3(0, 0, 5));
ArcballCameraController arcball_controller(&arcball_camera, SCR_WIDTH, SCR_HEIGHT);
TimelineOrthoController ortho_controller(SCR_WIDTH, SCR_HEIGHT);
float timestamp;
float loop_deltatime;
double last_mouse_x, last_mouse_y;
int frames_cnt = 0;
double last_fps_time = 0;
float FOV = 45.f;

mat4 global_view;
mat4 global_proj;

struct {
    GLenum polygon_mode = GL_FILL;
    bool drawPoints = true;
    int cursor_type = GLFW_CURSOR_NORMAL; // GLFW_CURSOR_DISABLED
    bool start_maximized = true;
    int transparent_background = GLFW_FALSE; // GLFW_FALSE
} global_settings;

int main() {
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, global_settings.transparent_background);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
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
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    // setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    // ==============================================================================

    SignalDrawer *drawer = new SignalDrawer();

    glLineWidth(3);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while (!glfwWindowShouldClose(window)) {
        //glClearColor(1.f * 57 / 255, 1.f * 57 / 255, 1.f * 57 / 255, 0.5f);
        if (global_settings.transparent_background) glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, global_settings.polygon_mode);

        // time stuff
        double time = glfwGetTime();
        loop_deltatime = time - timestamp;
        timestamp = time;

        // calculating MVP
        //global_view = fps_camera.view();
        //global_view = arcball_camera.view();
        global_view = ortho_controller.view();

        //global_view = look_at_camera.view();
        //float C = 10.f;
        //float R = 10;
        //vec3 cam_pos(cos(time) * R, 0, sin(time) * R);
        //look_at_camera.translate(vec3(cam_pos.x, cam_pos.y, cam_pos.z) * loop_deltatime);

        frames_cnt++;
        //global_proj = perspective(radians(FOV), SCR_WIDTH * 1.f / SCR_HEIGHT, 0.1f, 1000.f);
        //global_proj = ortho(-(float)SCR_WIDTH*C,(float)SCR_WIDTH*C, -(float)SCR_HEIGHT*C, (float)SCR_HEIGHT*C, .1f, 10000.f);
        global_proj = ortho_controller.proj();

        if (time - last_fps_time >= 1.0) {
            printf("%f ms/frame\n", 1000.0 / double(frames_cnt));
            frames_cnt = 0;
            last_fps_time += 1.0;
        }


        drawer->draw_signal_groups(get_signal_views());

        // start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show_demo_window);

        {
            static bool p_open = NULL;
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_MenuBar;
            ImGui::Begin("Signal Data", &p_open, window_flags);
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    ShowFileMenu();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            show_signal_groups(&signal_groups);
            ImGui::End();
        }

        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
// ========================================================================================================
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
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
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
            fps_controller.processKey(trackable_key, loop_deltatime);
            ortho_controller.processKey(trackable_key, loop_deltatime);
        }
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    static bool first_mouse = true;
    if (first_mouse) {
        first_mouse = false;
        last_mouse_x = xpos;
        last_mouse_y = ypos;
    }
    float dx = xpos - last_mouse_x;
    float dy = ypos - last_mouse_y;
    last_mouse_x = xpos;
    last_mouse_y = ypos;
    fps_controller.process_mouse(dx, dy);
    arcball_controller.mouseMove(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    arcball_controller.mouseButton(button, action, mods, mx, my);
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    arcball_controller.mouseScroll(xoffset, yoffset);
    ortho_controller.mouseScroll(xoffset, yoffset);
    //FOV += 1.f * yoffset;
    //SCR_WIDTH += 10 * yoffset;
    //SCR_HEIGHT += 10 * xoffset;
}