#include "stdafx.h"
#include "Direct3DView.h"
#include "Direct3DApp.h"

extern Direct3DApp _Module;

BOOL Direct3DView::PreTranslateMessage(MSG*)
{
    return FALSE;
}

LRESULT Direct3DView::OnCreate(LPCREATESTRUCT cs)
{
    auto hr = m_scene.Initialize(m_hWnd, cs->cx, cs->cy);
    if (FAILED(hr)) {
        return -1;
    }

    return 0;
}

LRESULT Direct3DView::OnDestroy()
{
    return 0;
}

LRESULT Direct3DView::OnSize(WPARAM /*wParam*/, const CSize& sz)
{
    m_scene.Resize(sz.cx, sz.cy);
    
    return 0;
}

void Direct3DView::OnPaint(CDCHandle)
{
    CPaintDC dc(*this);

    m_scene.Render();

    ValidateRect(&dc.m_ps.rcPaint);
}
