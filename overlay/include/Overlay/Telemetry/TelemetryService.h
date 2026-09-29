#pragma once

#include "Overlay/Telemetry/GpuProvider.h"
#include "Overlay/Telemetry/MetricSnapshot.h"
#include "Overlay/Telemetry/PdhSampler.h"
#include "Overlay/Telemetry/PresentMonClient.h"

#include <atomic>
#include <filesystem>
#include <mutex>
#include <thread>

namespace overlay::Telemetry {

class TelemetryService final {
public:
    TelemetryService() = default;
    ~TelemetryService();

    TelemetryService(const TelemetryService&) = delete;
    TelemetryService& operator=(const TelemetryService&) = delete;

    bool Start(const std::filesystem::path& app_directory);
    void Stop();

    MetricSnapshot Snapshot() const;

private:
    void ThreadMain();
    void Publish(const MetricSnapshot& snapshot);
    void SampleMemory(MetricSnapshot& snapshot);
    void SamplePing(MetricSnapshot& snapshot);

    mutable std::mutex snapshot_mutex_;
    MetricSnapshot snapshot_{};

    PdhSampler pdh_;
    GpuProvider gpu_;
    PresentMonClient present_mon_;

    std::thread worker_;
    std::atomic_bool running_{false};
    std::filesystem::path app_directory_;
};

} // namespace overlay::Telemetry
