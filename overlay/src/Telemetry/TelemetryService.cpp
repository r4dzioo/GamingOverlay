#include "Overlay/Telemetry/TelemetryService.h"

#include "Overlay/Log.h"

#include <windows.h>

#include <chrono>

namespace overlay::Telemetry {

TelemetryService::~TelemetryService() {
    Stop();
}

bool TelemetryService::Start(const std::filesystem::path& app_directory) {
    if (running_.exchange(true)) {
        return true;
    }

    app_directory_ = app_directory;
    pdh_.Initialize();
    gpu_.Initialize();
    present_mon_.Initialize(app_directory_ / "assets" / "presentmon.csv");

    worker_ = std::thread(&TelemetryService::ThreadMain, this);
    Log::Info(L"Telemetry service started.");
    return true;
}

void TelemetryService::Stop() {
    if (!running_.exchange(false)) {
        return;
    }

    if (worker_.joinable()) {
        worker_.join();
    }

    present_mon_.Shutdown();
    gpu_.Shutdown();
    pdh_.Shutdown();
    Log::Info(L"Telemetry service stopped.");
}

MetricSnapshot TelemetryService::Snapshot() const {
    std::lock_guard lock(snapshot_mutex_);
    return snapshot_;
}

void TelemetryService::ThreadMain() {
    using namespace std::chrono_literals;

    while (running_) {
        MetricSnapshot snapshot{};
        snapshot.sampled_at = std::chrono::steady_clock::now();

        present_mon_.Sample(snapshot);
        pdh_.Sample(snapshot);
        SampleMemory(snapshot);
        SamplePing(snapshot);
        gpu_.Sample(snapshot);

        Publish(snapshot);
        std::this_thread::sleep_for(250ms);
    }
}

void TelemetryService::Publish(const MetricSnapshot& snapshot) {
    std::lock_guard lock(snapshot_mutex_);
    snapshot_ = snapshot;
}

void TelemetryService::SampleMemory(MetricSnapshot& snapshot) {
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        snapshot.ram_total_mb = static_cast<float>(status.ullTotalPhys / (1024ull * 1024ull));
        snapshot.ram_used_mb = static_cast<float>((status.ullTotalPhys - status.ullAvailPhys) / (1024ull * 1024ull));
    }
}

void TelemetryService::SamplePing(MetricSnapshot& snapshot) {
    snapshot.ping_ms = 0.0f;
}

} // namespace overlay::Telemetry
