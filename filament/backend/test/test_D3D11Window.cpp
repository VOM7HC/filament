/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>
#include <private/backend/Driver.h>
#include <private/backend/PlatformFactory.h>

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <windows.h>

#include <exception>
#include <memory>

using namespace filament::backend;

namespace {

struct WindowState {
    bool running = true;
    bool minimized = false;
};

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        auto const* create = reinterpret_cast<CREATESTRUCTW const*>(lParam);
        state = static_cast<WindowState*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    }
    if (state) {
        switch (message) {
            case WM_CLOSE:
                state->running = false;
                return 0;
            case WM_SIZE:
                state->minimized = wParam == SIZE_MINIMIZED;
                return 0;
            case WM_ERASEBKGND:
                return 1;
            case WM_DESTROY:
                state->running = false;
                PostQuitMessage(0);
                return 0;
        }
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int runWindow(HWND window, WindowState& state) {
    Backend backend = Backend::D3D11;
    std::unique_ptr<Platform> platform(PlatformFactory::create(&backend));
    if (!platform) {
        utils::slog.e << "Failed to create D3D11 platform" << utils::io::endl;
        return 1;
    }
    Platform::DriverConfig driverConfig;
    std::unique_ptr<Driver> driver(platform->createDriver(nullptr, driverConfig));
    if (!driver) {
        return 1;
    }
    CommandBufferQueue queue(1024 * 1024, 3 * 1024 * 1024, false);
    CommandStream api(*driver, queue.getCircularBuffer());
    auto executeCommands = [&]() {
        queue.flush();
        auto buffers = queue.waitForCommands();
#if UTILS_HAS_EXCEPTIONS
        try {
#endif
            for (auto const& buffer : buffers) {
                if (buffer.begin) {
                    api.execute(buffer.begin);
                }
            }
#if UTILS_HAS_EXCEPTIONS
        } catch (...) {
            for (auto const& buffer : buffers) {
                queue.releaseBuffer(buffer);
            }
            throw;
        }
#endif
        for (auto const& buffer : buffers) {
            queue.releaseBuffer(buffer);
        }
    };

    SwapChainHandle const swapChain = api.createSwapChain(window, 0);
    RenderTargetHandle const renderTarget = api.createDefaultRenderTarget();
    api.makeCurrent(swapChain, swapChain);
    executeCommands();
    ShowWindow(window, SW_SHOW);
    utils::slog.i << "D3D11 window ready: resize, minimize, restore, then close"
                 << utils::io::endl;

    while (state.running) {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                state.running = false;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (!state.running) {
            break;
        }
        RECT client{};
        if (!GetClientRect(window, &client)) {
            utils::slog.e << "GetClientRect failed: " << GetLastError() << utils::io::endl;
            return 1;
        }
        if (state.minimized || client.right == 0 || client.bottom == 0) {
            MsgWaitForMultipleObjectsEx(0, nullptr, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
            continue;
        }

        RenderPassParams params{};
        params.flags.clear = TargetBufferFlags::COLOR0;
        params.viewport.width = uint32_t(client.right);
        params.viewport.height = uint32_t(client.bottom);
        params.clearColor = { 0.08, 0.25, 0.55, 1.0 };
        api.beginRenderPass(renderTarget, params);
        api.endRenderPass();
        api.commit(swapChain);
        executeCommands();
        MsgWaitForMultipleObjectsEx(0, nullptr, 16, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }

    api.destroyRenderTarget(renderTarget);
    api.destroySwapChain(swapChain);
    executeCommands();
    driver->terminate();
    utils::slog.i << "D3D11 window shutdown complete" << utils::io::endl;
    return 0;
}

} // namespace

int main() {
    HINSTANCE const instance = GetModuleHandleW(nullptr);
    wchar_t const* className = L"FilamentD3D11Window";
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    if (!RegisterClassExW(&windowClass)) {
        utils::slog.e << "RegisterClassExW failed: " << GetLastError() << utils::io::endl;
        return 1;
    }
    WindowState state;
    HWND const window = CreateWindowExW(0, className, L"Filament D3D11 clear / present",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 640,
            nullptr, nullptr, instance, &state);
    if (!window) {
        utils::slog.e << "CreateWindowExW failed: " << GetLastError() << utils::io::endl;
        UnregisterClassW(className, instance);
        return 1;
    }

    int result = 1;
#if UTILS_HAS_EXCEPTIONS
    try {
#endif
        result = runWindow(window, state);
#if UTILS_HAS_EXCEPTIONS
    } catch (std::exception const& error) {
        utils::slog.e << error.what() << utils::io::endl;
    }
#endif
    if (IsWindow(window)) {
        DestroyWindow(window);
    }
    UnregisterClassW(className, instance);
    return result;
}
