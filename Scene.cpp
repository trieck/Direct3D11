#include "stdafx.h"
#include "Scene.h"
#include "Direct3DApp.h"
#include "VertexShader.shh"
#include "PixelShader.shh"

extern Direct3DApp _Module;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

static constexpr Vertex vertices[] = {
    { { 0.5f, 0.5f, 0.0f }, { 0.5f, 0.0f, 0.5f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.0f, 0.5f, 1.0f } },
    { { -0.5f, 0.5f, 0.0f }, { 0.75f, 0.0f, 0.5f, 1.0f } },

    { { -0.5f, 0.5f, 0.0f }, { 0.75f, 0.0f, 0.5f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.5f, 0.0f, 0.5f, 1.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.5f, 0.0f, 0.5f, 1.0f } },
    
};

Scene::~Scene()
{
    Destroy();
}

HRESULT Scene::Initialize(HWND hWnd, int cx, int cy)
{
    ATLASSERT(IsWindow(hWnd));

    Destroy();

    auto& devResources = _Module.devResources();

    m_context = devResources.Context();
    if (!m_context) {
        return E_FAIL;
    }

    auto hr = CreateView(hWnd, cx, cy);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateVertexBuffer(vertices, _countof(vertices) * sizeof(Vertex), &m_vertexBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    hr = InitPipeline();
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

HRESULT Scene::CreateView(HWND hWnd, int cx, int cy)
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

    hr = devResources.CreateRenderTarget(backbuffer.Get(), &m_renderTarget);
    if (FAILED(hr)) {
        return hr;
    }

    SetViewport(cx, cy);

    return S_OK;
}

HRESULT Scene::Resize(int cx, int cy)
{
    ATLASSERT(m_swapChain);

    auto& devResources = _Module.devResources();

    m_renderTarget.Reset();

    auto hr = m_swapChain->ResizeBuffers(2, cx, cy, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        return hr;
    }

    ComPtr<ID3D11Texture2D> backbuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateRenderTarget(backbuffer.Get(), &m_renderTarget);
    if (FAILED(hr)) {
        return hr;
    }

    SetViewport(cx, cy);
    
    return S_OK;
}

HRESULT Scene::InitPipeline()
{
    auto& devResources = _Module.devResources();

    auto hr = devResources.CreateVertexShader(VertexShaderBytecode,
                                              _countof(VertexShaderBytecode) * sizeof(BYTE),
                                              &m_vertexShader);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreatePixelShader(PixelShaderBytecode,
                                        _countof(PixelShaderBytecode) * sizeof(BYTE),
                                        &m_pixelShader);
    if (FAILED(hr)) {
        return hr;
    }

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = devResources.CreateInputLayout(ied, _countof(ied), VertexShaderBytecode,
                                        _countof(VertexShaderBytecode) * sizeof(BYTE), &m_inputLayout);
    if (FAILED(hr)) {
        return hr;
    }

    m_context->IASetInputLayout(m_inputLayout.Get());

    return S_OK;
}

void Scene::SetViewport(int cx, int cy)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(cx);
    viewport.Height = static_cast<float>(cy);

    m_context->RSSetViewports(1, &viewport);
}

void Scene::Render()
{
    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr);

    // set the back buffer to deep blue
    float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_context->ClearRenderTargetView(m_renderTarget.Get(), color);

    // set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // set the primitive topology
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw vertices, starting from vertex 0
    m_context->Draw(_countof(vertices), 0);

    // flip the back buffer and front buffer
    m_swapChain->Present(1, 0);
}

void Scene::Destroy()
{
    m_inputLayout.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_vertexBuffer.Reset();
    m_renderTarget.Reset();
    m_swapChain.Reset();
    m_context.Reset();
}
