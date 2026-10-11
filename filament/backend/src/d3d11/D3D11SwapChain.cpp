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

#include "d3d11/D3D11SwapChain.h"

#include <utils/Panic.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace filament::backend {

using Microsoft::WRL::ComPtr;

D3D11SwapChain::~D3D11SwapChain() {
    if (mRenderTarget) {
        mRenderTarget->Release();
    }
    if (mSwapChain) {
        mSwapChain->Release();
    }
}

void D3D11SwapChain::initialize(ID3D11Device* device, void* nativeWindow) {
    HWND const window = static_cast<HWND>(nativeWindow);
    FILAMENT_CHECK_PRECONDITION(IsWindow(window)) << "Invalid D3D11 window";
    mNativeWindow = nativeWindow;

    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT result = device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result))
            << "QueryInterface(IDXGIDevice) failed: " << result;
    ComPtr<IDXGIAdapter> adapter;
    result = dxgiDevice->GetAdapter(adapter.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "GetAdapter failed: " << result;
    ComPtr<IDXGIFactory2> factory;
    result = adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf()));
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "GetParent(IDXGIFactory2) failed: " << result;

    RECT client{};
    FILAMENT_CHECK_POSTCONDITION(GetClientRect(window, &client)) << "GetClientRect failed";
    mWidth = client.right > 0 ? uint32_t(client.right) : 1;
    mHeight = client.bottom > 0 ? uint32_t(client.bottom) : 1;
    DXGI_SWAP_CHAIN_DESC1 description{};
    description.Width = mWidth;
    description.Height = mHeight;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount = 2;
    description.Scaling = DXGI_SCALING_STRETCH;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    result = factory->CreateSwapChainForHwnd(device, window, &description,
            nullptr, nullptr, &mSwapChain);
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateSwapChainForHwnd failed: " << result;
    result = factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "MakeWindowAssociation failed: " << result;
    createRenderTarget(device);
}

void D3D11SwapChain::createRenderTarget(ID3D11Device* device) {
    ComPtr<ID3D11Texture2D> buffer;
    HRESULT result = mSwapChain->GetBuffer(0, IID_PPV_ARGS(buffer.GetAddressOf()));
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "GetBuffer failed: " << result;
    result = device->CreateRenderTargetView(buffer.Get(), nullptr, &mRenderTarget);
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateRenderTargetView failed: " << result;
}

bool D3D11SwapChain::update(ID3D11Device* device, ID3D11DeviceContext* context) {
    HWND const window = static_cast<HWND>(mNativeWindow);
    RECT client{};
    FILAMENT_CHECK_POSTCONDITION(GetClientRect(window, &client)) << "GetClientRect failed";
    mVisible = !IsIconic(window) && client.right > 0 && client.bottom > 0;
    if (!mVisible) {
        return false;
    }
    uint32_t const width = uint32_t(client.right);
    uint32_t const height = uint32_t(client.bottom);
    if (width != mWidth || height != mHeight) {
        context->OMSetRenderTargets(0, nullptr, nullptr);
        mRenderTarget->Release();
        mRenderTarget = nullptr;
        HRESULT const result = mSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "ResizeBuffers failed: " << result;
        mWidth = width;
        mHeight = height;
        createRenderTarget(device);
    }
    return true;
}

void D3D11SwapChain::clear(ID3D11DeviceContext* context, float const* color) {
    context->OMSetRenderTargets(1, &mRenderTarget, nullptr);
    context->ClearRenderTargetView(mRenderTarget, color);
}

void D3D11SwapChain::present(ID3D11Device* device) {
    if (!mVisible) {
        return;
    }
    HRESULT const result = mSwapChain->Present(1, 0);
    if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
        PANIC_POSTCONDITION("D3D11 device lost: 0x%08x", uint32_t(device->GetDeviceRemovedReason()));
    }
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "Present failed: " << result;
}

} // namespace filament::backend
