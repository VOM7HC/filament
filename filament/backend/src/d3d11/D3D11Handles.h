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

#ifndef TNT_FILAMENT_BACKEND_D3D11HANDLES_H
#define TNT_FILAMENT_BACKEND_D3D11HANDLES_H

#include "DriverBase.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <d3d11.h>
#include <d3dcommon.h>
#include <wrl/client.h>

#include <array>
#include <vector>

namespace filament::backend {

struct D3D11VertexBufferInfo : HwVertexBufferInfo {
    AttributeArray attributes;
};

struct D3D11VertexBuffer : HwVertexBuffer {
    VertexBufferInfoHandle info;
    std::array<BufferObjectHandle, MAX_VERTEX_BUFFER_COUNT> buffers;
};

struct D3D11BufferObject : HwBufferObject {
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

struct D3D11IndexBuffer : HwIndexBuffer {
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

struct D3D11RenderPrimitive : HwRenderPrimitive {
    VertexBufferHandle vertices;
    IndexBufferHandle indices;
};

struct D3D11Program : HwProgram {
    struct InputLayout {
        VertexBufferInfoHandle info;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
    };
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> vertexCode;
    std::vector<InputLayout> inputLayouts;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_D3D11HANDLES_H
