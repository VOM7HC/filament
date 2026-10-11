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

#include "d3d11/D3D11Handles.h"
#include "d3d11/D3D11SwapChain.h"

#include <backend/PipelineState.h>

#include <utils/Panic.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif

namespace filament::backend {

using Microsoft::WRL::ComPtr;

static DXGI_FORMAT getVertexFormat(ElementType type) {
    switch (type) {
        case ElementType::FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case ElementType::FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
        case ElementType::FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case ElementType::FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default: PANIC_POSTCONDITION("D3D11 vertex format not implemented");
    }
    return DXGI_FORMAT_UNKNOWN;
}

static ComPtr<ID3DBlob> compileShader(Program::ShaderBlob const& source, char const* target) {
    FILAMENT_CHECK_PRECONDITION(!source.empty()) << "Missing D3D11 shader source";
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
    ComPtr<ID3DBlob> code;
    ComPtr<ID3DBlob> errors;
    HRESULT const result = D3DCompile(source.data(), source.size(), nullptr, nullptr, nullptr,
            "main", target, flags, 0, code.GetAddressOf(), errors.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "D3DCompile failed: "
            << (errors ? static_cast<char const*>(errors->GetBufferPointer()) : "No diagnostics");
    return code;
}

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
        for (auto handle : mPrograms) {
            mHandleAllocator.deallocate<D3D11Program>(handle);
        }
        mPrograms.clear();

        for (auto handle : mRenderPrimitives) {
            mHandleAllocator.deallocate<D3D11RenderPrimitive>(handle);
        }
        mRenderPrimitives.clear();

        for (auto handle : mIndexBuffers) {
            mHandleAllocator.deallocate<D3D11IndexBuffer>(handle);
        }
        mIndexBuffers.clear();

        for (auto handle : mBufferObjects) {
            mHandleAllocator.deallocate<D3D11BufferObject>(handle);
        }
        mBufferObjects.clear();

        for (auto handle : mVertexBuffers) {
            mHandleAllocator.deallocate<D3D11VertexBuffer>(handle);
        }
        mVertexBuffers.clear();

        for (auto handle : mVertexBufferInfos) {
            mHandleAllocator.deallocate<D3D11VertexBufferInfo>(handle);
        }
        mVertexBufferInfos.clear();

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
        mBoundProgram.clear();
        mBoundVertexBufferInfo.clear();
        mBoundPrimitive.clear();
        if (mRasterizerState) {
            mRasterizerState->Release();
            mRasterizerState = nullptr;
        }
        if (mDepthStencilState) {
            mDepthStencilState->Release();
            mDepthStencilState = nullptr;
        }
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
    return {};
}

Handle<HwStream> D3D11Driver::createStreamAcquired(utils::ImmutableCString tag) {
    PANIC_POSTCONDITION("D3D11::createStreamAcquired not implemented");
    return {};
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
    return {};
}

void D3D11Driver::updateStreams(CommandStream* driver) {
    PANIC_POSTCONDITION("D3D11::updateStreams not implemented");
}

FenceStatus D3D11Driver::getFenceStatus(Handle<HwFence> fh) {
    PANIC_POSTCONDITION("D3D11::getFenceStatus not implemented");
    return {};
}

FenceStatus D3D11Driver::fenceWait(Handle<HwFence> fh, uint64_t timeout) {
    PANIC_POSTCONDITION("D3D11::fenceWait not implemented");
    return {};
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
    return {};
}

bool D3D11Driver::isTextureSwizzleSupported() {
    PANIC_POSTCONDITION("D3D11::isTextureSwizzleSupported not implemented");
    return {};
}

bool D3D11Driver::isTextureFormatMipmappable(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isTextureFormatMipmappable not implemented");
    return {};
}

bool D3D11Driver::isTextureFormatFilterable(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isTextureFormatFilterable not implemented");
    return {};
}

bool D3D11Driver::isRenderTargetFormatSupported(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isRenderTargetFormatSupported not implemented");
    return {};
}

bool D3D11Driver::isFrameBufferFetchSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameBufferFetchSupported not implemented");
    return {};
}

bool D3D11Driver::isFrameBufferFetchMultiSampleSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameBufferFetchMultiSampleSupported not implemented");
    return {};
}

bool D3D11Driver::isFrameTimeSupported() {
    PANIC_POSTCONDITION("D3D11::isFrameTimeSupported not implemented");
    return {};
}

