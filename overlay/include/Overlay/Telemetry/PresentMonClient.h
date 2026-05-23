#pragma once

#include "Overlay/Telemetry/MetricSnapshot.h"

#include <array>
#include <filesystem>
#include <mutex>

namespace overlay::Telemetry {

class PresentMonClient final {
public:
    bool Initialize(const std::filesystem::path& optional_csv_path);
    void Shutdown();
    void Sample(MetricSnapshot& snapshot);

private:
    void PushFrametime(float ms, MetricSnapshot& snapshot);
    void ComputeLows(MetricSnapshot& snapshot);

    std::mutex mutex_;
    std::array<float, FrametimeHistorySize> frame_times_{};
    uint32_t write_index_{0};
    uint32_t count_{0};
    std::filesystem::path csv_path_;
};

} // namespace overlay::Telemetry

