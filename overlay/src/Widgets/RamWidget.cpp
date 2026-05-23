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

void RamWidget::Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) {
    if (!BeginWidget(Title(), layout)) {
        EndWidget(layout);
        return;
    }

    const float ratio = snapshot.ram_total_mb > 0.0f ? snapshot.ram_used_mb / snapshot.ram_total_mb : 0.0f;
    ImGui::TextColored(ImVec4(0.05f, 0.82f, 1.0f, 1.0f), "RAM");
    ImGui::ProgressBar(ratio, ImVec2(-1.0f, 10.0f));
    ImGui::Text("%.0f / %.0f MB", snapshot.ram_used_mb, snapshot.ram_total_mb);

    EndWidget(layout);
}

} // namespace overlay::Widgets

