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

#include "d3d11/D3D11SwapChain.h"

#include <utils/Panic.h>

#include <d3d11.h>

#include <algorithm>
#include <type_traits>
#include <utility>

namespace filament::backend {

static_assert(!std::is_abstract_v<D3D11Driver>, "D3D11 must cover the complete driver interface");

D3D11Driver::D3D11Driver(Platform::DriverConfig const& driverConfig, ID3D11Device* device,
        ID3D11DeviceContext* context, utils::CString deviceName,
        utils::CString deviceFeatureLevel) noexcept
        : DriverBase(driverConfig),
          mHandleAllocator("D3D11 handles", driverConfig.handleArenaSize
                  ? driverConfig.handleArenaSize : 1024 * 1024),
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
        for (auto handle : mSwapChains) {
            mHandleAllocator.deallocate<D3D11SwapChain>(handle);
        }
        mSwapChains.clear();
        for (auto handle : mRenderTargets) {
            mHandleAllocator.deallocate<HwRenderTarget>(handle);
        }
        mRenderTargets.clear();
        mCurrentSwapChain = nullptr;
        mRenderPassActive = false;
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

SwapChainHandle D3D11Driver::createSwapChainS() noexcept {
    return mHandleAllocator.allocateAndConstruct<D3D11SwapChain>();
}

void D3D11Driver::createSwapChainR(SwapChainHandle handle, void* nativeWindow, uint64_t flags,
        utils::ImmutableCString&& tag) {
    mSwapChains.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(flags == 0) << "Unsupported D3D11 swap chain flags";
    auto* swapChain = mHandleAllocator.handle_cast<D3D11SwapChain*>(handle);
    swapChain->initialize(mDevice, nativeWindow);
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

RenderTargetHandle D3D11Driver::createDefaultRenderTargetS() noexcept {
    return mHandleAllocator.allocateAndConstruct<HwRenderTarget>();
}

void D3D11Driver::createDefaultRenderTargetR(RenderTargetHandle handle,
        utils::ImmutableCString&& tag) {
    mRenderTargets.push_back(handle);
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

void D3D11Driver::destroySwapChain(SwapChainHandle handle) {
    FILAMENT_CHECK_PRECONDITION(!mRenderPassActive) << "Render pass is active";
    auto* swapChain = mHandleAllocator.handle_cast<D3D11SwapChain*>(handle);
    mContext->OMSetRenderTargets(0, nullptr, nullptr);
    if (mCurrentSwapChain == swapChain) {
        mCurrentSwapChain = nullptr;
    }
    mHandleAllocator.deallocate(handle, swapChain);
    std::erase(mSwapChains, handle);
    mContext->Flush();
}

void D3D11Driver::destroyRenderTarget(RenderTargetHandle handle) {
    FILAMENT_CHECK_PRECONDITION(!mRenderPassActive) << "Render pass is active";
    mHandleAllocator.deallocate<HwRenderTarget>(handle);
    std::erase(mRenderTargets, handle);
}

void D3D11Driver::makeCurrent(SwapChainHandle draw, SwapChainHandle read) {
    FILAMENT_CHECK_PRECONDITION(draw == read) << "Separate D3D11 read swap chains are not supported";
    FILAMENT_CHECK_PRECONDITION(!mRenderPassActive) << "Render pass is active";
    mCurrentSwapChain = mHandleAllocator.handle_cast<D3D11SwapChain*>(draw);
}

void D3D11Driver::beginRenderPass(RenderTargetHandle target, RenderPassParams const& params) {
    FILAMENT_CHECK_PRECONDITION(!mRenderPassActive && mCurrentSwapChain)
            << "Invalid D3D11 render pass";
    FILAMENT_CHECK_PRECONDITION(std::find(mRenderTargets.begin(), mRenderTargets.end(), target)
            != mRenderTargets.end()) << "Invalid D3D11 default render target";
    FILAMENT_CHECK_PRECONDITION(params.flags.clear == TargetBufferFlags::COLOR0)
            << "D3D11 render pass requires a color clear";
    FILAMENT_CHECK_PRECONDITION(params.subpassMask == 0) << "D3D11 subpasses are not supported";
    mRenderPassActive = true;
    if (mCurrentSwapChain->update(mDevice, mContext)) {
        float const color[] = { float(params.clearColor.r), float(params.clearColor.g),
                float(params.clearColor.b), float(params.clearColor.a) };
        mCurrentSwapChain->clear(mContext, color);
    }
}

void D3D11Driver::endRenderPass(int) {
    FILAMENT_CHECK_PRECONDITION(mRenderPassActive) << "No D3D11 render pass is active";
    mContext->OMSetRenderTargets(0, nullptr, nullptr);
    mRenderPassActive = false;
}

void D3D11Driver::commit(SwapChainHandle handle) {
    FILAMENT_CHECK_PRECONDITION(!mRenderPassActive) << "Render pass is active";
    auto* swapChain = mHandleAllocator.handle_cast<D3D11SwapChain*>(handle);
    FILAMENT_CHECK_PRECONDITION(swapChain == mCurrentSwapChain) << "D3D11 swap chain is not current";
    swapChain->present(mDevice);
}

void D3D11Driver::tick(int dummy) {
    PANIC_POSTCONDITION("D3D11::tick not implemented");
}

void D3D11Driver::beginFrame(
        int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs,
        uint32_t frameId) {
    PANIC_POSTCONDITION("D3D11::beginFrame not implemented");
}

void D3D11Driver::setFrameScheduledCallback(
        backend::SwapChainHandle sch,
        backend::CallbackHandler* handler,
        backend::FrameScheduledCallback&& callback,
        uint64_t flags) {
    PANIC_POSTCONDITION("D3D11::setFrameScheduledCallback not implemented");
}

void D3D11Driver::setFrameCompletedCallback(
        backend::SwapChainHandle sch,
        backend::CallbackHandler* handler,
        utils::Invocable<void(void)>&& callback) {
    PANIC_POSTCONDITION("D3D11::setFrameCompletedCallback not implemented");
}

void D3D11Driver::setPresentationTime(int64_t monotonic_clock_ns) {
    PANIC_POSTCONDITION("D3D11::setPresentationTime not implemented");
}

void D3D11Driver::endFrame(uint32_t frameId) {
    PANIC_POSTCONDITION("D3D11::endFrame not implemented");
}

void D3D11Driver::flush(int dummy) {
    PANIC_POSTCONDITION("D3D11::flush not implemented");
}

void D3D11Driver::finish(int dummy) {
    PANIC_POSTCONDITION("D3D11::finish not implemented");
}

void D3D11Driver::resetState(int dummy) {
    PANIC_POSTCONDITION("D3D11::resetState not implemented");
}

backend::VertexBufferInfoHandle D3D11Driver::createVertexBufferInfoS() noexcept {
    PANIC_POSTCONDITION("D3D11::createVertexBufferInfoS not implemented");
}

void D3D11Driver::createVertexBufferInfoR(
        backend::VertexBufferInfoHandle handle,
        uint8_t bufferCount,
        uint8_t attributeCount,
        backend::AttributeArray attributes,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createVertexBufferInfoR not implemented");
}

backend::VertexBufferHandle D3D11Driver::createVertexBufferS() noexcept {
    PANIC_POSTCONDITION("D3D11::createVertexBufferS not implemented");
}

void D3D11Driver::createVertexBufferR(
        backend::VertexBufferHandle handle,
        uint32_t vertexCount,
        backend::VertexBufferInfoHandle vbih,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createVertexBufferR not implemented");
}

backend::VertexBufferHandle D3D11Driver::createVertexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createVertexBufferAsyncS not implemented");
}

void D3D11Driver::createVertexBufferAsyncR(
        backend::VertexBufferHandle handle,
        uint32_t vertexCount,
        backend::VertexBufferInfoHandle vbih,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createVertexBufferAsyncR not implemented");
}

backend::IndexBufferHandle D3D11Driver::createIndexBufferS() noexcept {
    PANIC_POSTCONDITION("D3D11::createIndexBufferS not implemented");
}

void D3D11Driver::createIndexBufferR(
        backend::IndexBufferHandle handle,
        backend::ElementType elementType,
        uint32_t indexCount,
        backend::BufferUsage usage,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createIndexBufferR not implemented");
}

backend::IndexBufferHandle D3D11Driver::createIndexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createIndexBufferAsyncS not implemented");
}

void D3D11Driver::createIndexBufferAsyncR(
        backend::IndexBufferHandle handle,
        backend::ElementType elementType,
        uint32_t indexCount,
        backend::BufferUsage usage,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createIndexBufferAsyncR not implemented");
}

backend::BufferObjectHandle D3D11Driver::createBufferObjectS() noexcept {
    PANIC_POSTCONDITION("D3D11::createBufferObjectS not implemented");
}

void D3D11Driver::createBufferObjectR(
        backend::BufferObjectHandle handle,
        uint32_t byteCount,
        backend::BufferObjectBinding bindingType,
        backend::BufferUsage usage,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createBufferObjectR not implemented");
}

backend::BufferObjectHandle D3D11Driver::createBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createBufferObjectAsyncS not implemented");
}

