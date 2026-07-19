namespace imgui_app {

#include <imgui.h>

template <class F>
void modal_popup(const char* name, bool open, F&& body) {
    if (open) ImGui::OpenPopup(name);
    if (ImGui::BeginPopupModal(
            name, nullptr, ImGuiWindowFlags_AlwaysAutoResize
        )) {
        body();
        ImGui::EndPopup();
    }
}

}  // namespace imgui_app