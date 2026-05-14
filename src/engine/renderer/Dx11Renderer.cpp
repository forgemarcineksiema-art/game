#include "engine/renderer/Dx11Renderer.h"

#if defined(_WIN32)

#include "engine/core/Logger.h"

#include <d3dcompiler.h>

#include <array>
#include <cstring>
#include <iterator>
#include <vector>

namespace engine {
namespace {

const char* VertexShaderSource = R"(
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

} // namespace

bool Dx11Renderer::initialize(const RendererConfig& config)
{
    m_config = config;
    m_window = static_cast<HWND>(config.nativeWindow);
    if (!m_window) {
        Logger::error("DX11 renderer requires a Win32 window handle.");
        return false;
    }

    if (!createDeviceAndSwapChain() || !createRenderTarget() || !createShaders() || !createDebugGeometry()) {
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

bool Dx11Renderer::createShaders()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT result = D3DCompile(
        VertexShaderSource,
        strlen(VertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_4_0",
        0,
        0,
        vertexBlob.GetAddressOf(),
        errors.GetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 vertex shader compilation failed.");
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

    result = m_device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(result)) {
        Logger::error("DX11 vertex shader creation failed.");
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
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf());

    if (FAILED(result)) {
        Logger::error("DX11 input layout creation failed.");
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
    const float color[] = {
        m_config.clearColor.r,
        m_config.clearColor.g,
        m_config.clearColor.b,
        m_config.clearColor.a,
    };

    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);

    D3D11_VIEWPORT viewport {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_config.width);
    viewport.Height = static_cast<float>(m_config.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &viewport);
}

void Dx11Renderer::drawDebugGridAndAxes()
{
    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;

    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    ID3D11Buffer* gridBuffer = m_gridBuffer.Get();
    m_context->IASetVertexBuffers(0, 1, &gridBuffer, &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_context->Draw(m_gridVertexCount, 0);

    ID3D11Buffer* triangleBuffer = m_triangleBuffer.Get();
    m_context->IASetVertexBuffers(0, 1, &triangleBuffer, &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->Draw(3, 0);
}

void Dx11Renderer::endFrame()
{
    m_swapChain->Present(1, 0);
}

void Dx11Renderer::shutdown()
{
    m_triangleBuffer.Reset();
    m_gridBuffer.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_vertexShader.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_context.Reset();
    m_device.Reset();
}

std::string Dx11Renderer::name() const
{
    return "dx11";
}

} // namespace engine

#endif
