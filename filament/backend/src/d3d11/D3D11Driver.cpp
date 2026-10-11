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

#include <d3d11.h>

#include <type_traits>
#include <utility>

namespace filament::backend {

static_assert(!std::is_abstract_v<D3D11Driver>, "D3D11 must cover the complete driver interface");

D3D11Driver::D3D11Driver(Platform::DriverConfig const& driverConfig, ID3D11Device* device,
        ID3D11DeviceContext* context, utils::CString deviceName,
        utils::CString deviceFeatureLevel) noexcept
        : DriverBase(driverConfig),
          mDevice(device),
          mContext(context),
          mDeviceName(std::move(deviceName)),
          mDeviceFeatureLevel(std::move(deviceFeatureLevel)) {
    mDevice->AddRef();
    mContext->AddRef();
}

D3D11Driver::~D3D11Driver() noexcept {
    terminate();
}

void D3D11Driver::terminate() {
    if constexpr (UTILS_HAS_THREADING) {
        stopServiceThread();
    }
    if (mContext) {
        mContext->ClearState();
        mContext->Flush();
        mContext->Release();
        mContext = nullptr;
    }
    if (mDevice) {
        mDevice->Release();
        mDevice = nullptr;
    }
}

Dispatcher D3D11Driver::getDispatcher() const noexcept {
    return ConcreteDispatcher<D3D11Driver>::make();
}

ShaderModel D3D11Driver::getShaderModel() const noexcept {
    return ShaderModel::DESKTOP;
}

utils::FixedCapacityVector<ShaderLanguage> D3D11Driver::getShaderLanguages(
        ShaderLanguage /*preferredLanguage*/) const noexcept {
    return { ShaderLanguage::HLSL };
}

Handle<HwStream> D3D11Driver::createStreamNative(void* nativeStream, utils::ImmutableCString tag) {
    PANIC_POSTCONDITION("D3D11::createStreamNative not implemented");
}

Handle<HwStream> D3D11Driver::createStreamAcquired(utils::ImmutableCString tag) {
    PANIC_POSTCONDITION("D3D11::createStreamAcquired not implemented");
}

void D3D11Driver::setAcquiredImage(Handle<HwStream> sh, void* image, const math::mat3f& transform,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
    PANIC_POSTCONDITION("D3D11::setAcquiredImage not implemented");
}

void D3D11Driver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
    PANIC_POSTCONDITION("D3D11::setStreamDimensions not implemented");
}

int64_t D3D11Driver::getStreamTimestamp(Handle<HwStream> sh) {
    PANIC_POSTCONDITION("D3D11::getStreamTimestamp not implemented");
}

void D3D11Driver::updateStreams(CommandStream* driver) {
    PANIC_POSTCONDITION("D3D11::updateStreams not implemented");
}

FenceStatus D3D11Driver::getFenceStatus(Handle<HwFence> fh) {
    PANIC_POSTCONDITION("D3D11::getFenceStatus not implemented");
}

FenceStatus D3D11Driver::fenceWait(Handle<HwFence> fh, uint64_t timeout) {
    PANIC_POSTCONDITION("D3D11::fenceWait not implemented");
}

void D3D11Driver::fenceCancel(FenceHandle fh) {
    PANIC_POSTCONDITION("D3D11::fenceCancel not implemented");
}

void D3D11Driver::getPlatformSync(Handle<HwSync> sh, CallbackHandler* handler,
        Platform::SyncCallback cb, void* userData) {
    PANIC_POSTCONDITION("D3D11::getPlatformSync not implemented");
}

bool D3D11Driver::isTextureFormatSupported(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isTextureFormatSupported not implemented");
}

bool D3D11Driver::isTextureSwizzleSupported() {
    PANIC_POSTCONDITION("D3D11::isTextureSwizzleSupported not implemented");
}

bool D3D11Driver::isTextureFormatMipmappable(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isTextureFormatMipmappable not implemented");
}

bool D3D11Driver::isTextureFormatFilterable(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isTextureFormatFilterable not implemented");
}

bool D3D11Driver::isRenderTargetFormatSupported(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isRenderTargetFormatSupported not implemented");
}

bool D3D11Driver::isFrameBufferFetchSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameBufferFetchSupported not implemented");
}

bool D3D11Driver::isFrameBufferFetchMultiSampleSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameBufferFetchMultiSampleSupported not implemented");
}

bool D3D11Driver::isFrameTimeSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameTimeSupported not implemented");
}