void D3D11Driver::createBufferObjectAsyncR(
        backend::BufferObjectHandle handle,
        uint32_t byteCount,
        backend::BufferObjectBinding bindingType,
        backend::BufferUsage usage,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createBufferObjectAsyncR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureS not implemented");
}

void D3D11Driver::createTextureR(
        backend::TextureHandle handle,
        backend::SamplerType target,
        uint8_t levels,
        backend::TextureFormat format,
        uint8_t samples,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::TextureUsage usage,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureAsyncS not implemented");
}

void D3D11Driver::createTextureAsyncR(
        backend::TextureHandle handle,
        backend::SamplerType target,
        uint8_t levels,
        backend::TextureFormat format,
        uint8_t samples,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::TextureUsage usage,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureAsyncR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureViewS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureViewS not implemented");
}

void D3D11Driver::createTextureViewR(
        backend::TextureHandle handle,
        backend::TextureHandle texture,
        uint8_t baseLevel,
        uint8_t levelCount,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureViewR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureViewSwizzleS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureViewSwizzleS not implemented");
}

void D3D11Driver::createTextureViewSwizzleR(
        backend::TextureHandle handle,
        backend::TextureHandle texture,
        backend::TextureSwizzle r,
        backend::TextureSwizzle g,
        backend::TextureSwizzle b,
        backend::TextureSwizzle a,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureViewSwizzleR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureViewSwizzleAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureViewSwizzleAsyncS not implemented");
}

