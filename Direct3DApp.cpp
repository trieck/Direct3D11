#include "stdafx.h"
#include "Direct3DApp.h"
#include "MainFrame.h"

Direct3DApp _Module;

int Run(LPTSTR /*lpstrCmdLine*/  = nullptr, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);


    MainFrame wndMain;
    if (wndMain.CreateEx() == nullptr) {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    wndMain.ShowWindow(nCmdShow);

    const auto nRet = theLoop.Run();

    _Module.RemoveMessageLoop();

    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
    if (!XMVerifyCPUSupport()) {
        return 1;
    }

    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize)) {
        return 1;
    }

    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES); // add flags to support other controls

    auto hr = _Module.Init(nullptr, hInstance);
    if (FAILED(hr)) {
        return -1;
    }

    const auto nRet = Run(lpstrCmdLine, nCmdShow);

    _Module.Term();

    return nRet;
}

HRESULT Direct3DApp::Init(_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID)
{
    auto hr = CAppModule::Init(pObjMap, hInstance, pLibID);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    hr = m_devResources.Init();
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

void Direct3DApp::Term()
{
    m_devResources.Destroy();

    CAppModule::Term();
}

DevResources& Direct3DApp::devResources()
{
    return m_devResources;
}