bool D3D11Driver::isAutoDepthResolveSupported() {
    PANIC_POSTCONDITION("D3D11::isAutoDepthResolveSupported not implemented");
    return {};
}

bool D3D11Driver::isSRGBSwapChainSupported() {
    PANIC_POSTCONDITION("D3D11::isSRGBSwapChainSupported not implemented");
    return {};
}

bool D3D11Driver::isMSAASwapChainSupported(uint32_t) {
    PANIC_POSTCONDITION("D3D11::isMSAASwapChainSupported not implemented");
    return {};
}

bool D3D11Driver::isProtectedContentSupported() {
    PANIC_POSTCONDITION("D3D11::isProtectedContentSupported not implemented");
    return {};
}

bool D3D11Driver::isStereoSupported() {
    PANIC_POSTCONDITION("D3D11::isStereoSupported not implemented");
    return {};
}

bool D3D11Driver::isParallelShaderCompileSupported() {
    PANIC_POSTCONDITION("D3D11::isParallelShaderCompileSupported not implemented");
    return {};
}

bool D3D11Driver::isDepthStencilResolveSupported() {
    PANIC_POSTCONDITION("D3D11::isDepthStencilResolveSupported not implemented");
    return {};
}

bool D3D11Driver::isDepthStencilBlitSupported(TextureFormat format) {
    PANIC_POSTCONDITION("D3D11::isDepthStencilBlitSupported not implemented");
    return {};
}

bool D3D11Driver::isProtectedTexturesSupported() {
    PANIC_POSTCONDITION("D3D11::isProtectedTexturesSupported not implemented");
    return {};
}

bool D3D11Driver::isDepthClampSupported() {
    PANIC_POSTCONDITION("D3D11::isDepthClampSupported not implemented");
    return {};
}

bool D3D11Driver::isAsynchronousModeEnabled() {
    PANIC_POSTCONDITION("D3D11::isAsynchronousModeEnabled not implemented");
    return {};
}

uint8_t D3D11Driver::getMaxDrawBuffers() {
    PANIC_POSTCONDITION("D3D11::getMaxDrawBuffers not implemented");
    return {};
}

size_t D3D11Driver::getMaxUniformBufferSize() {
    PANIC_POSTCONDITION("D3D11::getMaxUniformBufferSize not implemented");
    return {};
}

size_t D3D11Driver::getMaxTextureSize(SamplerType target) {
    PANIC_POSTCONDITION("D3D11::getMaxTextureSize not implemented");
    return {};
}

size_t D3D11Driver::getMaxArrayTextureLayers() {
    PANIC_POSTCONDITION("D3D11::getMaxArrayTextureLayers not implemented");
    return {};
}

math::float2 D3D11Driver::getClipSpaceParams() {
    PANIC_POSTCONDITION("D3D11::getClipSpaceParams not implemented");
    return {};
}

void D3D11Driver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
    PANIC_POSTCONDITION("D3D11::setupExternalImage2 not implemented");
}

void D3D11Driver::setupExternalImage(void* image) {
    PANIC_POSTCONDITION("D3D11::setupExternalImage not implemented");
}

TimerQueryResult D3D11Driver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    PANIC_POSTCONDITION("D3D11::getTimerQueryValue not implemented");
    return {};
}

bool D3D11Driver::isWorkaroundNeeded(Workaround) {
    PANIC_POSTCONDITION("D3D11::isWorkaroundNeeded not implemented");
    return {};
}

FeatureLevel D3D11Driver::getFeatureLevel() {
    PANIC_POSTCONDITION("D3D11::getFeatureLevel not implemented");
    return {};
}

size_t D3D11Driver::getUniformBufferOffsetAlignment() {
    PANIC_POSTCONDITION("D3D11::getUniformBufferOffsetAlignment not implemented");
    return {};
}

bool D3D11Driver::isCompositorTimingSupported() {
    PANIC_POSTCONDITION("D3D11::isCompositorTimingSupported not implemented");
    return {};
}

bool D3D11Driver::isPresentationTimeSupported() {
    PANIC_POSTCONDITION("D3D11::isPresentationTimeSupported not implemented");
    return {};
}

bool D3D11Driver::queryFrameTimestamps(SwapChainHandle swapChain, uint64_t frameId,
        FrameTimestamps* outFrameTimestamps) {
    PANIC_POSTCONDITION("D3D11::queryFrameTimestamps not implemented");
    return {};
}

