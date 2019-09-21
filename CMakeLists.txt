cmake_minimum_required(VERSION 3.13)
project(DataViewer)

set(CMAKE_CXX_STANDARD 14)

add_executable(DataViewer src/main.cpp
        deps/glad/glad.c
        src/signal_data_struct.h
        src/file_loader.h
        deps/imgui/imgui.cpp
        deps/imgui/imgui_draw.cpp
        deps/imgui/imgui_demo.cpp
        deps/imgui/imgui_widgets.cpp
        deps/imgui/examples/imgui_impl_opengl3.cpp
        deps/imgui/examples/imgui_impl_glfw.cpp)

# configure GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)
target_link_libraries(DataViewer glfw)

# OpenGL
find_package(OpenGL REQUIRED)
target_include_directories(DataViewer PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(DataViewer ${OPENGL_gl_LIBRARY})

# GLAD
include_directories(deps/glad)

#STB
include_directories(deps/stb_image)

#GLM
include_directories(deps/glm)

# ImGui
include_directories(deps/imgui deps/imgui/examples)
