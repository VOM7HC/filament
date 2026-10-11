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

#include "D3D11Shader.h"

#include <private/backend/DriverApi.h>

#include <backend/Program.h>

#include <utility>

namespace test {

using namespace filament::backend;

namespace {

constexpr char VERTEX_SHADER[] = R"(
struct VertexOutput {
    float4 position : SV_Position;
    float3 color : TEXCOORD0;
};
VertexOutput main(float2 position : TEXCOORD0, uint vertexId : SV_VertexID) {
    VertexOutput output;
    output.position = float4(position * 0.7, 0.5, 1.0);
    output.color = vertexId == 0 ? float3(1.0, 0.0, 0.0)
            : vertexId == 1 ? float3(0.0, 1.0, 0.0) : float3(0.0, 0.0, 1.0);
    return output;
}
)";

constexpr char FRAGMENT_SHADER[] = R"(
float4 main(float4 position : SV_Position, float3 color : TEXCOORD0) : SV_Target0 {
    return float4(color, 1.0);
}
)";

} // namespace

D3D11Shader::D3D11Shader(DriverApi& api) : mDriverApi(api) {
    Program program;
    program.shaderLanguage(ShaderLanguage::HLSL);
    program.shader(ShaderStage::VERTEX, VERTEX_SHADER, sizeof(VERTEX_SHADER));
    program.shader(ShaderStage::FRAGMENT, FRAGMENT_SHADER, sizeof(FRAGMENT_SHADER));
    mProgram = mDriverApi.createProgram(std::move(program));
}

D3D11Shader::~D3D11Shader() {
    mDriverApi.destroyProgram(mProgram);
}

void D3D11Shader::addProgramToPipelineState(PipelineState& state) const {
    state.program = mProgram;
    state.pipelineLayout.setLayout = {};
}

} // namespace test