bool D3D11Driver::queryCompositorTiming(backend::SwapChainHandle swapChain,
        backend::CompositorTiming* outCompositorTiming) {
    PANIC_POSTCONDITION("D3D11::queryCompositorTiming not implemented");
    return {};
}

bool D3D11Driver::cancelAsyncJob(AsyncCallId jobId) {
    PANIC_POSTCONDITION("D3D11::cancelAsyncJob not implemented");
    return {};
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
    mRenderPassDrawable = mCurrentSwapChain->update(mDevice, mContext);
    if (mRenderPassDrawable) {
        D3D11_VIEWPORT viewport{};
        viewport.TopLeftX = float(params.viewport.left);
        viewport.TopLeftY = float(mCurrentSwapChain->getHeight())
                - float(params.viewport.bottom) - float(params.viewport.height);
        viewport.Width = float(params.viewport.width);
        viewport.Height = float(params.viewport.height);
        viewport.MinDepth = params.depthRange.near;
        viewport.MaxDepth = params.depthRange.far;
        mContext->RSSetViewports(1, &viewport);
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
    return mHandleAllocator.allocateAndConstruct<D3D11VertexBufferInfo>();
}

void D3D11Driver::createVertexBufferInfoR(
        backend::VertexBufferInfoHandle handle,
        uint8_t bufferCount,
        uint8_t attributeCount,
        backend::AttributeArray attributes,
        utils::ImmutableCString&& tag) {
    mVertexBufferInfos.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(bufferCount > 0 && bufferCount <= MAX_VERTEX_BUFFER_COUNT)
            << "Invalid vertex buffer count";
    uint8_t activeCount = 0;
    std::array<uint8_t, MAX_VERTEX_BUFFER_COUNT> strides{};
    for (auto const& attribute : attributes) {
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }
        FILAMENT_CHECK_PRECONDITION(attribute.buffer < bufferCount && attribute.stride > 0
                && attribute.flags == 0) << "Unsupported D3D11 vertex attribute";
        getVertexFormat(attribute.type);
        FILAMENT_CHECK_PRECONDITION(strides[attribute.buffer] == 0
                || strides[attribute.buffer] == attribute.stride)
                << "Attributes sharing a buffer must have the same stride";
        strides[attribute.buffer] = attribute.stride;
        ++activeCount;
    }
    FILAMENT_CHECK_PRECONDITION(activeCount > 0 && activeCount == attributeCount)
            << "Invalid vertex attribute count";
    auto* info = mHandleAllocator.handle_cast<D3D11VertexBufferInfo*>(handle);
    info->bufferCount = bufferCount;
    info->attributeCount = attributeCount;
    info->attributes = attributes;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::VertexBufferHandle D3D11Driver::createVertexBufferS() noexcept {
    return mHandleAllocator.allocateAndConstruct<D3D11VertexBuffer>();
}

void D3D11Driver::createVertexBufferR(
        backend::VertexBufferHandle handle,
        uint32_t vertexCount,
        backend::VertexBufferInfoHandle vbih,
        utils::ImmutableCString&& tag) {
    mVertexBuffers.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(vertexCount > 0) << "Empty vertex buffer";
    mHandleAllocator.handle_cast<D3D11VertexBufferInfo*>(vbih);
    auto* vertices = mHandleAllocator.handle_cast<D3D11VertexBuffer*>(handle);
    vertices->vertexCount = vertexCount;
    vertices->info = vbih;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::VertexBufferHandle D3D11Driver::createVertexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createVertexBufferAsyncS not implemented");
    return {};
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
    return mHandleAllocator.allocateAndConstruct<D3D11IndexBuffer>();
}

void D3D11Driver::createIndexBufferR(
        backend::IndexBufferHandle handle,
        backend::ElementType elementType,
        uint32_t indexCount,
        backend::BufferUsage usage,
        utils::ImmutableCString&& tag) {
    mIndexBuffers.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(elementType == ElementType::USHORT || elementType == ElementType::UINT)
            << "Unsupported D3D11 index type";
    FILAMENT_CHECK_PRECONDITION(indexCount > 0 && indexCount < (1u << 26))
            << "Invalid index count";
    auto* buffer = mHandleAllocator.handle_cast<D3D11IndexBuffer*>(handle);
    buffer->count = indexCount;
    buffer->elementSize = elementType == ElementType::USHORT ? 2 : 4;
    D3D11_BUFFER_DESC description{};
    description.ByteWidth = indexCount * buffer->elementSize;
    description.Usage = D3D11_USAGE_DEFAULT;
    description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    HRESULT const result = mDevice->CreateBuffer(&description, nullptr, buffer->buffer.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateBuffer failed: " << result;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::IndexBufferHandle D3D11Driver::createIndexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createIndexBufferAsyncS not implemented");
    return {};
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
    return mHandleAllocator.allocateAndConstruct<D3D11BufferObject>();
}

void D3D11Driver::createBufferObjectR(
        backend::BufferObjectHandle handle,
        uint32_t byteCount,
        backend::BufferObjectBinding bindingType,
        backend::BufferUsage usage,
        utils::ImmutableCString&& tag) {
    mBufferObjects.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(bindingType == BufferObjectBinding::VERTEX)
            << "D3D11 buffer binding not implemented";
    FILAMENT_CHECK_PRECONDITION(byteCount > 0 && byteCount < (1u << 31))
            << "Invalid vertex buffer size";
    auto* buffer = mHandleAllocator.handle_cast<D3D11BufferObject*>(handle);
    buffer->byteCount = byteCount;
    buffer->asynchronous = false;
    D3D11_BUFFER_DESC description{};
    description.ByteWidth = byteCount;
    description.Usage = D3D11_USAGE_DEFAULT;
    description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    HRESULT const result = mDevice->CreateBuffer(&description, nullptr, buffer->buffer.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateBuffer failed: " << result;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::BufferObjectHandle D3D11Driver::createBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createBufferObjectAsyncS not implemented");
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return {};
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
    return mHandleAllocator.allocateAndConstruct<D3D11RenderPrimitive>();
}

void D3D11Driver::createRenderPrimitiveR(
        backend::RenderPrimitiveHandle handle,
        backend::VertexBufferHandle vbh,
        backend::IndexBufferHandle ibh,
        backend::PrimitiveType pt,
        utils::ImmutableCString&& tag) {
    mRenderPrimitives.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(pt == PrimitiveType::TRIANGLES)
            << "D3D11 primitive type not implemented";
    mHandleAllocator.handle_cast<D3D11VertexBuffer*>(vbh);
    mHandleAllocator.handle_cast<D3D11IndexBuffer*>(ibh);
    auto* primitive = mHandleAllocator.handle_cast<D3D11RenderPrimitive*>(handle);
    primitive->vertices = vbh;
    primitive->indices = ibh;
    primitive->type = pt;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::ProgramHandle D3D11Driver::createProgramS() noexcept {
    return mHandleAllocator.allocateAndConstruct<D3D11Program>();
}

void D3D11Driver::createProgramR(
        backend::ProgramHandle handle,
        backend::Program&& program,
        utils::ImmutableCString&& tag) {
    Program builder(std::move(program));
    mPrograms.push_back(handle);
    FILAMENT_CHECK_PRECONDITION(builder.getShaderLanguage() == ShaderLanguage::HLSL)
            << "D3D11 requires HLSL";
    FILAMENT_CHECK_PRECONDITION(builder.getSpecializationConstants().empty() && !builder.isMultiview()
            && builder.getDescriptorSetLayouts().empty()) << "D3D11 program options not implemented";
    for (auto const& bindings : builder.getDescriptorBindings()) {
        FILAMENT_CHECK_PRECONDITION(bindings.empty()) << "D3D11 descriptors not implemented";
    }
    for (auto stage : { ShaderStage::VERTEX, ShaderStage::FRAGMENT, ShaderStage::COMPUTE }) {
        FILAMENT_CHECK_PRECONDITION(builder.getPushConstants(stage).empty())
                << "D3D11 push constants not implemented";
    }
    auto* shaders = mHandleAllocator.handle_cast<D3D11Program*>(handle);
    shaders->name = builder.getName();
    auto const& sources = builder.getShadersSource();
    FILAMENT_CHECK_PRECONDITION(sources[size_t(ShaderStage::COMPUTE)].empty())
            << "D3D11 compute not implemented";
    shaders->vertexCode = compileShader(sources[size_t(ShaderStage::VERTEX)], "vs_5_0");
    ComPtr<ID3DBlob> pixelCode = compileShader(sources[size_t(ShaderStage::FRAGMENT)], "ps_5_0");
    HRESULT result = mDevice->CreateVertexShader(shaders->vertexCode->GetBufferPointer(),
            shaders->vertexCode->GetBufferSize(), nullptr, shaders->vertexShader.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateVertexShader failed: " << result;
    result = mDevice->CreatePixelShader(pixelCode->GetBufferPointer(), pixelCode->GetBufferSize(),
            nullptr, shaders->pixelShader.GetAddressOf());
    FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreatePixelShader failed: " << result;
    mHandleAllocator.associateTagToHandle(handle.getId(), std::move(tag));
}

backend::RenderTargetHandle D3D11Driver::createRenderTargetS() noexcept {
    PANIC_POSTCONDITION("D3D11::createRenderTargetS not implemented");
    return {};
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
    return {};
}

void D3D11Driver::createFenceR(backend::FenceHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createFenceR not implemented");
}

backend::SyncHandle D3D11Driver::createSyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::createSyncS not implemented");
    return {};
}

void D3D11Driver::createSyncR(backend::SyncHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createSyncR not implemented");
}

backend::SwapChainHandle D3D11Driver::createSwapChainHeadlessS() noexcept {
    PANIC_POSTCONDITION("D3D11::createSwapChainHeadlessS not implemented");
    return {};
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
    return {};
}

void D3D11Driver::createTimerQueryR(backend::TimerQueryHandle handle, utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createTimerQueryR not implemented");
}

backend::DescriptorSetLayoutHandle D3D11Driver::createDescriptorSetLayoutS() noexcept {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetLayoutS not implemented");
    return {};
}

void D3D11Driver::createDescriptorSetLayoutR(
        backend::DescriptorSetLayoutHandle handle,
        backend::DescriptorSetLayout&& info,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetLayoutR not implemented");
}

backend::DescriptorSetHandle D3D11Driver::createDescriptorSetS() noexcept {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetS not implemented");
    return {};
}

void D3D11Driver::createDescriptorSetR(
        backend::DescriptorSetHandle handle,
        backend::DescriptorSetLayoutHandle dslh,
        utils::ImmutableCString&& tag) {
    PANIC_POSTCONDITION("D3D11::createDescriptorSetR not implemented");
}

backend::MemoryMappedBufferHandle D3D11Driver::mapBufferS() noexcept {
    PANIC_POSTCONDITION("D3D11::mapBufferS not implemented");
    return {};
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
    mHandleAllocator.deallocate<D3D11VertexBuffer>(vbh);
    std::erase(mVertexBuffers, vbh);
}

void D3D11Driver::destroyVertexBufferInfo(backend::VertexBufferInfoHandle vbih) {
    if (mBoundVertexBufferInfo == vbih) {
        mBoundVertexBufferInfo.clear();
    }
    for (auto programHandle : mPrograms) {
        auto* program = mHandleAllocator.handle_cast<D3D11Program*>(programHandle);
        std::erase_if(program->inputLayouts, [vbih](auto const& entry) {
            return entry.info == vbih;
        });
    }
    mHandleAllocator.deallocate<D3D11VertexBufferInfo>(vbih);
    std::erase(mVertexBufferInfos, vbih);
}

void D3D11Driver::destroyIndexBuffer(backend::IndexBufferHandle ibh) {
    mHandleAllocator.deallocate<D3D11IndexBuffer>(ibh);
    std::erase(mIndexBuffers, ibh);
}

void D3D11Driver::destroyBufferObject(backend::BufferObjectHandle boh) {
    mHandleAllocator.deallocate<D3D11BufferObject>(boh);
    std::erase(mBufferObjects, boh);
}

void D3D11Driver::destroyRenderPrimitive(backend::RenderPrimitiveHandle rph) {
    if (mBoundPrimitive == rph) {
        mBoundPrimitive.clear();
    }
    mHandleAllocator.deallocate<D3D11RenderPrimitive>(rph);
    std::erase(mRenderPrimitives, rph);
}

void D3D11Driver::destroyProgram(backend::ProgramHandle ph) {
    if (mBoundProgram == ph) {
        mBoundProgram.clear();
    }
    mHandleAllocator.deallocate<D3D11Program>(ph);
    std::erase(mPrograms, ph);
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
    auto* vertices = mHandleAllocator.handle_cast<D3D11VertexBuffer*>(vbh);
    auto* info = mHandleAllocator.handle_cast<D3D11VertexBufferInfo*>(vertices->info);
    FILAMENT_CHECK_PRECONDITION(index < info->bufferCount) << "Invalid vertex buffer slot";
    mHandleAllocator.handle_cast<D3D11BufferObject*>(bufferObject);
    vertices->buffers[index] = bufferObject;
}

backend::AsyncCallId D3D11Driver::setVertexBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::setVertexBufferObjectAsyncS not implemented");
    return {};
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
    auto* buffer = mHandleAllocator.handle_cast<D3D11IndexBuffer*>(ibh);
    uint32_t const capacity = buffer->count * buffer->elementSize;
    FILAMENT_CHECK_PRECONDITION(byteOffset <= capacity && data.size <= capacity - byteOffset)
            << "D3D11 buffer update out of bounds";
    if (data.size) {
        FILAMENT_CHECK_PRECONDITION(data.buffer) << "Missing D3D11 buffer data";
        D3D11_BOX const region = { byteOffset, 0, 0, byteOffset + uint32_t(data.size), 1, 1 };
        mContext->UpdateSubresource(buffer->buffer.Get(), 0, &region, data.buffer, 0, 0);
    }
    scheduleDestroy(std::move(data));
}

backend::AsyncCallId D3D11Driver::updateIndexBufferAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::updateIndexBufferAsyncS not implemented");
    return {};
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
    auto* buffer = mHandleAllocator.handle_cast<D3D11BufferObject*>(ibh);
    uint32_t const capacity = buffer->byteCount;
    FILAMENT_CHECK_PRECONDITION(byteOffset <= capacity && data.size <= capacity - byteOffset)
            << "D3D11 buffer update out of bounds";
    if (data.size) {
        FILAMENT_CHECK_PRECONDITION(data.buffer) << "Missing D3D11 buffer data";
        D3D11_BOX const region = { byteOffset, 0, 0, byteOffset + uint32_t(data.size), 1, 1 };
        mContext->UpdateSubresource(buffer->buffer.Get(), 0, &region, data.buffer, 0, 0);
    }
    scheduleDestroy(std::move(data));
}

