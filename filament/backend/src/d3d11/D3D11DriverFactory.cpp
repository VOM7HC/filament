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

#include <utils/Log.h>

namespace filament::backend {

Driver* D3D11DriverFactory::create(D3D11Platform* /*platform*/, void* /*sharedContext*/,
        Platform::DriverConfig const& /*driverConfig*/) noexcept {
    utils::slog.e << "D3D11 initialization not implemented" << utils::io::endl;
    return nullptr;
}

} // namespace filament::backend
