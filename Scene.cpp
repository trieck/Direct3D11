#include "stdafx.h"
#include "Scene.h"
#include "Direct3DApp.h"
#include "VertexShader.shh"
#include "PixelShader.shh"
#include "resource.h"

extern Direct3DApp _Module;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 texture;
    XMFLOAT3 normal;
};

struct alignas(16) MatrixBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
};

struct alignas(16) LightBuffer
{
    XMFLOAT4 diffuseColor;
    XMFLOAT3 lightDirection;
};

static constexpr Vertex vertices[] = {
    { { 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

    { { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }
};

static constexpr ULONG indices[] = {
    0, 1, 2, 3, 4, 5
};

static float wrapAngle(float theta)
{
    theta = fmodf(theta, XM_2PI);

    if (theta > XM_PI) {
        theta = theta - XM_2PI;
    }

    return theta;
}

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
    m_camera.SetPosition({ 0.0f, 0.0f, -2.0f });

    auto& devResources = _Module.devResources();

    m_context = devResources.Context();
    if (!m_context) {
        return E_FAIL;
    }

    auto hr = CreateView(hWnd, width, height);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(vertices, _countof(vertices) * sizeof(Vertex),
                                   D3D11_BIND_VERTEX_BUFFER,
                                   D3D11_USAGE_DEFAULT, 0, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(indices, _countof(indices) * sizeof(ULONG),
                                   D3D11_BIND_INDEX_BUFFER,
                                   D3D11_USAGE_DEFAULT, 0, m_indexBuffer.GetAddressOf());
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

    auto hr = devResources.LoadTextureFromResource(_Module.m_hInstResource, IDR_TEXTURE,
                                                   m_textureView.GetAddressOf());
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

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    hr = m_context->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }

    auto* lightBuffer = static_cast<LightBuffer*>(mappedResource.pData);
    lightBuffer->diffuseColor = { 1.0f, 0.5f, 0.75f, 1.0f };
    lightBuffer->lightDirection = { 0.0f, 0.0f, 1.0f };

    m_context->Unmap(m_lightBuffer.Get(), 0);
    m_context->PSSetConstantBuffers(0, 1, m_lightBuffer.GetAddressOf());

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
    // set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // set the index buffer
    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // set the primitive topology
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw vertices, starting from vertex 0
    m_context->DrawIndexed(_countof(indices), 0, 0);
}

HRESULT Scene::UpdateModel()
{
    static constexpr auto rotationSpeed = XM_PI / 4.0f;

    m_elapsed += m_timer.Mark();

    auto theta = wrapAngle(m_elapsed * rotationSpeed);

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

    return S_OK;
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

void Scene::Destroy()
{
    m_samplerState.Reset();
    m_textureView.Reset();
    m_inputLayout.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_lightBuffer.Reset();
    m_matrixBuffer.Reset();
    m_indexBuffer.Reset();
    m_vertexBuffer.Reset();
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
