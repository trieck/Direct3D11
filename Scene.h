#pragma once

class Scene
{
public:
    Scene() = default;
    ~Scene();

    HRESULT Initialize(HWND hWnd, int cx, int cy);
    HRESULT Resize(int cx, int cy);
    void Render();
    
private:
    void Destroy();
    HRESULT CreateView(HWND hWnd, int cx, int cy);
    HRESULT InitPipeline();
    void SetViewport(int cx, int cy);

    ComPtr<ID3D11DeviceContext1> m_context;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTarget;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_textureView;
    ComPtr<ID3D11SamplerState> m_samplerState;
};

