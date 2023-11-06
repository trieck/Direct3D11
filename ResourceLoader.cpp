#include "stdafx.h"
#include "ResourceLoader.h"

/////////////////////////////////////////////////////////////////////////////
HRESULT ResourceLoader::Load(HINSTANCE hInstance, INT nResourceID,
                             LPCWSTR type, LPCBYTE& pdata, UINT& size)
{
    return Load(hInstance, MAKEINTRESOURCE(nResourceID), type, pdata, size);
}

/////////////////////////////////////////////////////////////////////////////
HRESULT ResourceLoader::Load(HINSTANCE hInstance, LPCWSTR resource,
                             LPCWSTR type, LPCBYTE& pdata, UINT& size)
{
    ATLASSERT(hInstance);

    pdata = nullptr;
    size = 0;

    auto hResInfo = FindResource(hInstance, resource, type);
    if (hResInfo == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    size = SizeofResource(hInstance, hResInfo);

    auto hResData = LoadResource(hInstance, hResInfo);
    if (hResData == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    pdata = static_cast<LPBYTE>(LockResource(hResData));
    if (pdata == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}
