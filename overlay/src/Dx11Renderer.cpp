#include "Overlay/Dx11Renderer.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <iterator>

namespace overlay {

Dx11Renderer::~Dx11Renderer() {
    Shutdown();
}

bool Dx11Renderer::Initialize(HWND hwnd, const Config::Theme& theme) {
    hwnd_ = hwnd;
    if (!CreateDeviceAndSwapChain(hwnd_)) {
        return false;
    }
    if (!CreateRenderTarget()) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ApplyTheme(theme);

    if (!ImGui_ImplWin32_Init(hwnd_) || !ImGui_ImplDX11_Init(device_.Get(), context_.Get())) {
        return false;
    }

    initialized_ = true;
    return true;
}

void Dx11Renderer::Shutdown() {
    if (initialized_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        initialized_ = false;
    }
    ReleaseRenderTarget();
    swap_chain_.Reset();
    context_.Reset();
    device_.Reset();
}

void Dx11Renderer::Resize(UINT width, UINT height) {
    if (!swap_chain_ || width == 0 || height == 0) {
        return;
    }

    ReleaseRenderTarget();
    swap_chain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void Dx11Renderer::BeginFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Dx11Renderer::EndFrame(bool performance_mode) {
    ImGui::Render();

    constexpr float clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    context_->OMSetRenderTargets(1, render_target_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(render_target_.Get(), clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    const UINT sync_interval = performance_mode ? 0U : 1U;
    swap_chain_->Present(sync_interval, 0);
    last_present_ = std::chrono::steady_clock::now();
}

void Dx11Renderer::ApplyTheme(const Config::Theme& theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(14.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 7.0f);

    auto color = [](const Config::Color& c) { return ImVec4(c.r, c.g, c.b, c.a); };
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = color(theme.text);
    colors[ImGuiCol_TextDisabled] = color(theme.muted);
    colors[ImGuiCol_WindowBg] = color(theme.panel);
    colors[ImGuiCol_ChildBg] = color(theme.panel);
    colors[ImGuiCol_PopupBg] = color(theme.panel);
    colors[ImGuiCol_Border] = ImVec4(theme.accent.r, theme.accent.g, theme.accent.b, 0.38f);
    colors[ImGuiCol_FrameBg] = ImVec4(theme.background.r, theme.background.g, theme.background.b, 0.9f);
    colors[ImGuiCol_FrameBgHovered] = color(theme.panel_hover);
    colors[ImGuiCol_FrameBgActive] = color(theme.panel_hover);
    colors[ImGuiCol_TitleBg] = color(theme.panel);
    colors[ImGuiCol_TitleBgActive] = color(theme.panel_hover);
    colors[ImGuiCol_CheckMark] = color(theme.accent);
    colors[ImGuiCol_SliderGrab] = color(theme.accent);
    colors[ImGuiCol_Button] = ImVec4(theme.accent.r, theme.accent.g, theme.accent.b, 0.20f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(theme.accent.r, theme.accent.g, theme.accent.b, 0.32f);
    colors[ImGuiCol_ButtonActive] = ImVec4(theme.accent.r, theme.accent.g, theme.accent.b, 0.45f);
    colors[ImGuiCol_PlotLines] = color(theme.accent);
    colors[ImGuiCol_PlotHistogram] = color(theme.warning);
}

bool Dx11Renderer::CreateDeviceAndSwapChain(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 0;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = hwnd;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    constexpr D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL selected_level{};
    const HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        feature_levels,
        static_cast<UINT>(std::size(feature_levels)),
        D3D11_SDK_VERSION,
        &desc,
        swap_chain_.GetAddressOf(),
        device_.GetAddressOf(),
        &selected_level,
        context_.GetAddressOf());

    return SUCCEEDED(hr);
}

bool Dx11Renderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())))) {
        return false;
    }
    return SUCCEEDED(device_->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_.GetAddressOf()));
}

void Dx11Renderer::ReleaseRenderTarget() {
    render_target_.Reset();
}

} // namespace overlay
