#include "Overlay/Widgets/StockWidgets.h"

#include <imgui.h>

#include <string>

namespace overlay::Widgets {

namespace {

bool BeginWidget(const char* title, Config::WidgetLayout& layout) {
    ImGui::SetNextWindowPos(ImVec2(layout.position.x, layout.position.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(layout.size.x, layout.size.y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(layout.opacity);
    const std::string name = std::string(title) + "##" + layout.id;
    return ImGui::Begin(name.c_str(), nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing);
}

void EndWidget(Config::WidgetLayout& layout) {
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 size = ImGui::GetWindowSize();
    layout.position = {pos.x, pos.y};
    layout.size = {size.x, size.y};
    ImGui::End();
}

} // namespace

void FpsWidget::Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) {
    if (!BeginWidget(Title(), layout)) {
        EndWidget(layout);
        return;
    }

    ImGui::TextDisabled("REAL-TIME");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.05f, 0.82f, 1.0f, 1.0f), "FPS");
    ImGui::SetWindowFontScale(1.45f);
    ImGui::Text("%.0f", snapshot.fps);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Text("1%% low %.0f   0.1%% low %.0f", snapshot.fps_1_percent_low, snapshot.fps_0_1_percent_low);
    ImGui::TextDisabled("%.2f ms frame", snapshot.frametime_ms);

    EndWidget(layout);
}

} // namespace overlay::Widgets

