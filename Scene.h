#pragma once
#include "Camera.h"
#include "FrameTimer.h"
#include "Model.h"

class Scene
{
public:
    Scene() = default;
    ~Scene();

    HRESULT Initialize(HWND hWnd, int width, int height);
    HRESULT Resize(int width, int height);
    HRESULT RenderFrame();
    void Zoom(int step);

private:
    void Destroy();
    void BeginScene();
    HRESULT CreateView(HWND hWnd, int width, int height);
    HRESULT EndScene();
    HRESULT InitPipeline();
    void Render();
    HRESULT UpdateModel();
    void SetCameraPos();
    void SetView(int width, int height);

    ComPtr<ID3D11DeviceContext1> m_context;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTarget;
    ComPtr<ID3D11Buffer> m_matrixBuffer;
    ComPtr<ID3D11Buffer> m_lightBuffer;
    ComPtr<ID3D11Buffer> m_cameraBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11ShaderResourceView> m_textureView;
    ComPtr<ID3D11SamplerState> m_samplerState;

    XMMATRIX m_projectionMatrix{};
    XMMATRIX m_worldMatrix{};
    XMMATRIX m_orthoMatrix{};

    Model m_model;
    Camera m_camera;
    FrameTimer m_timer;
    float m_elapsed = 0.0f;
    float m_zoom = -6.0f;
};

