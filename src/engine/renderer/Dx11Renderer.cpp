#include "engine/renderer/Dx11Renderer.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"
#include "engine/math/BoxEdges.h"
#include "engine/renderer/BmpWriter.h"
#include "engine/renderer/DebugCameraMatrices.h"
#include "engine/renderer/DebugProjection.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <vector>

namespace engine {
namespace {

const char* NdcVertexShaderSource = R"(
struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput main(VSInput input) {
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    return output;
}
)";

const char* WorldVertexShaderSource = R"(
#pragma pack_matrix(row_major)

cbuffer MatrixConstants : register(b0) {
    row_major float4x4 worldViewProjection;
};

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput main(VSInput input) {
    PSInput output;
    output.position = mul(float4(input.position, 1.0), worldViewProjection);
    output.color = input.color;
    return output;
}
)";

const char* PixelShaderSource = R"(
struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET {
    return input.color;
}
)";

void AddLine(std::vector<Dx11Renderer::Vertex>& vertices,
    float x0,
    float y0,
    float x1,
    float y1,
    float r,
    float g,
    float b)
{
    vertices.push_back({x0, y0, 0.0f, r, g, b, 1.0f});
    vertices.push_back({x1, y1, 0.0f, r, g, b, 1.0f});
}

float AspectRatio(const RendererConfig& config)
{
    return static_cast<float>(std::max(config.width, 1)) / static_cast<float>(std::max(config.height, 1));
}

void AddProjectedLine(
    std::vector<Dx11Renderer::Vertex>& vertices,
    const DebugCamera& camera,
    float aspectRatio,
    Vec3 from,
    Vec3 to,
    Color color)
{
    ProjectedPoint a;
    ProjectedPoint b;
    if (!ProjectWorldLine(camera, aspectRatio, from, to, a, b)) {
        return;
    }

    vertices.push_back({a.x, a.y, 0.0f, color.r, color.g, color.b, color.a});
    vertices.push_back({b.x, b.y, 0.0f, color.r, color.g, color.b, color.a});
}

void AddWorldTriangle(
    std::vector<Dx11Renderer::Vertex>& vertices,
    Vec3 a,
    Vec3 b,
    Vec3 c,
    Color color)
{
    vertices.push_back({a.x, a.y, a.z, color.r, color.g, color.b, color.a});
    vertices.push_back({b.x, b.y, b.z, color.r, color.g, color.b, color.a});
    vertices.push_back({c.x, c.y, c.z, color.r, color.g, color.b, color.a});
}

} // namespace

bool Dx11Renderer::initialize(const RendererConfig& config)
{
    m_config = config;
    m_window = static_cast<HWND>(config.nativeWindow);
    if (!m_window) {
        Logger::error("DX11 renderer requires a Win32 window handle.");
        return false;
    }

    if (!createDeviceAndSwapChain()
        || !createRenderTarget()
        || !createDepthResources()
        || !createPipelineStates()
        || !createShaders()
        || !createDebugGeometry()) {
        shutdown();
        return false;
    }

    Logger::info("DirectX 11 renderer initialized.");
    return true;
}

bool Dx11Renderer::createDeviceAndSwapChain()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = static_cast<UINT>(m_config.width);
    swapChainDesc.BufferDesc.Height = static_cast<UINT>(m_config.height);
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = m_window;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL selectedFeatureLevel {};
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        m_swapChain.GetAddressOf(),
        m_device.GetAddressOf(),
        &selectedFeatureLevel,
        m_context.GetAddressOf());

    if (FAILED(result)) {
        Logger::warning("Hardware DX11 device failed; trying WARP.");
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &selectedFeatureLevel,
            m_context.GetAddressOf());
    }

    if (FAILED(result)) {
        Logger::error("D3D11CreateDeviceAndSwapChain failed.");
        return false;
    }

    return true;
}

bool Dx11Renderer::createRenderTarget()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(result)) {
        Logger::error("DX11 back buffer lookup failed.");
        return false;
    }

    result = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 render target view creation failed.");
        return false;
    }

    return true;
}

