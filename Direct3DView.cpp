#include "stdafx.h"
#include "Direct3DView.h"
#include "Direct3DApp.h"

extern Direct3DApp _Module;

static constexpr UINT_PTR TIMER_ID = 101;

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

    m_scene.RenderFrame();

    SetTimer(TIMER_ID, 40);

    return 0;
}

LRESULT Direct3DView::OnDestroy()
{
    KillTimer(TIMER_ID);

    return 0;
}

LRESULT Direct3DView::OnSize(WPARAM /*wParam*/, const CSize& sz)
{
    m_scene.Resize(sz.cx, sz.cy);
    
    return 0;
}

LRESULT Direct3DView::OnTimer(UINT_PTR id)
{
    if (id == TIMER_ID) {
        m_scene.RenderFrame();
    }

    return 0;
}
