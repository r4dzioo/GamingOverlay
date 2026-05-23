#include "Overlay/Win32Window.h"

#include <dwmapi.h>
#include <imgui.h>
#include <imgui_impl_win32.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

namespace overlay {

Win32Window::~Win32Window() {
    Destroy();
}

bool Win32Window::Create(HINSTANCE instance, int show_command, ResizeCallback resize_callback) {
    instance_ = instance;
    resize_callback_ = std::move(resize_callback);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &Win32Window::WindowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"GamingOverlay.TransparentWindow";

    RegisterClassExW(&wc);

    ResizeToVirtualDesktop();

    const DWORD ex_style = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT;
    hwnd_ = CreateWindowExW(
        ex_style,
        wc.lpszClassName,
        L"GamingOverlay",
        WS_POPUP,
        GetSystemMetrics(SM_XVIRTUALSCREEN),
        GetSystemMetrics(SM_YVIRTUALSCREEN),
        static_cast<int>(width_),
        static_cast<int>(height_),
        nullptr,
        nullptr,
        instance_,
        this);

    if (!hwnd_) {
        return false;
    }

    MARGINS margins{-1};
    DwmExtendFrameIntoClientArea(hwnd_, &margins);
    SetOpacity(1.0f);
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);
    ShowWindow(hwnd_, show_command);
    UpdateWindow(hwnd_);
    SetTopMost();
    return true;
}

void Win32Window::Destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

bool Win32Window::PumpMessages() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT) {
            return false;
        }
    }
    return true;
}

void Win32Window::SetClickThrough(bool enabled) {
    if (!hwnd_ || click_through_ == enabled) {
        return;
    }

    click_through_ = enabled;
    LONG_PTR style = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
    if (enabled) {
        style |= WS_EX_TRANSPARENT;
    } else {
        style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, style);
}

void Win32Window::SetTopMost() {
    if (!hwnd_) {
        return;
    }
    SetWindowPos(
        hwnd_,
        HWND_TOPMOST,
        GetSystemMetrics(SM_XVIRTUALSCREEN),
        GetSystemMetrics(SM_YVIRTUALSCREEN),
        static_cast<int>(width_),
        static_cast<int>(height_),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void Win32Window::SetOpacity(float opacity) {
    if (!hwnd_) {
        return;
    }

    opacity = opacity < 0.05f ? 0.05f : (opacity > 1.0f ? 1.0f : opacity);
    const BYTE alpha = static_cast<BYTE>(opacity * 255.0f);
    SetLayeredWindowAttributes(hwnd_, 0, alpha, LWA_ALPHA);
}

void Win32Window::ResizeToVirtualDesktop() {
    width_ = static_cast<UINT>(GetSystemMetrics(SM_CXVIRTUALSCREEN));
    height_ = static_cast<UINT>(GetSystemMetrics(SM_CYVIRTUALSCREEN));
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam)) {
        return true;
    }

    Win32Window* window = nullptr;
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        window = static_cast<Win32Window*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->HandleMessage(message, wparam, lparam);
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT Win32Window::HandleMessage(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED) {
            width_ = LOWORD(lparam);
            height_ = HIWORD(lparam);
            if (resize_callback_) {
                resize_callback_(width_, height_);
            }
        }
        return 0;
    case WM_DISPLAYCHANGE:
        ResizeToVirtualDesktop();
        SetTopMost();
        if (resize_callback_) {
            resize_callback_(width_, height_);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd_, message, wparam, lparam);
    }
}

} // namespace overlay
