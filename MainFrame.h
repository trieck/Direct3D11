#pragma once
#include "Direct3DView.h"
#include "resource.h"

using CMainFrameTraits = CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW |
                                    WS_EX_WINDOWEDGE>;


class MainFrame;
using CMainFrameImpl = CFrameWindowImpl<MainFrame, CWindow, CMainFrameTraits>;

class MainFrame :
    public CMainFrameImpl,
    public CUpdateUI<MainFrame>,
    public CMessageFilter, public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

    BEGIN_UPDATE_UI_MAP(MainFrame)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(MainFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
        COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
        COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
        CHAIN_MSG_MAP(CMainFrameImpl)
    END_MSG_MAP()

    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy();

    BOOL PreTranslateMessage(MSG* pMsg) override;
    LRESULT OnCreate(LPCREATESTRUCT pcs);
    BOOL OnIdle() override;

private:
    void ResizeFrame();

    CRect m_rcClient{ 0, 0, 600, 600 };
    Direct3DView m_view;
    CCommandBarCtrl m_CmdBar;
};
