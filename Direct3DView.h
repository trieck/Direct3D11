#pragma once
#include "Scene.h"

class Direct3DView : public CWindowImpl<Direct3DView>
{
public:
    DECLARE_WND_CLASS(NULL)

    BEGIN_MSG_MAP(CDirect3DView)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()

    Direct3DView() = default;

    BOOL PreTranslateMessage(MSG* /*pMsg*/);
    LRESULT OnCreate(LPCREATESTRUCT cs);
    LRESULT OnDestroy();
    LRESULT OnSize(WPARAM wParam, const CSize& sz);
    LRESULT OnTimer(UINT_PTR id);
    
private:
    Scene m_scene;
};
