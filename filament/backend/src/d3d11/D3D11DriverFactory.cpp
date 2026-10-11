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

#include "d3d11/D3D11DriverFactory.h"

#include "d3d11/D3D11Driver.h"

#include <utils/CString.h>
#include <utils/Log.h>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <stdint.h>

namespace filament::backend {

using Microsoft::WRL::ComPtr;

Driver* D3D11DriverFactory::create(D3D11Platform* /*platform*/, void* sharedContext,
        Platform::DriverConfig const& driverConfig) noexcept {
    if (sharedContext) {
        utils::slog.e << "D3D11 shared contexts are not supported" << utils::io::endl;
        return nullptr;
    }

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL const featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel{};
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            featureLevels, 1, D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel,
            context.GetAddressOf());
#ifndef NDEBUG
    if (result == DXGI_ERROR_SDK_COMPONENT_MISSING) {
        utils::slog.w << "D3D11 debug layer unavailable; retrying without it" << utils::io::endl;
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                featureLevels, 1, D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), &featureLevel,
                context.ReleaseAndGetAddressOf());
    }
#endif
    if (FAILED(result)) {
        utils::slog.e << "D3D11CreateDevice failed: " << utils::io::hex << uint32_t(result)
                     << utils::io::dec << utils::io::endl;
        return nullptr;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    result = device.As(&dxgiDevice);
    if (FAILED(result)) {
        utils::slog.e << "QueryInterface(IDXGIDevice) failed: " << utils::io::hex
                     << uint32_t(result) << utils::io::dec << utils::io::endl;
        return nullptr;
    }
    ComPtr<IDXGIAdapter> adapter;
    result = dxgiDevice->GetAdapter(adapter.GetAddressOf());
    if (FAILED(result)) {
        utils::slog.e << "IDXGIDevice::GetAdapter failed: " << utils::io::hex << uint32_t(result)
                     << utils::io::dec << utils::io::endl;
        return nullptr;
    }
    DXGI_ADAPTER_DESC description{};
    result = adapter->GetDesc(&description);
    if (FAILED(result)) {
        utils::slog.e << "IDXGIAdapter::GetDesc failed: " << utils::io::hex << uint32_t(result)
                     << utils::io::dec << utils::io::endl;
        return nullptr;
    }
    char deviceName[512];
    if (!WideCharToMultiByte(CP_UTF8, 0, description.Description, -1, deviceName,
            sizeof(deviceName), nullptr, nullptr)) {
        utils::slog.e << "Failed to convert D3D11 adapter name: " << GetLastError()
                     << utils::io::endl;
        return nullptr;
    }
    utils::slog.i << "D3D11 device: " << deviceName << utils::io::endl;
    utils::slog.i << "D3D11 feature level: " << utils::io::hex << uint32_t(featureLevel)
                 << utils::io::dec << utils::io::endl;
    utils::slog.i << "D3D11 debug layer: "
                 << ((flags & D3D11_CREATE_DEVICE_DEBUG) ? "enabled" : "disabled") << utils::io::endl;

    return new D3D11Driver(driverConfig, device.Get(), context.Get(),
            utils::CString(deviceName), utils::CString("11.0"));
}

} // namespace filament::backend
