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
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

};

static constexpr ULONG indices[] = {
    0, 1, 2, 3, 4, 5
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

    hr = devResources.CreateRenderTarget(backbuffer.Get(), m_renderTarget.GetAddressOf());
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

    hr = devResources.CreateRenderTarget(backbuffer.Get(), m_renderTarget.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    SetViewport(cx, cy);

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
        return false;
    }

    hr = devResources.CreateBuffer(sizeof(LightBuffer),
                                   D3D11_BIND_CONSTANT_BUFFER,
                                   D3D11_USAGE_DYNAMIC,
                                   D3D11_CPU_ACCESS_WRITE,
                                   m_lightBuffer.GetAddressOf());

    if (FAILED(hr)) {
        return hr;
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = m_context->Map(m_lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }

    auto* buffer = static_cast<LightBuffer*>(mappedResource.pData);
    buffer->diffuseColor = { 1.0f, 0.8f, 0.8f, 1.0f };
    buffer->lightDirection = { 0.0f, 0.0f, 1.0f };

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

void Scene::SetViewport(int cx, int cy)
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(cx);
    viewport.Height = static_cast<float>(cy);

    m_context->RSSetViewports(1, &viewport);
}

HRESULT Scene::Render()
{
    m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr);

    // set the back buffer to deep blue
    m_context->ClearRenderTargetView(m_renderTarget.Get(), Colors::DarkSlateBlue);

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

    // flip the back buffer and front buffer
    auto hr = m_swapChain->Present(1, 0);

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
    m_indexBuffer.Reset();
    m_vertexBuffer.Reset();
    m_renderTarget.Reset();
    m_swapChain.Reset();
    m_context.Reset();
}