backend::AsyncCallId D3D11Driver::updateBufferObjectAsyncS() noexcept {
    PANIC_POSTCONDITION("D3D11::updateBufferObjectAsyncS not implemented");
    return {};
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
    return {};
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
    FILAMENT_CHECK_PRECONDITION(mRenderPassActive) << "No D3D11 render pass is active";
    auto const& raster = state.rasterState;
    FILAMENT_CHECK_PRECONDITION(state.primitiveType == PrimitiveType::TRIANGLES
            && raster.culling == CullingMode::NONE && !raster.hasBlending()
            && !raster.depthWrite && raster.depthFunc == SamplerCompareFunc::A
            && raster.colorWrite && !raster.alphaToCoverage && !raster.depthClamp
            && !raster.inverseFrontFaces && !state.stencilState.stencilWrite
            && state.stencilState.front.stencilFunc == SamplerCompareFunc::A
            && state.stencilState.back.stencilFunc == SamplerCompareFunc::A
            && state.polygonOffset.slope == 0 && state.polygonOffset.constant == 0)
            << "D3D11 pipeline state not implemented";
    for (auto layout : state.pipelineLayout.setLayout) {
        FILAMENT_CHECK_PRECONDITION(!layout) << "D3D11 descriptors not implemented";
    }
    auto* program = mHandleAllocator.handle_cast<D3D11Program*>(state.program);
    auto* info = mHandleAllocator.handle_cast<D3D11VertexBufferInfo*>(state.vertexBufferInfo);
    auto layout = std::find_if(program->inputLayouts.begin(), program->inputLayouts.end(),
            [&state](auto const& entry) { return entry.info == state.vertexBufferInfo; });
    if (layout == program->inputLayouts.end()) {
        D3D11_INPUT_ELEMENT_DESC elements[MAX_VERTEX_ATTRIBUTE_COUNT]{};
        UINT count = 0;
        for (UINT location = 0; location < info->attributes.size(); ++location) {
            auto const& attribute = info->attributes[location];
            if (attribute.buffer == Attribute::BUFFER_UNUSED) {
                continue;
            }
            auto& element = elements[count++];
            element.SemanticName = "TEXCOORD";
            element.SemanticIndex = location;
            element.Format = getVertexFormat(attribute.type);
            element.InputSlot = attribute.buffer;
            element.AlignedByteOffset = attribute.offset;
            element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        }
        D3D11Program::InputLayout entry{ state.vertexBufferInfo, {} };
        HRESULT const result = mDevice->CreateInputLayout(elements, count,
                program->vertexCode->GetBufferPointer(), program->vertexCode->GetBufferSize(),
                entry.layout.GetAddressOf());
        FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateInputLayout failed: " << result;
        program->inputLayouts.push_back(std::move(entry));
        layout = program->inputLayouts.end() - 1;
    }
    if (!mRasterizerState) {
        D3D11_RASTERIZER_DESC description{};
        description.FillMode = D3D11_FILL_SOLID;
        description.CullMode = D3D11_CULL_NONE;
        description.DepthClipEnable = TRUE;
        HRESULT const result = mDevice->CreateRasterizerState(&description, &mRasterizerState);
        FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateRasterizerState failed: " << result;
    }
    if (!mDepthStencilState) {
        D3D11_DEPTH_STENCIL_DESC description{};
        description.DepthFunc = D3D11_COMPARISON_ALWAYS;
        description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        description.BackFace = description.FrontFace;
        HRESULT const result = mDevice->CreateDepthStencilState(&description, &mDepthStencilState);
        FILAMENT_CHECK_POSTCONDITION(SUCCEEDED(result)) << "CreateDepthStencilState failed: " << result;
    }
    mContext->RSSetState(mRasterizerState);
    mContext->OMSetDepthStencilState(mDepthStencilState, 0);
    mContext->OMSetBlendState(nullptr, nullptr, 0xffffffffu);
    mContext->IASetInputLayout(layout->layout.Get());
    mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mContext->VSSetShader(program->vertexShader.Get(), nullptr, 0);
    mContext->PSSetShader(program->pixelShader.Get(), nullptr, 0);
    mBoundProgram = state.program;
    mBoundVertexBufferInfo = state.vertexBufferInfo;
}

