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

using namespace std;

static bool vector_of_strings_getter(void *data, int n, const char **out_text) {
    auto *v = (vector<const char *> *) data;
    *out_text = (*v)[n];
    return true;
}

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

static void show_signal_group(signal_group_t *view, int group_id) {
    if (!view->keep) {
        remove_group(group_id);
        return;
    }
    ImGui::BeginGroup();
    if (ImGui::CollapsingHeader(("Group " + to_string(group_id)).c_str(),
                                &view->keep, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushItemWidth(-1);
        if (!view->signal_views.empty()) {
            ImGui::ListBox(("##" + to_string(group_id)).c_str(),
                           &view->current_item,
                           vector_of_strings_getter,
                           (void *) &view->file_names,
                           view->file_names.size(),
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
            remove_current(view);
            // ignoring outher loop skip next view single frame
        }
        ImGui::SameLine();
        if (ImGui::Button("<", size)) {
            move_current_up(view);
        }
        ImGui::SameLine();
        if (ImGui::Button(">", size)) {
            move_current_down(view);
        }
        if (!view->signal_files.empty()) {
            ImGui::Separator();
            ImGui::Text("Channels:");
            signal_file_t *signal_file = view->signal_files[0];
            for (int i = 0; i < signal_file->n_channels; i++) {
                ImGui::PushID(i);
                // TODO: color formula (mod)
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV((group_id + i) / 7.0f, 0.8f, 0.8f));
                ImGui::SameLine();
                if (ImGui::Button(to_string(i + 1).c_str(), size)) {
                    // TODO: recalc frustum height
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
            show_info(view->merged_info);
        }
        ImGui::PopID();
    }

    ImGui::EndGroup();
}

static void show_signal_groups(vector<signal_group_t *> *views) {
    for (int i = 0; i < views->size(); i++) {
        show_signal_group(signal_groups[i], i);
    }
}

static void ShowFileMenu() {
    if (ImGui::MenuItem("Open", "Ctrl+O")) {
        add_files(-1);
    }
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
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}


#endif //DATAVIEWER_GUI_H
