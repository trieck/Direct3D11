#include "stdafx.h"
#include "ResourceLoader.h"

/////////////////////////////////////////////////////////////////////////////
HRESULT ResourceLoader::Load(HINSTANCE hInstance, INT nResourceID, LPCWSTR type)
{
    ATLASSERT(hInstance);

    m_pData = nullptr;
    m_size = 0;

    auto hResInfo = FindResource(hInstance, MAKEINTRESOURCE(nResourceID), type);
    if (hResInfo == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_size = SizeofResource(hInstance, hResInfo);

    auto hResData = LoadResource(hInstance, hResInfo);
    if (hResData == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_pData = static_cast<LPBYTE>(LockResource(hResData));
    if (m_pData == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

LPCBYTE ResourceLoader::data() const
{
    return m_pData;
}

size_t ResourceLoader::size() const
{
    return m_size;
}
