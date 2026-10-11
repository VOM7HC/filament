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

#include "CommandStreamDispatcher.h"

#include <utils/Panic.h>

#include <type_traits>

namespace filament::backend {

static_assert(!std::is_abstract_v<D3D11Driver>, "D3D11 must cover the complete driver interface");

D3D11Driver::D3D11Driver(Platform::DriverConfig const& driverConfig) noexcept
        : DriverBase(driverConfig) {
}

D3D11Driver::~D3D11Driver() noexcept = default;

Dispatcher D3D11Driver::getDispatcher() const noexcept {
    return ConcreteDispatcher<D3D11Driver>::make();
}

ShaderModel D3D11Driver::getShaderModel() const noexcept {
    return ShaderModel::DESKTOP;
}

utils::FixedCapacityVector<ShaderLanguage> D3D11Driver::getShaderLanguages(
        ShaderLanguage /*preferredLanguage*/) const noexcept {
    return {};
}

void D3D11Driver::unsupported(char const* method) noexcept {
    PANIC_POSTCONDITION("D3D11 operation not implemented: %s", method);
}

template class ConcreteDispatcher<D3D11Driver>;

} // namespace filament::backend
