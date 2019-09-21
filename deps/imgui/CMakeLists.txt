cmake_minimum_required(VERSION 3.13)

project(imgui)

set(SRC imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp)

add_library(imgui ${SRC})