void D3D11Driver::createTextureViewSwizzleAsyncR(
        backend::TextureHandle handle,
        backend::TextureHandle texture,
        backend::TextureSwizzle r,
        backend::TextureSwizzle g,
        backend::TextureSwizzle b,
        backend::TextureSwizzle a,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureViewSwizzleAsyncR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureExternalImage2S() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImage2S not implemented");
}

void D3D11Driver::createTextureExternalImage2R(
        backend::TextureHandle handle,
        backend::SamplerType target,
        backend::TextureFormat format,
        uint32_t width,
        uint32_t height,
        backend::TextureUsage usage,
        backend::Platform::ExternalImageHandleRef image,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImage2R not implemented");
}

backend::TextureHandle D3D11Driver::createTextureExternalImageS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImageS not implemented");
}

void D3D11Driver::createTextureExternalImageR(
        backend::TextureHandle handle,
        backend::SamplerType target,
        backend::TextureFormat format,
        uint32_t width,
        uint32_t height,
        backend::TextureUsage usage,
        void* image,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImageR not implemented");
}

backend::TextureHandle D3D11Driver::createTextureExternalImagePlaneS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImagePlaneS not implemented");
}

void D3D11Driver::createTextureExternalImagePlaneR(
        backend::TextureHandle handle,
        backend::TextureFormat format,
        uint32_t width,
        uint32_t height,
        backend::TextureUsage usage,
        void* image,
        uint32_t plane,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTextureExternalImagePlaneR not implemented");
}

backend::TextureHandle D3D11Driver::importTextureS() noexcept {
    PANIC_POSTCONDITION("D3D11::importTextureS not implemented");
}

void D3D11Driver::importTextureR(
        backend::TextureHandle handle,
        intptr_t id,
        backend::SamplerType target,
        uint8_t levels,
        backend::TextureFormat format,
        uint8_t samples,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::TextureUsage usage,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::importTextureR not implemented");
}

