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
    return {};
}

void D3D11Driver::unsupported(char const* method) noexcept {
    PANIC_POSTCONDITION("D3D11 operation not implemented: %s", method);
}

Handle<HwStream> D3D11Driver::createStreamNative(void* nativeStream, utils::ImmutableCString tag) {
    unsupported("createStreamNative");
}

Handle<HwStream> D3D11Driver::createStreamAcquired(utils::ImmutableCString tag) {
    unsupported("createStreamAcquired");
}

void D3D11Driver::setAcquiredImage(Handle<HwStream> sh, void* image, const math::mat3f& transform,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
    unsupported("setAcquiredImage");
}

void D3D11Driver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
    unsupported("setStreamDimensions");
}

int64_t D3D11Driver::getStreamTimestamp(Handle<HwStream> sh) {
    unsupported("getStreamTimestamp");
}

void D3D11Driver::updateStreams(CommandStream* driver) {
    unsupported("updateStreams");
}

FenceStatus D3D11Driver::getFenceStatus(Handle<HwFence> fh) {
    unsupported("getFenceStatus");
}

FenceStatus D3D11Driver::fenceWait(Handle<HwFence> fh, uint64_t timeout) {
    unsupported("fenceWait");
}

void D3D11Driver::fenceCancel(FenceHandle fh) {
    unsupported("fenceCancel");
}

void D3D11Driver::getPlatformSync(Handle<HwSync> sh, CallbackHandler* handler,
        Platform::SyncCallback cb, void* userData) {
    unsupported("getPlatformSync");
}

bool D3D11Driver::isTextureFormatSupported(TextureFormat format) {
    unsupported("isTextureFormatSupported");
}

bool D3D11Driver::isTextureSwizzleSupported() {
    unsupported("isTextureSwizzleSupported");
}

bool D3D11Driver::isTextureFormatMipmappable(TextureFormat format) {
    unsupported("isTextureFormatMipmappable");
}

bool D3D11Driver::isTextureFormatFilterable(TextureFormat format) {
    unsupported("isTextureFormatFilterable");
}

bool D3D11Driver::isRenderTargetFormatSupported(TextureFormat format) {
    unsupported("isRenderTargetFormatSupported");
}

bool D3D11Driver::isFrameBufferFetchSupported() {
    unsupported("isFrameBufferFetchSupported");
}

bool D3D11Driver::isFrameBufferFetchMultiSampleSupported() {
    unsupported("isFrameBufferFetchMultiSampleSupported");
}

bool D3D11Driver::isFrameTimeSupported() {
    unsupported("isFrameTimeSupported");
}

bool D3D11Driver::isAutoDepthResolveSupported() {
    unsupported("isAutoDepthResolveSupported");
}

bool D3D11Driver::isSRGBSwapChainSupported() {
    unsupported("isSRGBSwapChainSupported");
}

bool D3D11Driver::isMSAASwapChainSupported(uint32_t) {
    unsupported("isMSAASwapChainSupported");
}

bool D3D11Driver::isProtectedContentSupported() {
    unsupported("isProtectedContentSupported");
}

bool D3D11Driver::isStereoSupported() {
    unsupported("isStereoSupported");
}

bool D3D11Driver::isParallelShaderCompileSupported() {
    unsupported("isParallelShaderCompileSupported");
}

bool D3D11Driver::isDepthStencilResolveSupported() {
    unsupported("isDepthStencilResolveSupported");
}

bool D3D11Driver::isDepthStencilBlitSupported(TextureFormat format) {
    unsupported("isDepthStencilBlitSupported");
}

bool D3D11Driver::isProtectedTexturesSupported() {
    unsupported("isProtectedTexturesSupported");
}

bool D3D11Driver::isDepthClampSupported() {
    unsupported("isDepthClampSupported");
}

bool D3D11Driver::isAsynchronousModeEnabled() {
    unsupported("isAsynchronousModeEnabled");
}

uint8_t D3D11Driver::getMaxDrawBuffers() {
    unsupported("getMaxDrawBuffers");
}

size_t D3D11Driver::getMaxUniformBufferSize() {
    unsupported("getMaxUniformBufferSize");
}

size_t D3D11Driver::getMaxTextureSize(SamplerType target) {
    unsupported("getMaxTextureSize");
}

size_t D3D11Driver::getMaxArrayTextureLayers() {
    unsupported("getMaxArrayTextureLayers");
}

math::float2 D3D11Driver::getClipSpaceParams() {
    unsupported("getClipSpaceParams");
}

void D3D11Driver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
    unsupported("setupExternalImage2");
}

void D3D11Driver::setupExternalImage(void* image) {
    unsupported("setupExternalImage");
}

TimerQueryResult D3D11Driver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    unsupported("getTimerQueryValue");
}

bool D3D11Driver::isWorkaroundNeeded(Workaround) {
    unsupported("isWorkaroundNeeded");
}

FeatureLevel D3D11Driver::getFeatureLevel() {
    unsupported("getFeatureLevel");
}

size_t D3D11Driver::getUniformBufferOffsetAlignment() {
    unsupported("getUniformBufferOffsetAlignment");
}

bool D3D11Driver::isCompositorTimingSupported() {
    unsupported("isCompositorTimingSupported");
}

bool D3D11Driver::isPresentationTimeSupported() {
    unsupported("isPresentationTimeSupported");
}

bool D3D11Driver::queryFrameTimestamps(SwapChainHandle swapChain, uint64_t frameId,
        FrameTimestamps* outFrameTimestamps) {
    unsupported("queryFrameTimestamps");
}

bool D3D11Driver::queryCompositorTiming(backend::SwapChainHandle swapChain,
        backend::CompositorTiming* outCompositorTiming) {
    unsupported("queryCompositorTiming");
}

bool D3D11Driver::cancelAsyncJob(AsyncCallId jobId) {
    unsupported("cancelAsyncJob");
}

template class ConcreteDispatcher<D3D11Driver>;

} // namespace filament::backend
