#pragma once

class Direct3DView : public CWindowImpl<Direct3DView>
{
public:
    DECLARE_WND_CLASS(NULL)

    BEGIN_MSG_MAP(CDirect3DView)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
    END_MSG_MAP()

    BOOL PreTranslateMessage(MSG* /*pMsg*/);
    LRESULT OnCreate(LPCREATESTRUCT cs);
    LRESULT OnDestroy();
    LRESULT OnSize(WPARAM wParam, const CSize& sz);
    void OnPaint(CDCHandle /*hDC*/);

private:
    void SetViewport(int cx, int cy);
    HRESULT Initialize(int cx, int cy);
    HRESULT InitPipeline();
    void Render();

    ComPtr<ID3D11DeviceContext1> m_context;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTarget;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
};