backend::TextureHandle D3D11Driver::importTextureAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::importTextureAsyncS not implemented");
}

void D3D11Driver::importTextureAsyncR(
        backend::TextureHandle handle,
        intptr_t id,
        backend::SamplerType target,
        uint8_t levels,
        backend::TextureFormat format,
        uint8_t samples,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::TextureUsage usage,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::importTextureAsyncR not implemented");
}

backend::RenderPrimitiveHandle D3D11Driver::createRenderPrimitiveS() noexcept {
    PANIC_POSTCONDITION("D3D11::createRenderPrimitiveS not implemented");
}

void D3D11Driver::createRenderPrimitiveR(
        backend::RenderPrimitiveHandle handle,
        backend::VertexBufferHandle vbh,
        backend::IndexBufferHandle ibh,
        backend::PrimitiveType pt,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createRenderPrimitiveR not implemented");
}

backend::ProgramHandle D3D11Driver::createProgramS() noexcept {
    PANIC_POSTCONDITION("D3D11::createProgramS not implemented");
}

void D3D11Driver::createProgramR(
        backend::ProgramHandle handle,
        backend::Program&& program,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createProgramR not implemented");
}

backend::RenderTargetHandle D3D11Driver::createRenderTargetS() noexcept {
    PANIC_POSTCONDITION("D3D11::createRenderTargetS not implemented");
}

void D3D11Driver::createRenderTargetR(
        backend::RenderTargetHandle handle,
        backend::TargetBufferFlags targetBufferFlags,
        uint32_t width,
        uint32_t height,
        uint8_t samples,
        uint8_t layerCount,
        backend::MRT color,
        backend::TargetBufferInfo depth,
        backend::TargetBufferInfo stencil,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createRenderTargetR not implemented");
}

backend::FenceHandle D3D11Driver::createFenceS() noexcept {
    PANIC_POSTCONDITION("D3D11::createFenceS not implemented");
}

void D3D11Driver::createFenceR(backend::FenceHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createFenceR not implemented");
}

backend::SyncHandle D3D11Driver::createSyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createSyncS not implemented");
}

void D3D11Driver::createSyncR(backend::SyncHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createSyncR not implemented");
}

backend::SwapChainHandle D3D11Driver::createSwapChainHeadlessS() noexcept {
    PANIC_POSTCONDITION("D3D11::createSwapChainHeadlessS not implemented");
}

void D3D11Driver::createSwapChainHeadlessR(
        backend::SwapChainHandle handle,
        uint32_t width,
        uint32_t height,
        uint64_t flags,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createSwapChainHeadlessR not implemented");
}

backend::TimerQueryHandle D3D11Driver::createTimerQueryS() noexcept {
    PANIC_POSTCONDITION("D3D11::createTimerQueryS not implemented");
}

void D3D11Driver::createTimerQueryR(backend::TimerQueryHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTimerQueryR not implemented");
}

backend::DescriptorSetLayoutHandle D3D11Driver::createDescriptorSetLayoutS() noexcept {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetLayoutS not implemented");
}

void D3D11Driver::createDescriptorSetLayoutR(
        backend::DescriptorSetLayoutHandle handle,
        backend::DescriptorSetLayout&& info,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetLayoutR not implemented");
}

backend::DescriptorSetHandle D3D11Driver::createDescriptorSetS() noexcept {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetS not implemented");
}

void D3D11Driver::createDescriptorSetR(
        backend::DescriptorSetHandle handle,
        backend::DescriptorSetLayoutHandle dslh,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetR not implemented");
}

backend::MemoryMappedBufferHandle D3D11Driver::mapBufferS() noexcept {
    PANIC_POSTCONDITION("D3D11::mapBufferS not implemented");
}

void D3D11Driver::mapBufferR(
        backend::MemoryMappedBufferHandle handle,
        backend::BufferObjectHandle boh,
        size_t offset,
        size_t size,
        backend::MapBufferAccessFlags access,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::mapBufferR not implemented");
}

void D3D11Driver::destroyVertexBuffer(backend::VertexBufferHandle vbh) {
    PANIC_POSTCONDITION("D3D11::destroyVertexBuffer not implemented");
}

