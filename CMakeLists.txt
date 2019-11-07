cmake_minimum_required(VERSION 3.13)
project(DataViewer)

set(CMAKE_CXX_STANDARD 14)

add_executable(DataViewer src/main.cpp
        deps/glad/glad.c
        src/signal_file.h
        src/file_loader.h
        deps/imgui/imgui.cpp
        deps/imgui/imgui_draw.cpp
        deps/imgui/imgui_demo.cpp
        deps/imgui/imgui_widgets.cpp
        deps/imgui/examples/imgui_impl_opengl3.cpp
        deps/imgui/examples/imgui_impl_glfw.cpp
        deps/nfd/nfd_gtk.c
        deps/nfd/nfd_common.c src/file_repository.h src/gui.h src/signal_drawer.h src/serial_com.h)

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

# portable file dialogs
include_directories(deps/portable-file-dialogs)
include_directories(${GTK3_INCLUDE_DIRS})

## nfd
include_directories(deps/nfd deps/nfd/include)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
TARGET_LINK_LIBRARIES(DataViewer ${GTK3_LIBRARIES})