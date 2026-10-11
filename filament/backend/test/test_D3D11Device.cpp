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

#include <private/backend/Driver.h>
#include <private/backend/PlatformFactory.h>

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/CString.h>
#include <utils/Log.h>

#include <memory>

using namespace filament::backend;

int main() {
    Backend backend = Backend::D3D11;
    std::unique_ptr<Platform> platform(PlatformFactory::create(&backend));
    if (!platform || backend != Backend::D3D11) {
        utils::slog.e << "Failed to create D3D11 platform" << utils::io::endl;
        return 1;
    }

    Platform::DriverConfig driverConfig;
    for (int iteration = 0; iteration < 2; ++iteration) {
        std::unique_ptr<Driver> driver(platform->createDriver(nullptr, driverConfig));
        if (!driver) {
            utils::slog.e << "Failed to create D3D11 device" << utils::io::endl;
            return 1;
        }
        utils::CString const name = platform->getDeviceInfo(
                Platform::DeviceInfoType::D3D11_DEVICE_NAME, driver.get());
        utils::CString const featureLevel = platform->getDeviceInfo(
                Platform::DeviceInfoType::D3D11_FEATURE_LEVEL, driver.get());
        if (name.empty() || featureLevel != utils::CString("11.0")) {
            utils::slog.e << "Invalid D3D11 device information" << utils::io::endl;
            return 1;
        }
        utils::slog.i << "Adapter: " << name << utils::io::endl;
        utils::slog.i << "Feature level: " << featureLevel << utils::io::endl;

        if (iteration == 0) {
            driver->terminate();
            driver->terminate();
        }
        driver.reset();
        utils::slog.i << (iteration == 0 ? "Explicit termination passed" : "Destruction passed")
                     << utils::io::endl;
    }

    platform.reset();
    utils::slog.i << "D3D11 device smoke test passed" << utils::io::endl;
    return 0;
}
