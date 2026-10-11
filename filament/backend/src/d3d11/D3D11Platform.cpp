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

#include "d3d11/D3D11Driver.h"
#include "d3d11/D3D11DriverFactory.h"

#include <backend/platforms/D3D11Platform.h>

#include <utils/Panic.h>

namespace filament::backend {

utils::CString D3D11Platform::getDeviceInfo(DeviceInfoType infoType, Driver* driver) const {
    if (!driver) {
        return {};
    }
    auto const* d3d11Driver = static_cast<D3D11Driver const*>(driver);
    switch (infoType) {
        case DeviceInfoType::D3D11_DEVICE_NAME:
            return d3d11Driver->getDeviceName();
        case DeviceInfoType::D3D11_FEATURE_LEVEL:
            return d3d11Driver->getDeviceFeatureLevel();
        default:
            FILAMENT_CHECK_POSTCONDITION(false) << "Unsupported DeviceInfoType for D3D11Platform";
            return {};
    }
}

Driver* D3D11Platform::createDriver(void* sharedContext, DriverConfig const& driverConfig) {
    return D3D11DriverFactory::create(this, sharedContext, driverConfig);
}

} // namespace filament::backend