bool Dx11Renderer::createDepthResources()
{
    D3D11_TEXTURE2D_DESC depthDesc {};
    depthDesc.Width = static_cast<UINT>(std::max(m_config.width, 1));
    depthDesc.Height = static_cast<UINT>(std::max(m_config.height, 1));
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT result = m_device->CreateTexture2D(&depthDesc, nullptr, m_depthStencilBuffer.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 depth buffer creation failed.");
        return false;
    }

    result = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_depthStencilView.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 depth stencil view creation failed.");
        return false;
    }

    return true;
}

bool Dx11Renderer::createPipelineStates()
{
    D3D11_DEPTH_STENCIL_DESC depthEnabled {};
    depthEnabled.DepthEnable = TRUE;
    depthEnabled.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthEnabled.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    HRESULT result = m_device->CreateDepthStencilState(&depthEnabled, m_depthEnabledState.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 depth-enabled state creation failed.");
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthDisabled {};
    depthDisabled.DepthEnable = FALSE;
    depthDisabled.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDisabled.DepthFunc = D3D11_COMPARISON_ALWAYS;

    result = m_device->CreateDepthStencilState(&depthDisabled, m_depthDisabledState.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 depth-disabled state creation failed.");
        return false;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    result = m_device->CreateRasterizerState(&rasterizerDesc, m_debugRasterizerState.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 debug rasterizer state creation failed.");
        return false;
    }

    return true;
}

bool Dx11Renderer::createShaders()
{
    Microsoft::WRL::ComPtr<ID3DBlob> ndcVertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> worldVertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT result = D3DCompile(
        NdcVertexShaderSource,
        strlen(NdcVertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        ndcVertexBlob.GetAddressOf(),
        errors.GetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 NDC vertex shader compilation failed.");
        return false;
    }

    result = D3DCompile(
        WorldVertexShaderSource,
        strlen(WorldVertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        worldVertexBlob.GetAddressOf(),
        errors.ReleaseAndGetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 world vertex shader compilation failed.");
        return false;
    }

    result = D3DCompile(
        PixelShaderSource,
        strlen(PixelShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_4_0",
        0,
        0,
        pixelBlob.GetAddressOf(),
        errors.ReleaseAndGetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 pixel shader compilation failed.");
        return false;
    }

    result = m_device->CreateVertexShader(ndcVertexBlob->GetBufferPointer(), ndcVertexBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 NDC vertex shader creation failed.");
        return false;
    }

    result = m_device->CreateVertexShader(worldVertexBlob->GetBufferPointer(), worldVertexBlob->GetBufferSize(), nullptr, m_worldVertexShader.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 world vertex shader creation failed.");
        return false;
    }

    result = m_device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 pixel shader creation failed.");
        return false;
    }

    const D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    result = m_device->CreateInputLayout(
        layout,
        static_cast<UINT>(std::size(layout)),
        ndcVertexBlob->GetBufferPointer(),
        ndcVertexBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 input layout creation failed.");
        return false;
    }

    D3D11_BUFFER_DESC matrixDesc {};
    matrixDesc.Usage = D3D11_USAGE_DEFAULT;
    matrixDesc.ByteWidth = 64;
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    result = m_device->CreateBuffer(&matrixDesc, nullptr, m_matrixConstantBuffer.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 matrix constant buffer creation failed.");
        return false;
    }

    return true;
}

bool Dx11Renderer::createDebugGeometry()
{
    std::vector<Vertex> gridVertices;
    for (int line = -10; line <= 10; ++line) {
        const float p = static_cast<float>(line) / 10.0f;
        AddLine(gridVertices, p, -1.0f, p, 1.0f, 0.22f, 0.28f, 0.36f);
        AddLine(gridVertices, -1.0f, p, 1.0f, p, 0.22f, 0.28f, 0.36f);
    }

    AddLine(gridVertices, -0.95f, 0.0f, 0.95f, 0.0f, 0.95f, 0.24f, 0.24f);
    AddLine(gridVertices, 0.0f, -0.95f, 0.0f, 0.95f, 0.24f, 0.85f, 0.38f);
    m_gridVertexCount = static_cast<unsigned int>(gridVertices.size());

    const std::array<Vertex, 3> triangle {{
        {0.0f, 0.55f, 0.0f, 0.25f, 0.55f, 1.0f, 1.0f},
        {-0.35f, -0.35f, 0.0f, 0.25f, 0.55f, 1.0f, 1.0f},
        {0.35f, -0.35f, 0.0f, 0.25f, 0.55f, 1.0f, 1.0f},
    }};

    return createBuffer(gridVertices.data(), m_gridVertexCount, m_gridBuffer)
        && createBuffer(triangle.data(), static_cast<unsigned int>(triangle.size()), m_triangleBuffer);
}

bool Dx11Renderer::createBuffer(const Vertex* vertices, unsigned int vertexCount, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
{
    D3D11_BUFFER_DESC desc {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(Vertex) * vertexCount;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data {};
    data.pSysMem = vertices;

    HRESULT result = m_device->CreateBuffer(&desc, &data, buffer.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 vertex buffer creation failed.");
        return false;
    }

    return true;
}

void Dx11Renderer::beginFrame(unsigned long long)
{
    m_debugText.clear();
    const float color[] = {
        m_config.clearColor.r,
        m_config.clearColor.g,
        m_config.clearColor.b,
        m_config.clearColor.a,
    };

    ID3D11RenderTargetView* renderTargets[] = {m_renderTargetView.Get()};
    m_context->OMSetRenderTargets(1, renderTargets, m_depthStencilView.Get());
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    if (m_depthStencilView) {
        m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    D3D11_VIEWPORT viewport {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_config.width);
    viewport.Height = static_cast<float>(m_config.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);
    m_context->RSSetState(m_debugRasterizerState.Get());
}

void Dx11Renderer::setDebugCamera(const DebugCamera& camera)
{
    m_debugCamera = camera;
}

void Dx11Renderer::drawDebugGridAndAxes()
{
    std::vector<Vertex> vertices;
    const float aspectRatio = AspectRatio(m_config);
    for (int line = -10; line <= 10; ++line) {
        const float p = static_cast<float>(line);
        AddProjectedLine(vertices, m_debugCamera, aspectRatio, {p, 0.0f, -10.0f}, {p, 0.0f, 10.0f}, {0.22f, 0.28f, 0.36f, 1.0f});
        AddProjectedLine(vertices, m_debugCamera, aspectRatio, {-10.0f, 0.0f, p}, {10.0f, 0.0f, p}, {0.22f, 0.28f, 0.36f, 1.0f});
    }

    AddProjectedLine(vertices, m_debugCamera, aspectRatio, {-10.0f, 0.02f, 0.0f}, {10.0f, 0.02f, 0.0f}, {0.95f, 0.24f, 0.24f, 1.0f});
    AddProjectedLine(vertices, m_debugCamera, aspectRatio, {0.0f, 0.02f, -10.0f}, {0.0f, 0.02f, 10.0f}, {0.24f, 0.85f, 0.38f, 1.0f});
    drawLineVertices(vertices);
}

void Dx11Renderer::drawDebugLine(Vec3 from, Vec3 to, Color color)
{
    std::vector<Vertex> vertices;
    AddProjectedLine(vertices, m_debugCamera, AspectRatio(m_config), from, to, color);
    drawLineVertices(vertices);
}

void Dx11Renderer::drawDebugSolidBox(Vec3 center, Vec3 halfExtents, Color color)
{
    const Vec3 corners[] = {
        center + Vec3 {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y,  halfExtents.z},
    };

    const int triangles[][3] = {
        {0, 1, 2}, {0, 2, 3},
        {4, 5, 6}, {4, 6, 7},
        {0, 1, 5}, {0, 5, 4},
        {1, 2, 6}, {1, 6, 5},
        {2, 3, 7}, {2, 7, 6},
        {3, 0, 4}, {3, 4, 7},
    };

    std::vector<Vertex> vertices;
    for (const auto& triangle : triangles) {
        AddWorldTriangle(vertices,
            corners[triangle[0]],
            corners[triangle[1]],
            corners[triangle[2]],
            color);
    }
    drawTriangleVertices(vertices);
}

void Dx11Renderer::drawDebugFlatTriangles(std::span<const Vec3> triangleVertices, Color color)
{
    if (triangleVertices.size() < 3) {
        return;
    }

    std::vector<Vertex> vertices;
    for (std::size_t index = 0; index + 2 < triangleVertices.size(); index += 3) {
        AddWorldTriangle(vertices,
            triangleVertices[index],
            triangleVertices[index + 1],
            triangleVertices[index + 2],
            color);
    }
    drawTriangleVertices(vertices);
}

void Dx11Renderer::drawDebugBox(Vec3 center, Vec3 halfExtents, Color color)
{
    const Vec3 corners[] = {
        center + Vec3 {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y, -halfExtents.z},
        center + Vec3 { halfExtents.x,  halfExtents.y,  halfExtents.z},
        center + Vec3 {-halfExtents.x,  halfExtents.y,  halfExtents.z},
    };

    std::vector<Vertex> vertices;
    for (const auto& edge : BoxEdgeIndices) {
        AddProjectedLine(vertices, m_debugCamera, AspectRatio(m_config), corners[edge[0]], corners[edge[1]], color);
    }
    drawLineVertices(vertices);
}

void Dx11Renderer::drawDebugText(std::string_view text)
{
    m_debugText = std::string(text);
}

bool Dx11Renderer::captureFrame(const std::filesystem::path& outputPath)
{
    if (!m_device || !m_context || !m_swapChain) {
        Logger::error("DX11 frame capture requested before the renderer was ready.");
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(result) || !backBuffer) {
        Logger::error("DX11 frame capture failed while reading the swap-chain back buffer.");
        return false;
    }

    D3D11_TEXTURE2D_DESC sourceDesc {};
    backBuffer->GetDesc(&sourceDesc);
    const bool sourceIsRgba = sourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM
        || sourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    const bool sourceIsBgra = sourceDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM
        || sourceDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    if (!sourceIsRgba && !sourceIsBgra) {
        Logger::error("DX11 frame capture only supports 32-bit RGBA/BGRA back buffers.");
        return false;
    }

    D3D11_TEXTURE2D_DESC stagingDesc = sourceDesc;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;
    stagingDesc.Usage = D3D11_USAGE_STAGING;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
    result = m_device->CreateTexture2D(&stagingDesc, nullptr, stagingTexture.GetAddressOf());
    if (FAILED(result) || !stagingTexture) {
        Logger::error("DX11 frame capture failed while creating a staging texture.");
        return false;
    }

    m_context->CopyResource(stagingTexture.Get(), backBuffer.Get());

    D3D11_MAPPED_SUBRESOURCE mapped {};
    result = m_context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(result)) {
        Logger::error("DX11 frame capture failed while mapping the staging texture.");
        return false;
    }

    const auto width = static_cast<int>(sourceDesc.Width);
    const auto height = static_cast<int>(sourceDesc.Height);
    std::vector<std::uint8_t> bgraPixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U);
    const auto* sourcePixels = static_cast<const std::uint8_t*>(mapped.pData);
    for (int y = 0; y < height; ++y) {
        const std::uint8_t* sourceRow = sourcePixels + static_cast<std::size_t>(y) * mapped.RowPitch;
        std::uint8_t* destinationRow = bgraPixels.data() + static_cast<std::size_t>(y) * static_cast<std::size_t>(width) * 4U;
        for (int x = 0; x < width; ++x) {
            const std::uint8_t* sourcePixel = sourceRow + static_cast<std::size_t>(x) * 4U;
            std::uint8_t* destinationPixel = destinationRow + static_cast<std::size_t>(x) * 4U;
            if (sourceIsBgra) {
                destinationPixel[0] = sourcePixel[0];
                destinationPixel[1] = sourcePixel[1];
                destinationPixel[2] = sourcePixel[2];
                destinationPixel[3] = sourcePixel[3];
            } else {
                destinationPixel[0] = sourcePixel[2];
                destinationPixel[1] = sourcePixel[1];
                destinationPixel[2] = sourcePixel[0];
                destinationPixel[3] = sourcePixel[3];
            }
        }
    }

    m_context->Unmap(stagingTexture.Get(), 0);

    if (!WriteBgraBmp(outputPath, width, height, bgraPixels)) {
        Logger::error("DX11 frame capture failed while writing BMP output.");
        return false;
    }

    return true;
}

bool Dx11Renderer::ensureDynamicBuffer(unsigned int vertexCount)
{
    if (vertexCount <= m_dynamicBufferCapacity) {
        return true;
    }

    m_dynamicVertexBuffer.Reset();
    m_dynamicBufferCapacity = 0;

    D3D11_BUFFER_DESC desc {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(Vertex) * vertexCount;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT result = m_device->CreateBuffer(&desc, nullptr, m_dynamicVertexBuffer.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 dynamic vertex buffer creation failed.");
        return false;
    }

    m_dynamicBufferCapacity = vertexCount;
    return true;
}

bool Dx11Renderer::updateWorldMatrixConstants()
{
    DebugWorldToClipMatrix matrix;
    if (!BuildDebugWorldToClipMatrix(m_debugCamera, AspectRatio(m_config), matrix)) {
        return false;
    }

    m_context->UpdateSubresource(m_matrixConstantBuffer.Get(), 0, nullptr, matrix.values.data(), 0, 0);
    return true;
}

void Dx11Renderer::drawLineVertices(const std::vector<Vertex>& vertices)
{
    if (vertices.empty()) {
        return;
    }

    const unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
    if (!ensureDynamicBuffer(vertexCount)) {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped {};
    HRESULT result = m_context->Map(m_dynamicVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(result)) {
        return;
    }
    std::memcpy(mapped.pData, vertices.data(), sizeof(Vertex) * vertexCount);
    m_context->Unmap(m_dynamicVertexBuffer.Get(), 0);

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;
    ID3D11Buffer* vertexBuffer = m_dynamicVertexBuffer.Get();
    m_context->OMSetDepthStencilState(m_depthDisabledState.Get(), 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->Draw(vertexCount, 0);
}

void Dx11Renderer::drawTriangleVertices(const std::vector<Vertex>& vertices)
{
    if (vertices.empty()) {
        return;
    }
    if (!updateWorldMatrixConstants()) {
        return;
    }

    const unsigned int vertexCount = static_cast<unsigned int>(vertices.size());
    if (!ensureDynamicBuffer(vertexCount)) {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped {};
    HRESULT result = m_context->Map(m_dynamicVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(result)) {
        return;
    }
    std::memcpy(mapped.pData, vertices.data(), sizeof(Vertex) * vertexCount);
    m_context->Unmap(m_dynamicVertexBuffer.Get(), 0);

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;
    ID3D11Buffer* vertexBuffer = m_dynamicVertexBuffer.Get();
    ID3D11Buffer* constantBuffer = m_matrixConstantBuffer.Get();
    m_context->OMSetDepthStencilState(m_depthEnabledState.Get(), 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->VSSetShader(m_worldVertexShader.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, &constantBuffer);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->Draw(vertexCount, 0);
}

void Dx11Renderer::endFrame()
{
    m_swapChain->Present(1, 0);
    drawDebugTextOverlay();
}

void Dx11Renderer::shutdown()
{
    m_triangleBuffer.Reset();
    m_gridBuffer.Reset();
    m_dynamicVertexBuffer.Reset();
    m_dynamicBufferCapacity = 0;
    m_matrixConstantBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_worldVertexShader.Reset();
    m_vertexShader.Reset();
    m_debugRasterizerState.Reset();
    m_depthDisabledState.Reset();
    m_depthEnabledState.Reset();
    m_depthStencilView.Reset();
    m_depthStencilBuffer.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_context.Reset();
    m_device.Reset();
}

std::string Dx11Renderer::name() const
{
    return "dx11";
}

void Dx11Renderer::drawDebugTextOverlay()
{
    if (!m_window || m_debugText.empty()) {
        return;
    }

    HDC deviceContext = GetDC(m_window);
    if (!deviceContext) {
        return;
    }

    RECT rect {};
    GetClientRect(m_window, &rect);
    RECT textRect {16, 16, rect.right - 16, rect.bottom - 16};
    SetBkMode(deviceContext, TRANSPARENT);
    SetTextColor(deviceContext, RGB(230, 235, 245));
    DrawTextA(deviceContext, m_debugText.c_str(), static_cast<int>(m_debugText.size()), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP);
    ReleaseDC(m_window, deviceContext);
}

} // namespace engine

#endif
