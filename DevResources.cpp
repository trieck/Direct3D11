#include "stdafx.h"
#include "DevResources.h"
#include "TextureLoader.h"
#include <DDSTextureLoader.h>

DevResources::~DevResources()
{
    Destroy();
}

void DevResources::Destroy()
{
    m_dxgiFactory.Reset();
    m_context.Reset();
    m_device.Reset();
}

ID3D11DeviceContext1* DevResources::Context()
{
    return m_context.Get();
}

HRESULT DevResources::Init()
{
    // Create the device and device context objects
    auto hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        reinterpret_cast<ID3D11Device**>(m_device.GetAddressOf()),
        nullptr,
        reinterpret_cast<ID3D11DeviceContext**>(m_context.GetAddressOf()));
    if (FAILED(hr)) {
        return hr;
    }

    ComPtr<IDXGIDevice1> dxgiDevice;
    hr = m_device.As(&dxgiDevice);
    if (FAILED(hr)) {
        return hr;
    }

    ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr)) {
        return hr;
    }

    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &m_dxgiFactory);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT DevResources::CreateSwapChain(HWND hWnd, IDXGISwapChain1** ppSwapChain)
{
    ATLASSERT(IsWindow(hWnd));
    ATLASSERT(ppSwapChain);
    ATLASSERT(m_dxgiFactory);

    *ppSwapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 scd{};
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scd.SampleDesc.Count = 1;

    auto hr = m_dxgiFactory->CreateSwapChainForHwnd(
        m_device.Get(),
        hWnd,
        &scd,
        nullptr,
        nullptr,
        ppSwapChain);

    return hr;
}

HRESULT DevResources::CreateRenderTarget(ID3D11Texture2D* pTexture, ID3D11RenderTargetView** ppRenderTarget)
{
    ATLASSERT(ppRenderTarget);
    ATLASSERT(m_device);

    *ppRenderTarget = nullptr;

    auto hr = m_device->CreateRenderTargetView(pTexture, nullptr, ppRenderTarget);

    return hr;
}

HRESULT DevResources::CreateBuffer(UINT byteWidth, UINT bindFlags, ID3D11Buffer** ppBuffer)
{
    ATLASSERT(ppBuffer);
    ATLASSERT(m_device);

    *ppBuffer = nullptr;

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = byteWidth;
    bd.BindFlags = bindFlags;

    auto hr = m_device->CreateBuffer(&bd, nullptr, ppBuffer);

    return hr;
}

HRESULT DevResources::CreateBuffer(const void* pSysMem, UINT byteWidth, UINT bindFlags, ID3D11Buffer** ppBuffer)
{
    ATLASSERT(pSysMem);
    ATLASSERT(ppBuffer);
    ATLASSERT(m_device);

    *ppBuffer = nullptr;

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = byteWidth;
    bd.BindFlags = bindFlags;

    D3D11_SUBRESOURCE_DATA srd{ pSysMem, 0, 0 };

    auto hr = m_device->CreateBuffer(&bd, &srd, ppBuffer);

    return hr;
}

HRESULT DevResources::CreateShaderResourceView(ID3D11Resource* pResource, DXGI_FORMAT format,
                                               D3D11_SRV_DIMENSION viewDimension,
                                               ID3D11ShaderResourceView** ppShaderResourceView)
{
    ATLASSERT(pResource);
    ATLASSERT(ppShaderResourceView);
    ATLASSERT(m_device);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = format;
    srvDesc.ViewDimension = viewDimension;
    srvDesc.Texture2D.MipLevels = 1;

    auto hr = m_device->CreateShaderResourceView(pResource, &srvDesc, ppShaderResourceView);

    return hr;
}

HRESULT DevResources::CreateSamplerState(D3D11_SAMPLER_DESC* sd, ID3D11SamplerState** ppSamplerState)
{
    ATLASSERT(sd);
    ATLASSERT(ppSamplerState);
    ATLASSERT(m_device);

    auto hr = m_device->CreateSamplerState(sd, ppSamplerState);

    return hr;
}

HRESULT DevResources::LoadTextureFromResource(HINSTANCE hInstance, INT nResourceID, ID3D11Resource** ppTexture,
    ID3D11ShaderResourceView** ppTextureView)
{
    ATLASSERT(hInstance);
    ATLASSERT(ppTexture);
    ATLASSERT(ppTextureView);
    ATLASSERT(m_device);

    *ppTexture = nullptr;
    *ppTextureView = nullptr;

    TextureLoader loader;
    auto hr = loader.Load(hInstance, nResourceID);
    if (FAILED(hr)) {
        return hr;
    }

    hr = CreateDDSTextureFromMemory(
        m_device.Get(),
        loader.dds(),
        loader.size(),
        ppTexture,
        ppTextureView);

    return hr;
}

HRESULT DevResources::CreateVertexShader(const void* byteCode, SIZE_T length, ID3D11VertexShader** ppVertexShader)
{
    ATLASSERT(byteCode);
    ATLASSERT(ppVertexShader);
    ATLASSERT(m_device);

    *ppVertexShader = nullptr;

    auto hr = m_device->CreateVertexShader(byteCode, length, nullptr, ppVertexShader);

    return hr;
}

HRESULT DevResources::CreatePixelShader(const void* byteCode, SIZE_T length, ID3D11PixelShader** ppPixelShader)
{
    ATLASSERT(byteCode);
    ATLASSERT(ppPixelShader);
    ATLASSERT(m_device);

    *ppPixelShader = nullptr;

    auto hr = m_device->CreatePixelShader(byteCode, length, nullptr, ppPixelShader);

    return hr;
}

HRESULT DevResources::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* pieds, UINT numElements, const void* byteCode,
                                        SIZE_T length, ID3D11InputLayout** ppInputLayout)
{
    ATLASSERT(pieds);
    ATLASSERT(m_device);

    *ppInputLayout = nullptr;

    auto hr = m_device->CreateInputLayout(pieds, numElements, byteCode, length, ppInputLayout);

    return hr;
}
