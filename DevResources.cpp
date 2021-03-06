#include "stdafx.h"
#include "DevResources.h"
#include "ResourceLoader.h"
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

HRESULT DevResources::CreateBuffer(UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT accessFlags,
                                   ID3D11Buffer** ppBuffer)
{
    ATLASSERT(ppBuffer);
    ATLASSERT(m_device);

    *ppBuffer = nullptr;

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = byteWidth;
    bd.BindFlags = bindFlags;
    bd.CPUAccessFlags = accessFlags;
    bd.Usage = usage;

    auto hr = m_device->CreateBuffer(&bd, nullptr, ppBuffer);

    return hr;
}

HRESULT DevResources::CreateBuffer(const void* pSysMem, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage,
                                   UINT accessFlags, ID3D11Buffer** ppBuffer)
{
    ATLASSERT(pSysMem);
    ATLASSERT(ppBuffer);
    ATLASSERT(m_device);

    *ppBuffer = nullptr;

    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = byteWidth;
    bd.BindFlags = bindFlags;
    bd.CPUAccessFlags = accessFlags;
    bd.Usage = usage;

    D3D11_SUBRESOURCE_DATA srd{ pSysMem, 0, 0 };

    auto hr = m_device->CreateBuffer(&bd, &srd, ppBuffer);

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

HRESULT DevResources::LoadTextureFromResource(HINSTANCE hInstance, INT nResourceID,
                                              LPCWSTR resType, ID3D11ShaderResourceView** ppTextureView)
{
    ATLASSERT(hInstance);
    ATLASSERT(ppTextureView);
    ATLASSERT(m_device);

    *ppTextureView = nullptr;

    LPCBYTE pdata;
    UINT size;
    auto hr = ResourceLoader::Load(hInstance, nResourceID, resType, pdata, size);
    if (FAILED(hr)) {
        return hr;
    }

    hr = CreateDDSTextureFromMemory(
        m_device.Get(),
        pdata,
        size,
        nullptr,
        ppTextureView);

    return hr;
}

HRESULT DevResources::CreateDepthStencil(UINT width, UINT height, ID3D11Texture2D** ppStencilBuffer,
                                         ID3D11DepthStencilState** ppStencilState,
                                         ID3D11DepthStencilView** ppStencilView)
{
    ATLASSERT(ppStencilBuffer);
    ATLASSERT(ppStencilState);
    ATLASSERT(ppStencilView);
    ATLASSERT(m_device);

    *ppStencilBuffer = nullptr;
    *ppStencilState = nullptr;

    D3D11_TEXTURE2D_DESC dbd{};
    dbd.Width = width;
    dbd.Height = height;
    dbd.MipLevels = 1;
    dbd.ArraySize = 1;
    dbd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dbd.SampleDesc.Count = 1;
    dbd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    auto hr = m_device->CreateTexture2D(&dbd, nullptr, ppStencilBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    D3D11_DEPTH_STENCIL_DESC dsc{};
    dsc.DepthEnable = TRUE;
    dsc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsc.DepthFunc = D3D11_COMPARISON_LESS;
    dsc.StencilEnable = TRUE;
    dsc.StencilReadMask = 0xFF;
    dsc.StencilWriteMask = 0xFF;

    // stencil operations if pixel is front-facing
    dsc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // stencil operations if pixel is back-facing
    dsc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // create the depth stencil state
    hr = m_device->CreateDepthStencilState(&dsc, ppStencilState);
    if (FAILED(hr)) {
        return hr;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
    dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;

    // create the depth stencil view
    hr = m_device->CreateDepthStencilView(*ppStencilBuffer, &dsvd, ppStencilView);

    return hr;
}

HRESULT DevResources::CreateRasterizerState(ID3D11RasterizerState** ppRasterState)
{
    ATLASSERT(ppRasterState);
    ATLASSERT(m_device);

    *ppRasterState = nullptr;

    D3D11_RASTERIZER_DESC rd{};
    rd.CullMode = D3D11_CULL_BACK;
    rd.DepthClipEnable = TRUE;
    rd.FillMode = D3D11_FILL_SOLID;
    rd.SlopeScaledDepthBias = 0.0f;

    auto hr = m_device->CreateRasterizerState(&rd, ppRasterState);

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