void D3D11Driver::destroyVertexBufferInfo(backend::VertexBufferInfoHandle vbih) {
    PANIC_POSTCONDITION("D3D11::destroyVertexBufferInfo not implemented");
}

void D3D11Driver::destroyIndexBuffer(backend::IndexBufferHandle ibh) {
    PANIC_POSTCONDITION("D3D11::destroyIndexBuffer not implemented");
}

void D3D11Driver::destroyBufferObject(backend::BufferObjectHandle boh) {
    PANIC_POSTCONDITION("D3D11::destroyBufferObject not implemented");
}

void D3D11Driver::destroyRenderPrimitive(backend::RenderPrimitiveHandle rph) {
    PANIC_POSTCONDITION("D3D11::destroyRenderPrimitive not implemented");
}

void D3D11Driver::destroyProgram(backend::ProgramHandle ph) {
    PANIC_POSTCONDITION("D3D11::destroyProgram not implemented");
}

void D3D11Driver::destroyTexture(backend::TextureHandle th) {
    PANIC_POSTCONDITION("D3D11::destroyTexture not implemented");
}

void D3D11Driver::destroyStream(backend::StreamHandle sh) {
    PANIC_POSTCONDITION("D3D11::destroyStream not implemented");
}

void D3D11Driver::destroyTimerQuery(backend::TimerQueryHandle sh) {
    PANIC_POSTCONDITION("D3D11::destroyTimerQuery not implemented");
}

void D3D11Driver::destroyFence(backend::FenceHandle fh) {
    PANIC_POSTCONDITION("D3D11::destroyFence not implemented");
}

void D3D11Driver::destroySync(backend::SyncHandle sh) {
    PANIC_POSTCONDITION("D3D11::destroySync not implemented");
}

void D3D11Driver::destroyDescriptorSetLayout(backend::DescriptorSetLayoutHandle dslh) {
    PANIC_POSTCONDITION("D3D11::destroyDescriptorSetLayout not implemented");
}

void D3D11Driver::destroyDescriptorSet(backend::DescriptorSetHandle dsh) {
    PANIC_POSTCONDITION("D3D11::destroyDescriptorSet not implemented");
}

void D3D11Driver::unmapBuffer(backend::MemoryMappedBufferHandle mmbh) {
    PANIC_POSTCONDITION("D3D11::unmapBuffer not implemented");
}

void D3D11Driver::setVertexBufferObject(
        backend::VertexBufferHandle vbh,
        uint32_t index,
        backend::BufferObjectHandle bufferObject) {
    PANIC_POSTCONDITION("D3D11::setVertexBufferObject not implemented");
}

backend::AsyncCallId D3D11Driver::setVertexBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::setVertexBufferObjectAsyncS not implemented");
}

void D3D11Driver::setVertexBufferObjectAsyncR(
        backend::AsyncCallId handle,
        backend::VertexBufferHandle vbh,
        uint32_t index,
        backend::BufferObjectHandle bufferObject,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::setVertexBufferObjectAsyncR not implemented");
}

void D3D11Driver::updateIndexBuffer(
        backend::IndexBufferHandle ibh,
        backend::BufferDescriptor&& data,
        uint32_t byteOffset) {
    PANIC_POSTCONDITION("D3D11::updateIndexBuffer not implemented");
}

backend::AsyncCallId D3D11Driver::updateIndexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::updateIndexBufferAsyncS not implemented");
}

void D3D11Driver::updateIndexBufferAsyncR(
        backend::AsyncCallId handle,
        backend::IndexBufferHandle ibh,
        backend::BufferDescriptor&& data,
        uint32_t byteOffset,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::updateIndexBufferAsyncR not implemented");
}

void D3D11Driver::updateBufferObject(
        backend::BufferObjectHandle ibh,
        backend::BufferDescriptor&& data,
        uint32_t byteOffset) {
    PANIC_POSTCONDITION("D3D11::updateBufferObject not implemented");
}

backend::AsyncCallId D3D11Driver::updateBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::updateBufferObjectAsyncS not implemented");
}