bool D3D11Driver::isAutoDepthResolveSupported() {
    PANIC_POSTCONDITION("D3D11::isAutoDepthResolveSupported not implemented");
}

bool D3D11Driver::isSRGBSwapChainSupported() {
    PANIC_POSTCONDITION("D3D11::isSRGBSwapChainSupported not implemented");
}

bool D3D11Driver::isMSAASwapChainSupported(uint32_t) {
    PANIC_POSTCONDITION("D3D11::isMSAASwapChainSupported not implemented");
}

bool D3D11Driver::isProtectedContentSupported() {
    PANIC_POSTCONDITION("D3D11::isProtectedContentSupported not implemented");
}

bool D3D11Driver::isStereoSupported() {
    PANIC_POSTCONDITION("D3D11::isStereoSupported not implemented");
}

bool D3D11Driver::isParallelShaderCompileSupported() {
    PANIC_POSTCONDITION("D3D11::isParallelShaderCompileSupported not implemented");
}

bool D3D11Driver::isDepthStencilResolveSupported() {
    PANIC_POSTCONDITION("D3D11::isDepthStencilResolveSupported not implemented");
}

bool D3D11Driver::isDepthStencilBlitSupported(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isDepthStencilBlitSupported not implemented");
}

bool D3D11Driver::isProtectedTexturesSupported() {
    PANIC_POSTCONDITION("D3D11::isProtectedTexturesSupported not implemented");
}

bool D3D11Driver::isDepthClampSupported() {
    PANIC_POSTCONDITION("D3D11::isDepthClampSupported not implemented");
}

bool D3D11Driver::isAsynchronousModeEnabled() {
    PANIC_POSTCONDITION("D3D11::isAsynchronousModeEnabled not implemented");
}

uint8_t D3D11Driver::getMaxDrawBuffers() {
    PANIC_POSTCONDITION("D3D11::getMaxDrawBuffers not implemented");
}

size_t D3D11Driver::getMaxUniformBufferSize() {
    PANIC_POSTCONDITION("D3D11::getMaxUniformBufferSize not implemented");
}

size_t D3D11Driver::getMaxTextureSize(SamplerType target) {
    PANIC_POSTCONDITION("D3D11::getMaxTextureSize not implemented");
}

size_t D3D11Driver::getMaxArrayTextureLayers() {
    PANIC_POSTCONDITION("D3D11::getMaxArrayTextureLayers not implemented");
}

math::float2 D3D11Driver::getClipSpaceParams() {
    PANIC_POSTCONDITION("D3D11::getClipSpaceParams not implemented");
}

void D3D11Driver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
    PANIC_POSTCONDITION("D3D11::setupExternalImage2 not implemented");
}

void D3D11Driver::setupExternalImage(void* image) {
    PANIC_POSTCONDITION("D3D11::setupExternalImage not implemented");
}

TimerQueryResult D3D11Driver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    PANIC_POSTCONDITION("D3D11::getTimerQueryValue not implemented");
}

bool D3D11Driver::isWorkaroundNeeded(Workaround) {
    PANIC_POSTCONDITION("D3D11::isWorkaroundNeeded not implemented");
}

FeatureLevel D3D11Driver::getFeatureLevel() {
    PANIC_POSTCONDITION("D3D11::getFeatureLevel not implemented");
}

size_t D3D11Driver::getUniformBufferOffsetAlignment() {
    PANIC_POSTCONDITION("D3D11::getUniformBufferOffsetAlignment not implemented");
}

bool D3D11Driver::isCompositorTimingSupported() {
    PANIC_POSTCONDITION("D3D11::isCompositorTimingSupported not implemented");
}

bool D3D11Driver::isPresentationTimeSupported() {
    PANIC_POSTCONDITION("D3D11::isPresentationTimeSupported not implemented");
}

bool D3D11Driver::queryFrameTimestamps(SwapChainHandle swapChain, uint64_t frameId,
        FrameTimestamps* outFrameTimestamps) {
    PANIC_POSTCONDITION("D3D11::queryFrameTimestamps not implemented");
}

bool D3D11Driver::queryCompositorTiming(backend::SwapChainHandle swapChain,
        backend::CompositorTiming* outCompositorTiming) {
    PANIC_POSTCONDITION("D3D11::queryCompositorTiming not implemented");
}

bool D3D11Driver::cancelAsyncJob(AsyncCallId jobId) {
    PANIC_POSTCONDITION("D3D11::cancelAsyncJob not implemented");
}

template class ConcreteDispatcher<D3D11Driver>;

} // namespace filament::backend
