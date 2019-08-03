#ifndef PHOS_FILEBROWSER_H
#define PHOS_FILEBROWSER_H

#include <filesystem>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

#include "imgui.h"

namespace fs = std::filesystem;

struct FileBrowser {
    const char* extensions[2] = {".gb", ".gbc"};
    bool onlyAllowValidExtensions = true;
    bool sort = true;

    bool badFile = false;
    unsigned selected = 0xFFFFFFFF;

    fs::path currentPath = fs::current_path();
    std::vector<fs::path> files;

    std::function<void(std::string&)> callback;

    FileBrowser(std::function<void(std::string&)> callback) : callback(std::move(callback)) {
        UpdateFiles();
    }

    bool checkExtension(const std::string& fileExt) {
        for (auto ext : extensions) {
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
        if (!sort) return;

        std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
            if (fs::is_directory(lhs) && !fs::is_directory(rhs)) {
                return true;
            } else if (fs::is_directory(rhs) && !fs::is_directory(lhs)) {
                return false;
            } else {
                auto s1 = lhs.filename().string();
                auto s2 = rhs.filename().string();
                std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
                std::transform(s2.begin(), s2.end(), s2.begin(), ::toupper);
                return s1 < s2;
            }
        });
    }

    void DrawWindow(bool* open, int width, int height) {
        ImGui::SetNextWindowSize(ImVec2(width, height));
        if (ImGui::BeginPopupModal("FileBrowser", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoResize)) {
            static bool invalidate = false;
            if (ImGui::Button("Down") && currentPath.has_parent_path()) {
                currentPath.assign(currentPath.parent_path());
                UpdateFiles();
            }
            ImGui::SameLine();
            ImGui::Text("Path: %s", currentPath.string().c_str());
            ImGui::Separator();
            ImGui::BeginChild("FileList",
                              ImVec2(ImGui::GetWindowWidth() - 5, -ImGui::GetItemsLineHeightWithSpacing() - 20),
                              false, ImGuiWindowFlags_HorizontalScrollbar);

            unsigned counter = 0;
            for (const auto &entry : files) {
                if (onlyAllowValidExtensions && !fs::is_directory(entry) &&
                    !checkExtension(entry.extension().string())) {
                    counter++;
                    continue;
                }
                std::ostringstream oss;
                oss << '[' << (fs::is_directory(entry) ? 'D' : 'F') << "] " << entry.filename().string();
                if (ImGui::Selectable(oss.str().c_str(), selected == counter,
                                      ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups)) {
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
            ImGui::Text("Selected: %s", (selected < files.size() ? files[selected].filename().string().c_str() : ""));

            ImGui::Separator();
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                *open = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Select")) {
                std::string file = GetSelected();
                if (file.empty()) {
                    badFile = true;
                } else {
                    ImGui::CloseCurrentPopup();
                    *open = false;
                    callback(file);
                }
            }
            if (badFile) {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
                ImGui::Text("Invalid File");
                ImGui::PopStyleColor();
            }

            if (ImGui::BeginPopup("Options")) {
                if (ImGui::Checkbox("Sort", &sort)) UpdateFiles();
                if (ImGui::Checkbox("Filter", &onlyAllowValidExtensions)) UpdateFiles();
                ImGui::EndPopup();
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::Button("Options")) ImGui::OpenPopup("Options");

            ImGui::EndPopup();

            if (invalidate) {
                invalidate = false;
                UpdateFiles();
            }
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

};

#endif //PHOS_FILEBROWSER_H