void D3D11Driver::updateBufferObjectAsyncR(
        backend::AsyncCallId handle,
        backend::BufferObjectHandle ibh,
        backend::BufferDescriptor&& data,
        uint32_t byteOffset,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::updateBufferObjectAsyncR not implemented");
}

void D3D11Driver::updateBufferObjectUnsynchronized(
        backend::BufferObjectHandle boh,
        backend::BufferDescriptor&& data,
        uint32_t byteOffset) {
    PANIC_POSTCONDITION("D3D11::updateBufferObjectUnsynchronized not implemented");
}

void D3D11Driver::resetBufferObject(backend::BufferObjectHandle boh) {
    PANIC_POSTCONDITION("D3D11::resetBufferObject not implemented");
}

void D3D11Driver::update3DImage(
        backend::TextureHandle th,
        uint32_t level,
        uint32_t xoffset,
        uint32_t yoffset,
        uint32_t zoffset,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::PixelBufferDescriptor&& data) {
    PANIC_POSTCONDITION("D3D11::update3DImage not implemented");
}

backend::AsyncCallId D3D11Driver::update3DImageAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::update3DImageAsyncS not implemented");
}

void D3D11Driver::update3DImageAsyncR(
        backend::AsyncCallId handle,
        backend::TextureHandle th,
        uint32_t level,
        uint32_t xoffset,
        uint32_t yoffset,
        uint32_t zoffset,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        backend::PixelBufferDescriptor&& data,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::update3DImageAsyncR not implemented");
}

void D3D11Driver::generateMipmaps(backend::TextureHandle th) {
    PANIC_POSTCONDITION("D3D11::generateMipmaps not implemented");
}

void D3D11Driver::setExternalStream(backend::TextureHandle th, backend::StreamHandle sh) {
    PANIC_POSTCONDITION("D3D11::setExternalStream not implemented");
}

void D3D11Driver::beginTimerQuery(backend::TimerQueryHandle tqh) {
    PANIC_POSTCONDITION("D3D11::beginTimerQuery not implemented");
}

void D3D11Driver::endTimerQuery(backend::TimerQueryHandle tqh) {
    PANIC_POSTCONDITION("D3D11::endTimerQuery not implemented");
}

void D3D11Driver::compilePrograms(
        backend::CompilerPriorityQueue priority,
        backend::CallbackHandler* handler,
        backend::CallbackHandler::Callback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::compilePrograms not implemented");
}

void D3D11Driver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
    PANIC_POSTCONDITION("D3D11::updateDescriptorSetBuffer not implemented");
}

void D3D11Driver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
    PANIC_POSTCONDITION("D3D11::updateDescriptorSetTexture not implemented");
}

void D3D11Driver::copyToMemoryMappedBuffer(
        backend::MemoryMappedBufferHandle mmbh,
        size_t offset,
        backend::BufferDescriptor&& data) {
    PANIC_POSTCONDITION("D3D11::copyToMemoryMappedBuffer not implemented");
}

void D3D11Driver::setPushConstant(
        backend::ShaderStage stage,
        uint8_t index,
        backend::PushConstantVariant value) {
    PANIC_POSTCONDITION("D3D11::setPushConstant not implemented");
}

void D3D11Driver::nextSubpass(int dummy) {
    PANIC_POSTCONDITION("D3D11::nextSubpass not implemented");
}

void D3D11Driver::bindPipeline(backend::PipelineState const& state) {
    PANIC_POSTCONDITION("D3D11::bindPipeline not implemented");
}

void D3D11Driver::bindRenderPrimitive(backend::RenderPrimitiveHandle rph) {
    PANIC_POSTCONDITION("D3D11::bindRenderPrimitive not implemented");
}

void D3D11Driver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t set,
        backend::DescriptorSetOffsetArray&& offsets) {
    PANIC_POSTCONDITION("D3D11::bindDescriptorSet not implemented");
}

void D3D11Driver::setFrameRate(
        backend::SwapChainHandle sch,
        float frameRate,
        backend::Platform::FrameRateCompatibility compatibility,
        backend::Platform::ChangeFrameRateStrategy changeFrameRateStrategy) {
    PANIC_POSTCONDITION("D3D11::setFrameRate not implemented");
}

