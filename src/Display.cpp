#include "Display.h"

Display::Display(SDL_Window* w, Emulator* emu) : IDisplay(w, emu) {
    loadTexture(&mainTextureHandler, WIDTH, HEIGHT, emulator->getDisplayState());
}

Display::~Display() {
    if (mainTextureHandler != 0) {
        glDeleteTextures(1, &mainTextureHandler);
    }
}

void Display::processEvent(SDL_Event &event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void Display::update(u8* data) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    glBindTexture(GL_TEXTURE_2D, mainTextureHandler);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    bool open = true;
    ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoResize;

    ImGui::Begin("Emu", &open, ImVec2(SCALED_WIDTH, SCALED_HEIGHT), -1.0f, flags);
    ImGui::SetWindowSize(ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::Image((void*)(intptr_t)mainTextureHandler, ImVec2(SCALED_WIDTH, SCALED_HEIGHT));
    ImGui::End();

    for (int i = 0; i < IM_ARRAYSIZE(ImGui::GetIO().MouseDown); i++) {
        if (ImGui::IsMouseClicked(i) && i == 1) {
            printf("Clicked mouse %d\n", i);
            ImGui::OpenPopup("menu_popup");
            break;
        }
    }
    if (ImGui::BeginPopup("menu_popup")) {
        showMenuPopup();
        ImGui::EndPopup();
    }

    ImGui::Render();
}

void Display::render() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Display::showMenuPopup() {
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                showMenuPopup();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
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
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
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
