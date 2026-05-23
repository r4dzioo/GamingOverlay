#pragma once

#include "Overlay/Widgets/IWidget.h"

namespace overlay::Widgets {

class FpsWidget final : public IWidget {
public:
    const char* Id() const override { return "fps"; }
    const char* Title() const override { return "FPS"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

class FrametimeWidget final : public IWidget {
public:
    const char* Id() const override { return "frametime"; }
    const char* Title() const override { return "Frametime"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

class CpuWidget final : public IWidget {
public:
    const char* Id() const override { return "cpu"; }
    const char* Title() const override { return "CPU"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

class GpuWidget final : public IWidget {
public:
    const char* Id() const override { return "gpu"; }
    const char* Title() const override { return "GPU"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

class RamWidget final : public IWidget {
public:
    const char* Id() const override { return "ram"; }
    const char* Title() const override { return "RAM"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

class NetworkWidget final : public IWidget {
public:
    const char* Id() const override { return "network"; }
    const char* Title() const override { return "Network"; }
    void Render(const Telemetry::MetricSnapshot& snapshot, Config::WidgetLayout& layout) override;
};

} // namespace overlay::Widgets

