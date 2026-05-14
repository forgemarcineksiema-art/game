#pragma once

#include "engine/renderer/Renderer.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d11.h>
#include <wrl/client.h>

namespace engine {

class Dx11Renderer final : public IRenderer {
public:
    bool initialize(const RendererConfig& config) override;
    void beginFrame(unsigned long long frameIndex) override;
    void drawDebugGridAndAxes() override;
    void endFrame() override;
    void shutdown() override;
    std::string name() const override;

    struct Vertex {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
        float a;
    };

private:
    bool createDeviceAndSwapChain();
    bool createRenderTarget();
    bool createShaders();
    bool createDebugGeometry();
    bool createBuffer(const Vertex* vertices, unsigned int vertexCount, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);

    RendererConfig m_config;
    HWND m_window = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_gridBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_triangleBuffer;
    unsigned int m_gridVertexCount = 0;
};

} // namespace engine

#endif
