#pragma once

class DevResources
{
public:
    ~DevResources();

    HRESULT Init();
    void Destroy();

    ID3D11DeviceContext1* Context();
    HRESULT CreateSwapChain(HWND hWnd, IDXGISwapChain1** ppSwapChain);
    HRESULT CreateRenderTarget(ID3D11Texture2D* pTexture, ID3D11RenderTargetView** ppRenderTarget);
    HRESULT CreateBuffer(UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT accessFlags, ID3D11Buffer** ppBuffer);
    HRESULT CreateBuffer(const void* pSysMem, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage,
                        UINT accessFlags, ID3D11Buffer** ppBuffer);
    HRESULT CreateVertexShader(const void* byteCode, SIZE_T length, ID3D11VertexShader** ppVertexShader);
    HRESULT CreatePixelShader(const void* byteCode, SIZE_T length, ID3D11PixelShader** ppPixelShader);
    HRESULT CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* pieds, UINT numElements, const void* byteCode,
                              SIZE_T length, ID3D11InputLayout** ppInputLayout);
    HRESULT CreateSamplerState(D3D11_SAMPLER_DESC* sd, ID3D11SamplerState** ppSamplerState);
    HRESULT LoadTextureFromResource(HINSTANCE hInstance, INT nResourceID,
                                    ID3D11ShaderResourceView** ppTextureView);

private:
    ComPtr<ID3D11Device1> m_device;
    ComPtr<ID3D11DeviceContext1> m_context;
    ComPtr<IDXGIFactory2> m_dxgiFactory;
    ComPtr<IDXGISwapChain1> m_swapchain;
};
