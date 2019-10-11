#ifndef DATAVIEWER_GUI_H
#define DATAVIEWER_GUI_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <include/nfd.h>
#include <vector>
#include <string>
#include "signal_struct.h"
#include "file_loader.h"
#include "file_repository/file_repository.h"

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
            signal_file *sfile = load_signal_file(path);
            if (sfile) {
                add_signal_file(sfile, path, group_id);
            }
            printf("Path %i: %s\n", (int) i, path);
        }
        NFD_PathSet_Free(&user_path);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }
}

static void show_signal_group(signal_view *view, int group_id) {
    ImGui::BeginGroup();
    if (ImGui::CollapsingHeader(("Group " + to_string(group_id)).c_str(), &view->keep)) {
        static int listbox_item_current = 0;
        //char **listbox_items = view->file_names.data();
        ImGui::PushItemWidth(-1);
        if (!view->file_names.empty()) {
            ImGui::ListBox(("##" + to_string(group_id)).c_str(),
                           &listbox_item_current,
                           vector_of_strings_getter,
                           (void *) &view->file_names,
                           view->file_names.size(),
                           -1);
            ImVec2 size{20.0f, 20.0f};
            if (ImGui::Button("+", size)) {
                add_files(group_id);
            }
            ImGui::SameLine();
            ImGui::Button("-", size);
            ImGui::SameLine();
            ImGui::Button("<", size);
            ImGui::SameLine();
            ImGui::Button(">", size);
        }
        ImGui::PopItemWidth();
    }

    ImGui::EndGroup();
}

static void show_signal_groups(vector<signal_view *> *views) {
    for (int i = 0; i < views->size(); i++) {
        show_signal_group(signal_views[i], i);
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
