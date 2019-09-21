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
#include "fps_camera_controller.h"
#include "camera/arcball_camera.h"
#include "arcball_camera_controller.h"
#include "utils.h"
#include "portable-file-dialogs.h"
#include "nfd.h"

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
float timestamp;
float loop_deltatime;
double last_mouse_x, last_mouse_y;
int frames_cnt = 0;
double last_fps_time = 0;

mat4 global_view;
mat4 global_proj;

struct {
    GLenum polygon_mode = GL_FILL;
    bool drawPoints = true;
    int cursor_type = GLFW_CURSOR_NORMAL; // GLFW_CURSOR_DISABLED
    bool start_maximized = true;
    int transparent_background = GLFW_FALSE; // GLFW_FALSE
} global_settings;

static vector<char*> file_names;

static void ShowExampleMenuFile() {
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {
        nfdpathset_t user_path;
        nfdresult_t result = NFD_OpenDialogMultiple("bin1;png,jpg;pdf", NULL, &user_path);
        if (result == NFD_OKAY) {
            size_t i;
            for (i = 0; i < NFD_PathSet_GetCount(&user_path); ++i) {
                nfdchar_t *path = NFD_PathSet_GetPath(&user_path, i);
                file_names.push_back(path);
                printf("Path %i: %s\n", (int) i, path);
            }
            NFD_PathSet_Free(&user_path);
        } else if (result == NFD_CANCEL) {
            puts("User pressed cancel.");
        } else {
            printf("Error: %s\n", NFD_GetError());
        }
    }
    if (ImGui::BeginMenu("Open Recent")) {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More..")) {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse..")) {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options")) {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        static bool b = true;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::Checkbox("Check", &b);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors")) {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            const char *name = ImGui::GetStyleColorName((ImGuiCol) i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol) i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

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
    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark(); // ImGui::StyleColorsClassic();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    // ==============================================================================

    Shader planeShader("shaders/texture.vs", "shaders/texture.fs");
    uint planeModelUniform = glGetUniformLocation(planeShader.ID, "model");
    uint planeViewUniform = glGetUniformLocation(planeShader.ID, "view");
    uint planeProjUniform = glGetUniformLocation(planeShader.ID, "projection");

    glLineWidth(2);
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
        global_view = fps_camera.view();
        //global_view = arcball_camera.view();
        global_proj = perspective(radians(45.f), SCR_WIDTH * 1.f / SCR_HEIGHT, 0.1f, 100.f);
        //mat4 projection = perspective(radians(45.f), SCR_WIDTH * 1.f / SCR_HEIGHT, 0.1f, 100.f);
        float R = 10;
        vec3 cam_pos(cos(time) * R, 0, sin(time) * R);
        look_at_camera.translate(vec3(cam_pos.x, cam_pos.y, cam_pos.z) * loop_deltatime);
        //mat4 view = look_at_camera.view();

        frames_cnt++;
        if (time - last_fps_time >= 1.0) {
            printf("%f ms/frame\n", 1000.0 / double(frames_cnt));
            frames_cnt = 0;
            last_fps_time += 1.0;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show_demo_window);

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin(
                    "Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text(
                    "This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }
        {

            static bool show_app_documents = false;
            static bool show_app_main_menu_bar = false;
            static bool show_app_console = false;
            static bool show_app_log = false;
            static bool show_app_layout = false;
            static bool show_app_property_editor = false;
            static bool show_app_long_text = false;
            static bool show_app_auto_resize = false;
            static bool show_app_constrained_resize = false;
            static bool show_app_simple_overlay = false;
            static bool show_app_window_titles = false;
            static bool show_app_custom_rendering = false;

            static bool p_open = NULL;
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_MenuBar;
            ImGui::Begin("Signal Data", &p_open, window_flags);


            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Menu")) {
                    ShowExampleMenuFile();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Examples")) {
                    ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
                    ImGui::MenuItem("Console", NULL, &show_app_console);
                    ImGui::MenuItem("Log", NULL, &show_app_log);
                    ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
                    ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
                    ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
                    ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
                    ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
                    ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
                    ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
                    ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
                    ImGui::MenuItem("Documents", NULL, &show_app_documents);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            static int listbox_item_current = 1;
            char **listbox_items = file_names.data();
            if(listbox_items)
                ImGui::ListBox("listbox\n(single select)", &listbox_item_current, listbox_items, file_names.size(), 4);
            ImGui::End();

        }

        if (show_another_window) {
            ImGui::Begin("Another Window",
                         &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
        if (glfwGetKey(window, trackable_key) == GLFW_PRESS)
            fps_controller.processKey(trackable_key, loop_deltatime);
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
}