void D3D11Driver::bindRenderPrimitive(backend::RenderPrimitiveHandle rph) {
    mHandleAllocator.handle_cast<D3D11RenderPrimitive*>(rph);
    mBoundPrimitive = rph;
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
    FILAMENT_CHECK_PRECONDITION(mRenderPassActive && mBoundProgram && mBoundPrimitive
            && mBoundVertexBufferInfo) << "Incomplete D3D11 draw state";
    auto* primitive = mHandleAllocator.handle_cast<D3D11RenderPrimitive*>(mBoundPrimitive);
    auto* vertices = mHandleAllocator.handle_cast<D3D11VertexBuffer*>(primitive->vertices);
    auto* indices = mHandleAllocator.handle_cast<D3D11IndexBuffer*>(primitive->indices);
    FILAMENT_CHECK_PRECONDITION(vertices->info == mBoundVertexBufferInfo)
            << "D3D11 vertex layout does not match the pipeline";
    FILAMENT_CHECK_PRECONDITION(indexOffset <= indices->count
            && indexCount <= indices->count - indexOffset) << "D3D11 index range out of bounds";
    auto* info = mHandleAllocator.handle_cast<D3D11VertexBufferInfo*>(vertices->info);
    ID3D11Buffer* buffers[MAX_VERTEX_BUFFER_COUNT]{};
    UINT strides[MAX_VERTEX_BUFFER_COUNT]{};
    UINT offsets[MAX_VERTEX_BUFFER_COUNT]{};
    for (auto const& attribute : info->attributes) {
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }
        auto* buffer = mHandleAllocator.handle_cast<D3D11BufferObject*>(
                vertices->buffers[attribute.buffer]);
        uint64_t const required = uint64_t(vertices->vertexCount - 1) * attribute.stride
                + attribute.offset + getElementTypeSize(attribute.type);
        FILAMENT_CHECK_PRECONDITION(required <= buffer->byteCount)
                << "D3D11 vertex buffer is too small";
        buffers[attribute.buffer] = buffer->buffer.Get();
        strides[attribute.buffer] = attribute.stride;
    }
    if (!mRenderPassDrawable || !indexCount || !instanceCount) {
        return;
    }
    mContext->IASetVertexBuffers(0, info->bufferCount, buffers, strides, offsets);
    mContext->IASetIndexBuffer(indices->buffer.Get(),
            indices->elementSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    mContext->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, 0, 0);
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
    return {};
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
