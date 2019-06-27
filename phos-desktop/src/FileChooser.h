#ifndef PHOS_FILECHOOSER_H
#define PHOS_FILECHOOSER_H

#include <filesystem>
#include <iostream>
#include <ostream>
#include <vector>

#include "imgui.h"

namespace fs = std::filesystem;

struct FileChooser {
    const char* extensions[2] = {".gb", ".gbc"};
    bool onlyAllowValidExtensions = true;

    bool badFile = false;
    unsigned selected = 0xFFFFFFFF;

    fs::path currentPath = fs::current_path();
    std::vector<fs::path> files;

    FileChooser() {
        UpdateFiles();
    }

    bool checkExtension(const std::string& fileExt) {
        for (const char* ext : extensions) {
            if (fileExt == ext) return true;
        }
        return false;
    }

    void UpdateFiles() {
        selected = 0xFFFFFFFF;
        files.clear();
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            files.push_back(entry.path());
        }
    }

    void DrawWindow() {
        static bool invalidate = false;
        if (ImGui::Button("Down") && currentPath.has_parent_path()) {
            currentPath.assign(currentPath.parent_path());
            UpdateFiles();
        }
        ImGui::SameLine();
        ImGui::Text("Path: %s", currentPath.c_str());
        ImGui::Separator();
        ImGui::BeginChild("FileList", ImVec2(ImGui::GetWindowWidth()-5, -ImGui::GetItemsLineHeightWithSpacing()-20),
                          false, ImGuiWindowFlags_HorizontalScrollbar);

        unsigned counter = 0;
        for (const auto& entry : files) {
            if (onlyAllowValidExtensions && !fs::is_directory(entry) && !checkExtension(entry.extension().string())) {
                counter++;
                continue;
            }
            std::ostringstream oss;
            oss << '[' << (fs::is_directory(entry) ? 'D' : 'F') << "] " << entry.filename().string();
            if (ImGui::Selectable(oss.str().c_str(), selected == counter, ImGuiSelectableFlags_AllowDoubleClick|ImGuiSelectableFlags_DontClosePopups)) {
                if (selected != counter) badFile = false;
                selected = counter;
                if (ImGui::IsMouseDoubleClicked(0) && fs::is_directory(entry)) {
                    currentPath.assign(entry);
                    invalidate = true;
                }
            }
            counter++;
        }
        ImGui::EndChild();
        ImGui::Separator();
        ImGui::Text("Selected: %s", (selected < files.size() ? files[selected].filename().c_str() : ""));

        if (invalidate) {
            invalidate = false;
            UpdateFiles();
        }
    }

    std::string GetSelected() {
        fs::path selectedPath = files[selected];
        if ((onlyAllowValidExtensions && !checkExtension(selectedPath.extension().string())) ||
            fs::is_directory(selectedPath)) {
            return std::string();
        } else {
            return selectedPath.string();
        }
    }

    void fsTest() {
        const fs::path pathToShow = fs::current_path();
        std::cout << "exists() = " << fs::exists(pathToShow) << "\n"
             << "root_name() = " << pathToShow.root_name() << "\n"
             << "root_path() = " << pathToShow.root_path() << "\n"
             << "relative_path() = " << pathToShow.relative_path() << "\n"
             << "parent_path() = " << pathToShow.parent_path() << "\n"
             << "filename() = " << pathToShow.filename() << "\n"
             << "stem() = " << pathToShow.stem() << "\n"
             << "isDirectory " << fs::is_directory(pathToShow) << "\n"
             << "absolutePath = " << pathToShow << "\n"
             << "extension() = " << pathToShow.extension() << "\n";
    }
};

#endif //PHOS_FILECHOOSER_H