void D3D11Driver::insertEventMarker(const char* string) {
    PANIC_POSTCONDITION("D3D11::insertEventMarker not implemented");
}

void D3D11Driver::pushGroupMarker(const char* string) {
    PANIC_POSTCONDITION("D3D11::pushGroupMarker not implemented");
}

void D3D11Driver::popGroupMarker(int dummy) {
    PANIC_POSTCONDITION("D3D11::popGroupMarker not implemented");
}

void D3D11Driver::startCapture(int dummy) {
    PANIC_POSTCONDITION("D3D11::startCapture not implemented");
}

void D3D11Driver::stopCapture(int dummy) {
    PANIC_POSTCONDITION("D3D11::stopCapture not implemented");
}

void D3D11Driver::readPixels(
        backend::RenderTargetHandle src,
        uint32_t x,
        uint32_t y,
        uint32_t width,
        uint32_t height,
        backend::PixelBufferDescriptor&& data) {
    PANIC_POSTCONDITION("D3D11::readPixels not implemented");
}

void D3D11Driver::readTexture(
        backend::TextureHandle src,
        uint8_t level,
        uint16_t layer,
        uint32_t x,
        uint32_t y,
        uint32_t width,
        uint32_t height,
        backend::PixelBufferDescriptor&& data) {
    PANIC_POSTCONDITION("D3D11::readTexture not implemented");
}

void D3D11Driver::readBufferSubData(
        backend::BufferObjectHandle src,
        uint32_t offset,
        uint32_t size,
        backend::BufferDescriptor&& data) {
    PANIC_POSTCONDITION("D3D11::readBufferSubData not implemented");
}

void D3D11Driver::blitDEPRECATED(
        backend::TargetBufferFlags buffers,
        backend::RenderTargetHandle dst,
        backend::Viewport dstRect,
        backend::RenderTargetHandle src,
        backend::Viewport srcRect,
        backend::SamplerMagFilter filter) {
    PANIC_POSTCONDITION("D3D11::blitDEPRECATED not implemented");
}

void D3D11Driver::resolve(
        backend::TextureHandle dst,
        uint8_t dstLevel,
        uint8_t dstLayer,
        backend::TextureHandle src,
        uint8_t srcLevel,
        uint8_t srcLayer) {
    PANIC_POSTCONDITION("D3D11::resolve not implemented");
}

void D3D11Driver::blit(
        backend::TextureHandle dst,
        uint8_t dstLevel,
        uint8_t dstLayer,
        math::uint2 dstOrigin,
        backend::TextureHandle src,
        uint8_t srcLevel,
        uint8_t srcLayer,
        math::uint2 srcOrigin,
        math::uint2 size) {
    PANIC_POSTCONDITION("D3D11::blit not implemented");
}

void D3D11Driver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    PANIC_POSTCONDITION("D3D11::draw2 not implemented");
}

void D3D11Driver::drawArrays(uint32_t vertexOffset, uint32_t vertexCount, uint32_t instanceCount) {
    PANIC_POSTCONDITION("D3D11::drawArrays not implemented");
}

void D3D11Driver::draw(
        backend::PipelineState state,
        backend::RenderPrimitiveHandle rph,
        uint32_t indexOffset,
        uint32_t indexCount,
        uint32_t instanceCount) {
    PANIC_POSTCONDITION("D3D11::draw not implemented");
}

void D3D11Driver::dispatchCompute(backend::ProgramHandle program, math::uint3 workGroupCount) {
    PANIC_POSTCONDITION("D3D11::dispatchCompute not implemented");
}

void D3D11Driver::scissor(Viewport scissor) {
    PANIC_POSTCONDITION("D3D11::scissor not implemented");
}

backend::AsyncCallId D3D11Driver::queueCommandAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::queueCommandAsyncS not implemented");
}

void D3D11Driver::queueCommandAsyncR(
        backend::AsyncCallId handle,
        utils::Invocable<void(void)>&& command,
        backend::CallbackHandler* handler,
        backend::AsyncCallback callback,
        void* user) {
    PANIC_POSTCONDITION("D3D11::queueCommandAsyncR not implemented");
}

template class ConcreteDispatcher<D3D11Driver>;

} // namespace filament::backend
