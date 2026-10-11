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

#ifndef TNT_FILAMENT_BACKEND_D3D11DRIVER_H
#define TNT_FILAMENT_BACKEND_D3D11DRIVER_H

#include "DriverBase.h"

#include <private/backend/Driver.h>
#include <private/backend/HandleAllocator.h>

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/Program.h>
#include <backend/Platform.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace filament::backend {

struct D3D11SwapChain;

class D3D11Driver final : public DriverBase {
public:
    utils::CString const& getDeviceName() const noexcept { return mDeviceName; }
    utils::CString const& getDeviceFeatureLevel() const noexcept { return mDeviceFeatureLevel; }

private:
    friend class D3D11DriverFactory;

    D3D11Driver(Platform::DriverConfig const& driverConfig, ID3D11Device* device,
            ID3D11DeviceContext* context, utils::CString deviceName,
            utils::CString deviceFeatureLevel) noexcept;
    ~D3D11Driver() noexcept override;

    Dispatcher getDispatcher() const noexcept override;
    ShaderModel getShaderModel() const noexcept override;
    utils::FixedCapacityVector<ShaderLanguage> getShaderLanguages(
            ShaderLanguage preferredLanguage) const noexcept override;

    template<typename T>
    friend class ConcreteDispatcher;

    // Driver API
#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    void methodName(paramsDecl);
#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    RetType methodName(paramsDecl) override;
#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    RetType methodName##S() noexcept override; \
    void methodName##R(RetType, paramsDecl);
#include <private/backend/DriverAPI.inc>

    HandleAllocatorD3D11 mHandleAllocator;
    std::vector<SwapChainHandle> mSwapChains;
    std::vector<RenderTargetHandle> mRenderTargets;
    D3D11SwapChain* mCurrentSwapChain = nullptr;
    bool mRenderPassActive = false;

    ID3D11Device* mDevice;
    ID3D11DeviceContext* mContext;
    utils::CString mDeviceName;
    utils::CString mDeviceFeatureLevel;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_D3D11DRIVER_H
