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

#ifndef TNT_FILAMENT_BACKEND_D3D11SWAPCHAIN_H
#define TNT_FILAMENT_BACKEND_D3D11SWAPCHAIN_H

#include "DriverBase.h"

#include <stdint.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct IDXGISwapChain1;

namespace filament::backend {

struct D3D11SwapChain final : HwSwapChain {
    ~D3D11SwapChain();

    void initialize(ID3D11Device* device, void* nativeWindow);
    bool update(ID3D11Device* device, ID3D11DeviceContext* context);
    void clear(ID3D11DeviceContext* context, float const* color);
    void present(ID3D11Device* device);

private:
    void createRenderTarget(ID3D11Device* device);

    void* mNativeWindow = nullptr;
    IDXGISwapChain1* mSwapChain = nullptr;
    ID3D11RenderTargetView* mRenderTarget = nullptr;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    bool mVisible = false;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_D3D11SWAPCHAIN_H
