#pragma once

#include "engine/renderer/Renderer.h"

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d11.h>
#include <wrl/client.h>

#include <string>
#include <vector>

namespace engine {

class Dx11Renderer final : public IRenderer {
public:
    bool initialize(const RendererConfig& config) override;
    void beginFrame(unsigned long long frameIndex) override;
    void setDebugCamera(const DebugCamera& camera) override;
    void drawDebugGridAndAxes() override;
    void drawDebugLine(Vec3 from, Vec3 to, Color color) override;
    void drawDebugSolidBox(Vec3 center, Vec3 halfExtents, Color color) override;
    void drawDebugFlatTriangles(std::span<const Vec3> triangleVertices, Color color) override;
    void drawDebugBox(Vec3 center, Vec3 halfExtents, Color color) override;
    void drawDebugText(std::string_view text) override;
    bool captureFrame(const std::filesystem::path& outputPath) override;
    void endFrame() override;
    void shutdown() override;
    bool isFramePaced() const override { return true; }
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
    bool createDepthResources();
    bool createPipelineStates();
    bool createShaders();
    bool createDebugGeometry();
    bool createBuffer(const Vertex* vertices, unsigned int vertexCount, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
    bool ensureDynamicBuffer(unsigned int vertexCount);
    bool updateWorldMatrixConstants();
    void drawLineVertices(const std::vector<Vertex>& vertices);
    void drawTriangleVertices(const std::vector<Vertex>& vertices);
    void drawDebugTextOverlay();

    RendererConfig m_config;
    DebugCamera m_debugCamera;
    std::string m_debugText;
    HWND m_window = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthEnabledState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_debugRasterizerState;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_worldVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_gridBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_triangleBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_dynamicVertexBuffer;
    unsigned int m_gridVertexCount = 0;
    unsigned int m_dynamicBufferCapacity = 0;
};

} // namespace engine

#endif
