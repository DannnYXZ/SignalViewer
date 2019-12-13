#ifndef DATAVIEWER_GUI_H
#define DATAVIEWER_GUI_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <include/nfd.h>
#include <vector>
#include <string>
#include "signal_file.h"
#include "file_loader.h"
#include "file_repository.h"
#include "signal_drawer.h"

using namespace std;

static void add_files(int group_id) {
    nfdpathset_t user_path;
    nfdresult_t result = NFD_OpenDialogMultiple("bin1;png,jpg;pdf", NULL, &user_path);
    if (result == NFD_OKAY) {
        size_t i;
        for (i = 0; i < NFD_PathSet_GetCount(&user_path); ++i) {
            nfdchar_t *path = NFD_PathSet_GetPath(&user_path, i);
            add_signal_file(path, group_id);
            printf("Path %i: %s\n", (int) i, path);
        }
        NFD_PathSet_Free(&user_path);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }
}

static void show_info(signal_file_t *signal_file) {
    ImGui::Text("Data size: %ub", signal_file->data_size);
    ImGui::Text("Blocks entered: %u", signal_file->n_blocks);
    ImGui::Text("Blocks captured: %u", signal_file->n_blocks_captured);
    ImGui::Text("Capture time: %us", signal_file->total_time_s);
    ImGui::Text("Time per block: %.3fs", signal_file->time_per_block_s);
    ImGui::Text("Samples per block: %u", signal_file->samples_per_block);
    ImGui::Text("Cutoff frequency: %uHz", signal_file->cutoff_freq);
    ImGui::Text("Spectral lines: %u", signal_file->n_spectra_lines);
    ImGui::Text("Max value: %.3f", signal_file->max_value);
    ImGui::Text("Min value: %.3f", signal_file->min_value);
}

static bool vector_of_strings_getter(void *data, int n, const char **out_text) {
    auto *v = (vector<signal_view_t *> *) data;
    *out_text = (*v)[n]->file_name;
    return true;
}

static void show_signal_group(signal_group_t *group, int group_id, SignalDrawer *signal_drawer) {
    if (!group->keep) {
        remove_group(group_id);
        return;
    }
    ImGui::BeginGroup();
    if (ImGui::CollapsingHeader(("Group " + to_string(group_id)).c_str(),
                                &group->keep, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushItemWidth(-1);
        if (!group->signal_views.empty()) {
            ImGui::ListBox(("##" + to_string(group_id)).c_str(),
                           &group->current_item,
                           vector_of_strings_getter,
                           (void *) &group->signal_views,
                           group->signal_views.size(),
                           -1);
        }
        ImGui::PopItemWidth();
        ImVec2 size{20.0f, 20.0f};
        ImGui::PushID(group_id);
        if (ImGui::Button("+", size)) {
            add_files(group_id);
        }
        ImGui::SameLine();
        if (ImGui::Button("-", size)) {
            remove_current(group);
            // ignoring outher loop skip next view single frame
        }
        ImGui::SameLine();
        if (ImGui::Button("<", size)) {
            move_current_up(group);
        }
        ImGui::SameLine();
        if (ImGui::Button(">", size)) {
            move_current_down(group);
        }
        if (!group->signal_views.empty()) {
            ImGui::Separator();
            ImGui::Text("Channels:");
            signal_file_t *signal_file = group->signal_views[0]->signal_file;
            for (int i = 0; i < signal_file->n_channels; i++) {
                ImGui::PushID(i);
                // TODO: color formula (mod)
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.8f, 0.8f));
                ImGui::SameLine();
                if (ImGui::Button(to_string(i + 1).c_str(), size)) {
                    signal_drawer->zoom(signal_file->max_value);
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
            show_info(group->merged_info);
        }
        ImGui::PopID();
    }
    ImGui::EndGroup();
}

static void show_signal_groups(vector<signal_group_t *> *views, SignalDrawer *signal_drawer) {
    for (int i = 0; i < views->size(); i++) {
        show_signal_group(signal_groups[i], i, signal_drawer);
    }
}

static void show_file_menu() {
    if (ImGui::MenuItem("Open")) {
        add_files(-1);
    }
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        exit(0);
    }
}

static void draw_signal_manager(SignalDrawer *signal_drawer) {
    static bool p_open = NULL;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_MenuBar;
    ImGui::Begin("Signal Data", &p_open, window_flags);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            show_file_menu();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("MyTabBar", true)) {
        if (ImGui::BeginTabItem("Explorer")) {
            show_signal_groups(&signal_groups, signal_drawer);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Capture")) {
            static char port_buf[64] = "ttyUSB0";
            ImGui::InputText("serial port", port_buf, 64);
            static bool capture_serial = false;
            if (ImGui::Button(capture_serial ? "STOP" : "START")) {
                capture_serial = !capture_serial;
                if (capture_serial) {
                    serial_com_init(port_buf);
                } else {
                    rolling_signal_destroy();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

#endif //DATAVIEWER_GUI_H
