#include "stdafx.h"
#include "MainFrame.h"
#include "AboutDlg.h"
#include "Direct3DApp.h"

extern Direct3DApp _Module;

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
    if (CMainFrameImpl::PreTranslateMessage(pMsg))
        return TRUE;

    return m_view.PreTranslateMessage(pMsg);
}

void MainFrame::ResizeFrame()
{
    CRect rc(m_rcClient);

    // make room for the status bar
    CRect rcStatus;
    const CStatusBarCtrl wndStatus(m_hWndStatusBar);
    wndStatus.GetWindowRect(rcStatus);
    rcStatus.OffsetRect(-rcStatus.left, -rcStatus.top);

    // make room for the toolbar
    CRect rcToolbar;
    const CReBarCtrl rebar = m_hWndToolBar;
    rebar.GetWindowRect(rcToolbar);
    rcToolbar.OffsetRect(-rcToolbar.left, -rcToolbar.top);

    // toolbar is 2nd added band
    const auto nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);

    REBARBANDINFO rbbi = {};
    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_STYLE;
    rebar.GetBandInfo(nBandIndex, &rbbi);

    if (wndStatus.IsWindowVisible())
        rc.bottom += rcStatus.Height();

    if ((rbbi.fStyle & RBBS_HIDDEN) == 0)
        rc.bottom += rcToolbar.Height();

    const auto style = GetStyle();
    const auto dwExStyle = GetExStyle() | WS_EX_CLIENTEDGE;
    AdjustWindowRectEx(&rc, style, TRUE, dwExStyle);

    SetWindowPos(nullptr, 0, 0, rc.Width(), rc.Height(),
        SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOZORDER);
}

LRESULT MainFrame::OnCreate(LPCREATESTRUCT pcs)
{
    // create command bar window
    const auto hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);

    m_CmdBar.AttachMenu(GetMenu());
    m_CmdBar.LoadImages(IDR_MAINFRAME);
    
    SetMenu(nullptr);

    const auto hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

    CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
    AddSimpleReBarBand(hWndCmdBar);
    AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);

    CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE |
        WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

    UIAddToolBar(hWndToolBar);
    UISetCheck(ID_VIEW_TOOLBAR, 1);
    UISetCheck(ID_VIEW_STATUS_BAR, 1);

    m_hWndClient = m_view.Create(m_hWnd, m_rcClient, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    if (m_hWndClient == nullptr) {
        return -1;
    }

    // register object for message filtering and idle updates
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    ResizeFrame();
    CenterWindow();

    return 0;
}

BOOL MainFrame::OnIdle()
{
    UIUpdateToolBar();
    return FALSE;
}

LRESULT MainFrame::OnDestroy()
{
    // unregister message filtering and idle updates
    auto* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    SetMsgHandled(FALSE);

    return 1;
}

LRESULT MainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT MainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT MainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    static BOOL bVisible = TRUE; // initially visible
    bVisible = !bVisible;
    CReBarCtrl rebar = m_hWndToolBar;
    const auto nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1); // toolbar is 2nd added band
    rebar.ShowBand(nBandIndex, bVisible);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();
    ResizeFrame();
    return 0;
}

LRESULT MainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    const auto bVisible = !::IsWindowVisible(m_hWndStatusBar);
    ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
    UpdateLayout();
    ResizeFrame();
    return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    AboutDlg dlg;
    dlg.DoModal(*this);
    return 0;
}