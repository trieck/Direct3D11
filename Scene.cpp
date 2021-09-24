#include "stdafx.h"
#include "Scene.h"
#include "Direct3DApp.h"
#include "VertexShader.shh"
#include "PixelShader.shh"
#include "resource.h"

extern Direct3DApp _Module;

struct alignas(16) MatrixBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

struct alignas(16) LightBuffer
{
    XMFLOAT4 ambientColor;
    XMFLOAT4 diffuseColor;
    XMFLOAT3 lightDirection;
    float specularPower;
    XMFLOAT4 specularColor;
};

struct alignas(16) CameraBuffer
{
    XMFLOAT3 cameraPosition;
};

Scene::~Scene()
{
    Destroy();
}

HRESULT Scene::Initialize(HWND hWnd, int width, int height)
{
    ATLASSERT(IsWindow(hWnd));

    Destroy();

    // Initialize the world matrix to the identity matrix
    m_worldMatrix = XMMatrixIdentity();

    // Set the initial position of the camera
    SetCameraPos();

    auto& devResources = _Module.devResources();

    m_context = devResources.Context();
    if (!m_context) {
        return E_FAIL;
    }

    auto hr = CreateView(hWnd, width, height);
    if (FAILED(hr)) {
        return hr;
    }
    
    hr = InitPipeline();
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

HRESULT Scene::CreateView(HWND hWnd, int width, int height)
{
    auto& devResources = _Module.devResources();

    m_renderTarget.Reset();
    m_swapChain.Reset();

    auto hr = devResources.CreateSwapChain(hWnd, m_swapChain.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    ComPtr<ID3D11Texture2D> backbuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateRenderTarget(backbuffer.Get(), m_renderTarget.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    SetView(width, height);

    return S_OK;
}

HRESULT Scene::EndScene()
{
    // flip the back buffer and front buffer
    auto hr = m_swapChain->Present(1, 0);

    return hr;
}

HRESULT Scene::Resize(int width, int height)
{
    ATLASSERT(m_swapChain);

    auto& devResources = _Module.devResources();

    m_renderTarget.Reset();

    auto hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        return hr;
    }

    ComPtr<ID3D11Texture2D> backbuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateRenderTarget(backbuffer.Get(), m_renderTarget.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    SetView(width, height);

    return S_OK;
}

HRESULT Scene::InitPipeline()
{
    auto& devResources = _Module.devResources();

    auto hr = m_model.Load(_Module.m_hInstResource, 
        IDR_MODEL, L"TEXT",
        IDR_TEXTURE, L"DDS");
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.LoadTextureFromResource(_Module.m_hInstResource, IDR_TEXTURE,
                                                   L"DDS", m_textureView.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MipLODBias = 0.0f;
    sd.MaxAnisotropy = 1;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    hr = devResources.CreateSamplerState(&sd, m_samplerState.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(sizeof(MatrixBuffer),
                                   D3D11_BIND_CONSTANT_BUFFER,
                                   D3D11_USAGE_DYNAMIC,
                                   D3D11_CPU_ACCESS_WRITE,
                                   m_matrixBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(sizeof(LightBuffer),
                                   D3D11_BIND_CONSTANT_BUFFER,
                                   D3D11_USAGE_DYNAMIC,
                                   D3D11_CPU_ACCESS_WRITE,
                                   m_lightBuffer.GetAddressOf());

    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(sizeof(CameraBuffer),
        D3D11_BIND_CONSTANT_BUFFER,
        D3D11_USAGE_DYNAMIC,
        D3D11_CPU_ACCESS_WRITE,
        m_cameraBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateVertexShader(VertexShaderBytecode,
                                         _countof(VertexShaderBytecode) * sizeof(BYTE),
                                         m_vertexShader.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreatePixelShader(PixelShaderBytecode,
                                        _countof(PixelShaderBytecode) * sizeof(BYTE),
                                        m_pixelShader.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());
    m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = devResources.CreateInputLayout(ied, _countof(ied), VertexShaderBytecode,
                                        _countof(VertexShaderBytecode) * sizeof(BYTE),
                                        m_inputLayout.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    m_context->IASetInputLayout(m_inputLayout.Get());

    return S_OK;
}

void Scene::Render()
{
    m_model.Render(m_context.Get());
}

HRESULT Scene::UpdateModel()
{
    static constexpr auto rotationSpeed = 1.0f;

    m_elapsed += m_timer.Mark();

    auto theta = fmodf(m_elapsed * rotationSpeed, XM_2PI);

    m_worldMatrix = XMMatrixRotationY(theta);

    auto cameraView = m_camera.view();

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    auto hr = m_context->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }

    auto* matrixBuffer = static_cast<MatrixBuffer*>(mappedResource.pData);
    matrixBuffer->world = XMMatrixTranspose(m_worldMatrix);
    matrixBuffer->view = XMMatrixTranspose(cameraView);
    matrixBuffer->projection = XMMatrixTranspose(m_projectionMatrix);

    m_context->Unmap(m_matrixBuffer.Get(), 0);
    m_context->VSSetConstantBuffers(0, 1, m_matrixBuffer.GetAddressOf());

    hr = m_context->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }

    auto* cameraBuffer = static_cast<CameraBuffer*>(mappedResource.pData);
    cameraBuffer->cameraPosition = m_camera.position();

    m_context->Unmap(m_cameraBuffer.Get(), 0);
    m_context->VSSetConstantBuffers(1, 1, m_cameraBuffer.GetAddressOf());

    hr = m_context->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }

    auto* lightBuffer = static_cast<LightBuffer*>(mappedResource.pData);
    lightBuffer->ambientColor = { 0.15f, 0.15f, 0.15f, 1.0f };
    lightBuffer->diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightBuffer->lightDirection = { 0.0f, 0.0f, 1.0f };
    lightBuffer->specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightBuffer->specularPower = 32.0f;

    m_context->Unmap(m_lightBuffer.Get(), 0);
    m_context->PSSetConstantBuffers(0, 1, m_lightBuffer.GetAddressOf());

    return S_OK;
}

void Scene::SetCameraPos()
{
    m_camera.SetPosition({ 0.0f, 0.0f, m_zoom });
}

void Scene::SetView(int width, int height)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);

    m_context->RSSetViewports(1, &viewport);

    if (height != 0) {
        auto fieldOfView = XM_PI / 4.0f;
        auto aspectRatio = viewport.Width / viewport.Height;

        // Create the projection matrix for 3D rendering
        static constexpr float nearZ = 0.1f;
        static constexpr float farZ = 1000.0f;
        m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearZ, farZ);

        // Create an orthographic projection matrix for 2D rendering
        m_orthoMatrix = XMMatrixOrthographicLH(viewport.Width, viewport.Height, nearZ, farZ);
    }
}

HRESULT Scene::RenderFrame()
{
    BeginScene();

    auto hr = UpdateModel();
    if (FAILED(hr)) {
        return hr;
    }

    Render();

    hr = EndScene();

    return hr;
}

void Scene::Zoom(int step)
{
    m_zoom += static_cast<float>(step);

    SetCameraPos();
}

void Scene::Destroy()
{
    m_samplerState.Reset();
    m_textureView.Reset();
    m_inputLayout.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_cameraBuffer.Reset();
    m_lightBuffer.Reset();
    m_matrixBuffer.Reset();
    m_renderTarget.Reset();
    m_swapChain.Reset();
    m_context.Reset();
}

void Scene::BeginScene()
{
    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr);

    // set the back buffer to deep blue
    m_context->ClearRenderTargetView(m_renderTarget.Get(), Colors::DarkSlateBlue);
}
