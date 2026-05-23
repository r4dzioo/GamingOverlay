#include "Overlay/Widgets/StockWidgets.h"

#include <imgui.h>

#include <algorithm>
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

void FrametimeWidget::Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) {
    if (!BeginWidget(Title(), layout)) {
        EndWidget(layout);
        return;
    }

    ImGui::TextColored(ImVec4(0.05f, 0.82f, 1.0f, 1.0f), "Frametime");
    ImGui::SameLine();
    ImGui::TextDisabled("%.2f ms", snapshot.frametime_ms);

    const float max_ms = std::max(20.0f, snapshot.frametime_ms * 1.8f);
    ImGui::PlotLines(
        "##frametime-plot",
        snapshot.frametime_history.data(),
        static_cast<int>(Telemetry::FrametimeHistorySize),
        0,
        nullptr,
        0.0f,
        max_ms,
        ImVec2(-1.0f, 78.0f));

    ImGui::TextDisabled("Lower and flatter is better");
    EndWidget(layout);
}

} // namespace overlay::Widgets

